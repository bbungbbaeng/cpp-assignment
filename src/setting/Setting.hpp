#ifndef SETTING_HPP
#define SETTING_HPP

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <array>

enum class ScreenState; // Forward declaration

struct Vec3D {
    float x, y, z;
};

struct Edge {
    int start, end;
};

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

    bool m_running;


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
    int m_selectedPollutantIndex;

    sf::Text m_buttonCreatePassage, m_buttonCreateWindow;
    sf::Text m_buttonRemovePassage, m_buttonRemoveWindow;
    sf::Text m_buttonStartSimulation;
    sf::RectangleShape m_shapeCreatePassage, m_shapeCreateWindow;
    sf::RectangleShape m_shapeRemovePassage, m_shapeRemoveWindow;
    sf::RectangleShape m_shapeStartSimulation;

    std::array<Vec3D, 8> m_cubeVertices;
    std::array<Vec3D, 8> m_transformedVertices;
    std::vector<Edge> m_cubeEdges;
    float m_roomWidth, m_roomDepth, m_roomHeight;
    float m_rotationX, m_rotationY;
    bool m_isDragging;
    sf::Vector2i m_lastMousePos;
    sf::View m_3dView;
    sf::View m_uiView;

    InputBox* m_activeInputBox;

    // --- START: Added for Passages and Windows ---
    struct OpeningDefinition {
        std::array<Vec3D, 4> local_coords; // Coordinates relative to cuboid center (-0.5 to 0.5 on each axis)
    };

    std::vector<OpeningDefinition> m_passages_defs;
    std::vector<OpeningDefinition> m_windows_defs;

    // Relative size definitions (as fractions of normalized dimension 1.0)
    static const float PASSAGE_RELATIVE_HEIGHT_FACTOR;
    static const float PASSAGE_RELATIVE_WIDTH_FACTOR;
    static const float WINDOW_RELATIVE_HEIGHT_FACTOR;
    static const float WINDOW_RELATIVE_WIDTH_FACTOR;

    void createPassage();
    void removePassage();
    void createWindow();
    void removeWindow();
    void drawOpenings(sf::RenderWindow& window);
    // --- END: Added for Passages and Windows ---

    void setupUI();
    void setup3D();
    void projectVertices();
    sf::Vector2f project(const Vec3D& p) const;
    void handleInputBoxEvents(sf::Event event);
    void drawCuboidEdges(sf::RenderWindow& window);
    void updateButtonHovers(const sf::Vector2f& mousePos);

    sf::Color m_buttonTextColorNormal;
    sf::Color m_buttonTextColorHover;
    sf::Color m_buttonBgColorNormal;
    sf::Color m_buttonBgColorHover;
    sf::Color m_buttonOutlineColor;
};

#endif