#include "ControllerInput.h"
#include <iostream>
#include <limits>

ControllerInput::ControllerInput()
{
    // Init SDL Gamepad
    if (SDL_Init(SDL_INIT_GAMECONTROLLER | SDL_INIT_EVENTS) != 0)
    {
        std::cerr << "SDL Init failed: [" << SDL_GetError() << "]" << std::endl;
        return;
    }
    else
    {
        std::cout << "SDL successfully initialized!" << std::endl;
    }

    for (int i = 0; i < SDL_NumJoysticks(); ++i)
    {
        if (SDL_IsGameController(i))
        {
            mController = SDL_GameControllerOpen(i);
            if (mController)
            {
                std::cout << "Opened controller: " << SDL_GameControllerName(mController) << std::endl;
                break;
            }
            else
            {
                std::cerr << "Could not open gamecontroller " << i << ": " << SDL_GetError() << std::endl;
            }
        }
    }
}

ControllerInput::~ControllerInput()
{
    if (mController)
        SDL_GameControllerClose(mController);
    
    SDL_QuitSubSystem(SDL_INIT_GAMECONTROLLER);
    SDL_Quit();
}

void ControllerInput::pollEvents()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        handleEvent(event);
    }
}

bool ControllerInput::isButtonPressed(SDL_GameControllerButton button) const
{
    auto it = mButtonStates.find(button);
    return (it != mButtonStates.end()) ? it->second : false;
}

float ControllerInput::getAxis(SDL_GameControllerAxis axis) const
{
    auto it = mAxisStates.find(axis);
    if (it != mAxisStates.end())
    {
        float value = it->second;
        constexpr float deadband = 0.15f;
        return (std::abs(value) < deadband) ? 0.0f : value;
    }
    return 0.0f;
}

void ControllerInput::handleEvent(const SDL_Event &event)
{
    switch (event.type)
    {
        case SDL_CONTROLLERBUTTONDOWN:
            mButtonStates[static_cast<SDL_GameControllerButton>(event.cbutton.button)] = true;
            break;
        case SDL_CONTROLLERBUTTONUP:
            mButtonStates[static_cast<SDL_GameControllerButton>(event.cbutton.button)] = false;
            break;
        case SDL_CONTROLLERAXISMOTION:
            mAxisStates[static_cast<SDL_GameControllerAxis>(event.caxis.axis)] = event.caxis.value / 32767.0f;
            break;
    }
}
