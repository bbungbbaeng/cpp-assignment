#include <SFML/Graphics.hpp>
#include "setting/Setting.hpp"
#include "screen/Screen.hpp"
#include "simulation/Simulation.hpp" // Simulation.hpp 포함
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
    SimulationScreen simulationScreen(window, neoFont); // SimulationScreen 객체 생성

    sf::Clock deltaClock;

    while (window.isOpen()) {
        sf::Time dt = deltaClock.restart();

        if (currentScreenState == ScreenState::START) {
            if (!startScreen.isRunning()) {
                currentScreenState = startScreen.getNextState();
                if (currentScreenState == ScreenState::SETTING) {
                    // settingScreen.m_running = true; // 생성자에서 true로 설정됨
                    // settingScreen.setNextState(ScreenState::SETTING); // 생성자에서 설정됨
                    settingScreen.reset(); // Setting 화면으로 전환 시 reset 호출
                } else if (currentScreenState == ScreenState::EXIT) {
                    window.close();
                }
            }
            if(currentScreenState == ScreenState::START){ // 여전히 StartScreen 상태인 경우에만 처리
                 startScreen.handleInput();
                 startScreen.update(dt);
                 startScreen.render();
            }
        } else if (currentScreenState == ScreenState::SETTING) {
            if (!settingScreen.isRunning()) {
                currentScreenState = settingScreen.getNextState();
                if (currentScreenState == ScreenState::START) {
                    startScreen.reset(); // Start 화면으로 전환 시 reset 호출
                } else if (currentScreenState == ScreenState::SIMULATION) {
                    std::cout << "Switching to SIMULATION screen" << std::endl; // 더 이상 "Not implemented yet" 아님
                    // simulationScreen.m_running = true; // 생성자에서 true로 설정됨
                    // simulationScreen.setNextState(ScreenState::SIMULATION); // 생성자에서 설정됨
                    simulationScreen.reset(); // Simulation 화면으로 전환 시 reset 호출
                } else if (currentScreenState == ScreenState::EXIT) {
                    window.close();
                }
            }
             if(currentScreenState == ScreenState::SETTING){ // 여전히 SettingScreen 상태인 경우에만 처리
                settingScreen.handleInput();
                settingScreen.update(dt);
                settingScreen.render();
            }
        } else if (currentScreenState == ScreenState::SIMULATION) { // SIMULATION 상태 처리
            if (!simulationScreen.isRunning()) {
                currentScreenState = simulationScreen.getNextState();
                if (currentScreenState == ScreenState::START) { // Simulation 화면에서 Start 화면으로 돌아갈 때
                    startScreen.reset(); // Start 화면 reset 호출
                } else if (currentScreenState == ScreenState::EXIT) {
                    window.close();
                }
                // SIMULATION -> SETTING 직접 전환은 현재 로직에 없음 (필요 시 추가)
            }
            if (currentScreenState == ScreenState::SIMULATION) { // 여전히 SimulationScreen 상태인 경우에만 처리
                simulationScreen.handleInput();
                simulationScreen.update(dt);
                simulationScreen.render();
            }
        } else if (currentScreenState == ScreenState::EXIT) {
            window.close();
        }
    }

    return 0;
}