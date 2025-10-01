// SPDX-FileCopyrightText: Copyright 2025 Eden Emulator Project
// SPDX-License-Identifier: GPL-3.0-or-later

// SPDX-FileCopyrightText: Copyright 2018 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include <atomic>
#include <span>
#include <vector>

#include <SDL3/SDL.h>

#include "audio_core/common/common.h"
#include "audio_core/sink/sdl2_sink.h"
#include "audio_core/sink/sink_stream.h"
#include "common/logging/log.h"
#include "core/core.h"

namespace AudioCore::Sink {
/**
 * SDL sink stream, responsible for sinking samples to hardware.
 */
class SDLSinkStream final : public SinkStream {
public:
    /**
     * Create a new sink stream.
     *
     * @param device_channels_ - Number of channels supported by the hardware.
     * @param system_channels_ - Number of channels the audio systems expect.
     * @param output_device    - Name of the output device to use for this stream.
     * @param input_device     - Name of the input device to use for this stream.
     * @param type_            - Type of this stream.
     * @param system_          - Core system.
     * @param event            - Event used only for audio renderer, signalled on buffer consume.
     */
    SDLSinkStream(u32 device_channels_, u32 system_channels_, const std::string& output_device,
                  const std::string& input_device, StreamType type_, Core::System& system_)
        : SinkStream{system_, type_} {
        system_channels = system_channels_;
        device_channels = device_channels_;

        SDL_AudioSpec spec{};
        spec.freq = TargetSampleRate;
        spec.channels = static_cast<int>(device_channels);
        spec.format = SDL_AUDIO_S16;

        SDL_AudioDeviceID device_id = 0;
        std::string device_name{output_device};
        bool is_capture{false};

        if (type == StreamType::In) {
            device_name = input_device;
            is_capture = true;
        }

        if (!device_name.empty()) {
            int count = 0;
            SDL_AudioDeviceID* devices = is_capture ? SDL_GetAudioRecordingDevices(&count)
                                                    : SDL_GetAudioPlaybackDevices(&count);

            if (devices) {
                for (int i = 0; i < count; ++i) {
                    const char* name = SDL_GetAudioDeviceName(devices[i]);
                    if (name && device_name == name) {
                        device_id = devices[i];
                        break;
                    }
                }
                SDL_free(devices);
            }
        }

        if (device_id == 0) {
            device_id =
                is_capture ? SDL_AUDIO_DEVICE_DEFAULT_RECORDING : SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK;
        }

        stream = SDL_OpenAudioDeviceStream(
            device_id, &spec,
            is_capture ? &SDLSinkStream::CaptureCallback : &SDLSinkStream::PlaybackCallback, this);

        if (!stream) {
            LOG_CRITICAL(Audio_Sink, "Error opening SDL audio stream: {}", SDL_GetError());
            return;
        }

        LOG_INFO(Service_Audio, "Opening SDL stream with: rate {} channels {} (system channels {})",
                 spec.freq, spec.channels, system_channels);
    }

    /**
     * Destroy the sink stream.
     */
    ~SDLSinkStream() override {
        LOG_DEBUG(Service_Audio, "Destructing SDL stream {}", name);
        Finalize();
    }

    /**
     * Finalize the sink stream.
     */
    void Finalize() override {
        if (!stream) {
            return;
        }

        Stop();
        SDL_ClearAudioStream(stream);
        SDL_DestroyAudioStream(stream);
        stream = nullptr;
    }

    /**
     * Start the sink stream.
     *
     * @param resume - Set to true if this is resuming the stream a previously-active stream.
     *                 Default false.
     */
    void Start(bool resume = false) override {
        if (!stream || !paused) {
            return;
        }

        paused = false;
        SDL_ResumeAudioStreamDevice(stream);
    }

    /**
     * Stop the sink stream.
     */
    void Stop() override {
        if (!stream || paused) {
            return;
        }
        SignalPause();
        SDL_PauseAudioStreamDevice(stream);
        paused = true;
    }

private:
    static void PlaybackCallback(void* userdata, SDL_AudioStream* stream, int additional_amount,
                                 int total_amount) {
        auto* impl = static_cast<SDLSinkStream*>(userdata);
        if (!impl) {
            return;
        }

        const std::size_t num_channels = impl->GetDeviceChannels();
        const std::size_t frame_size = num_channels;
        const std::size_t num_frames = additional_amount / (sizeof(s16) * frame_size);

        if (num_frames == 0) {
            return;
        }

        std::vector<s16> buffer(num_frames * frame_size);
        impl->ProcessAudioOutAndRender(buffer, num_frames);
        SDL_PutAudioStreamData(stream, buffer.data(),
                               static_cast<int>(buffer.size() * sizeof(s16)));
    }

    static void CaptureCallback(void* userdata, SDL_AudioStream* stream, int additional_amount,
                                int total_amount) {
        auto* impl = static_cast<SDLSinkStream*>(userdata);
        if (!impl) {
            return;
        }

        const std::size_t num_channels = impl->GetDeviceChannels();
        const std::size_t frame_size = num_channels;
        const std::size_t bytes_available = SDL_GetAudioStreamAvailable(stream);

        if (bytes_available == 0) {
            return;
        }

        const std::size_t num_frames = bytes_available / (sizeof(s16) * frame_size);
        std::vector<s16> buffer(num_frames * frame_size);

        int bytes_read =
            SDL_GetAudioStreamData(stream, buffer.data(), static_cast<int>(bytes_available));
        if (bytes_read > 0) {
            const std::size_t frames_read = bytes_read / (sizeof(s16) * frame_size);
            impl->ProcessAudioIn(buffer, frames_read);
        }
    }

    SDL_AudioStream* stream{nullptr};
};

SDLSink::SDLSink(std::string_view target_device_name) {
    if (!SDL_WasInit(SDL_INIT_AUDIO)) {
        if (!SDL_InitSubSystem(SDL_INIT_AUDIO)) {
            LOG_CRITICAL(Audio_Sink, "SDL_InitSubSystem audio failed: {}", SDL_GetError());
            return;
        }
    }

    if (target_device_name != auto_device_name && !target_device_name.empty()) {
        output_device = target_device_name;
    } else {
        output_device.clear();
    }

    device_channels = 2;
}

SDLSink::~SDLSink() = default;

SinkStream* SDLSink::AcquireSinkStream(Core::System& system, u32 system_channels_,
                                       const std::string&, StreamType type) {
    system_channels = system_channels_;
    SinkStreamPtr& stream = sink_streams.emplace_back(std::make_unique<SDLSinkStream>(
        device_channels, system_channels, output_device, input_device, type, system));
    return stream.get();
}

void SDLSink::CloseStream(SinkStream* stream) {
    for (size_t i = 0; i < sink_streams.size(); i++) {
        if (sink_streams[i].get() == stream) {
            sink_streams[i].reset();
            sink_streams.erase(sink_streams.begin() + i);
            break;
        }
    }
}

void SDLSink::CloseStreams() {
    sink_streams.clear();
}

f32 SDLSink::GetDeviceVolume() const {
    if (sink_streams.empty()) {
        return 1.0f;
    }

    return sink_streams[0]->GetDeviceVolume();
}

void SDLSink::SetDeviceVolume(f32 volume) {
    for (auto& stream : sink_streams) {
        stream->SetDeviceVolume(volume);
    }
}

void SDLSink::SetSystemVolume(f32 volume) {
    for (auto& stream : sink_streams) {
        stream->SetSystemVolume(volume);
    }
}

std::vector<std::string> ListSDLSinkDevices(bool capture) {
    std::vector<std::string> device_list;

    if (!SDL_WasInit(SDL_INIT_AUDIO)) {
        if (!SDL_InitSubSystem(SDL_INIT_AUDIO)) {
            LOG_CRITICAL(Audio_Sink, "SDL_InitSubSystem audio failed: {}", SDL_GetError());
            return {};
        }
    }

    int count = 0;
    SDL_AudioDeviceID* devices =
        capture ? SDL_GetAudioRecordingDevices(&count) : SDL_GetAudioPlaybackDevices(&count);

    if (devices) {
        for (int i = 0; i < count; ++i) {
            const char* name = SDL_GetAudioDeviceName(devices[i]);
            if (name) {
                device_list.emplace_back(name);
            }
        }
        SDL_free(devices);
    }

    return device_list;
}

u32 GetSDLLatency() {
    return TargetSampleCount * 2;
}

} // namespace AudioCore::Sink