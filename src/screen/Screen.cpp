#include "Screen.hpp"
#include <iostream>
#include <cmath>

// UI 및 시각 효과에 사용될 상수 정의
const float BUTTON_WIDTH = 120.f;                               // 버튼 너비
const float BUTTON_HEIGHT = 40.f;                               // 버튼 높이
const sf::Color BACKGROUND_COLOR = sf::Color::Black;            // 화면 배경색
const sf::Color TEXT_COLOR_NORMAL = sf::Color::White;           // 일반 텍스트 색상
const sf::Color TEXT_COLOR_HOVER = sf::Color::Black;            // 마우스 호버 시 텍스트 색상
const sf::Color BUTTON_OUTLINE_COLOR_NORMAL = sf::Color::White; // 버튼 일반 외곽선 색상
const sf::Color BUTTON_FILL_COLOR_HOVER = sf::Color::White;     // 버튼 마우스 호버 시 채우기 색상
const float BUTTON_OUTLINE_THICKNESS = 2.f;                     // 버튼 외곽선 두께
const float BUTTON_SPACING = 30.f;                              // UI 요소 간 간격
const float TITLE_CHAR_SIZE = 48;                               // 제목 텍스트 크기
const float BUTTON_CHAR_SIZE = 24;                              // 버튼 텍스트 크기
const float LEFT_MARGIN = 100.f;                                // 좌측 여백

// StartScreen 클래스 생성자
StartScreen::StartScreen(sf::RenderWindow& window, sf::Font& font)
    : m_window(window), m_font(font), // 멤버 변수 초기화 (창, 폰트 참조)
      m_isStartButtonHovered(false), m_isExitButtonHovered(false), // 버튼 호버 상태 초기화
      m_nextState(ScreenState::START), m_running(true), // 화면 상태 및 실행 여부 초기화
      m_mouseMovedSinceLastUpdate(false), // 마우스 움직임 플래그 초기화
      m_rng(std::random_device{}()), // 난수 생성기 초기화 (랜덤 시드 사용)
      // 먼지 입자 초기 위치 및 속도 설정을 위한 균등 분포 객체 초기화
      m_distX(0.f, static_cast<float>(m_window.getSize().x)), // X 좌표 랜덤 범위
      m_distY(0.f, static_cast<float>(m_window.getSize().y)), // Y 좌표 랜덤 범위
      m_distAngle(0.f, 2.f * 3.1415926535f), // 각도 랜덤 범위 (0 ~ 2PI)
      m_distSpeed(MAX_DUST_SPEED * 0.5f, MAX_DUST_SPEED) // 속도 랜덤 범위
{
    setupTexts();         // 텍스트 요소 초기 설정 함수 호출
    setupButtons();       // 버튼 요소 초기 설정 함수 호출
    setupDustParticles(); // 먼지 입자 초기 설정 함수 호출
    // 초기 마우스 위치 저장
    m_mousePosition = sf::Vector2f(m_window.mapPixelToCoords(sf::Mouse::getPosition(m_window)));
}

// StartScreen 클래스 소멸자
StartScreen::~StartScreen() {}

// StartScreen 상태 초기화 함수 (화면 재진입 시 호출)
void StartScreen::reset() {
    m_running = true; // 화면 실행 상태로 설정
    m_nextState = ScreenState::START; // 다음 화면 상태를 자기 자신으로 설정 (유지)
    m_isStartButtonHovered = false; // 버튼 호버 상태 초기화
    m_isExitButtonHovered = false;
    m_mouseMovedSinceLastUpdate = false; // 마우스 움직임 플래그 초기화
    // 현재 마우스 위치 다시 가져오기
    m_mousePosition = sf::Vector2f(m_window.mapPixelToCoords(sf::Mouse::getPosition(m_window)));
    // setupDustParticles(); // 필요 시 먼지 입자 재설정 (현재는 주석 처리)
}

// 다음 화면 상태 설정 함수
void StartScreen::setNextState(ScreenState state) {
    m_nextState = state; // 전달받은 상태로 다음 화면 상태 변경
}

// 화면에 표시될 텍스트 요소들 초기 설정
void StartScreen::setupTexts() {
    // 제목 텍스트 설정
    m_titleText.setFont(m_font);
    m_titleText.setString(L"실내 공기 오염 시뮬레이터"); // 유니코드 문자열 사용
    m_titleText.setCharacterSize(TITLE_CHAR_SIZE);
    m_titleText.setFillColor(TEXT_COLOR_NORMAL);
    sf::FloatRect titleBounds = m_titleText.getLocalBounds(); // 텍스트 영역 계산
    // 텍스트 원점을 왼쪽, 수직 중앙으로 설정하여 위치 조정 용이하게 함
    m_titleText.setOrigin(titleBounds.left, titleBounds.top + titleBounds.height / 2.f);
    m_titleText.setPosition(LEFT_MARGIN, static_cast<float>(m_window.getSize().y) / 2.f - BUTTON_HEIGHT * 2 - BUTTON_SPACING * 2);

    // "실행" 버튼 텍스트 설정
    m_startButtonText.setFont(m_font);
    m_startButtonText.setString(L"실행");
    m_startButtonText.setCharacterSize(BUTTON_CHAR_SIZE);
    m_startButtonText.setFillColor(TEXT_COLOR_NORMAL);

    // "종료" 버튼 텍스트 설정
    m_exitButtonText.setFont(m_font);
    m_exitButtonText.setString(L"종료");
    m_exitButtonText.setCharacterSize(BUTTON_CHAR_SIZE);
    m_exitButtonText.setFillColor(TEXT_COLOR_NORMAL);
}

// 화면에 표시될 버튼 요소들 초기 설정
void StartScreen::setupButtons() {
    // "실행" 버튼 모양 설정
    m_startButtonShape.setSize(sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT));
    m_startButtonShape.setOutlineColor(BUTTON_OUTLINE_COLOR_NORMAL);
    m_startButtonShape.setOutlineThickness(BUTTON_OUTLINE_THICKNESS);
    m_startButtonShape.setFillColor(sf::Color::Transparent); // 기본 배경은 투명
    m_startButtonShape.setPosition(LEFT_MARGIN, m_titleText.getPosition().y + m_titleText.getGlobalBounds().height + BUTTON_SPACING * 1.5f);

    // "실행" 버튼 텍스트 위치를 버튼 중앙으로 설정
    sf::FloatRect startTextBounds = m_startButtonText.getLocalBounds();
    m_startButtonText.setOrigin(startTextBounds.left + startTextBounds.width / 2.f, startTextBounds.top + startTextBounds.height / 2.f);
    m_startButtonText.setPosition(m_startButtonShape.getPosition().x + BUTTON_WIDTH / 2.f, m_startButtonShape.getPosition().y + BUTTON_HEIGHT / 2.f);

    // "종료" 버튼 모양 설정
    m_exitButtonShape.setSize(sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT));
    m_exitButtonShape.setOutlineColor(BUTTON_OUTLINE_COLOR_NORMAL);
    m_exitButtonShape.setOutlineThickness(BUTTON_OUTLINE_THICKNESS);
    m_exitButtonShape.setFillColor(sf::Color::Transparent);
    m_exitButtonShape.setPosition(LEFT_MARGIN, m_startButtonShape.getPosition().y + BUTTON_HEIGHT + BUTTON_SPACING);

    // "종료" 버튼 텍스트 위치를 버튼 중앙으로 설정
    sf::FloatRect exitTextBounds = m_exitButtonText.getLocalBounds();
    m_exitButtonText.setOrigin(exitTextBounds.left + exitTextBounds.width / 2.f, exitTextBounds.top + exitTextBounds.height / 2.f);
    m_exitButtonText.setPosition(m_exitButtonShape.getPosition().x + BUTTON_WIDTH / 2.f, m_exitButtonShape.getPosition().y + BUTTON_HEIGHT / 2.f);
}

// 배경에 떠다니는 먼지 입자들 초기 설정
void StartScreen::setupDustParticles() {
    m_dustParticles.clear(); // 기존 입자 제거
    m_dustParticles.reserve(NUM_DUST_PARTICLES); // 필요한 메모리 미리 할당

    // 정의된 개수만큼 먼지 입자 생성
    for (int i = 0; i < NUM_DUST_PARTICLES; ++i) {
        DustParticle p; // 새 입자 생성
        p.shape.setRadius(DUST_PARTICLE_RADIUS); // 반지름 설정
        p.shape.setFillColor(DUST_COLOR);      // 색상 설정
        p.shape.setOrigin(DUST_PARTICLE_RADIUS, DUST_PARTICLE_RADIUS); // 원점을 중심으로 설정
        p.shape.setPosition(m_distX(m_rng), m_distY(m_rng)); // 화면 내 랜덤 위치에 배치

        float angle = m_distAngle(m_rng); // 랜덤 이동 각도
        float speed = m_distSpeed(m_rng); // 랜덤 이동 속도
        p.velocity = sf::Vector2f(std::cos(angle) * speed, std::sin(angle) * speed); // 속도 벡터 계산

        m_dustParticles.push_back(p); // 생성된 입자를 벡터에 추가
    }
}

// 마우스 위치에 따른 버튼 스타일 업데이트 (호버 효과)
void StartScreen::updateButtonStyles() {
    // "실행" 버튼 호버 효과
    if (m_isStartButtonHovered) {
        m_startButtonText.setFillColor(TEXT_COLOR_HOVER);    // 텍스트 색 변경
        m_startButtonShape.setFillColor(BUTTON_FILL_COLOR_HOVER); // 배경색 변경
    } else {
        m_startButtonText.setFillColor(TEXT_COLOR_NORMAL);   // 기본 텍스트 색
        m_startButtonShape.setFillColor(sf::Color::Transparent); // 기본 배경 투명
    }

    // "종료" 버튼 호버 효과
    if (m_isExitButtonHovered) {
        m_exitButtonText.setFillColor(TEXT_COLOR_HOVER);
        m_exitButtonShape.setFillColor(BUTTON_FILL_COLOR_HOVER);
    } else {
        m_exitButtonText.setFillColor(TEXT_COLOR_NORMAL);
        m_exitButtonShape.setFillColor(sf::Color::Transparent);
    }
}

// 먼지 입자들의 위치 및 속도 업데이트
void StartScreen::updateDustParticles(sf::Time dt) {
    float deltaTime = dt.asSeconds(); // 경과 시간 (초 단위)
    sf::Vector2u windowSize = m_window.getSize(); // 현재 창 크기

    // 모든 먼지 입자에 대해 반복
    for (auto& p : m_dustParticles) {
        p.shape.move(p.velocity * deltaTime); // 현재 속도와 경과 시간만큼 이동

        // 화면 경계 처리 (벽에 부딪히면 튕기도록)
        sf::Vector2f pos = p.shape.getPosition(); // 현재 입자 위치
        bool bounced = false; // 부딪힘 여부 플래그
        // X축 경계 확인
        if (pos.x < 0 || pos.x > windowSize.x) {
            p.velocity.x *= -1; // X축 속도 반전
            // 입자 위치를 화면 안으로 강제 조정 (경계를 약간 벗어나는 것 방지)
            pos.x = std::max(0.f, std::min(pos.x, static_cast<float>(windowSize.x)));
            bounced = true;
        }
        // Y축 경계 확인
        if (pos.y < 0 || pos.y > windowSize.y) {
            p.velocity.y *= -1; // Y축 속도 반전
            pos.y = std::max(0.f, std::min(pos.y, static_cast<float>(windowSize.y)));
            bounced = true;
        }
        if (bounced) { // 만약 부딪혔다면, 조정된 위치로 입자 위치 설정
            p.shape.setPosition(pos);
        }

        // 마우스 반발 효과 (마우스가 움직였을 때만 활성화)
        if (m_mouseMovedSinceLastUpdate) {
            sf::Vector2f dirFromMouseToParticle = p.shape.getPosition() - m_mousePosition; // 마우스에서 입자로의 방향 벡터
            float distanceToMouseSquared = dirFromMouseToParticle.x * dirFromMouseToParticle.x + dirFromMouseToParticle.y * dirFromMouseToParticle.y; // 마우스까지의 거리 제곱

            // 마우스 반발 반경 내에 있고 거리가 0이 아니면 (입자가 마우스 위치와 정확히 겹치지 않으면)
            if (distanceToMouseSquared < MOUSE_REPEL_RADIUS * MOUSE_REPEL_RADIUS && distanceToMouseSquared > 0.0001f) {
                float distanceToMouse = std::sqrt(distanceToMouseSquared); // 실제 거리
                sf::Vector2f repelDirection = dirFromMouseToParticle / distanceToMouse; // 반발 방향 (단위 벡터)
                // 마우스에 가까울수록 반발력이 강해짐
                float strengthFactor = (MOUSE_REPEL_RADIUS - distanceToMouse) / MOUSE_REPEL_RADIUS;
                p.velocity = repelDirection * MOUSE_REPEL_STRENGTH * strengthFactor; // 새로운 속도 설정

                // 반발 시 속도 제한
                float currentSpeed = std::sqrt(p.velocity.x * p.velocity.x + p.velocity.y * p.velocity.y);
                float maxRepelSpeed = MAX_DUST_SPEED * 3.0f; // 반발 시 최대 속도
                if (currentSpeed > maxRepelSpeed) { // 최대 속도 초과 시
                    p.velocity = (p.velocity / currentSpeed) * maxRepelSpeed; // 속도를 정규화하고 최대 속도 적용
                }
            }
        } else { // 마우스가 움직이지 않으면 일반적인 속도 조절 로직 적용
            float currentSpeed = std::sqrt(p.velocity.x * p.velocity.x + p.velocity.y * p.velocity.y);
            float targetMinSpeed = MAX_DUST_SPEED * 0.5f; // 목표 최소 속도
            float targetMaxSpeed = MAX_DUST_SPEED;       // 목표 최대 속도

            // 현재 속도가 목표 최대 속도보다 빠르면 점진적 감속
            if (currentSpeed > targetMaxSpeed) {
                p.velocity *= (1.f - 2.0f * deltaTime); // 감속 (dt에 비례)
                // 감속 후 목표 최대 속도보다 느려졌으면, 방향 유지하며 최대 속도로 설정
                if (std::sqrt(p.velocity.x * p.velocity.x + p.velocity.y * p.velocity.y) < targetMaxSpeed) {
                     float angle = std::atan2(p.velocity.y, p.velocity.x); // 현재 이동 각도
                     if (currentSpeed > 0.001f) { // 속도가 0에 가깝지 않을 때만
                        p.velocity.x = std::cos(angle) * targetMaxSpeed;
                        p.velocity.y = std::sin(angle) * targetMaxSpeed;
                     }
                }
            } else if (currentSpeed < targetMinSpeed && currentSpeed > 0.01f) { // 현재 속도가 목표 최소 속도보다 느리고, 0에 가깝지 않으면 점진적 가속
                 p.velocity *= (1.f + 1.0f * deltaTime); // 가속 (dt에 비례)
                 // 가속 후 목표 최소 속도보다 빨라졌으면, 방향 유지하며 최소 속도로 설정
                 if (std::sqrt(p.velocity.x * p.velocity.x + p.velocity.y * p.velocity.y) > targetMinSpeed) {
                     float angle = std::atan2(p.velocity.y, p.velocity.x);
                     if (currentSpeed > 0.001f) {
                        p.velocity.x = std::cos(angle) * targetMinSpeed;
                        p.velocity.y = std::sin(angle) * targetMinSpeed;
                     }
                 }
            } else if (currentSpeed <= 0.01f) { // 입자가 거의 멈췄으면 새로운 랜덤 속도 부여
                float angle = m_distAngle(m_rng);
                float speed = m_distSpeed(m_rng);
                p.velocity = sf::Vector2f(std::cos(angle) * speed, std::sin(angle) * speed);
            }
        }
    }
}

// 사용자 입력 처리 함수
void StartScreen::handleInput() {
    m_mouseMovedSinceLastUpdate = false; // 매번 호출 시 마우스 움직임 플래그 초기화

    sf::Event event; // 이벤트 객체
    // 창에서 발생한 모든 이벤트 처리
    while (m_window.pollEvent(event)) {
        // 창 닫기 버튼 클릭 시
        if (event.type == sf::Event::Closed) {
            m_running = false; // 화면 실행 중단
            m_nextState = ScreenState::EXIT; // 다음 상태를 종료로 설정
        }
        // ESC 키 누를 시
        if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
            m_running = false;
            m_nextState = ScreenState::EXIT;
        }

        // 마우스 움직임 이벤트 처리
        if (event.type == sf::Event::MouseMoved) {
            // 현재 마우스 위치 (뷰 좌표계 기준) 업데이트
            m_mousePosition = m_window.mapPixelToCoords(sf::Vector2i(event.mouseMove.x, event.mouseMove.y));
            m_mouseMovedSinceLastUpdate = true; // 마우스 움직임 발생 플래그 설정

            // 버튼 위에 마우스가 있는지 확인하여 호버 상태 업데이트
            sf::Vector2f mousePosView = m_window.mapPixelToCoords(sf::Mouse::getPosition(m_window));
            m_isStartButtonHovered = m_startButtonShape.getGlobalBounds().contains(mousePosView);
            m_isExitButtonHovered = m_exitButtonShape.getGlobalBounds().contains(mousePosView);
        }

        // 마우스 버튼 클릭 이벤트 처리
        if (event.type == sf::Event::MouseButtonPressed) {
            if (event.mouseButton.button == sf::Mouse::Left) { // 왼쪽 버튼 클릭 시
                sf::Vector2f mousePosView = m_window.mapPixelToCoords(sf::Mouse::getPosition(m_window));
                // "실행" 버튼이 호버된 상태에서 클릭되었다면
                if (m_isStartButtonHovered) {
                    m_nextState = ScreenState::SETTING; // 다음 상태를 설정 화면으로
                    m_running = false; // 현재 화면(StartScreen) 실행 중단
                }
                // "종료" 버튼이 호버된 상태에서 클릭되었다면
                if (m_isExitButtonHovered) {
                    m_running = false;
                    m_nextState = ScreenState::EXIT;
                }
            }
        }
    }
}

// 화면 상태 업데이트 함수 (매 프레임 호출)
void StartScreen::update(sf::Time dt) {
    updateButtonStyles();    // 버튼 스타일 업데이트 (호버 효과 등)
    updateDustParticles(dt); // 먼지 입자 상태 업데이트
}

// 화면 렌더링 함수 (매 프레임 호출)
void StartScreen::render() {
    m_window.clear(BACKGROUND_COLOR); // 지정된 배경색으로 화면 지우기

    // 모든 먼지 입자 그리기
    for (const auto& p : m_dustParticles) {
        m_window.draw(p.shape);
    }

    // 텍스트 및 버튼 그리기
    m_window.draw(m_titleText);
    m_window.draw(m_startButtonShape);
    m_window.draw(m_startButtonText);
    m_window.draw(m_exitButtonShape);
    m_window.draw(m_exitButtonText);

    m_window.display(); // 그려진 내용 실제 화면에 표시
}

// 다음 화면 상태 반환 함수
ScreenState StartScreen::getNextState() const {
    return m_nextState;
}

// 현재 화면 실행 여부 반환 함수
bool StartScreen::isRunning() const {
    return m_running;
}