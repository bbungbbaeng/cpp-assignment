#include "Screen.hpp"
#include <iostream>
#include <cmath>

const float BUTTON_WIDTH = 120.f;
const float BUTTON_HEIGHT = 40.f;
const sf::Color BACKGROUND_COLOR = sf::Color::Black;
const sf::Color TEXT_COLOR_NORMAL = sf::Color::White;
const sf::Color TEXT_COLOR_HOVER = sf::Color::Black;
const sf::Color BUTTON_OUTLINE_COLOR_NORMAL = sf::Color::White;
const sf::Color BUTTON_FILL_COLOR_HOVER = sf::Color::White;
const float BUTTON_OUTLINE_THICKNESS = 2.f;
const float BUTTON_SPACING = 30.f;
const float TITLE_CHAR_SIZE = 48;
const float BUTTON_CHAR_SIZE = 24;
const float LEFT_MARGIN = 100.f;


StartScreen::StartScreen(sf::RenderWindow& window, sf::Font& font)
    : m_window(window), m_font(font),
      m_isStartButtonHovered(false), m_isExitButtonHovered(false),
      m_nextState(ScreenState::START), m_running(true),
      m_mouseMovedSinceLastUpdate(false),
      m_rng(std::random_device{}()),
      m_distX(0.f, static_cast<float>(m_window.getSize().x)),
      m_distY(0.f, static_cast<float>(m_window.getSize().y)),
      m_distAngle(0.f, 2.f * 3.1415926535f),
      m_distSpeed(MAX_DUST_SPEED * 0.5f, MAX_DUST_SPEED)
{
    setupTexts();
    setupButtons();
    setupDustParticles();
    m_mousePosition = sf::Vector2f(m_window.mapPixelToCoords(sf::Mouse::getPosition(m_window)));
}

StartScreen::~StartScreen() {}

void StartScreen::reset() {
    m_running = true;
    m_nextState = ScreenState::START;
    m_isStartButtonHovered = false;
    m_isExitButtonHovered = false;
    m_mouseMovedSinceLastUpdate = false;
    // setupDustParticles(); // 필요에 따라 파티클 재설정
    m_mousePosition = sf::Vector2f(m_window.mapPixelToCoords(sf::Mouse::getPosition(m_window)));
}

void StartScreen::setupTexts() {
    m_titleText.setFont(m_font);
    m_titleText.setString(L"실내 공기 오염 시뮬레이터");
    m_titleText.setCharacterSize(TITLE_CHAR_SIZE);
    m_titleText.setFillColor(TEXT_COLOR_NORMAL);
    sf::FloatRect titleBounds = m_titleText.getLocalBounds();
    m_titleText.setOrigin(titleBounds.left, titleBounds.top + titleBounds.height / 2.f);
    m_titleText.setPosition(LEFT_MARGIN, static_cast<float>(m_window.getSize().y) / 2.f - BUTTON_HEIGHT * 2 - BUTTON_SPACING * 2);

    m_startButtonText.setFont(m_font);
    m_startButtonText.setString(L"실행");
    m_startButtonText.setCharacterSize(BUTTON_CHAR_SIZE);
    m_startButtonText.setFillColor(TEXT_COLOR_NORMAL);

    m_exitButtonText.setFont(m_font);
    m_exitButtonText.setString(L"종료");
    m_exitButtonText.setCharacterSize(BUTTON_CHAR_SIZE);
    m_exitButtonText.setFillColor(TEXT_COLOR_NORMAL);
}

void StartScreen::setupButtons() {
    m_startButtonShape.setSize(sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT));
    m_startButtonShape.setOutlineColor(BUTTON_OUTLINE_COLOR_NORMAL);
    m_startButtonShape.setOutlineThickness(BUTTON_OUTLINE_THICKNESS);
    m_startButtonShape.setFillColor(sf::Color::Transparent);
    m_startButtonShape.setPosition(LEFT_MARGIN, m_titleText.getPosition().y + m_titleText.getGlobalBounds().height + BUTTON_SPACING * 1.5f);

    sf::FloatRect startTextBounds = m_startButtonText.getLocalBounds();
    m_startButtonText.setOrigin(startTextBounds.left + startTextBounds.width / 2.f, startTextBounds.top + startTextBounds.height / 2.f);
    m_startButtonText.setPosition(m_startButtonShape.getPosition().x + BUTTON_WIDTH / 2.f, m_startButtonShape.getPosition().y + BUTTON_HEIGHT / 2.f);

    m_exitButtonShape.setSize(sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT));
    m_exitButtonShape.setOutlineColor(BUTTON_OUTLINE_COLOR_NORMAL);
    m_exitButtonShape.setOutlineThickness(BUTTON_OUTLINE_THICKNESS);
    m_exitButtonShape.setFillColor(sf::Color::Transparent);
    m_exitButtonShape.setPosition(LEFT_MARGIN, m_startButtonShape.getPosition().y + BUTTON_HEIGHT + BUTTON_SPACING);

    sf::FloatRect exitTextBounds = m_exitButtonText.getLocalBounds();
    m_exitButtonText.setOrigin(exitTextBounds.left + exitTextBounds.width / 2.f, exitTextBounds.top + exitTextBounds.height / 2.f);
    m_exitButtonText.setPosition(m_exitButtonShape.getPosition().x + BUTTON_WIDTH / 2.f, m_exitButtonShape.getPosition().y + BUTTON_HEIGHT / 2.f);
}

void StartScreen::setupDustParticles() {
    m_dustParticles.clear();
    m_dustParticles.reserve(NUM_DUST_PARTICLES);

    for (int i = 0; i < NUM_DUST_PARTICLES; ++i) {
        DustParticle p;
        p.shape.setRadius(DUST_PARTICLE_RADIUS);
        p.shape.setFillColor(DUST_COLOR);
        p.shape.setOrigin(DUST_PARTICLE_RADIUS, DUST_PARTICLE_RADIUS);
        p.shape.setPosition(m_distX(m_rng), m_distY(m_rng));

        float angle = m_distAngle(m_rng);
        float speed = m_distSpeed(m_rng);
        p.velocity = sf::Vector2f(std::cos(angle) * speed, std::sin(angle) * speed);

        m_dustParticles.push_back(p);
    }
}

void StartScreen::updateButtonStyles() {
    if (m_isStartButtonHovered) {
        m_startButtonText.setFillColor(TEXT_COLOR_HOVER);
        m_startButtonShape.setFillColor(BUTTON_FILL_COLOR_HOVER);
    } else {
        m_startButtonText.setFillColor(TEXT_COLOR_NORMAL);
        m_startButtonShape.setFillColor(sf::Color::Transparent);
    }

    if (m_isExitButtonHovered) {
        m_exitButtonText.setFillColor(TEXT_COLOR_HOVER);
        m_exitButtonShape.setFillColor(BUTTON_FILL_COLOR_HOVER);
    } else {
        m_exitButtonText.setFillColor(TEXT_COLOR_NORMAL);
        m_exitButtonShape.setFillColor(sf::Color::Transparent);
    }
}

void StartScreen::updateDustParticles(sf::Time dt) {
    float deltaTime = dt.asSeconds();
    sf::Vector2u windowSize = m_window.getSize();

    for (auto& p : m_dustParticles) {
        p.shape.move(p.velocity * deltaTime);

        sf::Vector2f pos = p.shape.getPosition();
        bool bounced = false;
        if (pos.x < 0 || pos.x > windowSize.x) {
            p.velocity.x *= -1;
            pos.x = std::max(0.f, std::min(pos.x, static_cast<float>(windowSize.x)));
            bounced = true;
        }
        if (pos.y < 0 || pos.y > windowSize.y) {
            p.velocity.y *= -1;
            pos.y = std::max(0.f, std::min(pos.y, static_cast<float>(windowSize.y)));
            bounced = true;
        }
        if(bounced) {
            p.shape.setPosition(pos);
        }

        if (m_mouseMovedSinceLastUpdate) {
            sf::Vector2f dirFromMouseToParticle = p.shape.getPosition() - m_mousePosition;
            float distanceToMouseSquared = dirFromMouseToParticle.x * dirFromMouseToParticle.x + dirFromMouseToParticle.y * dirFromMouseToParticle.y;

            if (distanceToMouseSquared < MOUSE_REPEL_RADIUS * MOUSE_REPEL_RADIUS && distanceToMouseSquared > 0.0001f) {
                float distanceToMouse = std::sqrt(distanceToMouseSquared);
                sf::Vector2f repelDirection = dirFromMouseToParticle / distanceToMouse;
                float strengthFactor = (MOUSE_REPEL_RADIUS - distanceToMouse) / MOUSE_REPEL_RADIUS;
                p.velocity = repelDirection * MOUSE_REPEL_STRENGTH * strengthFactor;

                float currentSpeed = std::sqrt(p.velocity.x * p.velocity.x + p.velocity.y * p.velocity.y);
                float maxRepelSpeed = MAX_DUST_SPEED * 3.0f;
                if (currentSpeed > maxRepelSpeed) {
                    p.velocity = (p.velocity / currentSpeed) * maxRepelSpeed;
                }
            }
        } else {
            float currentSpeed = std::sqrt(p.velocity.x * p.velocity.x + p.velocity.y * p.velocity.y);
            float targetMinSpeed = MAX_DUST_SPEED * 0.5f;
            float targetMaxSpeed = MAX_DUST_SPEED;

            if (currentSpeed > targetMaxSpeed) {
                p.velocity *= (1.f - 2.0f * deltaTime);
                if (std::sqrt(p.velocity.x * p.velocity.x + p.velocity.y * p.velocity.y) < targetMaxSpeed) {
                     float angle = std::atan2(p.velocity.y, p.velocity.x);
                     if (currentSpeed > 0.001f) { 
                        p.velocity.x = std::cos(angle) * targetMaxSpeed;
                        p.velocity.y = std::sin(angle) * targetMaxSpeed;
                     }
                }
            } else if (currentSpeed < targetMinSpeed && currentSpeed > 0.01f) {
                 p.velocity *= (1.f + 1.0f * deltaTime);
                 if (std::sqrt(p.velocity.x * p.velocity.x + p.velocity.y * p.velocity.y) > targetMinSpeed) {
                     float angle = std::atan2(p.velocity.y, p.velocity.x);
                     if (currentSpeed > 0.001f) {
                        p.velocity.x = std::cos(angle) * targetMinSpeed;
                        p.velocity.y = std::sin(angle) * targetMinSpeed;
                     }
                 }
            } else if (currentSpeed <= 0.01f) { 
                float angle = m_distAngle(m_rng);
                float speed = m_distSpeed(m_rng);
                p.velocity = sf::Vector2f(std::cos(angle) * speed, std::sin(angle) * speed);
            }
        }
    }
}

void StartScreen::handleInput() {
    m_mouseMovedSinceLastUpdate = false;

    sf::Event event;
    while (m_window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            m_running = false;
            m_nextState = ScreenState::EXIT;
        }
        if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
            m_running = false;
            m_nextState = ScreenState::EXIT;
        }

        if (event.type == sf::Event::MouseMoved) {
            m_mousePosition = m_window.mapPixelToCoords(sf::Vector2i(event.mouseMove.x, event.mouseMove.y));
            m_mouseMovedSinceLastUpdate = true;

            sf::Vector2f mousePosView = m_window.mapPixelToCoords(sf::Mouse::getPosition(m_window));
            m_isStartButtonHovered = m_startButtonShape.getGlobalBounds().contains(mousePosView);
            m_isExitButtonHovered = m_exitButtonShape.getGlobalBounds().contains(mousePosView);
        }

        if (event.type == sf::Event::MouseButtonPressed) {
            if (event.mouseButton.button == sf::Mouse::Left) {
                sf::Vector2f mousePosView = m_window.mapPixelToCoords(sf::Mouse::getPosition(m_window));
                if (m_isStartButtonHovered) {
                    m_nextState = ScreenState::SETTING;
                    m_running = false;
                }
                if (m_isExitButtonHovered) {
                    m_running = false;
                    m_nextState = ScreenState::EXIT;
                }
            }
        }
    }
}

void StartScreen::update(sf::Time dt) {
    updateButtonStyles();
    updateDustParticles(dt);
}

void StartScreen::render() {
    m_window.clear(BACKGROUND_COLOR);

    for (const auto& p : m_dustParticles) {
        m_window.draw(p.shape);
    }

    m_window.draw(m_titleText);
    m_window.draw(m_startButtonShape);
    m_window.draw(m_startButtonText);
    m_window.draw(m_exitButtonShape);
    m_window.draw(m_exitButtonText);

    m_window.display();
}

ScreenState StartScreen::getNextState() const {
    return m_nextState;
}

bool StartScreen::isRunning() const {
    return m_running;
}