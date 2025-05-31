#include <SFML/Graphics.hpp>
#include "setting/Setting.hpp"
#include "screen/Screen.hpp"
#include <iostream>

const unsigned int WINDOW_WIDTH = 1366;
const unsigned int WINDOW_HEIGHT = 768;

int main() {
    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Indoor Air Pollution Simulator");
    window.setFramerateLimit(60);

    sf::Font neoFont;
    if (!neoFont.loadFromFile("../resources/fonts/NeoDunggeunmoPro-Regular.ttf")) {
        std::cerr << "Error: Could not load font!" << std::endl;
        return -1;
    }

    ScreenState currentScreenState = ScreenState::START;
    
    StartScreen startScreen(window, neoFont);
    SettingScreen settingScreen(window, neoFont);

    sf::Clock deltaClock;

    while (window.isOpen()) {
        sf::Time dt = deltaClock.restart();

        if (currentScreenState == ScreenState::START) {
            if (!startScreen.isRunning()) {
                currentScreenState = startScreen.getNextState();
                if (currentScreenState == ScreenState::SETTING) {
                    settingScreen.m_running = true;
                    settingScreen.setNextState(ScreenState::SETTING);
                } else if (currentScreenState == ScreenState::EXIT) {
                    window.close();
                }
            }
            if(currentScreenState == ScreenState::START){
                 startScreen.handleInput();
                 startScreen.update(dt);
                 startScreen.render();
            }
        } else if (currentScreenState == ScreenState::SETTING) {
            if (!settingScreen.isRunning()) {
                currentScreenState = settingScreen.getNextState();
                if (currentScreenState == ScreenState::START) {
                    startScreen.reset();
                } else if (currentScreenState == ScreenState::SIMULATION) {
                    std::cout << "Switching to SIMULATION screen (Not implemented yet)" << std::endl;
                    window.close();
                } else if (currentScreenState == ScreenState::EXIT) {
                    window.close();
                }
            }
             if(currentScreenState == ScreenState::SETTING){
                settingScreen.handleInput();
                settingScreen.update(dt);
                settingScreen.render();
            }
        } else if (currentScreenState == ScreenState::SIMULATION) {
            std::cout << "In SIMULATION screen (Not implemented yet)" << std::endl;
            window.close();
        } else if (currentScreenState == ScreenState::EXIT) {
            window.close();
        }
    }

    return 0;
}