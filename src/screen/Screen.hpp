#ifndef SCREEN_HPP
#define SCREEN_HPP

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <random>

// 프로그램의 여러 화면 상태를 정의하는 클래스
enum class ScreenState {
    START,      // 시작 화면 상태
    SETTING,    // 설정 화면 상태
    SIMULATION, // 시뮬레이션 화면 상태
    EXIT        // 프로그램 종료 상태
};

// 시작 화면을 담당하는 클래스
class StartScreen {
public:
    // 생성자: 렌더링 창과 폰트에 대한 참조를 받음
    StartScreen(sf::RenderWindow& window, sf::Font& font);
    // 소멸자
    ~StartScreen();

    // 사용자 입력을 처리하는 함수
    void handleInput();
    // 화면의 상태를 업데이트하는 함수 (시간 경과에 따른 변화 등)
    void update(sf::Time dt); // dt: delta time (프레임 간 시간 간격)
    // 화면의 모든 요소를 그리는 함수
    void render();

    // 다음으로 전환될 화면 상태를 반환하는 함수
    ScreenState getNextState() const;
    // 다음 화면 상태를 설정하는 함수
    void setNextState(ScreenState state);
    // 현재 화면이 계속 실행 중인지 여부를 반환하는 함수
    bool isRunning() const;
    // 화면의 상태를 초기 상태로 리셋하는 함수
    void reset();

private:
    // SFML 렌더링 창에 대한 참조 (main에서 생성된 창 사용)
    sf::RenderWindow& m_window;
    // 사용할 폰트에 대한 참조 (main에서 로드된 폰트 사용)
    sf::Font& m_font;

    // 화면에 표시될 텍스트 객체들
    sf::Text m_titleText;       // 제목 텍스트
    sf::Text m_startButtonText; // "실행" 버튼 텍스트
    sf::Text m_exitButtonText;  // "종료" 버튼 텍스트

    // 버튼의 시각적 형태를 나타내는 사각형 객체들
    sf::RectangleShape m_startButtonShape; // "실행" 버튼 모양
    sf::RectangleShape m_exitButtonShape;  // "종료" 버튼 모양

    // 버튼 위에 마우스가 올라와 있는지 여부를 나타내는 플래그
    bool m_isStartButtonHovered;
    bool m_isExitButtonHovered;

    // 다음으로 전환될 화면 상태 저장 변수
    ScreenState m_nextState;
    // 현재 화면이 실행 중인지 여부 저장 변수
    bool m_running;

    // 먼지 입자를 표현하기 위한 내부 구조체
    struct DustParticle {
        sf::CircleShape shape;    // 입자의 시각적 모양 (원)
        sf::Vector2f velocity; // 입자의 이동 속도 (2D 벡터)
    };
    // 여러 먼지 입자들을 저장하는 벡터
    std::vector<DustParticle> m_dustParticles;

    // 먼지 입자 관련 상수들
    const int NUM_DUST_PARTICLES = 700;          // 생성할 먼지 입자 개수
    const float DUST_PARTICLE_RADIUS = 1.5f;     // 먼지 입자 반지름
    const sf::Color DUST_COLOR = sf::Color(100, 100, 100); // 먼지 입자 색상
    const float MAX_DUST_SPEED = 50.f;           // 먼지 입자 최대 속도
    const float MOUSE_REPEL_RADIUS = 100.f;      // 마우스 반발 효과 반경
    const float MOUSE_REPEL_STRENGTH = 200.f;    // 마우스 반발 효과 강도

    // 현재 마우스 위치 (뷰 좌표계 기준)
    sf::Vector2f m_mousePosition;
    // 마지막 업데이트 이후 마우스가 움직였는지 여부 플래그
    bool m_mouseMovedSinceLastUpdate;

    // 난수 생성 관련 멤버 변수들
    std::mt19937 m_rng; // 메르센 트위스터 난수 엔진
    std::uniform_real_distribution<float> m_distX;     // X 좌표 균등 분포
    std::uniform_real_distribution<float> m_distY;     // Y 좌표 균등 분포
    std::uniform_real_distribution<float> m_distAngle; // 각도 균등 분포
    std::uniform_real_distribution<float> m_distSpeed; // 속도 균등 분포

    // private 헬퍼 함수들: 클래스 내부에서만 호출되어 특정 초기화 작업 수행
    void setupTexts();         // 텍스트 요소들 설정
    void setupButtons();       // 버튼 요소들 설정
    void updateButtonStyles(); // 버튼 스타일 (호버 효과) 업데이트
    void setupDustParticles(); // 먼지 입자들 생성 및 초기화
    void updateDustParticles(sf::Time dt); // 먼지 입자들 상태 업데이트
};

#endif