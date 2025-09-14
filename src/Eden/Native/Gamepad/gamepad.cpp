#include "gamepad.h"

// TODO(crueter): This is just temporary
Gamepad::Gamepad(QObject *parent)
    : QObject(parent)
{
    SDL_Init(SDL_INIT_GAMECONTROLLER);
}

Gamepad::~Gamepad()
{
    if (controller)
        SDL_GameControllerClose(controller);
    SDL_Quit();
}

void Gamepad::openController(int deviceIndex)
{
    if (controller) {
        closeController();
    }

    controller = SDL_GameControllerOpen(deviceIndex);
}

void Gamepad::closeController()
{
    if (controller) {
        SDL_GameControllerClose(controller);
        controller = nullptr;
    }
}
void Gamepad::pollEvents()
{
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_CONTROLLERDEVICEADDED:
            openController(event.cdevice.which);
            break;
        case SDL_CONTROLLERDEVICEREMOVED:
            if (controller
                && event.cdevice.which
                       == SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(controller))) {
                closeController();
            }
            break;

        case SDL_CONTROLLERBUTTONDOWN:
            switch (event.cbutton.button) {
            case SDL_CONTROLLER_BUTTON_DPAD_UP:
                emit upPressed();
                break;
            case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
                emit downPressed();
                break;
            case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
                emit leftPressed();
                break;
            case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
                emit rightPressed();
                break;
            case SDL_CONTROLLER_BUTTON_A:
                emit aPressed();
                break;
            }
            break;

        case SDL_CONTROLLERAXISMOTION:
            if (event.caxis.axis == SDL_CONTROLLER_AXIS_LEFTX
                || event.caxis.axis == SDL_CONTROLLER_AXIS_LEFTY) {
                int x = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTX);
                int y = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTY);
                emit leftStickMoved(x, y);
            }
            break;
        }
    }
}
