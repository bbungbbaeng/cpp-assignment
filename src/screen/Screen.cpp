#include "Screen.hpp"
#include <iostream> // std::cout, std::cerr 등 (디버깅용으로 사용될 수 있음)
#include <cmath>    // std::cos, std::sin, std::sqrt, std::atan2 등

// 버튼 및 UI 관련 상수들
const float BUTTON_WIDTH = 120.f;
const float BUTTON_HEIGHT = 40.f;
const sf::Color BACKGROUND_COLOR = sf::Color::Black;
const sf::Color TEXT_COLOR_NORMAL = sf::Color::White;
const sf::Color TEXT_COLOR_HOVER = sf::Color::Black; // 버튼 위에 마우스 올렸을 때 텍스트 색
const sf::Color BUTTON_OUTLINE_COLOR_NORMAL = sf::Color::White;
const sf::Color BUTTON_FILL_COLOR_HOVER = sf::Color::White; // 버튼 위에 마우스 올렸을 때 배경색
const float BUTTON_OUTLINE_THICKNESS = 2.f;
const float BUTTON_SPACING = 30.f; // 버튼과 다른 요소들 사이의 간격
const float TITLE_CHAR_SIZE = 48;
const float BUTTON_CHAR_SIZE = 24;
const float LEFT_MARGIN = 100.f; // 화면 왼쪽 여백


StartScreen::StartScreen(sf::RenderWindow& window, sf::Font& font)
    : m_window(window), m_font(font),
      m_isStartButtonHovered(false), m_isExitButtonHovered(false),
      m_nextState(ScreenState::START), m_running(true),
      m_mouseMovedSinceLastUpdate(false),
      m_rng(std::random_device{}()), // 난수 생성기 시드 초기화
      m_distX(0.f, static_cast<float>(m_window.getSize().x)), // X 좌표 분포
      m_distY(0.f, static_cast<float>(m_window.getSize().y)), // Y 좌표 분포
      m_distAngle(0.f, 2.f * 3.1415926535f), // 각도 분포 (0 ~ 2PI 라디안)
      m_distSpeed(MAX_DUST_SPEED * 0.5f, MAX_DUST_SPEED) // 속도 분포
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
    // setupDustParticles(); // 필요에 따라 파티클 재설정 (주석 처리된 상태 유지)
    m_mousePosition = sf::Vector2f(m_window.mapPixelToCoords(sf::Mouse::getPosition(m_window)));
}

// setNextState 함수 구현 (Screen.hpp에 선언 추가됨)
void StartScreen::setNextState(ScreenState state) {
    m_nextState = state;
}

void StartScreen::setupTexts() {
    m_titleText.setFont(m_font);
    m_titleText.setString(L"실내 공기 오염 시뮬레이터");
    m_titleText.setCharacterSize(TITLE_CHAR_SIZE);
    m_titleText.setFillColor(TEXT_COLOR_NORMAL);
    sf::FloatRect titleBounds = m_titleText.getLocalBounds();
    // 텍스트의 원점을 왼쪽, 수직 중앙으로 설정
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
    m_startButtonShape.setFillColor(sf::Color::Transparent); // 기본 배경 투명
    // 버튼 위치 설정 (제목 아래)
    m_startButtonShape.setPosition(LEFT_MARGIN, m_titleText.getPosition().y + m_titleText.getGlobalBounds().height + BUTTON_SPACING * 1.5f);

    // 버튼 텍스트 중앙 정렬
    sf::FloatRect startTextBounds = m_startButtonText.getLocalBounds();
    m_startButtonText.setOrigin(startTextBounds.left + startTextBounds.width / 2.f, startTextBounds.top + startTextBounds.height / 2.f);
    m_startButtonText.setPosition(m_startButtonShape.getPosition().x + BUTTON_WIDTH / 2.f, m_startButtonShape.getPosition().y + BUTTON_HEIGHT / 2.f);

    m_exitButtonShape.setSize(sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT));
    m_exitButtonShape.setOutlineColor(BUTTON_OUTLINE_COLOR_NORMAL);
    m_exitButtonShape.setOutlineThickness(BUTTON_OUTLINE_THICKNESS);
    m_exitButtonShape.setFillColor(sf::Color::Transparent);
    // 종료 버튼 위치 설정 (실행 버튼 아래)
    m_exitButtonShape.setPosition(LEFT_MARGIN, m_startButtonShape.getPosition().y + BUTTON_HEIGHT + BUTTON_SPACING);

    sf::FloatRect exitTextBounds = m_exitButtonText.getLocalBounds();
    m_exitButtonText.setOrigin(exitTextBounds.left + exitTextBounds.width / 2.f, exitTextBounds.top + exitTextBounds.height / 2.f);
    m_exitButtonText.setPosition(m_exitButtonShape.getPosition().x + BUTTON_WIDTH / 2.f, m_exitButtonShape.getPosition().y + BUTTON_HEIGHT / 2.f);
}

void StartScreen::setupDustParticles() {
    m_dustParticles.clear();
    m_dustParticles.reserve(NUM_DUST_PARTICLES); // 메모리 미리 할당

    for (int i = 0; i < NUM_DUST_PARTICLES; ++i) {
        DustParticle p;
        p.shape.setRadius(DUST_PARTICLE_RADIUS);
        p.shape.setFillColor(DUST_COLOR);
        p.shape.setOrigin(DUST_PARTICLE_RADIUS, DUST_PARTICLE_RADIUS); // 중심점 설정
        p.shape.setPosition(m_distX(m_rng), m_distY(m_rng)); // 랜덤 위치

        float angle = m_distAngle(m_rng); // 랜덤 각도
        float speed = m_distSpeed(m_rng); // 랜덤 속도
        p.velocity = sf::Vector2f(std::cos(angle) * speed, std::sin(angle) * speed); // 속도 벡터 설정

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
        p.shape.move(p.velocity * deltaTime); // 속도에 따라 이동

        sf::Vector2f pos = p.shape.getPosition();
        bool bounced = false; // 벽에 부딪혔는지 여부
        // X축 경계 처리
        if (pos.x < 0 || pos.x > windowSize.x) {
            p.velocity.x *= -1; // X축 속도 반전
            pos.x = std::max(0.f, std::min(pos.x, static_cast<float>(windowSize.x))); // 화면 안으로 위치 조정
            bounced = true;
        }
        // Y축 경계 처리
        if (pos.y < 0 || pos.y > windowSize.y) {
            p.velocity.y *= -1; // Y축 속도 반전
            pos.y = std::max(0.f, std::min(pos.y, static_cast<float>(windowSize.y))); // 화면 안으로 위치 조정
            bounced = true;
        }
        if(bounced) { // 부딪혔다면 조정된 위치로 설정
            p.shape.setPosition(pos);
        }

        // 마우스 반발 효과
        if (m_mouseMovedSinceLastUpdate) { // 마우스가 움직였을 때만 계산
            sf::Vector2f dirFromMouseToParticle = p.shape.getPosition() - m_mousePosition;
            float distanceToMouseSquared = dirFromMouseToParticle.x * dirFromMouseToParticle.x + dirFromMouseToParticle.y * dirFromMouseToParticle.y;

            // 마우스 반발 반경 내에 있고, 거리가 0이 아니면
            if (distanceToMouseSquared < MOUSE_REPEL_RADIUS * MOUSE_REPEL_RADIUS && distanceToMouseSquared > 0.0001f) {
                float distanceToMouse = std::sqrt(distanceToMouseSquared);
                sf::Vector2f repelDirection = dirFromMouseToParticle / distanceToMouse; // 반발 방향 (정규화)
                // 거리에 반비례하는 힘 (가까울수록 강하게)
                float strengthFactor = (MOUSE_REPEL_RADIUS - distanceToMouse) / MOUSE_REPEL_RADIUS;
                p.velocity = repelDirection * MOUSE_REPEL_STRENGTH * strengthFactor;

                // 반발 시 최대 속도 제한
                float currentSpeed = std::sqrt(p.velocity.x * p.velocity.x + p.velocity.y * p.velocity.y);
                float maxRepelSpeed = MAX_DUST_SPEED * 3.0f; // 반발 시 최대 속도는 일반 최대 속도보다 크게
                if (currentSpeed > maxRepelSpeed) {
                    p.velocity = (p.velocity / currentSpeed) * maxRepelSpeed; // 속도 정규화 후 최대 속도 적용
                }
            }
        } else { // 마우스가 움직이지 않으면 일반 속도 조절
            float currentSpeed = std::sqrt(p.velocity.x * p.velocity.x + p.velocity.y * p.velocity.y);
            float targetMinSpeed = MAX_DUST_SPEED * 0.5f;
            float targetMaxSpeed = MAX_DUST_SPEED;

            // 너무 빠르면 감속
            if (currentSpeed > targetMaxSpeed) {
                p.velocity *= (1.f - 2.0f * deltaTime); // 점진적 감속
                // 감속 후에도 여전히 빠르면 최대 속도로 설정
                if (std::sqrt(p.velocity.x * p.velocity.x + p.velocity.y * p.velocity.y) < targetMaxSpeed) {
                     float angle = std::atan2(p.velocity.y, p.velocity.x);
                     if (currentSpeed > 0.001f) { // 속도가 0이 아닐 때만 각도 기반 속도 재설정
                        p.velocity.x = std::cos(angle) * targetMaxSpeed;
                        p.velocity.y = std::sin(angle) * targetMaxSpeed;
                     }
                }
            } else if (currentSpeed < targetMinSpeed && currentSpeed > 0.01f) { // 너무 느리면 가속 (0에 가까우면 제외)
                 p.velocity *= (1.f + 1.0f * deltaTime); // 점진적 가속
                 // 가속 후에도 여전히 느리면 최소 속도로 설정
                 if (std::sqrt(p.velocity.x * p.velocity.x + p.velocity.y * p.velocity.y) > targetMinSpeed) {
                     float angle = std::atan2(p.velocity.y, p.velocity.x);
                     if (currentSpeed > 0.001f) {
                        p.velocity.x = std::cos(angle) * targetMinSpeed;
                        p.velocity.y = std::sin(angle) * targetMinSpeed;
                     }
                 }
            } else if (currentSpeed <= 0.01f) { // 거의 멈췄으면 새로운 랜덤 속도 부여
                float angle = m_distAngle(m_rng);
                float speed = m_distSpeed(m_rng);
                p.velocity = sf::Vector2f(std::cos(angle) * speed, std::sin(angle) * speed);
            }
        }
    }
}

void StartScreen::handleInput() {
    m_mouseMovedSinceLastUpdate = false; // 매 프레임 시작 시 마우스 움직임 플래그 초기화

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
            // 뷰 좌표계로 변환된 마우스 위치 저장
            m_mousePosition = m_window.mapPixelToCoords(sf::Vector2i(event.mouseMove.x, event.mouseMove.y));
            m_mouseMovedSinceLastUpdate = true; // 마우스 움직임 감지

            // 버튼 호버 상태 업데이트
            sf::Vector2f mousePosView = m_window.mapPixelToCoords(sf::Mouse::getPosition(m_window));
            m_isStartButtonHovered = m_startButtonShape.getGlobalBounds().contains(mousePosView);
            m_isExitButtonHovered = m_exitButtonShape.getGlobalBounds().contains(mousePosView);
        }

        if (event.type == sf::Event::MouseButtonPressed) {
            if (event.mouseButton.button == sf::Mouse::Left) {
                sf::Vector2f mousePosView = m_window.mapPixelToCoords(sf::Mouse::getPosition(m_window));
                if (m_isStartButtonHovered) { // 실행 버튼 클릭
                    m_nextState = ScreenState::SETTING;
                    m_running = false; // StartScreen 종료
                }
                if (m_isExitButtonHovered) { // 종료 버튼 클릭
                    m_running = false;
                    m_nextState = ScreenState::EXIT;
                }
            }
        }
    }
}

void StartScreen::update(sf::Time dt) {
    updateButtonStyles();    // 버튼 스타일 (호버 효과) 업데이트
    updateDustParticles(dt); // 먼지 입자 움직임 업데이트
}

void StartScreen::render() {
    m_window.clear(BACKGROUND_COLOR); // 배경색으로 클리어

    // 먼지 입자 그리기
    for (const auto& p : m_dustParticles) {
        m_window.draw(p.shape);
    }

    // 텍스트 및 버튼 그리기
    m_window.draw(m_titleText);
    m_window.draw(m_startButtonShape);
    m_window.draw(m_startButtonText);
    m_window.draw(m_exitButtonShape);
    m_window.draw(m_exitButtonText);

    m_window.display(); // 최종 화면 표시
}

ScreenState StartScreen::getNextState() const {
    return m_nextState;
}

bool StartScreen::isRunning() const {
    return m_running;
}