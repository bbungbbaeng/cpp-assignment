#include "Setting.hpp"
#include <cmath>
#include <iostream>
#include <algorithm>
#include <fstream>

// PI 상수 정의 (원주율)
const float PI = 3.1415926535f;

// SettingScreen 클래스의 static const 멤버 변수 정의
// 통로의 상대적 높이 비율 (방 높이 기준)
const float SettingScreen::PASSAGE_RELATIVE_HEIGHT_FACTOR = 0.7f;
// 통로의 상대적 너비 비율 (방 너비 기준)
const float SettingScreen::PASSAGE_RELATIVE_WIDTH_FACTOR = 0.25f;
// 창문의 상대적 높이 비율 (방 높이 기준)
const float SettingScreen::WINDOW_RELATIVE_HEIGHT_FACTOR = 0.5f;
// 창문의 상대적 너비 비율 (방 깊이/너비 기준)
const float SettingScreen::WINDOW_RELATIVE_WIDTH_FACTOR = 0.4f;

// InputBox 클래스 생성자: 멤버 변수 초기화
InputBox::InputBox() : m_isActive(false), m_showCursor(false), m_font(nullptr) {}

// InputBox 초기화 함수: 폰트, 위치, 크기, 플레이스홀더 텍스트 설정
void InputBox::setup(const sf::Font& font, sf::Vector2f position, sf::Vector2f size, const std::wstring& placeholder) {
    m_font = &font; // 폰트 포인터 설정
    // 입력 상자 모양 설정
    m_shape.setPosition(std::round(position.x), std::round(position.y)); // 정수 좌표로 반올림
    m_shape.setSize(size);
    m_shape.setFillColor(sf::Color(50, 50, 50)); // 배경색
    m_shape.setOutlineThickness(1.f);             // 외곽선 두께
    m_shape.setOutlineColor(sf::Color(100, 100, 100)); // 외곽선 색상

    // 입력 텍스트 기본 설정
    m_text.setFont(font);
    m_text.setCharacterSize(static_cast<unsigned int>(size.y * 0.6f)); // 문자 크기 (상자 높이에 비례)
    m_text.setFillColor(sf::Color::White); // 텍스트 색상
    // 텍스트 원점 및 위치 설정 (수직 중앙 정렬)
    sf::FloatRect textBounds = m_text.getLocalBounds();
    m_text.setOrigin(std::round(textBounds.left), std::round(textBounds.top + textBounds.height / 2.0f));
    m_text.setPosition(std::round(position.x + 5.f), std::round(position.y + size.y / 2.f)); // 약간의 왼쪽 여백

    // 플레이스홀더 텍스트 설정 (입력 텍스트 속성 기반)
    m_placeholderText = m_text;
    m_placeholderText.setString(placeholder);
    m_placeholderText.setFillColor(sf::Color(150,150,150)); // 플레이스홀더 색상
    sf::FloatRect placeholderBounds = m_placeholderText.getLocalBounds();
    m_placeholderText.setOrigin(std::round(placeholderBounds.left), std::round(placeholderBounds.top + placeholderBounds.height / 2.0f));
    m_placeholderText.setPosition(std::round(position.x + 5.f), std::round(position.y + size.y / 2.f));
}

// InputBox 이벤트 처리 함수 (키 입력 등)
void InputBox::handleEvent(sf::Event event) {
    if (!m_isActive) return; // 활성화 상태가 아니면 아무것도 안 함

    // 텍스트 입력 이벤트 처리
    if (event.type == sf::Event::TextEntered) {
        if (event.text.unicode == '\b') { // 백스페이스 처리
            if (!m_inputString.empty()) {
                m_inputString.pop_back(); // 문자열 끝 문자 제거
            }
        } else if (event.text.unicode < 128 && event.text.unicode != '\r' && event.text.unicode != '\n') { // 일반 ASCII 문자 (엔터 제외)
            char enteredChar = static_cast<char>(event.text.unicode);
            // 숫자, 소수점(하나만 허용), 마이너스 부호(맨 앞에만 허용) 입력 가능
            if (std::isdigit(enteredChar) ||
                (enteredChar == '.' && m_inputString.find('.') == std::string::npos) ||
                (enteredChar == '-' && m_inputString.empty())) {
                if (m_inputString.length() < 10) { // 최대 입력 길이 제한
                    m_inputString += enteredChar; // 입력 문자 추가
                }
            }
        }
        m_text.setString(m_inputString); // SFML 텍스트 객체 업데이트
        // 텍스트 변경에 따른 원점 및 위치 재조정
        sf::FloatRect textBounds = m_text.getLocalBounds();
        m_text.setOrigin(std::round(textBounds.left), std::round(textBounds.top + textBounds.height / 2.0f));
        m_text.setPosition(std::round(m_shape.getPosition().x + 5.f), std::round(m_shape.getPosition().y + m_shape.getSize().y / 2.f));
    }
}

// InputBox 상태 업데이트 함수 (커서 깜빡임 등)
void InputBox::update() {
    if (m_isActive) { // 활성화 상태일 때만
        // 일정 시간마다 커서 보이기/숨기기 토글
        if (m_cursorClock.getElapsedTime().asSeconds() > 0.5f) {
            m_showCursor = !m_showCursor;
            m_cursorClock.restart(); // 타이머 재시작
        }
    } else {
        m_showCursor = false; // 비활성 시 커서 숨김
    }
}

// InputBox 렌더링 함수
void InputBox::render(sf::RenderWindow& window) {
    window.draw(m_shape); // 입력 상자 배경 그리기
    // 입력 문자열이 비어있고 비활성 상태면 플레이스홀더 텍스트 표시
    if (m_inputString.empty() && !m_isActive) {
        window.draw(m_placeholderText);
    } else {
        // 현재 입력된 텍스트 (커서 포함 가능)
        std::string currentTextStr = m_inputString;
        if (m_isActive && m_showCursor) { // 활성 상태이고 커서 보일 시간이면
            currentTextStr += "|"; // 커서 문자 추가
        }
        // 임시 텍스트 객체를 사용하여 커서가 포함된 텍스트를 그림 (원래 m_text는 변경 안 함)
        sf::Text tempText = m_text;
        tempText.setString(currentTextStr);
        // 커서 포함 시 텍스트 폭이 변하므로 원점 및 위치 다시 설정
        sf::FloatRect tempBounds = tempText.getLocalBounds();
        tempText.setOrigin(std::round(tempBounds.left), std::round(tempBounds.top + tempBounds.height / 2.0f));
        tempText.setPosition(std::round(m_shape.getPosition().x + 5.f), std::round(m_shape.getPosition().y + m_shape.getSize().y / 2.f));
        window.draw(tempText); // 최종 텍스트 그리기
    }
}

// InputBox 활성화 상태 설정 함수
void InputBox::setActive(bool active) {
    m_isActive = active;
    if (m_isActive) { // 활성화 시
        m_shape.setOutlineColor(sf::Color::White); // 외곽선 색 변경 (강조)
        m_cursorClock.restart(); // 커서 타이머 재시작
        m_showCursor = true;     // 커서 보이도록 설정
    } else { // 비활성화 시
        m_shape.setOutlineColor(sf::Color(100, 100, 100)); // 기본 외곽선 색
        m_showCursor = false; // 커서 숨김
    }
    // 비활성화 시 입력 문자열이 비어있으면 플레이스홀더 위치 재조정 (정렬 유지)
    if (m_inputString.empty() && !m_isActive) {
        sf::FloatRect placeholderBounds = m_placeholderText.getLocalBounds();
        m_placeholderText.setOrigin(std::round(placeholderBounds.left), std::round(placeholderBounds.top + placeholderBounds.height / 2.0f));
        m_placeholderText.setPosition(std::round(m_shape.getPosition().x + 5.f), std::round(m_shape.getPosition().y + m_shape.getSize().y / 2.f));
    }
}

// InputBox 활성화 상태 반환
bool InputBox::isActive() const { return m_isActive; }
// InputBox에 입력된 텍스트 반환
std::string InputBox::getText() const { return m_inputString; }
// InputBox의 텍스트 설정
void InputBox::setText(const std::string& text) {
    m_inputString = text;
    m_text.setString(m_inputString); // SFML 텍스트 객체 업데이트
    // 텍스트 변경에 따른 원점 및 위치 재조정
    sf::FloatRect textBounds = m_text.getLocalBounds();
    m_text.setOrigin(std::round(textBounds.left), std::round(textBounds.top + textBounds.height / 2.0f));
    m_text.setPosition(std::round(m_shape.getPosition().x + 5.f), std::round(m_shape.getPosition().y + m_shape.getSize().y / 2.f));
}

// InputBox의 텍스트를 float 값으로 변환하여 반환
float InputBox::getFloatValue() const {
    try {
        // 변환이 어려운 특정 문자열 예외 처리
        if (m_inputString == "-" || m_inputString == "." || m_inputString == "-.") return 0.0f;
        if (m_inputString.empty()) return 0.0f; // 빈 문자열은 0.0f로 처리
        return std::stof(m_inputString); // 문자열을 float으로 변환
    } catch (const std::invalid_argument& ia) { // 변환 불가 시
        std::cerr << "Invalid argument for stof: " << m_inputString << " (" << ia.what() << ")" << std::endl;
        return 0.0f; // 오류 발생 시 0.0f 반환
    } catch (const std::out_of_range& oor) { // 변환 결과가 float 범위 초과 시
        std::cerr << "Out of range for stof: " << m_inputString << " (" << oor.what() << ")" << std::endl;
        return 0.0f; // 오류 발생 시 0.0f 반환
    }
}
// InputBox의 전역 경계(위치 및 크기) 반환
sf::FloatRect InputBox::getGlobalBounds() const {
    return m_shape.getGlobalBounds();
}

// SettingScreen 클래스 생성자
SettingScreen::SettingScreen(sf::RenderWindow& window, sf::Font& font)
    : m_window(window), m_font(font), // 멤버 변수 초기화 (창, 폰트)
      m_nextState(ScreenState::SETTING), m_running(true), // 화면 상태 및 실행 여부 초기화
      m_roomWidth(5.f), m_roomDepth(5.f), m_roomHeight(3.f), // 방 기본 크기 초기화
      m_rotationX(25.f * PI / 180.f), // 3D 뷰 X축 초기 회전각 (라디안)
      m_rotationY(-35.f * PI / 180.f), // 3D 뷰 Y축 초기 회전각 (라디안)
      m_isDragging(false), m_selectedPollutantIndex(0), m_activeInputBox(nullptr) { // 기타 상태 변수 초기화

    // 버튼 스타일 색상 초기화
    m_buttonTextColorNormal = sf::Color::White;
    m_buttonTextColorHover = sf::Color::Black;
    m_buttonBgColorNormal = sf::Color(80,80,80);
    m_buttonBgColorHover = sf::Color::White;
    m_buttonOutlineColor = sf::Color::White;

    // 통로 및 창문 정의 벡터 메모리 예약 (최대 2개씩)
    m_passages_defs.reserve(2);
    m_windows_defs.reserve(2);

    // 3D 렌더링을 위한 뷰 설정
    m_3dView.setSize(static_cast<float>(m_window.getSize().x) * 0.6f, static_cast<float>(m_window.getSize().y)); // 뷰 크기 (창의 60% 너비)
    m_3dView.setCenter(m_3dView.getSize().x / 2.f, m_3dView.getSize().y / 2.f - 50.f); // 뷰 중심 (약간 위로 이동)
    m_3dView.setViewport(sf::FloatRect(0.f, 0.f, 0.6f, 1.f)); // 창의 왼쪽 60% 영역 사용

    // UI 렌더링을 위한 뷰 설정
    m_uiView.setSize(static_cast<float>(m_window.getSize().x) * 0.4f, static_cast<float>(m_window.getSize().y)); // 뷰 크기 (창의 40% 너비)
    m_uiView.setCenter(m_uiView.getSize().x / 2.f, m_uiView.getSize().y / 2.f); // 뷰 중심
    m_uiView.setViewport(sf::FloatRect(0.6f, 0.f, 0.4f, 1.f)); // 창의 오른쪽 40% 영역 사용

    setupUI();         // UI 요소 초기 설정 함수 호출
    setup3D();         // 3D 모델 기본 구조 초기 설정 함수 호출
    projectVertices(); // 3D 정점 초기 투영 계산 함수 호출
}

// SettingScreen 클래스 소멸자 (현재 특별한 작업 없음)
SettingScreen::~SettingScreen() {}

// SettingScreen 상태 초기화 함수 (화면 재진입 시 호출)
void SettingScreen::reset() {
    m_running = true; // 화면 실행 상태로 설정
    m_nextState = ScreenState::SETTING; // 다음 화면 상태를 자기 자신으로 (유지)

    // 활성화된 입력창이 있다면 비활성화
    if (m_activeInputBox) {
        m_activeInputBox->setActive(false);
        m_activeInputBox = nullptr;
    }
    // 필요에 따라 추가적인 초기화 로직 (예: 입력 필드 값 초기화 등)
}

// UI 요소들(텍스트, 입력창, 버튼 등) 초기 설정 함수
void SettingScreen::setupUI() {
    // UI 요소 배치 관련 변수 설정
    float currentY = 20.f; // 현재 UI 요소의 Y축 시작 위치
    float inputHeight = 28.f; // 입력창 높이
    float spacing = 35.f;     // UI 요소 간 기본 간격
    unsigned int charSize = 18; // 일반 텍스트 크기
    float titleCharSize = 28;   // 제목 텍스트 크기

    // UI 요소 너비 계산용 변수
    float labelWidthForCalc = 90.f;    // 라벨 너비
    float inputBoxWidthForCalc = 180.f; // 입력창 너비
    float gapBetweenLabelInput = 10.f; // 라벨과 입력창 사이 간격
    float maxUiElementWidth = labelWidthForCalc + gapBetweenLabelInput + inputBoxWidthForCalc; // UI 요소 최대 너비

    float rightPadding = 30.f; // UI 영역 오른쪽 여백
    // UI 요소의 X축 시작 위치 (UI 뷰의 오른쪽에 정렬되도록 계산)
    float uiX = m_uiView.getSize().x - maxUiElementWidth - rightPadding;

    // 화면 제목 텍스트 설정
    m_titleText.setFont(m_font);
    m_titleText.setString(L"시뮬레이션 설정");
    m_titleText.setCharacterSize(titleCharSize);
    m_titleText.setFillColor(sf::Color::White);
    sf::FloatRect titleBounds = m_titleText.getLocalBounds();
    m_titleText.setOrigin(std::round(titleBounds.left + titleBounds.width / 2.f), std::round(titleBounds.top)); // 중앙 정렬
    m_titleText.setPosition(std::round(uiX + maxUiElementWidth / 2.f), std::round(currentY));
    currentY += titleCharSize + spacing * 0.8f; // 다음 Y 위치 조정

    // 입력 필드(라벨 + InputBox) 설정 람다 함수
    auto setupInputField = [&](sf::Text& label, InputBox& inputBox, const std::wstring& labelText, const std::string& defaultVal, const std::wstring& placeholder) {
        // 라벨 설정
        label.setFont(m_font);
        label.setString(labelText);
        label.setCharacterSize(charSize);
        label.setFillColor(sf::Color::White);
        sf::FloatRect labelBounds = label.getLocalBounds();
        label.setOrigin(std::round(labelBounds.left), std::round(labelBounds.top + labelBounds.height / 2.f)); // 수직 중앙 정렬
        label.setPosition(std::round(uiX), std::round(currentY + inputHeight / 2.f));

        // InputBox 설정
        inputBox.setup(m_font, sf::Vector2f(uiX + labelWidthForCalc + gapBetweenLabelInput, currentY), sf::Vector2f(inputBoxWidthForCalc, inputHeight), placeholder);
        inputBox.setText(defaultVal); // 기본값 설정
        currentY += spacing; // 다음 Y 위치 조정
    };

    // 방 크기 입력 필드들 설정
    setupInputField(m_labelWidth, m_inputWidth, L"가로 (m):", "5.0", L"0.0 - 100.0");
    setupInputField(m_labelDepth, m_inputDepth, L"세로 (m):", "5.0", L"0.0 - 100.0");
    setupInputField(m_labelHeight, m_inputHeight, L"높이 (m):", "3.0", L"0.0 - 10.0");
    currentY += spacing * 0.5f; // 추가 간격

    // 오염 물질 선택 라벨 설정
    m_labelPollutant.setFont(m_font);
    m_labelPollutant.setString(L"오염 물질:");
    m_labelPollutant.setCharacterSize(charSize);
    m_labelPollutant.setFillColor(sf::Color::White);
    sf::FloatRect pollutantLabelBounds = m_labelPollutant.getLocalBounds();
    m_labelPollutant.setOrigin(std::round(pollutantLabelBounds.left), std::round(pollutantLabelBounds.top + pollutantLabelBounds.height / 2.f));
    m_labelPollutant.setPosition(std::round(uiX), std::round(currentY + (inputHeight * 0.9f) / 2.f));
    currentY += spacing * 0.8f; // 다음 Y 위치 조정

    // 오염 물질 선택 옵션들 설정
    std::vector<std::wstring> pollutants = {L"미세먼지 (PM10)", L"일산화탄소 (CO)", L"염소가스 (Cl₂)"};
    m_pollutantOptions.resize(pollutants.size());
    m_pollutantOptionShapes.resize(pollutants.size());
    float pollutantOptionItemWidth = maxUiElementWidth; // 옵션 너비

    for (size_t i = 0; i < pollutants.size(); ++i) {
        // 옵션 버튼 모양 설정
        m_pollutantOptionShapes[i].setSize(sf::Vector2f(pollutantOptionItemWidth, inputHeight * 0.9f));
        m_pollutantOptionShapes[i].setPosition(std::round(uiX), std::round(currentY));
        m_pollutantOptionShapes[i].setOutlineThickness(1.f);

        // 옵션 텍스트 설정
        m_pollutantOptions[i].setFont(m_font);
        m_pollutantOptions[i].setString(pollutants[i]);
        m_pollutantOptions[i].setCharacterSize(charSize - 2); // 약간 작은 크기

        sf::FloatRect textBounds = m_pollutantOptions[i].getLocalBounds();
        m_pollutantOptions[i].setOrigin(std::round(textBounds.left + textBounds.width / 2.f), std::round(textBounds.top + textBounds.height / 2.f)); // 중앙 정렬
        m_pollutantOptions[i].setPosition(std::round(uiX + pollutantOptionItemWidth / 2.f), std::round(currentY + (inputHeight * 0.9f) / 2.f));
        
        currentY += (inputHeight * 0.9f) + 5.f; // 다음 Y 위치 조정 (옵션 간 간격 포함)
    }
    currentY += spacing * 0.5f; // 추가 간격

    // 버튼(텍스트 + 모양) 설정 람다 함수
    auto setupButtonLambda = [&](sf::Text& text, sf::RectangleShape& shape, const std::wstring& str, float yPos, float btnWidth, float btnXOffset = 0.f) {
        // 버튼 모양 설정
        shape.setSize(sf::Vector2f(btnWidth, inputHeight));
        shape.setPosition(std::round(uiX + btnXOffset), std::round(yPos));
        shape.setOutlineThickness(1.f);

        // 버튼 텍스트 설정
        text.setFont(m_font);
        text.setString(str);
        text.setCharacterSize(charSize - 2);

        sf::FloatRect textBounds = text.getLocalBounds();
        text.setOrigin(std::round(textBounds.left + textBounds.width / 2.f), std::round(textBounds.top + textBounds.height / 2.f)); // 중앙 정렬
        text.setPosition(std::round(shape.getPosition().x + btnWidth / 2.f), std::round(yPos + inputHeight / 2.f));
    };
    
    // 버튼 너비 계산
    float singleButtonWidth = maxUiElementWidth; // 단일 버튼 너비
    float pairedButtonWidth = (maxUiElementWidth - 10.f) / 2.f; // 나란히 배치될 두 버튼의 너비 (사이 간격 10px 고려)

    // 통로/창문 생성 버튼 설정
    setupButtonLambda(m_buttonCreatePassage, m_shapeCreatePassage, L"통로 생성", currentY, pairedButtonWidth);
    setupButtonLambda(m_buttonCreateWindow, m_shapeCreateWindow, L"창문 생성", currentY, pairedButtonWidth, pairedButtonWidth + 10.f); // X 오프셋으로 옆에 배치
    currentY += spacing; // 다음 Y 위치 조정

    // 통로/창문 제거 버튼 설정
    float removeButtonsY = currentY; // 제거 버튼들의 Y 위치 저장
    setupButtonLambda(m_buttonRemovePassage, m_shapeRemovePassage, L"통로 제거", removeButtonsY, pairedButtonWidth);
    setupButtonLambda(m_buttonRemoveWindow, m_shapeRemoveWindow, L"창문 제거", removeButtonsY, pairedButtonWidth, pairedButtonWidth + 10.f);
    
    // 통로/창문 개수 표시 텍스트 설정
    float countTextOffsetY = inputHeight + 10.f; // 버튼 아래 Y 오프셋
    unsigned int countCharSize = charSize - 4;   // 개수 텍스트 크기

    // 통로 개수 텍스트
    m_textPassageCount.setFont(m_font);
    m_textPassageCount.setCharacterSize(countCharSize);
    m_textPassageCount.setFillColor(sf::Color::White);
    updatePassageCountText(); // 초기 텍스트 내용 설정

    sf::FloatRect passageCountBounds = m_textPassageCount.getLocalBounds();
    // "통로 제거" 버튼 중앙 하단에 위치하도록 설정
    m_textPassageCount.setPosition(
        std::round(m_shapeRemovePassage.getPosition().x + m_shapeRemovePassage.getSize().x / 2.f),
        std::round(removeButtonsY + countTextOffsetY + passageCountBounds.height / 2.f)
    );

    // 창문 개수 텍스트
    m_textWindowCount.setFont(m_font);
    m_textWindowCount.setCharacterSize(countCharSize);
    m_textWindowCount.setFillColor(sf::Color::White);
    updateWindowCountText(); // 초기 텍스트 내용 설정

    sf::FloatRect windowCountBounds = m_textWindowCount.getLocalBounds();
    // "창문 제거" 버튼 중앙 하단에 위치하도록 설정
    m_textWindowCount.setPosition(
        std::round(m_shapeRemoveWindow.getPosition().x + m_shapeRemoveWindow.getSize().x / 2.f),
        std::round(removeButtonsY + countTextOffsetY + windowCountBounds.height / 2.f)
    );

    currentY += spacing; // 다음 Y 위치 조정
    currentY += spacing * 0.5f; // 추가 간격

    // "시뮬레이션 시작" 버튼 설정 (화면 하단에 위치)
    float startButtonY = m_uiView.getSize().y - spacing - inputHeight;
    setupButtonLambda(m_buttonStartSimulation, m_shapeStartSimulation, L"시뮬레이션 시작", startButtonY, singleButtonWidth);
}

// 3D 육면체 모델의 기본 정점 및 모서리 정보 설정
void SettingScreen::setup3D() {
    // 육면체 8개 정점의 로컬 좌표 (-0.5 ~ 0.5 범위로 정규화)
    m_cubeVertices = {{
        {-0.5f, -0.5f, -0.5f}, {0.5f, -0.5f, -0.5f}, {0.5f, 0.5f, -0.5f}, {-0.5f, 0.5f, -0.5f}, // 앞면 (z=-0.5)
        {-0.5f, -0.5f, 0.5f},  {0.5f, -0.5f, 0.5f},  {0.5f, 0.5f, 0.5f},  {-0.5f, 0.5f, 0.5f}     // 뒷면 (z=0.5)
    }};
    // 육면체 12개 모서리 (각 모서리를 이루는 두 정점의 인덱스)
    m_cubeEdges = {
        {0,1}, {1,2}, {2,3}, {3,0}, // 앞면 모서리
        {4,5}, {5,6}, {6,7}, {7,4}, // 뒷면 모서리
        {0,4}, {1,5}, {2,6}, {3,7}  // 앞면과 뒷면을 잇는 모서리
    };
    m_transformedVertices.fill({}); // 변환된 정점 배열 초기화 (0으로 채움)
}

// 3D 정점들을 현재 방 크기, 회전각에 따라 변환하는 함수
void SettingScreen::projectVertices() {
    // 입력된 방 크기 가져오기 (최소값 0.1f 보장)
    float w = (m_roomWidth > 0.1f) ? m_roomWidth : 0.1f;
    float d = (m_roomDepth > 0.1f) ? m_roomDepth : 0.1f;
    float h = (m_roomHeight > 0.1f) ? m_roomHeight : 0.1f;

    // 3D 뷰에 맞게 스케일링하기 위한 최대 차원값 및 스케일 팩터 계산
    float maxDim = std::max({w, d, h, 1.f}); // 방 크기 중 가장 큰 값 (최소 1.0f)
    float scaleFactor = 350.f / maxDim;      // 뷰 크기에 맞추기 위한 비율 (350은 임의의 기준 크기)

    // 모든 정점에 대해 변환 적용
    for (size_t i = 0; i < m_cubeVertices.size(); ++i) {
        const Vec3D& v_orig = m_cubeVertices[i]; // 원본 정규화 좌표
        Vec3D v_scaled; // 방 크기 적용된 좌표
        v_scaled.x = v_orig.x * w;
        v_scaled.y = v_orig.y * h; // Y축은 SFML 화면 좌표계와 유사하게 아래로 갈수록 증가하는 것으로 간주
        v_scaled.z = v_orig.z * d;

        // Y축 기준 회전 (방향키 좌우 또는 마우스 좌우 드래그에 해당)
        float x_rot_y = v_scaled.x * std::cos(m_rotationY) - v_scaled.z * std::sin(m_rotationY);
        float z_rot_y = v_scaled.x * std::sin(m_rotationY) + v_scaled.z * std::cos(m_rotationY);

        // X축 기준 회전 (방향키 상하 또는 마우스 상하 드래그에 해당)
        float y_final = v_scaled.y * std::cos(m_rotationX) - z_rot_y * std::sin(m_rotationX);
        float z_final = v_scaled.y * std::sin(m_rotationX) + z_rot_y * std::cos(m_rotationX);

        // 최종 변환된 좌표 (스케일 팩터 적용)
        m_transformedVertices[i] = {
            x_rot_y * scaleFactor,
            y_final * scaleFactor,
            z_final * scaleFactor
        };
    }
}

// 3D 좌표를 2D 화면 좌표로 투영하는 함수 (간단한 원근 투영)
sf::Vector2f SettingScreen::project(const Vec3D& p) const {
    sf::Vector2f viewCenter = m_3dView.getCenter(); // 3D 뷰의 중심 좌표
    // 원근 효과 계수: z값이 클수록 (멀리 있을수록) 작게, z값이 -500에 가까워지면 무한대로 발산 (시야각 효과)
    float perspectiveFactor = 500.f / (500.f + p.z); 
    // 투영된 2D 화면 좌표 계산 및 반환
    return sf::Vector2f(
        std::round(p.x * perspectiveFactor + viewCenter.x), // X 좌표 투영 및 뷰 중심으로 이동
        std::round(p.y * perspectiveFactor + viewCenter.y)  // Y 좌표 투영 및 뷰 중심으로 이동
    );
}

// 활성화된 입력 상자의 이벤트를 처리하는 함수
void SettingScreen::handleInputBoxEvents(sf::Event event) {
    InputBox* currentActive = m_activeInputBox; // 현재 활성화된 입력 상자 포인터
    if (currentActive) { // 활성화된 입력 상자가 있다면
        currentActive->handleEvent(event); // 해당 입력 상자로 이벤트 전달
        // 입력 상자 내용 변경 시, 연결된 멤버 변수(방 크기) 업데이트 및 3D 모델 재투영
        if (m_activeInputBox == currentActive) { // 이벤트 처리 후에도 여전히 동일한 입력 상자가 활성 상태인지 확인
            if (currentActive == &m_inputWidth) m_roomWidth = m_inputWidth.getFloatValue();
            else if (currentActive == &m_inputDepth) m_roomDepth = m_inputDepth.getFloatValue();
            else if (currentActive == &m_inputHeight) m_roomHeight = m_inputHeight.getFloatValue();
            projectVertices(); // 방 크기 변경 시 3D 모델 즉시 업데이트
        }
    }
}

// 현재 설정값들을 파일에 저장하는 함수
void SettingScreen::saveSettingsToFile(const std::string& filename) const {
    std::ofstream outFile(filename); // 파일 출력 스트림 열기
    if (outFile.is_open()) { // 파일 열기 성공 시
        // "키:값" 형식으로 설정 정보 저장
        outFile << "width:" << m_roomWidth << std::endl;
        outFile << "depth:" << m_roomDepth << std::endl;
        outFile << "height:" << m_roomHeight << std::endl;
        outFile << "pollutant_index:" << m_selectedPollutantIndex << std::endl;
        outFile << "passages_count:" << m_passages_defs.size() << std::endl;
        outFile << "windows_count:" << m_windows_defs.size() << std::endl;
        outFile.close(); // 파일 닫기
        std::cout << "Settings saved to " << filename << std::endl; // 저장 완료 메시지 (디버깅용)
    } else { // 파일 열기 실패 시
        std::cerr << "Error: Could not open file to save settings: " << filename << std::endl;
    }
}

// 사용자 입력 처리 함수 (메인 루프에서 호출)
void SettingScreen::handleInput() {
    sf::Event event; // SFML 이벤트 객체
    // 창에서 발생한 모든 이벤트 폴링
    while (m_window.pollEvent(event)) {
        // 창 닫기 이벤트 처리
        if (event.type == sf::Event::Closed) {
            m_running = false; // 화면 실행 중단
            m_nextState = ScreenState::EXIT; // 다음 상태를 종료로 설정
        }
        // ESC 키 누름 이벤트 처리
        if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
            if (m_activeInputBox) { // 활성화된 입력창이 있으면
                m_activeInputBox->setActive(false); // 입력창 비활성화
                m_activeInputBox = nullptr;
            } else { // 활성화된 입력창이 없으면
                m_running = false; // 화면 실행 중단
                m_nextState = ScreenState::START; // 다음 상태를 시작 화면으로 설정
            }
        }

        // 활성화된 입력창의 텍스트 입력 이벤트 우선 처리
        if (m_activeInputBox && event.type == sf::Event::TextEntered) {
            handleInputBoxEvents(event);
            continue; // 이 이벤트는 처리했으므로 다음 이벤트 폴링으로 넘어감
        }
        // 활성화된 입력창의 키 누름 이벤트 처리 (백스페이스 등)
        if (m_activeInputBox && event.type == sf::Event::KeyPressed) {
            handleInputBoxEvents(event);
            // 여기서 continue를 하면 다른 키 입력(예: ESC)이 막힐 수 있으므로 주의
        }

        // 마우스 버튼 누름 이벤트 처리
        if (event.type == sf::Event::MouseButtonPressed) {
            if (event.mouseButton.button == sf::Mouse::Left) { // 왼쪽 마우스 버튼
                sf::Vector2f mousePosWindow = static_cast<sf::Vector2f>(sf::Mouse::getPosition(m_window)); // 창 기준 마우스 위치
                sf::Vector2f mousePosUI = m_window.mapPixelToCoords(sf::Mouse::getPosition(m_window), m_uiView); // UI 뷰 기준 마우스 위치

                InputBox* previouslyActive = m_activeInputBox; // 이전 활성 입력창
                InputBox* clickedBox = nullptr; // 새로 클릭된 입력창

                // 어떤 입력창이 클릭되었는지 확인
                if (m_inputWidth.getGlobalBounds().contains(mousePosUI)) clickedBox = &m_inputWidth;
                else if (m_inputDepth.getGlobalBounds().contains(mousePosUI)) clickedBox = &m_inputDepth;
                else if (m_inputHeight.getGlobalBounds().contains(mousePosUI)) clickedBox = &m_inputHeight;

                // 활성 입력창 변경 처리
                if (previouslyActive && previouslyActive != clickedBox) {
                    previouslyActive->setActive(false); // 이전 입력창 비활성화
                }
                m_activeInputBox = clickedBox; // 새 입력창을 활성 입력창으로
                if (m_activeInputBox) {
                    m_activeInputBox->setActive(true); // 활성화 시각 효과 적용
                }

                // 입력창이 아닌 곳을 클릭했다면 오염물질 선택 옵션 확인
                if (!m_activeInputBox) {
                    for(size_t i=0; i<m_pollutantOptionShapes.size(); ++i) {
                        if (m_pollutantOptionShapes[i].getGlobalBounds().contains(mousePosUI)) {
                            m_selectedPollutantIndex = static_cast<int>(i); // 선택된 오염물질 인덱스 업데이트
                            break; // 하나만 선택 가능
                        }
                    }
                }

                // 입력창이 활성화되지 않았을 때만 다른 버튼들 작동
                if (!m_activeInputBox) {
                    if (m_shapeCreatePassage.getGlobalBounds().contains(mousePosUI)) {
                        createPassage(); // 통로 생성
                    } else if (m_shapeRemovePassage.getGlobalBounds().contains(mousePosUI)) {
                        removePassage(); // 통로 제거
                    } else if (m_shapeCreateWindow.getGlobalBounds().contains(mousePosUI)) {
                        createWindow(); // 창문 생성
                    } else if (m_shapeRemoveWindow.getGlobalBounds().contains(mousePosUI)) {
                        removeWindow(); // 창문 제거
                    } else if (m_shapeStartSimulation.getGlobalBounds().contains(mousePosUI)) { // "시뮬레이션 시작" 버튼 클릭
                        saveSettingsToFile("Setting_values.text"); // 현재 설정값 파일에 저장
                        m_nextState = ScreenState::SIMULATION;     // 다음 화면 상태를 시뮬레이션으로
                        m_running = false;                         // 현재 설정 화면 종료
                    }
                }

                // 3D 뷰 영역 클릭 시 마우스 드래그 시작 (입력창이 활성화되지 않았을 때만)
                sf::FloatRect view3DViewportRect( // 3D 뷰의 화면상 실제 영역 계산
                    m_3dView.getViewport().left * m_window.getSize().x,
                    m_3dView.getViewport().top * m_window.getSize().y,
                    m_3dView.getViewport().width * m_window.getSize().x,
                    m_3dView.getViewport().height * m_window.getSize().y
                );
                if (view3DViewportRect.contains(mousePosWindow) && !m_activeInputBox) { // 3D 뷰 영역 내 클릭이고 입력창 비활성 시
                    m_isDragging = true; // 드래그 시작 플래그
                    m_lastMousePos = sf::Mouse::getPosition(m_window); // 현재 마우스 위치 저장 (드래그 기준점)
                }
            }
        }
        // 마우스 버튼 뗌 이벤트 처리
        if (event.type == sf::Event::MouseButtonReleased) {
            if (event.mouseButton.button == sf::Mouse::Left) {
                m_isDragging = false; // 드래그 종료
            }
        }
        // 마우스 이동 이벤트 처리 (3D 뷰 회전용)
        if (event.type == sf::Event::MouseMoved) {
            if (m_isDragging && !m_activeInputBox) { // 드래그 중이고 입력창 비활성 시
                sf::Vector2i currentMousePos = sf::Mouse::getPosition(m_window);
                float dx = static_cast<float>(currentMousePos.x - m_lastMousePos.x); // X축 이동량

                m_rotationY += dx * 0.005f; // Y축 회전각 업데이트 (회전 민감도 0.005)
                m_lastMousePos = currentMousePos; // 마지막 마우스 위치 갱신
                projectVertices(); // 회전된 3D 모델 재투영
            }
        }
    }
}

// 마우스 위치에 따른 버튼 호버 스타일 업데이트 함수
void SettingScreen::updateButtonHovers(const sf::Vector2f& mousePos) {
    // 버튼 시각 효과 업데이트 람다 함수
    auto updateVisuals = [&](sf::Text& text, sf::RectangleShape& shape, bool isHovered, bool isSelected = false) {
        // 호버 또는 선택 시 색상 변경
        text.setFillColor(isHovered || isSelected ? m_buttonTextColorHover : m_buttonTextColorNormal);
        shape.setFillColor(isHovered || isSelected ? m_buttonBgColorHover : m_buttonBgColorNormal);
        // 선택된 항목(주로 오염물질)은 외곽선 색상으로 구분
        if (isSelected) {
            shape.setOutlineColor(sf::Color::Yellow); 
        } else {
            shape.setOutlineColor(m_buttonOutlineColor);
        }
    };

    // 오염 물질 선택 옵션들의 호버 및 선택 상태 업데이트
    for(size_t i=0; i<m_pollutantOptions.size(); ++i) {
        bool isSelected = (static_cast<int>(i) == m_selectedPollutantIndex); // 현재 선택된 항목인지
        bool isHovered = m_pollutantOptionShapes[i].getGlobalBounds().contains(mousePos); // 마우스가 위에 있는지
        updateVisuals(m_pollutantOptions[i], m_pollutantOptionShapes[i], isHovered, isSelected);
    }

    // 일반 버튼들의 호버 상태 업데이트
    updateVisuals(m_buttonCreatePassage, m_shapeCreatePassage, m_shapeCreatePassage.getGlobalBounds().contains(mousePos));
    updateVisuals(m_buttonCreateWindow, m_shapeCreateWindow, m_shapeCreateWindow.getGlobalBounds().contains(mousePos));
    updateVisuals(m_buttonRemovePassage, m_shapeRemovePassage, m_shapeRemovePassage.getGlobalBounds().contains(mousePos));
    updateVisuals(m_buttonRemoveWindow, m_shapeRemoveWindow, m_shapeRemoveWindow.getGlobalBounds().contains(mousePos));
    updateVisuals(m_buttonStartSimulation, m_shapeStartSimulation, m_shapeStartSimulation.getGlobalBounds().contains(mousePos));
}

// 화면 상태 업데이트 함수 (매 프레임 호출)
void SettingScreen::update(sf::Time dt) {
    // 각 입력창의 상태 업데이트 (커서 깜빡임 등)
    m_inputWidth.update();
    m_inputDepth.update();
    m_inputHeight.update();

    // UI 뷰 기준 마우스 좌표로 버튼 호버 효과 업데이트
    sf::Vector2f mousePosUI = m_window.mapPixelToCoords(sf::Mouse::getPosition(m_window), m_uiView);
    updateButtonHovers(mousePosUI);
}

// 3D 육면체의 모서리를 그리는 함수
void SettingScreen::drawCuboidEdges(sf::RenderWindow& window) {
    // 모든 모서리에 대해 반복
    for(const auto& edge : m_cubeEdges) {
        // 모서리를 이루는 두 정점의 2D 화면 좌표 계산
        sf::Vector2f p1_screen = project(m_transformedVertices[edge.start]);
        sf::Vector2f p2_screen = project(m_transformedVertices[edge.end]);
        // 두 점을 잇는 선분 생성 및 그리기
        sf::Vertex line[] = {
            sf::Vertex(p1_screen, sf::Color::White),
            sf::Vertex(p2_screen, sf::Color::White)
        };
        window.draw(line, 2, sf::Lines); // sf::Lines 프리미티브 타입으로 그리기
    }
}

// 통로 생성 함수 (최대 2개)
void SettingScreen::createPassage() {
    if (m_passages_defs.size() < 2) { // 통로 개수 제한 확인
        OpeningDefinition new_passage; // 새 통로 정의 객체
        // 통로의 정규화된 높이/너비 절반 계산
        float p_h_norm = PASSAGE_RELATIVE_HEIGHT_FACTOR * 0.5f; 
        float p_w_norm = PASSAGE_RELATIVE_WIDTH_FACTOR * 0.5f;  

        // 첫 번째 통로는 앞면(z=-0.5) 중앙에 생성
        if (m_passages_defs.empty()) { 
            new_passage.local_coords = {{
                {-p_w_norm, -p_h_norm, -0.5f}, { p_w_norm, -p_h_norm, -0.5f},
                { p_w_norm,  p_h_norm, -0.5f}, {-p_w_norm,  p_h_norm, -0.5f}
            }};
        } else { // 두 번째 통로는 뒷면(z=0.5) 중앙에 생성
             new_passage.local_coords = {{
                {-p_w_norm, -p_h_norm,  0.5f}, { p_w_norm, -p_h_norm,  0.5f},
                { p_w_norm,  p_h_norm,  0.5f}, {-p_w_norm,  p_h_norm,  0.5f}
            }};
        }
        m_passages_defs.push_back(new_passage); // 생성된 통로 정보 추가
        updatePassageCountText(); // 통로 개수 텍스트 업데이트
        projectVertices(); // 3D 뷰 갱신 (실제로는 drawOpenings에서 그려짐)
    }
}

// 통로 제거 함수 (가장 최근에 추가된 통로부터 제거)
void SettingScreen::removePassage() {
    if (!m_passages_defs.empty()) { // 통로가 존재할 때만
        m_passages_defs.pop_back(); // 마지막 통로 제거
        updatePassageCountText(); // 통로 개수 텍스트 업데이트
        projectVertices(); // 3D 뷰 갱신
    }
}

// 창문 생성 함수 (최대 2개)
void SettingScreen::createWindow() {
    if (m_windows_defs.size() < 2) { // 창문 개수 제한 확인
        OpeningDefinition new_window; // 새 창문 정의 객체
        // 창문의 정규화된 높이/깊이(또는 너비) 절반 계산
        float w_h_norm = WINDOW_RELATIVE_HEIGHT_FACTOR * 0.5f; 
        float w_d_norm = WINDOW_RELATIVE_WIDTH_FACTOR * 0.5f;  

        // 첫 번째 창문은 왼쪽 면(x=-0.5) 중앙에 생성
        if (m_windows_defs.empty()) { 
            new_window.local_coords = {{
                {-0.5f, -w_h_norm, -w_d_norm}, {-0.5f, -w_h_norm,  w_d_norm},
                {-0.5f,  w_h_norm,  w_d_norm}, {-0.5f,  w_h_norm, -w_d_norm}
            }};
        } else { // 두 번째 창문은 오른쪽 면(x=0.5) 중앙에 생성
            new_window.local_coords = {{
                { 0.5f, -w_h_norm, -w_d_norm}, { 0.5f, -w_h_norm,  w_d_norm},
                { 0.5f,  w_h_norm,  w_d_norm}, { 0.5f,  w_h_norm, -w_d_norm}
            }};
        }
        m_windows_defs.push_back(new_window); // 생성된 창문 정보 추가
        updateWindowCountText(); // 창문 개수 텍스트 업데이트
        projectVertices(); // 3D 뷰 갱신
    }
}

// 창문 제거 함수 (가장 최근에 추가된 창문부터 제거)
void SettingScreen::removeWindow() {
    if (!m_windows_defs.empty()) { // 창문이 존재할 때만
        m_windows_defs.pop_back(); // 마지막 창문 제거
        updateWindowCountText(); // 창문 개수 텍스트 업데이트
        projectVertices(); // 3D 뷰 갱신
    }
}

// 생성된 통로 및 창문(개구부)들을 3D 공간에 그리는 함수
void SettingScreen::drawOpenings(sf::RenderWindow& window) {
    // 단일 개구부(사각형)를 그리는 람다 함수
    auto draw_one_opening_shape = [&](const OpeningDefinition& def) {
        std::array<sf::Vector2f, 4> screen_points; // 개구부의 4개 꼭짓점의 2D 화면 좌표
        
        // 현재 방 크기 및 3D 뷰 스케일 팩터 (projectVertices와 동일 로직)
        float w = (m_roomWidth > 0.1f) ? m_roomWidth : 0.1f;
        float d = (m_roomDepth > 0.1f) ? m_roomDepth : 0.1f;
        float h = (m_roomHeight > 0.1f) ? m_roomHeight : 0.1f;
        float maxDim = std::max({w, d, h, 1.f});
        float viewScaleFactor = 350.f / maxDim;

        // 개구부의 4개 로컬 정규화 좌표를 월드 좌표로 변환 후 화면에 투영
        for (size_t i = 0; i < 4; ++i) {
            const Vec3D& v_local_norm = def.local_coords[i]; // 개구부의 로컬 정규화 좌표

            // 로컬 좌표를 실제 방 크기에 맞게 스케일링 (월드 좌표로 변환)
            Vec3D v_world_scaled;
            v_world_scaled.x = v_local_norm.x * w;
            v_world_scaled.y = v_local_norm.y * h;
            v_world_scaled.z = v_local_norm.z * d;

            // 시점 변환 (회전) 적용
            float x_rot_y = v_world_scaled.x * std::cos(m_rotationY) - v_world_scaled.z * std::sin(m_rotationY);
            float z_rot_y = v_world_scaled.x * std::sin(m_rotationY) + v_world_scaled.z * std::cos(m_rotationY);
            float y_rotated = v_world_scaled.y * std::cos(m_rotationX) - z_rot_y * std::sin(m_rotationX);
            float z_rotated_final = v_world_scaled.y * std::sin(m_rotationX) + z_rot_y * std::cos(m_rotationX);

            // 3D 뷰 스케일 적용
            Vec3D v_transformed_for_projection = {
                x_rot_y * viewScaleFactor,
                y_rotated * viewScaleFactor,
                z_rotated_final * viewScaleFactor
            };
            
            screen_points[i] = project(v_transformed_for_projection); // 2D 화면 좌표로 투영
        }

        // 투영된 4개의 점을 이어 사각형 모서리 그리기
        for (size_t i = 0; i < 4; ++i) {
            sf::Vertex line[] = {
                sf::Vertex(screen_points[i], sf::Color::White), // 개구부 선 색상
                sf::Vertex(screen_points[(i + 1) % 4], sf::Color::White) // 다음 점과 연결 ((i+1)%4는 순환 위함)
            };
            window.draw(line, 2, sf::Lines);
        }
    };

    // 모든 통로 그리기
    for (const auto& passage_def : m_passages_defs) {
        draw_one_opening_shape(passage_def);
    }
    // 모든 창문 그리기
    for (const auto& window_def : m_windows_defs) {
        draw_one_opening_shape(window_def);
    }
}

// 통로 개수 표시 텍스트 업데이트 함수
void SettingScreen::updatePassageCountText() {
    std::wstring text = L"통로 개수 (" + std::to_wstring(m_passages_defs.size()) + L"/2)";
    m_textPassageCount.setString(text);
    // 텍스트 내용 변경 시 원점 재설정 (중앙 정렬 유지)
    sf::FloatRect bounds = m_textPassageCount.getLocalBounds();
    m_textPassageCount.setOrigin(std::round(bounds.left + bounds.width / 2.f), std::round(bounds.top + bounds.height / 2.f));
}

// 창문 개수 표시 텍스트 업데이트 함수
void SettingScreen::updateWindowCountText() {
    std::wstring text = L"창문 개수 (" + std::to_wstring(m_windows_defs.size()) + L"/2)";
    m_textWindowCount.setString(text);
    sf::FloatRect bounds = m_textWindowCount.getLocalBounds();
    m_textWindowCount.setOrigin(std::round(bounds.left + bounds.width / 2.f), std::round(bounds.top + bounds.height / 2.f));
}

// 화면 렌더링 함수 (매 프레임 호출)
void SettingScreen::render() {
    m_window.clear(sf::Color::Black); // 검은색으로 화면 지우기

    // 3D 뷰 렌더링
    m_window.setView(m_3dView);      // 3D 뷰 활성화
    drawCuboidEdges(m_window);       // 육면체 모서리 그리기
    drawOpenings(m_window);          // 통로 및 창문 그리기

    // UI 뷰 렌더링
    m_window.setView(m_uiView);      // UI 뷰 활성화
    m_window.draw(m_titleText);      // 제목 텍스트 그리기

    // 입력 필드(InputBox + 라벨) 그리기
    m_inputWidth.render(m_window); m_window.draw(m_labelWidth);
    m_inputDepth.render(m_window); m_window.draw(m_labelDepth);
    m_inputHeight.render(m_window); m_window.draw(m_labelHeight);

    // 오염 물질 선택 UI 그리기
    m_window.draw(m_labelPollutant);
    for(size_t i=0; i<m_pollutantOptions.size(); ++i) {
        m_window.draw(m_pollutantOptionShapes[i]); // 옵션 버튼 모양
        m_window.draw(m_pollutantOptions[i]);      // 옵션 텍스트
    }

    // 통로/창문 생성 및 제거 버튼 그리기
    m_window.draw(m_shapeCreatePassage); m_window.draw(m_buttonCreatePassage);
    m_window.draw(m_shapeCreateWindow);  m_window.draw(m_buttonCreateWindow);
    m_window.draw(m_shapeRemovePassage); m_window.draw(m_buttonRemovePassage);
    m_window.draw(m_shapeRemoveWindow);  m_window.draw(m_buttonRemoveWindow);

    // 통로/창문 개수 텍스트 그리기
    m_window.draw(m_textPassageCount);
    m_window.draw(m_textWindowCount);

    // "시뮬레이션 시작" 버튼 그리기
    m_window.draw(m_shapeStartSimulation); m_window.draw(m_buttonStartSimulation);

    m_window.setView(m_window.getDefaultView()); // 뷰를 기본값으로 복원
    m_window.display(); // 그려진 내용 화면에 최종 표시
}

// 다음 화면 상태 반환
ScreenState SettingScreen::getNextState() const {
    return m_nextState;
}
// 다음 화면 상태 설정
void SettingScreen::setNextState(ScreenState state) {
    m_nextState = state;
}

// 현재 화면 실행 여부 반환
bool SettingScreen::isRunning() const {
    return m_running;
}