#include <SFML/Graphics.hpp>
#include "setting/Setting.hpp"
#include "screen/Screen.hpp"
#include "simulation/Simulation.hpp"
#include <iostream> 

const unsigned int WINDOW_WIDTH = 1366; // 창 너비 상수 정의
const unsigned int WINDOW_HEIGHT = 768; // 창 높이 상수 정의

int main() {
    // 렌더링 창 생성 (너비, 높이, 창 제목)
    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Indoor Air Pollution Simulator");
    // 초당 프레임 수 제한 설정 (60 FPS)
    window.setFramerateLimit(60);

    // 폰트 객체 생성
    sf::Font neoFont;
    // 폰트 파일 로드
    if (!neoFont.loadFromFile("../resources/fonts/NeoDunggeunmoPro-Regular.ttf")) {
        // 폰트 로드 실패 시 오류 메시지 출력 및 프로그램 종료
        std::cerr << "Error: Could not load font!" << std::endl;
        return -1;
    }

    // 현재 화면 상태를 나타내는 변수, 초기 상태는 시작 화면(START)
    ScreenState currentScreenState = ScreenState::START;

    // 각 화면 객체 생성 (창과 폰트 전달)
    StartScreen startScreen(window, neoFont);           // 시작 화면 객체
    SettingScreen settingScreen(window, neoFont);       // 설정 화면 객체
    SimulationScreen simulationScreen(window, neoFont); // 시뮬레이션 화면 객체

    // 시간 측정용 시계 객체 (델타 타임 계산용)
    sf::Clock deltaClock;

    // 메인 게임 루프: 창이 열려있는 동안 반복
    while (window.isOpen()) {
        // 이전 프레임 이후 경과 시간 계산 및 시계 재시작
        sf::Time dt = deltaClock.restart();

        // 현재 화면 상태에 따라 분기 처리
        if (currentScreenState == ScreenState::START) { // 현재 시작 화면일 경우
            // 시작 화면이 더 이상 실행 중이 아니면 (다음 화면으로 전환 요청 시)
            if (!startScreen.isRunning()) {
                currentScreenState = startScreen.getNextState(); // 다음 화면 상태 가져오기
                if (currentScreenState == ScreenState::SETTING) { // 다음 화면이 설정 화면이면
                    settingScreen.reset(); // 설정 화면 상태 초기화
                } else if (currentScreenState == ScreenState::EXIT) { // 다음 화면이 종료면
                    window.close(); // 창 닫기
                }
            }
            // 시작 화면이 여전히 현재 화면이면 (화면 전환이 아직 안 됐으면)
            if (currentScreenState == ScreenState::START) {
                startScreen.handleInput(); // 시작 화면 입력 처리
                startScreen.update(dt);    // 시작 화면 상태 업데이트
                startScreen.render();      // 시작 화면 렌더링
            }
        } else if (currentScreenState == ScreenState::SETTING) { // 현재 설정 화면일 경우
            // 설정 화면이 더 이상 실행 중이 아니면
            if (!settingScreen.isRunning()) {
                currentScreenState = settingScreen.getNextState(); // 다음 화면 상태 가져오기
                if (currentScreenState == ScreenState::START) { // 다음 화면이 시작 화면이면
                    startScreen.reset(); // 시작 화면 상태 초기화
                } else if (currentScreenState == ScreenState::SIMULATION) { // 다음 화면이 시뮬레이션 화면이면
                    std::cout << "Switching to SIMULATION screen" << std::endl; // 콘솔 메시지 출력
                    simulationScreen.reset(); // 시뮬레이션 화면 상태 초기화
                } else if (currentScreenState == ScreenState::EXIT) { // 다음 화면이 종료면
                    window.close(); // 창 닫기
                }
            }
            // 설정 화면이 여전히 현재 화면이면
            if (currentScreenState == ScreenState::SETTING) {
                settingScreen.handleInput(); // 설정 화면 입력 처리
                settingScreen.update(dt);    // 설정 화면 상태 업데이트
                settingScreen.render();      // 설정 화면 렌더링
            }
        } else if (currentScreenState == ScreenState::SIMULATION) { // 현재 시뮬레이션 화면일 경우
            // 시뮬레이션 화면이 더 이상 실행 중이 아니면
            if (!simulationScreen.isRunning()) {
                currentScreenState = simulationScreen.getNextState(); // 다음 화면 상태 가져오기
                if (currentScreenState == ScreenState::START) { // 다음 화면이 시작 화면이면
                    startScreen.reset(); // 시작 화면 상태 초기화
                } else if (currentScreenState == ScreenState::EXIT) { // 다음 화면이 종료면
                    window.close(); // 창 닫기
                }
            }
            // 시뮬레이션 화면이 여전히 현재 화면이면
            if (currentScreenState == ScreenState::SIMULATION) {
                simulationScreen.handleInput(); // 시뮬레이션 화면 입력 처리
                simulationScreen.update(dt);    // 시뮬레이션 화면 상태 업데이트
                simulationScreen.render();      // 시뮬레이션 화면 렌더링
            }
        } else if (currentScreenState == ScreenState::EXIT) { // 현재 상태가 종료면
            window.close(); // 창 닫기
        }
    }

    // 프로그램 정상 종료
    return 0;
}