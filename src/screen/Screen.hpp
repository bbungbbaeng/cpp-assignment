#ifndef SCREEN_HPP
#define SCREEN_HPP

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <random>

enum class ScreenState {
    START,
    SETTING,
    SIMULATION,
    EXIT
};

class StartScreen {
public:
    StartScreen(sf::RenderWindow& window, sf::Font& font);
    ~StartScreen();

    void handleInput();
    void update(sf::Time dt);
    void render();

    ScreenState getNextState() const;
    bool isRunning() const;
    void reset(); // reset 메서드 추가

private:
    sf::RenderWindow& m_window;
    sf::Font& m_font;

    sf::Text m_titleText;
    sf::Text m_startButtonText;
    sf::Text m_exitButtonText;

    sf::RectangleShape m_startButtonShape;
    sf::RectangleShape m_exitButtonShape;

    bool m_isStartButtonHovered;
    bool m_isExitButtonHovered;

    ScreenState m_nextState;
    bool m_running;

    struct DustParticle {
        sf::CircleShape shape;
        sf::Vector2f velocity;
    };
    std::vector<DustParticle> m_dustParticles;

    const int NUM_DUST_PARTICLES = 700;
    const float DUST_PARTICLE_RADIUS = 1.5f;
    const sf::Color DUST_COLOR = sf::Color(100, 100, 100);
    const float MAX_DUST_SPEED = 50.f;
    const float MOUSE_REPEL_RADIUS = 100.f;
    const float MOUSE_REPEL_STRENGTH = 200.f;

    sf::Vector2f m_mousePosition;
    bool m_mouseMovedSinceLastUpdate;

    std::mt19937 m_rng;
    std::uniform_real_distribution<float> m_distX;
    std::uniform_real_distribution<float> m_distY;
    std::uniform_real_distribution<float> m_distAngle;
    std::uniform_real_distribution<float> m_distSpeed;

    void setupTexts();
    void setupButtons();
    void updateButtonStyles();
    void setupDustParticles();
    void updateDustParticles(sf::Time dt);
};

#endif