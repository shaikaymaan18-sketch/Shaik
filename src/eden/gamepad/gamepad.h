#pragma once
#include <QElapsedTimer>
#include <QObject>
#include <QQmlEngine>

#include <SDL2/SDL.h>

class Gamepad : public QObject {
    Q_OBJECT
    QML_ELEMENT
public:
    explicit Gamepad(QObject *parent = nullptr);
    ~Gamepad();

    Q_INVOKABLE void pollEvents();

signals:
    void upPressed();
    void downPressed();
    void leftPressed();
    void rightPressed();
    void aPressed();

    void leftStickMoved(int x, int y);

private:
    SDL_GameController *controller = nullptr;

    void closeController();
    void openController(int deviceIndex);
};
