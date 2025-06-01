#ifndef SIMULATION_HPP
#define SIMULATION_HPP

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <array>
#include "../setting/Setting.hpp" // InputBox, Vec3D, Edge, OpeningDefinition, PI 등을 위해 포함
#include "../screen/Screen.hpp"   // ScreenState 를 위해 포함

// InputBox 등의 클래스를 Setting.hpp 전체를 포함하지 않고 전방 선언할 수도 있지만,
// Vec3D 등 다른 구조체도 필요하므로 포함하는 것이 이 구조에서는 더 간단합니다.

// 파티클(먼지) 표현을 위한 구조체
struct Particle {
    Vec3D position3D; // 방 중심 기준 상대 좌표 (방 크기에 따라 스케일됨)
    sf::CircleShape shape; // 화면에 그려질 모양
    Vec3D velocity; // 간단한 움직임을 위한 속도
};

class SimulationScreen {
public:
    SimulationScreen(sf::RenderWindow& window, sf::Font& font);
    ~SimulationScreen();

    void handleInput();
    void update(sf::Time dt);
    void render();

    ScreenState getNextState() const;
    bool isRunning() const;
    void setNextState(ScreenState state);
    void reset(); // 이 화면으로 다시 돌아올 때 호출되어 상태를 초기화

    bool m_running; // main 루프에서 접근하기 위해 public (SettingScreen과 동일)

private:
    sf::RenderWindow& m_window;
    sf::Font& m_font;
    ScreenState m_nextState;

    // --- UI 요소 ---
    sf::Text m_titleText;
    InputBox m_inputC0, m_inputS, m_inputK; // 초기 농도, 유입 속도, 제거 속도 상수 입력창
    sf::Text m_labelC0, m_labelS, m_labelK; // 입력창 라벨
    sf::Text m_displayVolume, m_displayTime, m_displayConcentration; // 정보 표시 텍스트
    sf::Text m_labelVolume, m_labelTime, m_labelConcentration; // 정보 표시 라벨

    sf::Text m_buttonRun, m_buttonStop, m_buttonReset, m_buttonBack; // 버튼 텍스트
    sf::RectangleShape m_shapeRun, m_shapeStop, m_shapeReset, m_shapeBack; // 버튼 모양

    // 버튼 시각 효과를 위한 색상
    sf::Color m_buttonTextColorNormal;
    sf::Color m_buttonTextColorHover;
    sf::Color m_buttonBgColorNormal;
    sf::Color m_buttonBgColorHover;
    sf::Color m_buttonOutlineColor;

    // --- 3D 뷰 요소 ---
    sf::View m_3dView; // 3D 공간 렌더링 뷰
    sf::View m_uiView; // UI 렌더링 뷰
    std::array<Vec3D, 8> m_cubeVertices; // 방의 기본 정점
    std::array<Vec3D, 8> m_transformedVertices; // 변환된 정점 (회전, 크기 조절 등)
    std::vector<Edge> m_cubeEdges; // 방의 모서리
    float m_rotationX, m_rotationY; // 3D 뷰 회전 각도
    bool m_isDragging; // 마우스 드래그로 회전 중인지 여부
    sf::Vector2i m_lastMousePos; // 마지막 마우스 위치 (드래그 계산용)

    // Setting 화면에서 설정된 통로/창문을 시각적으로 표현하기 위해 재구성된 정보
    std::vector<SettingScreen::OpeningDefinition> m_passages_vis;
    std::vector<SettingScreen::OpeningDefinition> m_windows_vis;

    // --- 시뮬레이션 파라미터 및 상태 ---
    float m_roomWidth, m_roomDepth, m_roomHeight; // 방 크기 (설정 파일에서 로드)
    float m_volumeV; // 방 부피 (계산됨)
    int m_selectedPollutantIndex; // 선택된 오염물질 인덱스 (설정 파일에서 로드)
    int m_numPassages, m_numWindows; // 통로 및 창문 개수 (설정 파일에서 로드)

    float m_C0; // 초기 농도 (시뮬레이션 시작 시 고정)
    float m_S_param;  // 유입 속도 (사용자 조절 가능)
    float m_K_param;  // 제거 속도 상수 (사용자 조절 가능)
    
    float m_currentTime_t;       // 시뮬레이션 경과 시간 (분 단위)
    float m_currentConcentration_Ct; // 현재 시간 t에서의 농도

    bool m_simulationActive;     // 시뮬레이션이 현재 실행 중인지 (중단 상태가 아닌지)
    bool m_simulationStartedOnce; // "실행" 버튼이 한 번이라도 눌렸는지 (C0 고정용)
    InputBox* m_activeInputBox; // 현재 활성화된 입력창 포인터

    // 파티클 시스템 (먼지 효과)
    std::vector<Particle> m_particles; // 화면에 표시될 모든 파티클
    sf::Color m_particleColor; // 오염물질 종류에 따른 파티클 색상
    int m_maxParticles; // 화면에 표시될 최대 파티클 수
    float m_particleUpdateTimer; // 파티클 및 시뮬레이션 시간 업데이트 간격 제어용 타이머

    // --- 메소드 ---
    void setupUI(); // UI 요소 초기화 및 배치
    void setup3D(); // 3D 뷰 관련 설정 초기화
    void loadSettingsFromFile(const std::string& filename); // "Setting_values.text" 에서 설정값 로드
    void reconstructOpenings(); // 로드된 통로/창문 개수에 따라 시각적 개구부 정보 생성

    void projectVertices(); // 3D 정점을 2D 화면에 맞게 변환
    sf::Vector2f project(const Vec3D& p) const; // 단일 3D 점을 2D 화면 좌표로 변환
    void drawCuboidEdges(sf::RenderWindow& window); // 방의 모서리 그리기
    void drawOpeningsVisual(sf::RenderWindow& window); // 재구성된 통로/창문 그리기

    void updateButtonHovers(const sf::Vector2f& mousePos); // 버튼 위에 마우스 호버 시 시각 효과 업데이트
    void handleInputBoxEvents(sf::Event event); // 입력창 이벤트 처리

    void runSimulation(); // 시뮬레이션 시작
    void stopSimulation(); // 시뮬레이션 일시 중단
    void resetSimulationState(); // 시뮬레이션 상태 초기화 (값, 시간 등)
    void calculateCurrentConcentration(); // 농도 공식에 따라 현재 농도 계산
    
    void initializeDefaultSK(); // 오염물질 및 개구부에 따라 S, K 기본값 설정 및 입력창 업데이트
    void updateParticleSystem(sf::Time dt); // 파티클 이동 및 상태 업데이트
    void spawnParticles(); // 현재 농도에 따라 파티클 생성/제거

    // 오염물질별 기본 S, K 값 및 개구부 효과 상수
    // (헤더에 static const float로 선언하고 cpp에서 정의)
    static const float BASE_S_PM10; static const float BASE_K_PM10;
    static const float BASE_S_CO;   static const float BASE_K_CO;
    static const float BASE_S_CL2;  static const float BASE_K_CL2;

    static const float S_ADJUST_PASSAGE; static const float K_ADJUST_PASSAGE;
    static const float S_ADJUST_WINDOW;  static const float K_ADJUST_WINDOW;

    // SettingScreen과 동일한 개구부 상대 크기 계수 (시각적 일관성)
    static const float PASSAGE_RELATIVE_HEIGHT_FACTOR_SIM;
    static const float PASSAGE_RELATIVE_WIDTH_FACTOR_SIM;
    static const float WINDOW_RELATIVE_HEIGHT_FACTOR_SIM;
    static const float WINDOW_RELATIVE_WIDTH_FACTOR_SIM;
};

#endif // SIMULATION_HPP