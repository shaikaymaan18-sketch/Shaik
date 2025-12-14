#include <glad/glad.h>

#include <QQuickWindow>
#include "RenderWindow.h"
#include "common/scm_rev.h"
#include "common/settings.h"
#include "common/settings_enums.h"
#include "input_common/main.h"
#include "qt_common/qt_common.h"
#include "qt_common/render/context.h"
#include "qt_common/abstract/frontend.h"

struct OpenGLRenderItem : public QQuickItem {
    explicit OpenGLRenderItem(RenderWindow* parent) : QQuickItem(parent) {
        window()->setSurfaceType(QWindow::OpenGLSurface);
    }

    void SetContext(std::unique_ptr<Core::Frontend::GraphicsContext>&& context_) {
        context = std::move(context_);
    }

private:
    std::unique_ptr<Core::Frontend::GraphicsContext> context;
};

struct VulkanRenderItem : public QQuickItem {
    explicit VulkanRenderItem(QQuickItem* parent) : QQuickItem(parent) {
        // window()->setSurfaceType(QWindow::VulkanSurface);
    }
};

struct NullRenderItem : public QQuickItem {
    explicit NullRenderItem(RenderWindow* parent) : QQuickItem(parent) {}
};

bool RenderWindow::initializeOpenGL() {
#ifdef HAS_OPENGL
    if (!QOpenGLContext::supportsThreadedOpenGL()) {
        QtCommon::Frontend::Warning(tr("OpenGL not available!"),
                                    tr("OpenGL shared contexts are not supported."));
        return false;
    }

    // TODO: One of these flags might be interesting: WA_OpaquePaintEvent, WA_NoBackground,
    // WA_DontShowOnScreen, WA_DeleteOnClose
    auto child = new OpenGLRenderItem(this);
    child_item = child;
    child->window()->create();
    auto context = std::make_shared<OpenGLSharedContext>(m_window);
    main_context = context;
    child->SetContext(
        std::make_unique<OpenGLSharedContext>(context->GetShareContext(), m_window));

    return true;
#else
    QtCommon::Frontend::Warning(tr("OpenGL not available!"),
                                tr("Eden has not been compiled with OpenGL support."));
    return false;
#endif
}

bool RenderWindow::initializeVulkan() {
    qDebug() << "initializing Vulkan.";
    auto child = new VulkanRenderItem(m_parent);
    child_item = child;
    // child->window()->create();
    main_context = std::make_unique<DummyContext>();

    return true;
}

void RenderWindow::initializeNull() {
    child_item = new NullRenderItem(this);
    main_context = std::make_unique<DummyContext>();
}

RenderWindow::RenderWindow(QQuickWindow* window,
                           std::shared_ptr<InputCommon::InputSubsystem> input_subsystem_)
    : QQuickItem(), input_subsystem{std::move(input_subsystem_)}, m_window(window) {
    // STUBBED

    QQuickItem* renderHost = m_window->findChild<QQuickItem*>("renderHost");

    assert(renderHost);

    setParentItem(renderHost);
    m_parent = renderHost;

    setWidth(renderHost->width());
    setHeight(renderHost->height());
    setX(0);
    setY(0);

    window->setTitle(QStringLiteral("Eden %1 | %2-%3")
                         .arg(QString::fromUtf8(Common::g_build_name),
                              QString::fromUtf8(Common::g_scm_branch),
                              QString::fromUtf8(Common::g_scm_desc)));
    input_subsystem->Initialize();

    strict_context_required = QGuiApplication::platformName() == QStringLiteral("wayland") ||
                              QGuiApplication::platformName() == QStringLiteral("wayland-egl");
}

void RenderWindow::OnFrameDisplayed() {
    input_subsystem->GetTas()->UpdateThread();
    const InputCommon::TasInput::TasState new_tas_state =
        std::get<0>(input_subsystem->GetTas()->GetStatus());

    if (!first_frame) {
        last_tas_state = new_tas_state;
        first_frame = true;
        emit FirstFrameDisplayed();
    }

    if (new_tas_state != last_tas_state) {
        last_tas_state = new_tas_state;
        emit TasPlaybackStateChanged();
    }
}

std::unique_ptr<Core::Frontend::GraphicsContext> RenderWindow::CreateSharedContext() const {
#ifdef HAS_OPENGL
    if (Settings::values.renderer_backend.GetValue() == Settings::RendererBackend::OpenGL) {
        auto c = static_cast<OpenGLSharedContext*>(main_context.get());
        // Bind the shared contexts to the main surface in case the backend wants to take over
        // presentation
        return std::make_unique<OpenGLSharedContext>(c->GetShareContext(), m_window);
    }
#endif
    return std::make_unique<DummyContext>();
}

bool RenderWindow::IsShown() const {
    // STUBBED
    return true;
}

bool RenderWindow::initRenderTarget() {
    // STUBBED
    main_context.reset();

    {
        // Create a dummy render widget so that Qt
        // places the render window at the correct position.
        const QQuickItem dummy_item{this};
    }

    first_frame = false;

    switch (Settings::values.renderer_backend.GetValue()) {
    case Settings::RendererBackend::OpenGL:
        if (!initializeOpenGL()) {
            return false;
        }
        break;
    case Settings::RendererBackend::Vulkan:
        if (!initializeVulkan()) {
            return false;
        }
        break;
    case Settings::RendererBackend::Null:
        initializeNull();
        break;
    }

    // Update the Window System information with the new render target
    window_info = QtCommon::GetWindowSystemInfo(m_window);

    OnMinimalClientAreaChangeRequest(GetActiveConfig().min_client_area_size);
    onFramebufferSizeChanged();
    // BackupGeometry();

    if (Settings::values.renderer_backend.GetValue() == Settings::RendererBackend::OpenGL) {
        if (!loadOpenGL()) {
            return false;
        }
    }

    return true;
}

void RenderWindow::OnMinimalClientAreaChangeRequest(std::pair<u32, u32> minimal_size) {
    m_window->setMinimumSize(QSize(minimal_size.first, minimal_size.second));
}

bool RenderWindow::loadOpenGL() {
    auto context = CreateSharedContext();
    auto scope = context->Acquire();
    if (!gladLoadGL()) {
        QtCommon::Frontend::Warning(
            tr("Error while initializing OpenGL!"),
            tr("Your GPU may not support OpenGL, or you do not have the latest graphics driver."));
        return false;
    }
    // Display various warnings (but not fatal errors) for missing OpenGL extensions or lack of
    // OpenGL 4.6 support
    const QString renderer = QString::fromUtf8(reinterpret_cast<const char*>(glGetString(GL_RENDERER)));
    if (!GLAD_GL_VERSION_4_6) {
        QtCommon::Frontend::Warning(tr("Error while initializing OpenGL 4.6!"),
                             tr("Your GPU may not support OpenGL 4.6, or you do not have the "
                                "latest graphics driver.<br><br>GL Renderer:<br>%1")
                                 .arg(renderer));
        return false;
    }
    if (QStringList missing_ext = getUnsupportedGLExtensions(); !missing_ext.empty()) {
        QtCommon::Frontend::Warning(
            tr("Error while initializing OpenGL!"),
            tr("Your GPU may not support one or more required OpenGL extensions. Please ensure you "
               "have the latest graphics driver.<br><br>GL Renderer:<br>%1<br><br>Unsupported "
               "extensions:<br>%2")
                .arg(renderer, missing_ext.join(QStringLiteral("<br>"))));
        // Non fatal
    }
    return true;
}

QStringList RenderWindow::getUnsupportedGLExtensions() const {
    QStringList missing_ext;
    // Extensions required to support some texture formats.
    if (!GLAD_GL_EXT_texture_compression_s3tc)
        missing_ext.append(QStringLiteral("EXT_texture_compression_s3tc"));
    if (!GLAD_GL_ARB_texture_compression_rgtc)
        missing_ext.append(QStringLiteral("ARB_texture_compression_rgtc"));
    if (!missing_ext.empty())
        LOG_ERROR(Frontend, "GPU does not support all required extensions");
    for (const QString& ext : missing_ext)
        LOG_ERROR(Frontend, "Unsupported GL extension: {}", ext.toStdString());
    return missing_ext;
}

// On Qt 5.0+, this correctly gets the size of the framebuffer (pixels).
//
// Older versions get the window size (density independent pixels),
// and hence, do not support DPI scaling ("retina" displays).
// The result will be a viewport that is smaller than the extent of the window.
void RenderWindow::onFramebufferSizeChanged() {
    // Screen changes potentially incur a change in screen DPI, hence we should update the
    // framebuffer size
    qDebug() << width() << height();
    const qreal pixel_ratio = m_window->devicePixelRatio();
    const u32 width = (this->width()) * pixel_ratio;
    const u32 height = (this->height()) * pixel_ratio;
    UpdateCurrentFramebufferLayout(width, height);
}
