#pragma once

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_gamecontroller.h>

#include <unordered_map>

class ControllerInput
{
public:
    ControllerInput();
    ~ControllerInput();

    bool isConnected() const { return mController != nullptr; };
    void pollEvents();
    bool isButtonPressed(SDL_GameControllerButton button) const;
    float getAxis(SDL_GameControllerAxis axis) const;
private:
    void handleEvent(const SDL_Event& event);

    SDL_GameController* mController;
    std::unordered_map<SDL_GameControllerButton, bool> mButtonStates;
    std::unordered_map<SDL_GameControllerAxis, float> mAxisStates;
};