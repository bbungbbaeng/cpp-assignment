#ifndef SETTING_HPP
#define SETTING_HPP

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <array>
// #include "../screen/Screen.hpp" // ScreenState 때문에 필요하지만, 순환 참조를 피하기 위해 전방 선언 사용 가능
// 여기서는 main.cpp에서 ScreenState를 관리하므로 Setting.hpp에서 직접 Screen.hpp를 포함하지 않아도 될 수 있음.
// 그러나 SettingScreen 생성자 등에서 ScreenState를 사용하므로, 포함하거나 전방 선언해야 함.
// 제공된 코드에서는 Screen.hpp를 포함하고 있었으므로 그대로 둠.
#include "../screen/Screen.hpp"


// 3D 좌표 표현을 위한 구조체
struct Vec3D { // SimulationScreen에서도 사용하므로 여기에 두거나 공용 헤더로 이동 가능
    float x, y, z;
};

// 3D 모델의 모서리 표현을 위한 구조체
struct Edge { // SimulationScreen에서도 사용 가능
    int start, end;
};

// 사용자 입력을 받는 상자 클래스
class InputBox {
public:
    InputBox();
    void setup(const sf::Font& font, sf::Vector2f position, sf::Vector2f size, const std::wstring& placeholder);
    void handleEvent(sf::Event event);
    void update();
    void render(sf::RenderWindow& window);
    void setActive(bool active);
    bool isActive() const;
    std::string getText() const;
    void setText(const std::string& text);
    float getFloatValue() const;
    sf::FloatRect getGlobalBounds() const;

private:
    sf::RectangleShape m_shape;
    sf::Text m_text;
    sf::Text m_placeholderText;
    std::string m_inputString;
    const sf::Font* m_font;
    bool m_isActive;
    sf::Clock m_cursorClock;
    bool m_showCursor;
};


class SettingScreen {
public:
    SettingScreen(sf::RenderWindow& window, sf::Font& font);
    ~SettingScreen();

    void handleInput();
    void update(sf::Time dt);
    void render();

    ScreenState getNextState() const;
    bool isRunning() const;
    void setNextState(ScreenState state);
    void reset(); // Setting 화면으로 돌아올 때 상태 초기화 (main.cpp에서 호출 가정)

    bool m_running; // main 루프에서 접근하기 위해 public

    // 통로나 창문의 정의 (SimulationScreen에서도 시각적 재구성을 위해 이 구조체 정의를 알아야 함)
    // SimulationScreen이 이 구조체를 직접 사용하지 않고 개수만 전달받는다면 private이어도 무방.
    // 현재 SimulationScreen은 개수만 받고 자체 로직으로 재구성하므로 private 유지 가능.
    // 만약 SettingScreen과 동일한 개구부 객체를 전달하려면 public으로 하거나 getter 필요.
    struct OpeningDefinition {
        std::array<Vec3D, 4> local_coords; // 개구부의 로컬 정규화 좌표
    };


private:
    sf::RenderWindow& m_window;
    sf::Font& m_font;
    ScreenState m_nextState;

    sf::Text m_titleText;

    InputBox m_inputWidth;
    InputBox m_inputDepth;
    InputBox m_inputHeight;
    sf::Text m_labelWidth, m_labelDepth, m_labelHeight;

    sf::Text m_labelPollutant;
    std::vector<sf::Text> m_pollutantOptions;
    std::vector<sf::RectangleShape> m_pollutantOptionShapes;
    int m_selectedPollutantIndex; // 선택된 오염물질 인덱스

    sf::Text m_buttonCreatePassage, m_buttonCreateWindow;
    sf::Text m_buttonRemovePassage, m_buttonRemoveWindow;
    sf::Text m_buttonStartSimulation; // 시뮬레이션 시작 버튼
    sf::RectangleShape m_shapeCreatePassage, m_shapeCreateWindow;
    sf::RectangleShape m_shapeRemovePassage, m_shapeRemoveWindow;
    sf::RectangleShape m_shapeStartSimulation;

    sf::Text m_textPassageCount; // 통로 개수 표시 텍스트
    sf::Text m_textWindowCount;  // 창문 개수 표시 텍스트

    // 3D 모델링 관련 변수
    std::array<Vec3D, 8> m_cubeVertices;
    std::array<Vec3D, 8> m_transformedVertices;
    std::vector<Edge> m_cubeEdges;
    float m_roomWidth, m_roomDepth, m_roomHeight; // 현재 설정된 방 크기
    float m_rotationX, m_rotationY; // 3D 뷰 회전 각도
    bool m_isDragging;
    sf::Vector2i m_lastMousePos;
    sf::View m_3dView;
    sf::View m_uiView;

    InputBox* m_activeInputBox; // 현재 활성화된 입력창

    std::vector<OpeningDefinition> m_passages_defs; // 생성된 통로 정보
    std::vector<OpeningDefinition> m_windows_defs;  // 생성된 창문 정보

    // 개구부 상대 크기 정의 (SettingScreen 전용)
    static const float PASSAGE_RELATIVE_HEIGHT_FACTOR;
    static const float PASSAGE_RELATIVE_WIDTH_FACTOR;
    static const float WINDOW_RELATIVE_HEIGHT_FACTOR;
    static const float WINDOW_RELATIVE_WIDTH_FACTOR;
    
    // 설정 저장 함수 (SimulationScreen으로 값 전달용)
    void saveSettingsToFile(const std::string& filename) const; // 추가된 함수

    // 내부 헬퍼 함수들
    void createPassage();
    void removePassage();
    void createWindow();
    void removeWindow();
    void drawOpenings(sf::RenderWindow& window);
    
    void updatePassageCountText();
    void updateWindowCountText();

    void setupUI();
    void setup3D();
    void projectVertices();
    sf::Vector2f project(const Vec3D& p) const;
    void handleInputBoxEvents(sf::Event event);
    void drawCuboidEdges(sf::RenderWindow& window);
    void updateButtonHovers(const sf::Vector2f& mousePos);

    // 버튼 시각 효과용 색상
    sf::Color m_buttonTextColorNormal;
    sf::Color m_buttonTextColorHover;
    sf::Color m_buttonBgColorNormal;
    sf::Color m_buttonBgColorHover;
    sf::Color m_buttonOutlineColor;
};

// PI 상수 (필요시 공용 헤더로 이동)
extern const float PI; // 정의는 Setting.cpp 에 있음

#endif // SETTING_HPP