#ifndef SIMULATION_HPP
#define SIMULATION_HPP

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <array>
#include <list>
#include "../setting/Setting.hpp"
#include "../screen/Screen.hpp"

// 시뮬레이션 내의 먼지(오염물질) 입자를 나타내는 구조체
struct Particle {
    Vec3D position3D;           // 입자의 3D 공간 내 위치 (정규화된 좌표 또는 월드 좌표)
    sf::CircleShape shape;      // 입자의 시각적 표현 (SFML 원 모양)
    Vec3D velocity;             // 입자의 3D 공간 내 이동 속도
    float currentAlpha;         // 입자의 현재 투명도 (0.0 ~ 255.0)
    float lifetime;             // 입자의 남은 수명 (초 단위)
};

// 시뮬레이션 화면을 담당하는 클래스
class SimulationScreen {
public:
    // 생성자: 렌더링 창과 폰트에 대한 참조를 받음
    SimulationScreen(sf::RenderWindow& window, sf::Font& font);
    // 소멸자
    ~SimulationScreen();

    // 사용자 입력을 처리하는 함수
    void handleInput();
    // 화면의 상태를 업데이트하는 함수 (시간 경과, 농도 변화, 파티클 움직임 등)
    void update(sf::Time dt); // dt: delta time (프레임 간 시간 간격)
    // 화면의 모든 요소를 그리는 함수
    void render();

    // 다음으로 전환될 화면 상태를 반환하는 함수
    ScreenState getNextState() const;
    // 현재 화면이 계속 실행 중인지 여부를 반환하는 함수
    bool isRunning() const;
    // 다음 화면 상태를 설정하는 함수
    void setNextState(ScreenState state);
    // 화면의 상태를 초기 상태로 리셋하는 함수 (설정값 다시 로드 등)
    void reset();

    // 화면 실행 여부 플래그 (main 루프에서 접근 가능하도록 public)
    bool m_running;

private:
    // SFML 렌더링 창 및 폰트에 대한 참조
    sf::RenderWindow& m_window;
    sf::Font& m_font;
    // 다음 화면 상태 저장 변수
    ScreenState m_nextState;

    // UI 요소: 제목 텍스트
    sf::Text m_titleText;
    // UI 요소: 시뮬레이션 파라미터 입력을 위한 InputBox 객체들 및 해당 라벨
    InputBox m_inputC0, m_inputS, m_inputK; // 초기 농도, 유입 속도, 제거 상수
    sf::Text m_labelC0, m_labelS, m_labelK;
    // UI 요소: 계산된 값 또는 상태 표시 텍스트 및 해당 라벨
    sf::Text m_displayVolume, m_displayTime, m_displayConcentration; // 부피, 시간, 현재 농도
    sf::Text m_labelVolume, m_labelTime, m_labelConcentration;

    // UI 요소: 시뮬레이션 제어 버튼 텍스트들
    sf::Text m_buttonRun, m_buttonStop, m_buttonReset, m_buttonBack;
    // UI 요소: 시뮬레이션 제어 버튼 모양들
    sf::RectangleShape m_shapeRun, m_shapeStop, m_shapeReset, m_shapeBack;

    // 버튼 스타일(색상) 관련 멤버 변수
    sf::Color m_buttonTextColorNormal;
    sf::Color m_buttonTextColorHover;
    sf::Color m_buttonBgColorNormal;
    sf::Color m_buttonBgColorHover;
    sf::Color m_buttonOutlineColor;

    // 3D 및 UI 렌더링을 위한 뷰 객체
    sf::View m_3dView;  // 3D 장면용 뷰
    sf::View m_uiView;  // UI 요소용 뷰
    // 3D 육면체 모델 관련 변수
    std::array<Vec3D, 8> m_cubeVertices;        // 육면체 기본 정점 (로컬 정규화 좌표)
    std::array<Vec3D, 8> m_transformedVertices; // 변환(회전, 크기 조절)된 정점 좌표
    std::vector<Edge> m_cubeEdges;              // 육면체 모서리 정보
    float m_rotationX, m_rotationY;             // 3D 뷰 회전 각도 (라디안)
    bool m_isDragging;                          // 마우스 드래그로 3D 뷰 회전 중인지 여부
    sf::Vector2i m_lastMousePos;                // 마지막 마우스 위치 (드래그 계산용)

    // 설정 화면에서 로드한 통로/창문 정보를 시각적으로 표현하기 위한 구조체 벡터
    std::vector<SettingScreen::OpeningDefinition> m_passages_vis;
    std::vector<SettingScreen::OpeningDefinition> m_windows_vis;

    // 시뮬레이션 공간 및 오염물질 관련 파라미터 (설정 파일에서 로드)
    float m_roomWidth, m_roomDepth, m_roomHeight; // 방의 실제 크기
    float m_volumeV;                              // 방의 부피 (계산됨)
    int m_selectedPollutantIndex;                 // 선택된 오염 물질 인덱스
    int m_numPassages, m_numWindows;              // 통로 및 창문 개수

    // 시뮬레이션 핵심 파라미터 (사용자 입력 또는 계산)
    float m_C0;        // 초기 농도 (시뮬레이션 시작 시 고정)
    float m_S_param;   // 유입 속도 (사용자 조절 가능)
    float m_K_param;   // 제거 속도 상수 (사용자 조절 가능)

    // 시뮬레이션 진행 상태 변수
    float m_currentTime_t;                       // 현재 시뮬레이션 경과 시간 (분)
    float m_currentConcentration_Ct;             // 현재 시간 t에서의 실제 농도
    float m_targetConcentration_Ct_for_particles; // 파티클 수 조절을 위한 목표 농도 (부드러운 전환용)

    // 시뮬레이션 제어 플래그
    bool m_simulationActive;      // 시뮬레이션이 현재 실행(활성) 상태인지 여부
    bool m_simulationStartedOnce; // "실행" 버튼이 한 번이라도 눌렸는지 (C0 고정 판단용)
    InputBox* m_activeInputBox;   // 현재 활성화된 InputBox 포인터

    // 파티클 시스템 관련 멤버 변수
    std::list<Particle> m_particles;              // 화면에 표시될 모든 파티클 저장 (list로 변경)
    sf::Color m_particleColor;                    // 오염물질 종류에 따른 기본 파티클 색상 (알파값은 개별 조절)
    int m_maxParticles;                           // 화면에 표시될 최대 파티클 수
    float m_simulationTimeStepAccumulator;      // 시뮬레이션 시간 1분 단위 진행을 위한 누적 시간

    // 파티클 동작 관련 static const 상수 (선언부, 실제 값은 .cpp에 정의)
    static const float PARTICLE_MAX_LIFETIME;      // 파티클 최대 수명 (초)
    static const float PARTICLE_FADE_RATE;         // 파티클 사라지는 속도 (초당 알파 감소량)
    static const int PARTICLES_PER_FRAME_ADJUST; // 프레임당 추가/제거할 파티클 수 (부드러운 변화용)

    // private 헬퍼 함수들: 클래스 내부 로직 구현
    void setupUI();    // UI 요소 초기화 및 배치
    void setup3D();    // 3D 뷰 관련 설정 초기화
    void loadSettingsFromFile(const std::string& filename); // 설정 파일에서 값 로드
    void reconstructOpenings(); // 로드된 통로/창문 개수에 따라 시각적 개구부 정보 생성

    void projectVertices(); // 3D 정점을 현재 설정에 맞게 변환
    sf::Vector2f project(const Vec3D& p) const; // 단일 3D 점을 2D 화면 좌표로 투영
    void drawCuboidEdges(sf::RenderWindow& window);    // 3D 육면체 모서리 그리기
    void drawOpeningsVisual(sf::RenderWindow& window); // 재구성된 통로/창문 그리기

    void updateButtonHovers(const sf::Vector2f& mousePos); // 버튼 호버 스타일 업데이트
    void handleInputBoxEvents(sf::Event event);           // 활성화된 입력 상자 이벤트 처리

    void runSimulation();        // 시뮬레이션 시작
    void stopSimulation();       // 시뮬레이션 일시 중단
    void resetSimulationState(); // 시뮬레이션 상태 전체 초기화
    void calculateCurrentConcentration(); // 현재 농도 계산

    void initializeDefaultSK(); // 오염물질 및 개구부에 따른 S, K 기본값 설정
    void updateParticleSystem(sf::Time dt); // 파티클 이동, 수명, 알파값 등 업데이트
    void adjustParticleCount();  // 목표 농도에 맞춰 파티클 수 점진적 조절
    void spawnNewParticle();     // 새로운 단일 파티클 생성

    // 오염물질별 기본 S, K 값 및 개구부 효과에 대한 static const 상수 (선언부)
    // 실제 값은 .cpp 파일에 정의됨
    static const float BASE_S_PM10; static const float BASE_K_PM10;
    static const float BASE_S_CO;   static const float BASE_K_CO;
    static const float BASE_S_CL2;  static const float BASE_K_CL2;

    static const float S_ADJUST_PASSAGE; static const float K_ADJUST_PASSAGE;
    static const float S_ADJUST_WINDOW;  static const float K_ADJUST_WINDOW;

    // SettingScreen과 시각적 일관성을 위한 개구부 상대 크기 계수 (선언부)
    static const float PASSAGE_RELATIVE_HEIGHT_FACTOR_SIM;
    static const float PASSAGE_RELATIVE_WIDTH_FACTOR_SIM;
    static const float WINDOW_RELATIVE_HEIGHT_FACTOR_SIM;
    static const float WINDOW_RELATIVE_WIDTH_FACTOR_SIM;
};

#endif