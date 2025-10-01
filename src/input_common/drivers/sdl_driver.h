// SPDX-FileCopyrightText: 2018 Citra Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <atomic>
#include <mutex>
#include <thread>
#include <unordered_map>

#include <SDL3/SDL.h>

#include "common/common_types.h"
#include "common/threadsafe_queue.h"
#include "input_common/input_engine.h"

union SDL_Event;
using SDL_Gamepad = struct SDL_Gamepad;
using SDL_Joystick = struct SDL_Joystick;
using SDL_JoystickID = Uint32;

namespace InputCommon {

class SDLJoystick;

using ButtonBindings = std::array<std::pair<Settings::NativeButton::Values, SDL_GamepadButton>, 20>;
using ZButtonBindings = std::array<std::pair<Settings::NativeButton::Values, SDL_GamepadAxis>, 2>;

class SDLDriver : public InputEngine {
public:
    explicit SDLDriver(std::string input_engine_);
    ~SDLDriver() override;

    void PumpEvents() const;
    void HandleGameControllerEvent(const SDL_Event& event);

    std::shared_ptr<SDLJoystick> GetSDLJoystickBySDLID(SDL_JoystickID sdl_id);
    std::shared_ptr<SDLJoystick> GetSDLJoystickByGUID(const Common::UUID& guid, int port);
    std::shared_ptr<SDLJoystick> GetSDLJoystickByGUID(const std::string& guid, int port);

    std::vector<Common::ParamPackage> GetInputDevices() const override;

    ButtonMapping GetButtonMappingForDevice(const Common::ParamPackage& params) override;
    AnalogMapping GetAnalogMappingForDevice(const Common::ParamPackage& params) override;
    MotionMapping GetMotionMappingForDevice(const Common::ParamPackage& params) override;
    Common::Input::ButtonNames GetUIName(const Common::ParamPackage& params) const override;

    std::string GetHatButtonName(u8 direction_value) const override;
    u8 GetHatButtonId(const std::string& direction_name) const override;

    bool IsStickInverted(const Common::ParamPackage& params) override;

    Common::Input::DriverResult SetVibration(
        const PadIdentifier& identifier, const Common::Input::VibrationStatus& vibration) override;

    bool IsVibrationEnabled(const PadIdentifier& identifier) override;

private:
    void InitJoystick(SDL_JoystickID joystick_id);
    void CloseJoystick(SDL_Joystick* sdl_joystick);
    void CloseJoysticks();
    void SendVibrations();

    Common::ParamPackage BuildAnalogParamPackageForButton(int port, const Common::UUID& guid,
                                                          s32 axis, float value = 0.1f) const;
    Common::ParamPackage BuildButtonParamPackageForButton(int port, const Common::UUID& guid,
                                                          s32 button) const;
    Common::ParamPackage BuildHatParamPackageForButton(int port, const Common::UUID& guid, s32 hat,
                                                       u8 value) const;
    Common::ParamPackage BuildMotionParam(int port, const Common::UUID& guid) const;
    Common::ParamPackage BuildParamPackageForBinding(int port, const Common::UUID& guid,
                                                     const SDL_GamepadBinding& binding) const;
    Common::ParamPackage BuildParamPackageForAnalog(PadIdentifier identifier, int axis_x,
                                                    int axis_y, float offset_x,
                                                    float offset_y) const;

    ButtonBindings GetDefaultButtonBinding(const std::shared_ptr<SDLJoystick>& joystick) const;
    ButtonMapping GetSingleControllerMapping(const std::shared_ptr<SDLJoystick>& joystick,
                                             const ButtonBindings& switch_to_sdl_button,
                                             const ZButtonBindings& switch_to_sdl_axis) const;
    ButtonMapping GetDualControllerMapping(const std::shared_ptr<SDLJoystick>& joystick,
                                           const std::shared_ptr<SDLJoystick>& joystick2,
                                           const ButtonBindings& switch_to_sdl_button,
                                           const ZButtonBindings& switch_to_sdl_axis) const;
    bool IsButtonOnLeftSide(Settings::NativeButton::Values button) const;

    Common::SPSCQueue<VibrationRequest> vibration_queue;
    std::unordered_map<Common::UUID, std::vector<std::shared_ptr<SDLJoystick>>> joystick_map;
    std::mutex joystick_map_mutex;

    bool start_thread = false;
    std::atomic<bool> initialized = false;
    std::thread vibration_thread;
};
} // namespace InputCommon