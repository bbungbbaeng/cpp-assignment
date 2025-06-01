#ifndef SETTING_HPP
#define SETTING_HPP

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <array>
#include "../screen/Screen.hpp"

// 3D 좌표를 나타내는 간단한 구조체
struct Vec3D {
    float x, y, z; // x, y, z 좌표
};

// 3D 모델의 모서리를 나타내는 구조체 (두 정점 인덱스 저장)
struct Edge {
    int start, end; // 모서리를 구성하는 시작 정점과 끝 정점의 인덱스
};

// 사용자 입력을 받는 텍스트 상자 클래스
class InputBox {
public:
    // 생성자
    InputBox();
    // InputBox 초기화 함수: 폰트, 위치, 크기, 플레이스홀더 텍스트 설정
    void setup(const sf::Font& font, sf::Vector2f position, sf::Vector2f size, const std::wstring& placeholder);
    // 이벤트 처리 함수 (주로 키 입력)
    void handleEvent(sf::Event event);
    // 상태 업데이트 함수 (커서 깜빡임 등)
    void update();
    // 렌더링 함수 (화면에 InputBox 그리기)
    void render(sf::RenderWindow& window);
    // InputBox 활성화/비활성화 설정 함수
    void setActive(bool active);
    // InputBox 활성화 상태 반환 함수
    bool isActive() const;
    // InputBox의 현재 텍스트 반환 함수
    std::string getText() const;
    // InputBox의 텍스트 설정 함수
    void setText(const std::string& text);
    // InputBox의 텍스트를 float 값으로 변환하여 반환하는 함수
    float getFloatValue() const;
    // InputBox의 전역 경계(위치 및 크기) 반환 함수 (마우스 클릭 감지 등에 사용)
    sf::FloatRect getGlobalBounds() const;

private:
    sf::RectangleShape m_shape;         // InputBox의 배경 모양 (사각형)
    sf::Text m_text;                  // 입력된 텍스트를 표시하는 SFML 텍스트 객체
    sf::Text m_placeholderText;       // 입력 내용이 없을 때 표시되는 플레이스홀더 텍스트
    std::string m_inputString;        // 현재 입력된 문자열 저장
    const sf::Font* m_font;           // 사용할 폰트에 대한 포인터
    bool m_isActive;                  // InputBox가 현재 활성화(입력 가능) 상태인지 여부
    sf::Clock m_cursorClock;          // 커서 깜빡임 타이밍을 위한 시계
    bool m_showCursor;                // 커서를 현재 보여줄지 여부
};

// 시뮬레이션 설정을 담당하는 화면 클래스
class SettingScreen {
public:
    // 생성자: 렌더링 창과 폰트에 대한 참조를 받음
    SettingScreen(sf::RenderWindow& window, sf::Font& font);
    // 소멸자
    ~SettingScreen();

    // 사용자 입력을 처리하는 함수
    void handleInput();
    // 화면의 상태를 업데이트하는 함수 (시간 경과에 따른 변화 등)
    void update(sf::Time dt); // dt: delta time (프레임 간 시간 간격)
    // 화면의 모든 요소를 그리는 함수
    void render();

    // 다음으로 전환될 화면 상태를 반환하는 함수
    ScreenState getNextState() const;
    // 현재 화면이 계속 실행 중인지 여부를 반환하는 함수
    bool isRunning() const;
    // 다음 화면 상태를 설정하는 함수
    void setNextState(ScreenState state);
    // 화면의 상태를 초기 상태로 리셋하는 함수
    void reset();

    // 화면 실행 여부 플래그 (main 루프에서 접근 가능하도록 public)
    bool m_running;

    // 통로나 창문의 정의를 위한 내부 구조체
    // SimulationScreen에서도 시각적 재구성을 위해 이 정의를 사용할 수 있도록 public으로 선언
    struct OpeningDefinition {
        std::array<Vec3D, 4> local_coords; // 개구부를 구성하는 4개 정점의 로컬 정규화 좌표
    };

private:
    // SFML 렌더링 창 및 폰트에 대한 참조
    sf::RenderWindow& m_window;
    sf::Font& m_font;
    // 다음 화면 상태 저장 변수
    ScreenState m_nextState;

    // UI 요소: 제목 텍스트
    sf::Text m_titleText;

    // UI 요소: 방 크기 입력을 위한 InputBox 객체들 및 해당 라벨
    InputBox m_inputWidth;
    InputBox m_inputDepth;
    InputBox m_inputHeight;
    sf::Text m_labelWidth, m_labelDepth, m_labelHeight;

    // UI 요소: 오염 물질 선택 관련
    sf::Text m_labelPollutant;                          // "오염 물질:" 라벨
    std::vector<sf::Text> m_pollutantOptions;           // 각 오염 물질 옵션 텍스트
    std::vector<sf::RectangleShape> m_pollutantOptionShapes; // 각 오염 물질 옵션 버튼 모양
    int m_selectedPollutantIndex;                       // 현재 선택된 오염 물질의 인덱스

    // UI 요소: 버튼 텍스트들
    sf::Text m_buttonCreatePassage, m_buttonCreateWindow; // 통로/창문 생성 버튼
    sf::Text m_buttonRemovePassage, m_buttonRemoveWindow; // 통로/창문 제거 버튼
    sf::Text m_buttonStartSimulation;                   // 시뮬레이션 시작 버튼
    // UI 요소: 버튼 모양들
    sf::RectangleShape m_shapeCreatePassage, m_shapeCreateWindow;
    sf::RectangleShape m_shapeRemovePassage, m_shapeRemoveWindow;
    sf::RectangleShape m_shapeStartSimulation;

    // UI 요소: 통로 및 창문 개수 표시 텍스트
    sf::Text m_textPassageCount;
    sf::Text m_textWindowCount;

    // 3D 모델링 관련 멤버 변수
    std::array<Vec3D, 8> m_cubeVertices;        // 육면체의 기본 8개 정점 (로컬 정규화 좌표)
    std::array<Vec3D, 8> m_transformedVertices; // 변환(회전, 크기 조절)된 정점 좌표
    std::vector<Edge> m_cubeEdges;              // 육면체의 모서리 정보
    float m_roomWidth, m_roomDepth, m_roomHeight; // 현재 설정된 방의 실제 크기
    float m_rotationX, m_rotationY;             // 3D 뷰의 X축, Y축 회전 각도 (라디안)
    bool m_isDragging;                          // 마우스로 3D 뷰를 드래그 중인지 여부
    sf::Vector2i m_lastMousePos;                // 마지막 마우스 위치 (드래그 계산용)
    sf::View m_3dView;                          // 3D 장면을 렌더링하기 위한 뷰
    sf::View m_uiView;                          // UI 요소를 렌더링하기 위한 뷰

    // 현재 활성화된(클릭된) InputBox 객체를 가리키는 포인터
    InputBox* m_activeInputBox;

    // 생성된 통로 및 창문들의 정의를 저장하는 벡터
    std::vector<OpeningDefinition> m_passages_defs;
    std::vector<OpeningDefinition> m_windows_defs;

    // 통로 및 창문의 상대적 크기를 정의하는 static const 멤버 상수 (선언부)
    // 실제 값은 .cpp 파일에 정의됨
    static const float PASSAGE_RELATIVE_HEIGHT_FACTOR; // 통로 높이 비율
    static const float PASSAGE_RELATIVE_WIDTH_FACTOR;  // 통로 너비 비율
    static const float WINDOW_RELATIVE_HEIGHT_FACTOR;  // 창문 높이 비율
    static const float WINDOW_RELATIVE_WIDTH_FACTOR;   // 창문 너비 비율

    // private 헬퍼 함수들: 클래스 내부 로직 구현
    // 현재 설정된 값들을 파일에 저장하는 함수
    void saveSettingsToFile(const std::string& filename) const;

    // 통로/창문 생성 및 제거 함수
    void createPassage();
    void removePassage();
    void createWindow();
    void removeWindow();
    // 생성된 통로/창문을 3D 뷰에 그리는 함수
    void drawOpenings(sf::RenderWindow& window);

    // 통로/창문 개수 표시 텍스트 업데이트 함수
    void updatePassageCountText();
    void updateWindowCountText();

    // UI 및 3D 요소 초기 설정 함수
    void setupUI();
    void setup3D();
    // 3D 정점들을 현재 설정에 맞게 변환하는 함수
    void projectVertices();
    // 3D 좌표를 2D 화면 좌표로 투영하는 함수
    sf::Vector2f project(const Vec3D& p) const;
    // 활성화된 입력 상자의 이벤트를 처리하는 함수
    void handleInputBoxEvents(sf::Event event);
    // 3D 육면체의 모서리를 그리는 함수
    void drawCuboidEdges(sf::RenderWindow& window);
    // 마우스 위치에 따라 버튼의 호버 스타일을 업데이트하는 함수
    void updateButtonHovers(const sf::Vector2f& mousePos);

    // 버튼 시각 효과에 사용될 색상 변수들
    sf::Color m_buttonTextColorNormal; // 버튼 텍스트 일반 색상
    sf::Color m_buttonTextColorHover;  // 버튼 텍스트 호버 시 색상
    sf::Color m_buttonBgColorNormal;   // 버튼 배경 일반 색상
    sf::Color m_buttonBgColorHover;    // 버튼 배경 호버 시 색상
    sf::Color m_buttonOutlineColor;    // 버튼 외곽선 색상
};

extern const float PI;

#endif