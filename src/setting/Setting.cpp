#include "Setting.hpp"
// #include "../screen/Screen.hpp" // ScreenState 때문에 필요. Setting.hpp에서 이미 포함.
#include <cmath>
#include <iomanip>   // std::fixed, std::setprecision
#include <sstream>   // std::stringstream
#include <iostream>  // std::cerr
#include <algorithm> // std::max
#include <fstream>   // std::ofstream (파일 쓰기용)


// PI 상수 정의
const float PI = 3.1415926535f;

// 개구부 크기 상수 정의
const float SettingScreen::PASSAGE_RELATIVE_HEIGHT_FACTOR = 0.7f;
const float SettingScreen::PASSAGE_RELATIVE_WIDTH_FACTOR = 0.25f;
const float SettingScreen::WINDOW_RELATIVE_HEIGHT_FACTOR = 0.5f;
const float SettingScreen::WINDOW_RELATIVE_WIDTH_FACTOR = 0.4f;

// InputBox 클래스 구현 (이전과 동일, 생략 가능하나 완전성을 위해 포함)
InputBox::InputBox() : m_isActive(false), m_showCursor(false), m_font(nullptr) {}

void InputBox::setup(const sf::Font& font, sf::Vector2f position, sf::Vector2f size, const std::wstring& placeholder) {
    m_font = &font;
    m_shape.setPosition(std::round(position.x), std::round(position.y));
    m_shape.setSize(size);
    m_shape.setFillColor(sf::Color(50, 50, 50));
    m_shape.setOutlineThickness(1.f);
    m_shape.setOutlineColor(sf::Color(100, 100, 100));

    m_text.setFont(font);
    m_text.setCharacterSize(static_cast<unsigned int>(size.y * 0.6f));
    m_text.setFillColor(sf::Color::White);
    // 텍스트 정렬 기준 및 위치 설정 (수직 중앙 정렬)
    sf::FloatRect textBounds = m_text.getLocalBounds();
    m_text.setOrigin(std::round(textBounds.left), std::round(textBounds.top + textBounds.height / 2.0f));
    m_text.setPosition(std::round(position.x + 5.f), std::round(position.y + size.y / 2.f));

    m_placeholderText = m_text; // 기본 텍스트 속성 복사
    m_placeholderText.setString(placeholder);
    m_placeholderText.setFillColor(sf::Color(150,150,150)); // Placeholder 색상
    sf::FloatRect placeholderBounds = m_placeholderText.getLocalBounds();
    m_placeholderText.setOrigin(std::round(placeholderBounds.left), std::round(placeholderBounds.top + placeholderBounds.height / 2.0f));
    m_placeholderText.setPosition(std::round(position.x + 5.f), std::round(position.y + size.y / 2.f));
}

void InputBox::handleEvent(sf::Event event) {
    if (!m_isActive) return;

    if (event.type == sf::Event::TextEntered) {
        if (event.text.unicode == '\b') { // 백스페이스
            if (!m_inputString.empty()) {
                m_inputString.pop_back();
            }
        } else if (event.text.unicode < 128 && event.text.unicode != '\r' && event.text.unicode != '\n') { // ASCII 문자, 엔터 제외
            char enteredChar = static_cast<char>(event.text.unicode);
            // 숫자, 소수점(하나만), 마이너스 부호(맨 앞만) 허용
            if (std::isdigit(enteredChar) ||
                (enteredChar == '.' && m_inputString.find('.') == std::string::npos) || 
                (enteredChar == '-' && m_inputString.empty())) { 
                 if (m_inputString.length() < 10) { // 최대 길이 제한
                    m_inputString += enteredChar;
                 }
            }
        }
        m_text.setString(m_inputString);
        // 텍스트 변경 시 원점 및 위치 재조정 (수직 중앙 정렬 유지)
        sf::FloatRect textBounds = m_text.getLocalBounds();
        m_text.setOrigin(std::round(textBounds.left), std::round(textBounds.top + textBounds.height / 2.0f));
        m_text.setPosition(std::round(m_shape.getPosition().x + 5.f), std::round(m_shape.getPosition().y + m_shape.getSize().y / 2.f));
    }
}

void InputBox::update() {
    if (m_isActive) {
        if (m_cursorClock.getElapsedTime().asSeconds() > 0.5f) { // 커서 깜빡임
            m_showCursor = !m_showCursor;
            m_cursorClock.restart();
        }
    } else {
        m_showCursor = false;
    }
}

void InputBox::render(sf::RenderWindow& window) {
    window.draw(m_shape);
    if (m_inputString.empty() && !m_isActive) { // 비활성 & 빈 문자열이면 placeholder 표시
        window.draw(m_placeholderText);
    } else {
        std::string currentTextStr = m_inputString;
        if (m_isActive && m_showCursor) { // 활성 & 커서 보임 상태면 커서 추가
            currentTextStr += "|";
        }
        // 임시 텍스트 객체를 사용하여 커서 포함 문자열 렌더링 (원래 m_text 객체는 커서 없이 유지)
        sf::Text tempText = m_text; 
        tempText.setString(currentTextStr);
        // 커서 포함 시 너비가 달라지므로 원점 및 위치 재조정 필요
        sf::FloatRect tempBounds = tempText.getLocalBounds();
        tempText.setOrigin(std::round(tempBounds.left), std::round(tempBounds.top + tempBounds.height / 2.0f));
        tempText.setPosition(std::round(m_shape.getPosition().x + 5.f), std::round(m_shape.getPosition().y + m_shape.getSize().y / 2.f));
        window.draw(tempText);
    }
}

void InputBox::setActive(bool active) {
    m_isActive = active;
    if (m_isActive) {
        m_shape.setOutlineColor(sf::Color::White); // 활성 시 외곽선 강조
        m_cursorClock.restart();
        m_showCursor = true;
    } else {
        m_shape.setOutlineColor(sf::Color(100, 100, 100)); // 비활성 시 기본 외곽선
        m_showCursor = false;
    }
    // 비활성화 시 빈 문자열이면 placeholder 위치 재조정 (필요시)
    if (m_inputString.empty() && !m_isActive) {
        sf::FloatRect placeholderBounds = m_placeholderText.getLocalBounds();
        m_placeholderText.setOrigin(std::round(placeholderBounds.left), std::round(placeholderBounds.top + placeholderBounds.height / 2.0f));
        m_placeholderText.setPosition(std::round(m_shape.getPosition().x + 5.f), std::round(m_shape.getPosition().y + m_shape.getSize().y / 2.f));
    }
}

bool InputBox::isActive() const { return m_isActive; }
std::string InputBox::getText() const { return m_inputString; }
void InputBox::setText(const std::string& text) {
    m_inputString = text;
    m_text.setString(m_inputString);
    // 텍스트 변경 시 원점 및 위치 재조정
    sf::FloatRect textBounds = m_text.getLocalBounds();
    m_text.setOrigin(std::round(textBounds.left), std::round(textBounds.top + textBounds.height / 2.0f));
    m_text.setPosition(std::round(m_shape.getPosition().x + 5.f), std::round(m_shape.getPosition().y + m_shape.getSize().y / 2.f));
}

float InputBox::getFloatValue() const {
    try {
        // 부동 소수점 변환이 어려운 문자열 처리 (예: "-", ".", "-.")
        if (m_inputString == "-" || m_inputString == "." || m_inputString == "-.") return 0.0f; 
        if (m_inputString.empty()) return 0.0f; // 빈 문자열은 0.0f 반환
        return std::stof(m_inputString);
    } catch (const std::invalid_argument& ia) {
        std::cerr << "Invalid argument for stof: " << m_inputString << " (" << ia.what() << ")" << std::endl;
        return 0.0f; // 변환 실패 시 0.0f 반환
    } catch (const std::out_of_range& oor) {
        std::cerr << "Out of range for stof: " << m_inputString << " (" << oor.what() << ")" << std::endl;
        return 0.0f; // 범위 초과 시 0.0f 반환
    }
}
sf::FloatRect InputBox::getGlobalBounds() const {
    return m_shape.getGlobalBounds();
}
// --- InputBox 구현 끝 ---


SettingScreen::SettingScreen(sf::RenderWindow& window, sf::Font& font)
    : m_window(window), m_font(font),
      m_nextState(ScreenState::SETTING), m_running(true),
      m_roomWidth(5.f), m_roomDepth(5.f), m_roomHeight(3.f), // 기본 방 크기
      m_rotationX(25.f * PI / 180.f), // 초기 시점 각도 (라디안)
      m_rotationY(-35.f * PI / 180.f),
      m_isDragging(false), m_selectedPollutantIndex(0), m_activeInputBox(nullptr) {

    // 버튼 색상 초기화
    m_buttonTextColorNormal = sf::Color::White;
    m_buttonTextColorHover = sf::Color::Black;
    m_buttonBgColorNormal = sf::Color(80,80,80);
    m_buttonBgColorHover = sf::Color::White;
    m_buttonOutlineColor = sf::Color::White;

    // 개구부 정보 벡터 메모리 미리 할당 (최대 2개)
    m_passages_defs.reserve(2); 
    m_windows_defs.reserve(2);  

    // 3D 뷰 및 UI 뷰 설정
    m_3dView.setSize(static_cast<float>(m_window.getSize().x) * 0.6f, static_cast<float>(m_window.getSize().y));
    m_3dView.setCenter(m_3dView.getSize().x / 2.f, m_3dView.getSize().y / 2.f - 50.f); // 약간 위로 올려서 바닥이 잘 보이도록
    m_3dView.setViewport(sf::FloatRect(0.f, 0.f, 0.6f, 1.f)); // 창의 왼쪽 60% 사용

    m_uiView.setSize(static_cast<float>(m_window.getSize().x) * 0.4f, static_cast<float>(m_window.getSize().y));
    m_uiView.setCenter(m_uiView.getSize().x / 2.f, m_uiView.getSize().y / 2.f);
    m_uiView.setViewport(sf::FloatRect(0.6f, 0.f, 0.4f, 1.f)); // 창의 오른쪽 40% 사용

    setupUI();    // UI 요소 초기화
    setup3D();    // 3D 모델 기본 구조 초기화
    projectVertices(); // 초기 3D 투영
}

SettingScreen::~SettingScreen() {}

// Setting 화면으로 돌아올 때 호출되는 함수 (main.cpp에서 상태 전환 시 호출)
void SettingScreen::reset() {
    m_running = true; // 화면 실행 상태로
    m_nextState = ScreenState::SETTING; // 다음 상태는 자기 자신 (유지)

    // 필요한 경우 여기에 설정값 초기화 로직 추가 (예: 방 크기, 오염물질 선택 등)
    // 현재는 별도 초기화 없이 이전 상태 유지 또는 생성자에서 설정된 기본값 사용
    // 만약 StartScreen 등에서 이 화면으로 올 때 특정 값을 초기화해야 한다면 여기에 구현.
    // 예:
    // m_inputWidth.setText("5.0"); m_roomWidth = 5.0f;
    // m_inputDepth.setText("5.0"); m_roomDepth = 5.0f;
    // m_inputHeight.setText("3.0"); m_roomHeight = 3.0f;
    // m_selectedPollutantIndex = 0;
    // m_passages_defs.clear(); updatePassageCountText();
    // m_windows_defs.clear(); updateWindowCountText();
    // projectVertices();

    if (m_activeInputBox) { // 활성화된 입력창이 있다면 비활성화
        m_activeInputBox->setActive(false);
        m_activeInputBox = nullptr;
    }
}


void SettingScreen::setupUI() {
    // UI 요소 위치 및 크기 설정 (이전과 동일)
    float currentY = 20.f;
    float inputHeight = 28.f;
    float spacing = 35.f;
    unsigned int charSize = 18;
    float titleCharSize = 28;

    float labelWidthForCalc = 90.f; 
    float inputBoxWidthForCalc = 180.f; 
    float gapBetweenLabelInput = 10.f;
    float maxUiElementWidth = labelWidthForCalc + gapBetweenLabelInput + inputBoxWidthForCalc;

    float rightPadding = 30.f; 
    float uiX = m_uiView.getSize().x - maxUiElementWidth - rightPadding; 

    // 제목
    m_titleText.setFont(m_font);
    m_titleText.setString(L"시뮬레이션 설정");
    m_titleText.setCharacterSize(titleCharSize);
    m_titleText.setFillColor(sf::Color::White);
    sf::FloatRect titleBounds = m_titleText.getLocalBounds();
    m_titleText.setOrigin(std::round(titleBounds.left + titleBounds.width / 2.f), std::round(titleBounds.top));
    m_titleText.setPosition(std::round(uiX + maxUiElementWidth / 2.f), std::round(currentY));
    currentY += titleCharSize + spacing * 0.8f; 

    // 입력 필드 설정 람다
    auto setupInputField = [&](sf::Text& label, InputBox& inputBox, const std::wstring& labelText, const std::string& defaultVal, const std::wstring& placeholder) {
        label.setFont(m_font);
        label.setString(labelText);
        label.setCharacterSize(charSize);
        label.setFillColor(sf::Color::White);
        sf::FloatRect labelBounds = label.getLocalBounds();
        label.setOrigin(std::round(labelBounds.left), std::round(labelBounds.top + labelBounds.height / 2.f)); // 라벨 수직 중앙 정렬
        label.setPosition(std::round(uiX), std::round(currentY + inputHeight / 2.f));

        inputBox.setup(m_font, sf::Vector2f(uiX + labelWidthForCalc + gapBetweenLabelInput, currentY), sf::Vector2f(inputBoxWidthForCalc, inputHeight), placeholder);
        inputBox.setText(defaultVal);
        currentY += spacing;
    };

    setupInputField(m_labelWidth, m_inputWidth, L"가로 (m):", "5.0", L"0.0 - 100.0");
    setupInputField(m_labelDepth, m_inputDepth, L"세로 (m):", "5.0", L"0.0 - 100.0");
    setupInputField(m_labelHeight, m_inputHeight, L"높이 (m):", "3.0", L"0.0 - 10.0");
    currentY += spacing * 0.5f; // 추가 간격

    // 오염 물질 선택 UI
    m_labelPollutant.setFont(m_font);
    m_labelPollutant.setString(L"오염 물질:");
    m_labelPollutant.setCharacterSize(charSize);
    m_labelPollutant.setFillColor(sf::Color::White);
    sf::FloatRect pollutantLabelBounds = m_labelPollutant.getLocalBounds();
    m_labelPollutant.setOrigin(std::round(pollutantLabelBounds.left), std::round(pollutantLabelBounds.top + pollutantLabelBounds.height / 2.f));
    m_labelPollutant.setPosition(std::round(uiX), std::round(currentY + (inputHeight * 0.9f) / 2.f)); 
    currentY += spacing * 0.8f;

    std::vector<std::wstring> pollutants = {L"미세먼지 (PM10)", L"일산화탄소 (CO)", L"염소가스 (Cl₂)"};
    m_pollutantOptions.resize(pollutants.size());
    m_pollutantOptionShapes.resize(pollutants.size());
    float pollutantOptionItemWidth = maxUiElementWidth; 

    for (size_t i = 0; i < pollutants.size(); ++i) {
        m_pollutantOptionShapes[i].setSize(sf::Vector2f(pollutantOptionItemWidth, inputHeight * 0.9f));
        m_pollutantOptionShapes[i].setPosition(std::round(uiX), std::round(currentY));
        m_pollutantOptionShapes[i].setOutlineThickness(1.f);

        m_pollutantOptions[i].setFont(m_font);
        m_pollutantOptions[i].setString(pollutants[i]);
        m_pollutantOptions[i].setCharacterSize(charSize - 2); 
        
        sf::FloatRect textBounds = m_pollutantOptions[i].getLocalBounds();
        m_pollutantOptions[i].setOrigin(std::round(textBounds.left + textBounds.width / 2.f), std::round(textBounds.top + textBounds.height / 2.f));
        m_pollutantOptions[i].setPosition(std::round(uiX + pollutantOptionItemWidth / 2.f), std::round(currentY + (inputHeight * 0.9f) / 2.f));
        
        currentY += (inputHeight * 0.9f) + 5.f; // 각 옵션 사이 간격
    }
    currentY += spacing * 0.5f; // 추가 간격

    // 버튼 설정 람다
    auto setupButtonLambda = [&](sf::Text& text, sf::RectangleShape& shape, const std::wstring& str, float yPos, float btnWidth, float btnXOffset = 0.f) {
        shape.setSize(sf::Vector2f(btnWidth, inputHeight));
        shape.setPosition(std::round(uiX + btnXOffset), std::round(yPos));
        shape.setOutlineThickness(1.f);

        text.setFont(m_font);
        text.setString(str);
        text.setCharacterSize(charSize - 2); 
        
        sf::FloatRect textBounds = text.getLocalBounds();
        text.setOrigin(std::round(textBounds.left + textBounds.width / 2.f), std::round(textBounds.top + textBounds.height / 2.f));
        text.setPosition(std::round(shape.getPosition().x + btnWidth / 2.f), std::round(yPos + inputHeight / 2.f));
    };
    
    float singleButtonWidth = maxUiElementWidth;
    float pairedButtonWidth = (maxUiElementWidth - 10.f) / 2.f; // 두 버튼 나란히 배치 시 너비

    // 통로/창문 생성 버튼
    setupButtonLambda(m_buttonCreatePassage, m_shapeCreatePassage, L"통로 생성", currentY, pairedButtonWidth);
    setupButtonLambda(m_buttonCreateWindow, m_shapeCreateWindow, L"창문 생성", currentY, pairedButtonWidth, pairedButtonWidth + 10.f);
    currentY += spacing; 

    // 통로/창문 제거 버튼
    float removeButtonsY = currentY; 
    setupButtonLambda(m_buttonRemovePassage, m_shapeRemovePassage, L"통로 제거", removeButtonsY, pairedButtonWidth);
    setupButtonLambda(m_buttonRemoveWindow, m_shapeRemoveWindow, L"창문 제거", removeButtonsY, pairedButtonWidth, pairedButtonWidth + 10.f);
    
    // 개수 표시 텍스트
    float countTextOffsetY = inputHeight + 10.f; 
    unsigned int countCharSize = charSize - 4; 

    m_textPassageCount.setFont(m_font);
    m_textPassageCount.setCharacterSize(countCharSize);
    m_textPassageCount.setFillColor(sf::Color::White);
    updatePassageCountText(); // 초기 텍스트 설정

    sf::FloatRect passageCountBounds = m_textPassageCount.getLocalBounds(); 
    // 제거 버튼 중앙 아래에 위치
    m_textPassageCount.setPosition(
        std::round(m_shapeRemovePassage.getPosition().x + m_shapeRemovePassage.getSize().x / 2.f),
        std::round(removeButtonsY + countTextOffsetY + passageCountBounds.height / 2.f) 
    );

    m_textWindowCount.setFont(m_font);
    m_textWindowCount.setCharacterSize(countCharSize);
    m_textWindowCount.setFillColor(sf::Color::White);
    updateWindowCountText(); // 초기 텍스트 설정

    sf::FloatRect windowCountBounds = m_textWindowCount.getLocalBounds(); 
    // 제거 버튼 중앙 아래에 위치
    m_textWindowCount.setPosition(
        std::round(m_shapeRemoveWindow.getPosition().x + m_shapeRemoveWindow.getSize().x / 2.f),
        std::round(removeButtonsY + countTextOffsetY + windowCountBounds.height / 2.f)
    );

    currentY += spacing; // 개수 텍스트와 다음 버튼 사이 간격
    currentY += spacing * 0.5f; // 추가 간격

    // 시뮬레이션 시작 버튼 (화면 하단에 배치)
    float startButtonY = m_uiView.getSize().y - spacing - inputHeight; 
    setupButtonLambda(m_buttonStartSimulation, m_shapeStartSimulation, L"시뮬레이션 시작", startButtonY, singleButtonWidth);
}

// 3D 모델 기본 구조 설정 (이전과 동일)
void SettingScreen::setup3D() {
    m_cubeVertices = {{
        {-0.5f, -0.5f, -0.5f}, {0.5f, -0.5f, -0.5f}, {0.5f, 0.5f, -0.5f}, {-0.5f, 0.5f, -0.5f}, 
        {-0.5f, -0.5f, 0.5f}, {0.5f, -0.5f, 0.5f}, {0.5f, 0.5f, 0.5f}, {-0.5f, 0.5f, 0.5f}    
    }};
    m_cubeEdges = {
        {0,1}, {1,2}, {2,3}, {3,0}, 
        {4,5}, {5,6}, {6,7}, {7,4}, 
        {0,4}, {1,5}, {2,6}, {3,7}  
    };
    m_transformedVertices.fill({}); 
}

// 3D 정점 변환 (방 크기, 회전 적용) - 이전과 동일
void SettingScreen::projectVertices() {
    float w = (m_roomWidth > 0.1f) ? m_roomWidth : 0.1f;
    float d = (m_roomDepth > 0.1f) ? m_roomDepth : 0.1f;
    float h = (m_roomHeight > 0.1f) ? m_roomHeight : 0.1f;

    float maxDim = std::max({w, d, h, 1.f}); 
    float scaleFactor = 350.f / maxDim; // 뷰 크기에 맞게 스케일 조절

    for (size_t i = 0; i < m_cubeVertices.size(); ++i) {
        const Vec3D& v_orig = m_cubeVertices[i]; 
        Vec3D v_scaled; 
        v_scaled.x = v_orig.x * w;
        v_scaled.y = v_orig.y * h; // Y축 방향 일관성 유지 (SimulationScreen과 동일하게)
        v_scaled.z = v_orig.z * d;

        // Y축 회전
        float x_rot_y = v_scaled.x * std::cos(m_rotationY) - v_scaled.z * std::sin(m_rotationY);
        float z_rot_y = v_scaled.x * std::sin(m_rotationY) + v_scaled.z * std::cos(m_rotationY);
        
        // X축 회전
        float y_final = v_scaled.y * std::cos(m_rotationX) - z_rot_y * std::sin(m_rotationX);
        float z_final = v_scaled.y * std::sin(m_rotationX) + z_rot_y * std::cos(m_rotationX); 
        
        m_transformedVertices[i] = {
            x_rot_y * scaleFactor, 
            y_final * scaleFactor, 
            z_final * scaleFactor 
        };
    }
}

// 3D 점을 2D 화면 좌표로 투영 - 이전과 동일
sf::Vector2f SettingScreen::project(const Vec3D& p) const {
    sf::Vector2f viewCenter = m_3dView.getCenter();
    float perspectiveFactor = 500.f / (500.f + p.z); // 간단한 원근 투영
    
    return sf::Vector2f(
        std::round(p.x * perspectiveFactor + viewCenter.x), 
        std::round(p.y * perspectiveFactor + viewCenter.y) 
    );
}

// 입력창 이벤트 처리 - 이전과 동일
void SettingScreen::handleInputBoxEvents(sf::Event event) {
    InputBox* currentActive = m_activeInputBox; 
    if (currentActive) {
        currentActive->handleEvent(event);
        // 입력창 내용 변경 시 m_roomWidth 등 관련 변수 업데이트 및 3D 뷰 갱신
        if (m_activeInputBox == currentActive) { // 이벤트 처리 후에도 여전히 활성 상태인지 확인
            if (currentActive == &m_inputWidth) m_roomWidth = m_inputWidth.getFloatValue();
            else if (currentActive == &m_inputDepth) m_roomDepth = m_inputDepth.getFloatValue();
            else if (currentActive == &m_inputHeight) m_roomHeight = m_inputHeight.getFloatValue();
            projectVertices(); // 방 크기 변경 시 3D 모델 즉시 갱신
        }
    }
}

// *** 추가된 함수: 설정을 파일에 저장 ***
void SettingScreen::saveSettingsToFile(const std::string& filename) const {
    std::ofstream outFile(filename); // 출력 파일 스트림 열기
    if (outFile.is_open()) {
        // 현재 설정된 값들을 "키:값" 형식으로 저장
        outFile << "width:" << m_roomWidth << std::endl;
        outFile << "depth:" << m_roomDepth << std::endl;
        outFile << "height:" << m_roomHeight << std::endl;
        outFile << "pollutant_index:" << m_selectedPollutantIndex << std::endl;
        outFile << "passages_count:" << m_passages_defs.size() << std::endl;
        outFile << "windows_count:" << m_windows_defs.size() << std::endl;
        outFile.close(); // 파일 닫기
        std::cout << "Settings saved to " << filename << std::endl;
    } else {
        std::cerr << "Error: Could not open file to save settings: " << filename << std::endl;
    }
}


// 사용자 입력 처리
void SettingScreen::handleInput() {
    sf::Event event;
    while (m_window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            m_running = false;
            m_nextState = ScreenState::EXIT;
        }
        if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
            if (m_activeInputBox) { // 입력창 활성화 중이면 비활성화
                m_activeInputBox->setActive(false); 
                m_activeInputBox = nullptr;
            } else { // 그렇지 않으면 StartScreen으로 돌아감
                m_running = false; 
                m_nextState = ScreenState::START;
            }
        }
        
        // 입력창 관련 이벤트 우선 처리
        if (m_activeInputBox && event.type == sf::Event::TextEntered) {
            handleInputBoxEvents(event);
            continue; // 이벤트 처리했으므로 다음 루프로
        }
        if (m_activeInputBox && event.type == sf::Event::KeyPressed) {
             handleInputBoxEvents(event); // 키 입력도 입력창으로 전달 (백스페이스 등)
             // 여기서 continue를 하면 다른 키 입력 처리를 막을 수 있으나,
             // 현재는 특별히 다른 키 입력과 충돌하지 않으므로 생략 가능
        }


        if (event.type == sf::Event::MouseButtonPressed) {
            if (event.mouseButton.button == sf::Mouse::Left) {
                sf::Vector2f mousePosWindow = static_cast<sf::Vector2f>(sf::Mouse::getPosition(m_window));
                // UI 뷰 좌표로 변환
                sf::Vector2f mousePosUI = m_window.mapPixelToCoords(sf::Mouse::getPosition(m_window), m_uiView);

                InputBox* previouslyActive = m_activeInputBox;
                InputBox* clickedBox = nullptr;

                // 어떤 입력창이 클릭되었는지 확인
                if (m_inputWidth.getGlobalBounds().contains(mousePosUI)) clickedBox = &m_inputWidth;
                else if (m_inputDepth.getGlobalBounds().contains(mousePosUI)) clickedBox = &m_inputDepth;
                else if (m_inputHeight.getGlobalBounds().contains(mousePosUI)) clickedBox = &m_inputHeight;

                if (previouslyActive && previouslyActive != clickedBox) {
                    previouslyActive->setActive(false); // 이전 활성 입력창 비활성화
                }
                m_activeInputBox = clickedBox; // 새로 클릭된 입력창을 활성 입력창으로 설정
                if (m_activeInputBox) {
                    m_activeInputBox->setActive(true); // 활성화 시각 효과 적용
                }
                
                // 입력창이 아닌 곳을 클릭했다면 오염물질 선택 또는 버튼 클릭 확인
                if (!m_activeInputBox) {
                    for(size_t i=0; i<m_pollutantOptionShapes.size(); ++i) {
                        if (m_pollutantOptionShapes[i].getGlobalBounds().contains(mousePosUI)) {
                            m_selectedPollutantIndex = static_cast<int>(i); // 선택된 오염물질 인덱스 업데이트
                            break; 
                        }
                    }
                }
                
                if (!m_activeInputBox) { // 입력창이 활성화되지 않았을 때만 버튼 작동
                    if (m_shapeCreatePassage.getGlobalBounds().contains(mousePosUI)) {
                        createPassage();
                    } else if (m_shapeRemovePassage.getGlobalBounds().contains(mousePosUI)) {
                        removePassage();
                    } else if (m_shapeCreateWindow.getGlobalBounds().contains(mousePosUI)) {
                        createWindow();
                    } else if (m_shapeRemoveWindow.getGlobalBounds().contains(mousePosUI)) {
                        removeWindow();
                    }
                    // *** "시뮬레이션 시작" 버튼 클릭 시 설정 저장 및 상태 변경 ***
                    else if (m_shapeStartSimulation.getGlobalBounds().contains(mousePosUI)) {
                        saveSettingsToFile("Setting_values.text"); // 설정 파일 저장!
                        m_nextState = ScreenState::SIMULATION;     // 다음 상태를 SIMULATION으로
                        m_running = false;                         // 현재 화면(SettingScreen) 종료
                    }
                }

                // 3D 뷰 영역 클릭 시 드래그 시작 (입력창 비활성 상태일 때)
                sf::FloatRect view3DViewportRect(
                    m_3dView.getViewport().left * m_window.getSize().x,
                    m_3dView.getViewport().top * m_window.getSize().y,
                    m_3dView.getViewport().width * m_window.getSize().x,
                    m_3dView.getViewport().height * m_window.getSize().y
                );

                if (view3DViewportRect.contains(mousePosWindow) && !m_activeInputBox) {
                     m_isDragging = true;
                     m_lastMousePos = sf::Mouse::getPosition(m_window);
                }
            }
        }
        if (event.type == sf::Event::MouseButtonReleased) {
            if (event.mouseButton.button == sf::Mouse::Left) {
                m_isDragging = false; // 드래그 종료
            }
        }
        if (event.type == sf::Event::MouseMoved) {
            if (m_isDragging && !m_activeInputBox) { // 드래그 중이고 입력창 비활성 상태
                sf::Vector2i currentMousePos = sf::Mouse::getPosition(m_window);
                float dx = static_cast<float>(currentMousePos.x - m_lastMousePos.x);
                // float dy = static_cast<float>(currentMousePos.y - m_lastMousePos.y); // X축 회전용

                m_rotationY += dx * 0.005f; // 마우스 X축 이동량에 따라 Y축 회전
                // m_rotationX -= dy * 0.005f; // 마우스 Y축 이동량에 따라 X축 회전 (선택 사항)

                m_lastMousePos = currentMousePos;
                projectVertices(); // 회전된 정점 다시 계산
            }
        }
    }
}

// 버튼 호버 효과 업데이트 - 이전과 동일
void SettingScreen::updateButtonHovers(const sf::Vector2f& mousePos) {
    auto updateVisuals = [&](sf::Text& text, sf::RectangleShape& shape, bool isHovered, bool isSelected = false) {
        text.setFillColor(isHovered || isSelected ? m_buttonTextColorHover : m_buttonTextColorNormal);
        shape.setFillColor(isHovered || isSelected ? m_buttonBgColorHover : m_buttonBgColorNormal);
        if (isSelected) { // 오염물질 선택 시 특별한 외곽선 색상
            shape.setOutlineColor(sf::Color::Yellow); 
        } else {
            shape.setOutlineColor(m_buttonOutlineColor);
        }
    };
    
    // 오염물질 선택 옵션 시각 효과
    for(size_t i=0; i<m_pollutantOptions.size(); ++i) {
        bool isSelected = (static_cast<int>(i) == m_selectedPollutantIndex);
        bool isHovered = m_pollutantOptionShapes[i].getGlobalBounds().contains(mousePos);
        updateVisuals(m_pollutantOptions[i], m_pollutantOptionShapes[i], isHovered, isSelected);
    }

    // 일반 버튼 시각 효과
    updateVisuals(m_buttonCreatePassage, m_shapeCreatePassage, m_shapeCreatePassage.getGlobalBounds().contains(mousePos));
    updateVisuals(m_buttonCreateWindow, m_shapeCreateWindow, m_shapeCreateWindow.getGlobalBounds().contains(mousePos));
    updateVisuals(m_buttonRemovePassage, m_shapeRemovePassage, m_shapeRemovePassage.getGlobalBounds().contains(mousePos));
    updateVisuals(m_buttonRemoveWindow, m_shapeRemoveWindow, m_shapeRemoveWindow.getGlobalBounds().contains(mousePos));
    updateVisuals(m_buttonStartSimulation, m_shapeStartSimulation, m_shapeStartSimulation.getGlobalBounds().contains(mousePos));
}

// 매 프레임 업데이트 - 이전과 동일
void SettingScreen::update(sf::Time dt) {
    m_inputWidth.update(); // 각 입력창 커서 등 상태 업데이트
    m_inputDepth.update();
    m_inputHeight.update();

    // UI 뷰 기준 마우스 좌표로 버튼 호버 효과 업데이트
    sf::Vector2f mousePosUI = m_window.mapPixelToCoords(sf::Mouse::getPosition(m_window), m_uiView);
    updateButtonHovers(mousePosUI);
}

// 방 모서리 그리기 - 이전과 동일
void SettingScreen::drawCuboidEdges(sf::RenderWindow& window) {
    for(const auto& edge : m_cubeEdges) {
        sf::Vector2f p1_screen = project(m_transformedVertices[edge.start]);
        sf::Vector2f p2_screen = project(m_transformedVertices[edge.end]);
        sf::Vertex line[] = {
            sf::Vertex(p1_screen, sf::Color::White),
            sf::Vertex(p2_screen, sf::Color::White)
        };
        window.draw(line, 2, sf::Lines);
    }
}

// 통로 생성 - 이전과 동일
void SettingScreen::createPassage() {
    if (m_passages_defs.size() < 2) { // 최대 2개
        OpeningDefinition new_passage;
        float p_h_norm = PASSAGE_RELATIVE_HEIGHT_FACTOR * 0.5f; // 높이의 절반 (중심 기준)
        float p_w_norm = PASSAGE_RELATIVE_WIDTH_FACTOR * 0.5f;  // 너비의 절반

        if (m_passages_defs.empty()) { // 첫 번째 통로는 z = -0.5 면에
            new_passage.local_coords = {{
                {-p_w_norm, -p_h_norm, -0.5f}, { p_w_norm, -p_h_norm, -0.5f},
                { p_w_norm,  p_h_norm, -0.5f}, {-p_w_norm,  p_h_norm, -0.5f}
            }};
        } else { // 두 번째 통로는 z = 0.5 면에
             new_passage.local_coords = {{
                {-p_w_norm, -p_h_norm,  0.5f}, { p_w_norm, -p_h_norm,  0.5f},
                { p_w_norm,  p_h_norm,  0.5f}, {-p_w_norm,  p_h_norm,  0.5f}
            }};
        }
        m_passages_defs.push_back(new_passage);
        updatePassageCountText(); // 개수 텍스트 업데이트
        projectVertices(); // 3D 뷰 갱신 (실제로는 통로 추가로 정점이 바뀌진 않음, drawOpenings에서 그림)
    }
}

// 통로 제거 - 이전과 동일
void SettingScreen::removePassage() {
    if (!m_passages_defs.empty()) {
        m_passages_defs.pop_back(); 
        updatePassageCountText(); 
        projectVertices(); 
    }
}

// 창문 생성 - 이전과 동일
void SettingScreen::createWindow() {
    if (m_windows_defs.size() < 2) { // 최대 2개
        OpeningDefinition new_window;
        float w_h_norm = WINDOW_RELATIVE_HEIGHT_FACTOR * 0.5f; 
        float w_d_norm = WINDOW_RELATIVE_WIDTH_FACTOR * 0.5f;  

        if (m_windows_defs.empty()) { // 첫 번째 창문은 x = -0.5 면에
            new_window.local_coords = {{
                {-0.5f, -w_h_norm, -w_d_norm}, {-0.5f, -w_h_norm,  w_d_norm},
                {-0.5f,  w_h_norm,  w_d_norm}, {-0.5f,  w_h_norm, -w_d_norm}
            }};
        } else { // 두 번째 창문은 x = 0.5 면에
            new_window.local_coords = {{
                { 0.5f, -w_h_norm, -w_d_norm}, { 0.5f, -w_h_norm,  w_d_norm},
                { 0.5f,  w_h_norm,  w_d_norm}, { 0.5f,  w_h_norm, -w_d_norm}
            }};
        }
        m_windows_defs.push_back(new_window);
        updateWindowCountText(); 
        projectVertices();
    }
}

// 창문 제거 - 이전과 동일
void SettingScreen::removeWindow() {
    if (!m_windows_defs.empty()) {
        m_windows_defs.pop_back(); 
        updateWindowCountText(); 
        projectVertices();
    }
}

// 개구부(통로, 창문) 그리기 - 이전과 동일
void SettingScreen::drawOpenings(sf::RenderWindow& window) {
    auto draw_one_opening_shape = [&](const OpeningDefinition& def) {
        std::array<sf::Vector2f, 4> screen_points;
        
        float w = (m_roomWidth > 0.1f) ? m_roomWidth : 0.1f;
        float d = (m_roomDepth > 0.1f) ? m_roomDepth : 0.1f;
        float h = (m_roomHeight > 0.1f) ? m_roomHeight : 0.1f;

        float maxDim = std::max({w, d, h, 1.f});
        float viewScaleFactor = 350.f / maxDim; // projectVertices와 동일한 스케일

        for (size_t i = 0; i < 4; ++i) {
            const Vec3D& v_local_norm = def.local_coords[i]; // 개구부의 정규화된 로컬 좌표

            // 로컬 좌표를 실제 방 크기에 맞게 스케일링
            Vec3D v_world_scaled;
            v_world_scaled.x = v_local_norm.x * w;
            v_world_scaled.y = v_local_norm.y * h; 
            v_world_scaled.z = v_local_norm.z * d;

            // 시점 변환 (회전)
            float x_rot_y = v_world_scaled.x * std::cos(m_rotationY) - v_world_scaled.z * std::sin(m_rotationY);
            float z_rot_y = v_world_scaled.x * std::sin(m_rotationY) + v_world_scaled.z * std::cos(m_rotationY);
            float y_rotated = v_world_scaled.y * std::cos(m_rotationX) - z_rot_y * std::sin(m_rotationX);
            float z_rotated_final = v_world_scaled.y * std::sin(m_rotationX) + z_rot_y * std::cos(m_rotationX);
            
            // 최종 3D 뷰 스케일 적용
            Vec3D v_transformed_for_projection = {
                x_rot_y * viewScaleFactor,
                y_rotated * viewScaleFactor,
                z_rotated_final * viewScaleFactor
            };
            
            screen_points[i] = project(v_transformed_for_projection); // 2D 화면 투영
        }

        // 개구부 모서리 그리기
        for (size_t i = 0; i < 4; ++i) {
            sf::Vertex line[] = {
                sf::Vertex(screen_points[i], sf::Color::White), // 개구부도 흰색으로
                sf::Vertex(screen_points[(i + 1) % 4], sf::Color::White) 
            };
            window.draw(line, 2, sf::Lines);
        }
    };

    for (const auto& passage_def : m_passages_defs) {
        draw_one_opening_shape(passage_def);
    }
    for (const auto& window_def : m_windows_defs) {
        draw_one_opening_shape(window_def);
    }
}

// 통로 개수 텍스트 업데이트 - 이전과 동일
void SettingScreen::updatePassageCountText() {
    std::wstring text = L"통로 개수 (" + std::to_wstring(m_passages_defs.size()) + L"/2)";
    m_textPassageCount.setString(text);
    // 텍스트 변경 시 원점 재설정 (중앙 정렬 유지)
    sf::FloatRect bounds = m_textPassageCount.getLocalBounds();
    m_textPassageCount.setOrigin(std::round(bounds.left + bounds.width / 2.f), std::round(bounds.top + bounds.height / 2.f));
}

// 창문 개수 텍스트 업데이트 - 이전과 동일
void SettingScreen::updateWindowCountText() {
    std::wstring text = L"창문 개수 (" + std::to_wstring(m_windows_defs.size()) + L"/2)";
    m_textWindowCount.setString(text);
    sf::FloatRect bounds = m_textWindowCount.getLocalBounds();
    m_textWindowCount.setOrigin(std::round(bounds.left + bounds.width / 2.f), std::round(bounds.top + bounds.height / 2.f));
}

// 화면 렌더링 - 이전과 동일
void SettingScreen::render() {
    m_window.clear(sf::Color::Black);

    // 3D 뷰 그리기
    m_window.setView(m_3dView);
    drawCuboidEdges(m_window);
    drawOpenings(m_window); 

    // UI 뷰 그리기
    m_window.setView(m_uiView);
    m_window.draw(m_titleText);

    m_inputWidth.render(m_window); m_window.draw(m_labelWidth);
    m_inputDepth.render(m_window); m_window.draw(m_labelDepth);
    m_inputHeight.render(m_window); m_window.draw(m_labelHeight);

    m_window.draw(m_labelPollutant);
    for(size_t i=0; i<m_pollutantOptions.size(); ++i) {
        m_window.draw(m_pollutantOptionShapes[i]);
        m_window.draw(m_pollutantOptions[i]);
    }

    m_window.draw(m_shapeCreatePassage); m_window.draw(m_buttonCreatePassage);
    m_window.draw(m_shapeCreateWindow);  m_window.draw(m_buttonCreateWindow);
    m_window.draw(m_shapeRemovePassage); m_window.draw(m_buttonRemovePassage);
    m_window.draw(m_shapeRemoveWindow);  m_window.draw(m_buttonRemoveWindow);

    m_window.draw(m_textPassageCount);
    m_window.draw(m_textWindowCount);

    m_window.draw(m_shapeStartSimulation);m_window.draw(m_buttonStartSimulation);

    m_window.setView(m_window.getDefaultView()); // 기본 뷰로 복원
    m_window.display();
}

// 다음 화면 상태 getter/setter - 이전과 동일
ScreenState SettingScreen::getNextState() const {
    return m_nextState;
}
void SettingScreen::setNextState(ScreenState state) {
    m_nextState = state;
}

bool SettingScreen::isRunning() const {
    return m_running;
}