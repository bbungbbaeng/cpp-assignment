#include "Simulation.hpp"
#include <cmath>
#include <iomanip>
#include <sstream>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <random>

// --- SimulationScreen 클래스의 static const 멤버 변수 정의 ---
// 오염물질별 기본 유입 속도(S) 및 제거 속도 상수(K) (밀폐 상태 기준)
const float SimulationScreen::BASE_S_PM10 = 1.0f;
const float SimulationScreen::BASE_K_PM10 = 0.005f;
const float SimulationScreen::BASE_S_CO = 5.0f;
const float SimulationScreen::BASE_K_CO = 0.002f;
const float SimulationScreen::BASE_S_CL2 = 0.1f;
const float SimulationScreen::BASE_K_CL2 = 0.05f;

// 통로 및 창문 개수에 따른 S, K 값 조정량
const float SimulationScreen::S_ADJUST_PASSAGE = 5.0f;
const float SimulationScreen::K_ADJUST_PASSAGE = 0.02f;
const float SimulationScreen::S_ADJUST_WINDOW = 3.0f;
const float SimulationScreen::K_ADJUST_WINDOW = 0.05f;

// 시뮬레이션 화면에서 개구부(통로/창문)를 그릴 때 사용될 상대적 크기 계수
const float SimulationScreen::PASSAGE_RELATIVE_HEIGHT_FACTOR_SIM = 0.7f;
const float SimulationScreen::PASSAGE_RELATIVE_WIDTH_FACTOR_SIM = 0.25f;
const float SimulationScreen::WINDOW_RELATIVE_HEIGHT_FACTOR_SIM = 0.5f;
const float SimulationScreen::WINDOW_RELATIVE_WIDTH_FACTOR_SIM = 0.4f;

// 파티클 시스템 관련 상수
const float SimulationScreen::PARTICLE_MAX_LIFETIME = 5.0f; // 파티클 최대 수명 (초)
// 파티클 초당 알파 감소율 (0으로 나누기 방지 포함)
const float SimulationScreen::PARTICLE_FADE_RATE = (SimulationScreen::PARTICLE_MAX_LIFETIME > 0.0001f) ? (255.0f / SimulationScreen::PARTICLE_MAX_LIFETIME) : 25500.0f; 
const int SimulationScreen::PARTICLES_PER_FRAME_ADJUST = 2; // 프레임당 파티클 수 조절량 (부드러운 변화용)
// --- End of static const member variable definitions ---


// float 값을 지정된 정밀도의 유니코드 문자열(wstring)로 변환하는 헬퍼 함수
std::wstring floatToWString(float value, int precision = 2) {
    std::wstringstream wss; // 유니코드 문자열 스트림
    wss << std::fixed << std::setprecision(precision) << value; // 고정 소수점 및 정밀도 설정
    return wss.str();
}

// SimulationScreen 클래스 생성자
SimulationScreen::SimulationScreen(sf::RenderWindow& window, sf::Font& font)
    : m_window(window), m_font(font), // 멤버 변수 초기화 (창, 폰트 참조)
      m_nextState(ScreenState::SIMULATION), m_running(true), // 화면 상태 및 실행 여부 초기화
      m_rotationX(25.f * PI / 180.f), // 3D 뷰 X축 초기 회전각 (라디안)
      m_rotationY(-35.f * PI / 180.f), // 3D 뷰 Y축 초기 회전각 (라디안)
      m_isDragging(false), m_activeInputBox(nullptr), // 드래그, 활성 입력창 상태 초기화
      m_simulationActive(false), m_simulationStartedOnce(false), // 시뮬레이션 상태 플래그 초기화
      m_currentTime_t(0.0f), m_currentConcentration_Ct(0.0f), // 시뮬레이션 시간 및 현재 농도 초기화
      m_targetConcentration_Ct_for_particles(0.0f), // 파티클 목표 농도 초기화
      m_C0(100.0f), m_S_param(0.0f), m_K_param(0.0f), // 시뮬레이션 파라미터 초기화 (C0는 기본값)
      m_roomWidth(5.f), m_roomDepth(5.f), m_roomHeight(3.f), // 방 기본 크기 초기화
      m_selectedPollutantIndex(0), m_numPassages(0), m_numWindows(0), // 오염물질, 개구부 정보 초기화
      m_maxParticles(500), m_simulationTimeStepAccumulator(0.0f) { // 파티클 및 시간 제어 변수 초기화

    // 버튼 스타일 색상 초기화
    m_buttonTextColorNormal = sf::Color::White;
    m_buttonTextColorHover = sf::Color::Black;
    m_buttonBgColorNormal = sf::Color(80,80,80);
    m_buttonBgColorHover = sf::Color::White;
    m_buttonOutlineColor = sf::Color::White;

    // 3D 렌더링 뷰 설정
    m_3dView.setSize(static_cast<float>(m_window.getSize().x) * 0.6f, static_cast<float>(m_window.getSize().y)); // 뷰 크기
    m_3dView.setCenter(m_3dView.getSize().x / 2.f, m_3dView.getSize().y / 2.f); // 뷰 중심을 뷰 자체의 중앙으로
    m_3dView.setViewport(sf::FloatRect(0.f, 0.f, 0.6f, 1.f)); // 창의 왼쪽 60% 영역 사용

    // UI 렌더링 뷰 설정
    m_uiView.setSize(static_cast<float>(m_window.getSize().x) * 0.4f, static_cast<float>(m_window.getSize().y)); // 뷰 크기
    m_uiView.setCenter(m_uiView.getSize().x / 2.f, m_uiView.getSize().y / 2.f); // 뷰 중심
    m_uiView.setViewport(sf::FloatRect(0.6f, 0.f, 0.4f, 1.f)); // 창의 오른쪽 40% 영역 사용

    // 초기 설정 로드 및 UI, 3D 요소 설정
    loadSettingsFromFile("Setting_values.text"); // 설정 파일에서 값 로드
    setupUI();         // UI 요소 초기화
    setup3D();         // 3D 모델 기본 구조 초기화
    reconstructOpenings(); // 로드된 개구부 정보로 시각적 요소 생성
    projectVertices(); // 3D 정점 투영
    initializeDefaultSK(); // S, K 기본값 설정 및 입력창 업데이트
    
    // 초기 농도(C0) 입력창 설정 및 관련 변수 초기화
    m_inputC0.setText(sf::String(floatToWString(m_C0,1)).toAnsiString()); // C0 입력창에 기본값 표시
    m_C0 = m_inputC0.getFloatValue(); // 입력창 값으로 C0 설정 (실제로는 m_C0 기본값 사용)
    m_currentConcentration_Ct = m_C0; // 현재 농도를 C0로 초기화
    m_targetConcentration_Ct_for_particles = m_C0; // 파티클 목표 농도를 C0로 초기화

    adjustParticleCount(); // 초기 C0 값에 맞춰 파티클 수 조절
}

// SimulationScreen 클래스 소멸자 (현재 특별한 작업 없음)
SimulationScreen::~SimulationScreen() {}

// SimulationScreen 상태 초기화 함수 (화면 재진입 시 호출)
void SimulationScreen::reset() {
    m_running = true; // 화면 실행 상태로 설정
    m_nextState = ScreenState::SIMULATION; // 다음 화면 상태를 자기 자신으로 (유지)
    loadSettingsFromFile("Setting_values.text"); // 설정 파일 다시 로드
    setup3D();         // 3D 모델 재설정 (방 크기 변경 가능성 고려)
    reconstructOpenings(); // 개구부 시각 정보 재구성
    projectVertices(); // 3D 정점 재투영
    resetSimulationState(); // 시뮬레이션 관련 변수들 초기화
}

// "Setting_values.text" 파일에서 시뮬레이션 설정값 로드
void SimulationScreen::loadSettingsFromFile(const std::string& filename) {
    std::ifstream inFile(filename); // 파일 입력 스트림
    if (inFile.is_open()) { // 파일 열기 성공 시
        std::string line;
        // 파일 끝까지 한 줄씩 읽기
        while (std::getline(inFile, line)) {
            std::stringstream ss(line); // 문자열 스트림으로 변환
            std::string key, value;
            // ':' 기준으로 키와 값 분리
            if (std::getline(ss, key, ':') && std::getline(ss, value)) {
                try { // 문자열을 적절한 타입으로 변환
                    if (key == "width") m_roomWidth = std::stof(value);
                    else if (key == "depth") m_roomDepth = std::stof(value);
                    else if (key == "height") m_roomHeight = std::stof(value);
                    else if (key == "pollutant_index") m_selectedPollutantIndex = std::stoi(value);
                    else if (key == "passages_count") m_numPassages = std::stoi(value);
                    else if (key == "windows_count") m_numWindows = std::stoi(value);
                } catch (const std::invalid_argument& ia) { // 변환 실패 (잘못된 인수)
                    std::cerr << "Invalid argument parsing setting: " << key << ":" << value << " - " << ia.what() << std::endl;
                } catch (const std::out_of_range& oor) { // 변환 실패 (범위 초과)
                    std::cerr << "Out of range parsing setting: " << key << ":" << value << " - " << oor.what() << std::endl;
                }
            }
        }
        inFile.close(); // 파일 닫기
    } else { // 파일 열기 실패 시
        std::cerr << "Error: Could not open file to load settings: " << filename << ". Using defaults." << std::endl;
        // 생성자에서 설정된 기본값 유지됨
    }
    m_volumeV = m_roomWidth * m_roomDepth * m_roomHeight; // 방 부피 계산
    if (m_volumeV < 0.001f) m_volumeV = 0.001f; // 부피가 0 또는 음수 되는 것 방지

    // 선택된 오염물질 인덱스에 따라 파티클 색상 설정
    if (m_selectedPollutantIndex == 0) { // 미세먼지
        m_particleColor = sf::Color(200, 200, 200); // 밝은 회색 (알파는 개별 파티클에서 조절)
    } else if (m_selectedPollutantIndex == 1) { // 일산화탄소
        m_particleColor = sf::Color(100, 100, 100); // 짙은 회색
    } else { // 염소가스
        m_particleColor = sf::Color(70, 70, 180);   // 진한 파란색
    }
}

// 로드된 통로/창문 개수에 따라 3D 화면에 표시할 시각적 개구부 정보 생성
void SimulationScreen::reconstructOpenings() {
    m_passages_vis.clear(); // 기존 통로 시각 정보 제거
    m_windows_vis.clear();  // 기존 창문 시각 정보 제거
    // 개구부 상대 크기 계산 (정규화된 좌표계 기준)
    float p_h_norm = PASSAGE_RELATIVE_HEIGHT_FACTOR_SIM * 0.5f;
    float p_w_norm = PASSAGE_RELATIVE_WIDTH_FACTOR_SIM * 0.5f;
    float w_h_norm = WINDOW_RELATIVE_HEIGHT_FACTOR_SIM * 0.5f;
    float w_d_norm = WINDOW_RELATIVE_WIDTH_FACTOR_SIM * 0.5f;
    // 통로 생성 로직 (SettingScreen과 유사)
    if (m_numPassages >= 1) {
        SettingScreen::OpeningDefinition passage1;
        passage1.local_coords = {{{-p_w_norm, -p_h_norm, -0.5f}, { p_w_norm, -p_h_norm, -0.5f}, { p_w_norm,  p_h_norm, -0.5f}, {-p_w_norm,  p_h_norm, -0.5f}}};
        m_passages_vis.push_back(passage1);
    }
    if (m_numPassages >= 2) {
        SettingScreen::OpeningDefinition passage2;
        passage2.local_coords = {{{-p_w_norm, -p_h_norm,  0.5f}, { p_w_norm, -p_h_norm,  0.5f}, { p_w_norm,  p_h_norm,  0.5f}, {-p_w_norm,  p_h_norm,  0.5f}}};
        m_passages_vis.push_back(passage2);
    }
    // 창문 생성 로직 (SettingScreen과 유사)
    if (m_numWindows >= 1) {
        SettingScreen::OpeningDefinition window1;
         window1.local_coords = {{{-0.5f, -w_h_norm, -w_d_norm}, {-0.5f, -w_h_norm,  w_d_norm}, {-0.5f,  w_h_norm,  w_d_norm}, {-0.5f,  w_h_norm, -w_d_norm}}};
        m_windows_vis.push_back(window1);
    }
    if (m_numWindows >= 2) {
        SettingScreen::OpeningDefinition window2;
        window2.local_coords = {{{ 0.5f, -w_h_norm, -w_d_norm}, { 0.5f, -w_h_norm,  w_d_norm}, { 0.5f,  w_h_norm,  w_d_norm}, { 0.5f,  w_h_norm, -w_d_norm}}};
        m_windows_vis.push_back(window2);
    }
}

// UI 요소들 초기 설정 함수
void SimulationScreen::setupUI() {
    // UI 요소 배치 관련 변수 설정
    float currentY = 20.f; float inputHeight = 28.f; float spacing = 35.f;
    unsigned int charSize = 18; float titleCharSize = 28;
    float labelWidth = 130.f; float inputBoxWidth = 140.f; float gapBetweenLabelInput = 10.f;
    float maxUiElementWidth = labelWidth + gapBetweenLabelInput + inputBoxWidth;
    float rightPadding = 30.f; float uiX = m_uiView.getSize().x - maxUiElementWidth - rightPadding; 
    // 화면 제목 설정
    m_titleText.setFont(m_font); m_titleText.setString(L"시뮬레이션"); m_titleText.setCharacterSize(titleCharSize);
    m_titleText.setFillColor(sf::Color::White); sf::FloatRect titleBounds = m_titleText.getLocalBounds();
    m_titleText.setOrigin(std::round(titleBounds.left + titleBounds.width / 2.f), std::round(titleBounds.top));
    m_titleText.setPosition(std::round(uiX + maxUiElementWidth / 2.f), std::round(currentY)); currentY += titleCharSize + spacing * 0.8f; 
    // 입력 필드 설정 람다
    auto setupInputField = [&](sf::Text& label, InputBox& inputBox, const std::wstring& labelText, const std::string& defaultVal, const std::wstring& placeholder) {
        label.setFont(m_font); label.setString(labelText); label.setCharacterSize(charSize); label.setFillColor(sf::Color::White);
        sf::FloatRect labelBounds = label.getLocalBounds(); label.setOrigin(std::round(labelBounds.left), std::round(labelBounds.top + labelBounds.height / 2.f));
        label.setPosition(std::round(uiX), std::round(currentY + inputHeight / 2.f));
        inputBox.setup(m_font, sf::Vector2f(uiX + labelWidth + gapBetweenLabelInput, currentY), sf::Vector2f(inputBoxWidth, inputHeight), placeholder);
        inputBox.setText(defaultVal); currentY += spacing;
    };
    // 각 입력 필드 생성
    setupInputField(m_labelC0, m_inputC0, L"초기 농도 C0:", "100.0", L"예: 100.0");
    setupInputField(m_labelS, m_inputS, L"유입 속도 S:", "10.0", L"예: 10.0");
    setupInputField(m_labelK, m_inputK, L"제거 상수 K:", "0.1", L"예: 0.1"); currentY += spacing * 0.2f;
    // 정보 표시 필드 설정 람다
    auto setupDisplayField = [&](sf::Text& label, sf::Text& display, const std::wstring& labelText, const std::wstring& initialValText) {
        label.setFont(m_font); label.setString(labelText); label.setCharacterSize(charSize); label.setFillColor(sf::Color::White);
        sf::FloatRect labelBounds = label.getLocalBounds(); label.setOrigin(std::round(labelBounds.left), std::round(labelBounds.top + labelBounds.height / 2.f));
        label.setPosition(std::round(uiX), std::round(currentY + inputHeight / 2.f));
        display.setFont(m_font); display.setString(initialValText); display.setCharacterSize(charSize); display.setFillColor(sf::Color::White);
        sf::FloatRect displayBounds = display.getLocalBounds(); display.setOrigin(std::round(displayBounds.left + displayBounds.width), std::round(displayBounds.top + displayBounds.height / 2.f)); // 우측 정렬
        display.setPosition(std::round(uiX + labelWidth + gapBetweenLabelInput + inputBoxWidth), std::round(currentY + inputHeight / 2.f)); currentY += spacing;
    };
    // 각 정보 표시 필드 생성
    setupDisplayField(m_labelVolume, m_displayVolume, L"공간 부피 V (m³):", floatToWString(m_volumeV));
    setupDisplayField(m_labelTime, m_displayTime, L"시간 t (min):", floatToWString(m_currentTime_t, 0));
    setupDisplayField(m_labelConcentration, m_displayConcentration, L"현재 농도 C(t):", floatToWString(m_currentConcentration_Ct)); currentY += spacing * 0.5f; 
    // 버튼 너비 및 첫 번째 버튼 그룹 Y 위치
    float buttonWidth = (maxUiElementWidth - 10.f) / 2.f; float buttonY1 = currentY;
    // 버튼 설정 람다
    auto setupButtonLambda = [&](sf::Text& text, sf::RectangleShape& shape, const std::wstring& str, float yPos, float btnWidth, float btnXOffset = 0.f) {
        shape.setSize(sf::Vector2f(btnWidth, inputHeight)); shape.setPosition(std::round(uiX + btnXOffset), std::round(yPos)); shape.setOutlineThickness(1.f);
        text.setFont(m_font); text.setString(str); text.setCharacterSize(charSize - 2);
        sf::FloatRect textBounds = text.getLocalBounds(); text.setOrigin(std::round(textBounds.left + textBounds.width / 2.f), std::round(textBounds.top + textBounds.height / 2.f));
        text.setPosition(std::round(shape.getPosition().x + btnWidth / 2.f), std::round(yPos + inputHeight / 2.f));
    };
    // 버튼 생성
    setupButtonLambda(m_buttonRun, m_shapeRun, L"실행", buttonY1, buttonWidth);
    setupButtonLambda(m_buttonStop, m_shapeStop, L"중단", buttonY1, buttonWidth, buttonWidth + 10.f); currentY += spacing;
    float buttonY2 = currentY; // 두 번째 버튼 그룹 Y 위치
    setupButtonLambda(m_buttonReset, m_shapeReset, L"초기화", buttonY2, buttonWidth);
    setupButtonLambda(m_buttonBack, m_shapeBack, L"돌아가기", buttonY2, buttonWidth, buttonWidth + 10.f);
}

// 3D 육면체 모델의 기본 정점 및 모서리 정보 설정
void SimulationScreen::setup3D() { 
    m_cubeVertices = {{{-0.5f,-0.5f,-0.5f},{0.5f,-0.5f,-0.5f},{0.5f,0.5f,-0.5f},{-0.5f,0.5f,-0.5f},{-0.5f,-0.5f,0.5f},{0.5f,-0.5f,0.5f},{0.5f,0.5f,0.5f},{-0.5f,0.5f,0.5f}}};
    m_cubeEdges = {{0,1},{1,2},{2,3},{3,0},{4,5},{5,6},{6,7},{7,4},{0,4},{1,5},{2,6},{3,7}};
    m_transformedVertices.fill({}); // 변환된 정점 배열 초기화
}

// 3D 정점들을 현재 방 크기, 회전각에 따라 변환하는 함수
void SimulationScreen::projectVertices() { 
    float w=(m_roomWidth>0.01f)?m_roomWidth:0.01f; float d=(m_roomDepth>0.01f)?m_roomDepth:0.01f; float h=(m_roomHeight>0.01f)?m_roomHeight:0.01f;
    float maxDim=std::max({w,d,h,1.f}); float scaleFactor=350.f/maxDim; // 뷰 크기에 맞춘 스케일 팩터
    for(size_t i=0;i<m_cubeVertices.size();++i){
        const Vec3D& v_orig=m_cubeVertices[i]; Vec3D v_scaled; v_scaled.x=v_orig.x*w; v_scaled.y=v_orig.y*h; v_scaled.z=v_orig.z*d;
        // Y축 회전 및 X축 회전 적용
        float x_r_y=v_scaled.x*std::cos(m_rotationY)-v_scaled.z*std::sin(m_rotationY); float z_r_y=v_scaled.x*std::sin(m_rotationY)+v_scaled.z*std::cos(m_rotationY);
        float y_f=v_scaled.y*std::cos(m_rotationX)-z_r_y*std::sin(m_rotationX); float z_f=v_scaled.y*std::sin(m_rotationX)+z_r_y*std::cos(m_rotationX);
        m_transformedVertices[i]={x_r_y*scaleFactor,y_f*scaleFactor,z_f*scaleFactor}; // 최종 변환된 좌표 저장
    }
}

// 3D 좌표를 2D 화면 좌표로 투영하는 함수 (간단한 원근 투영)
sf::Vector2f SimulationScreen::project(const Vec3D& p) const { 
    sf::Vector2f viewCenter=m_3dView.getCenter(); // 3D 뷰의 중심
    float perspF=500.f/(500.f+p.z); // 원근 계수 (거리에 따라 크기 조절)
    return sf::Vector2f(std::round(p.x*perspF+viewCenter.x),std::round(p.y*perspF+viewCenter.y)); // 투영 및 뷰 중심으로 이동
}

// 선택된 오염물질 및 통로/창문 개수에 따라 S, K 기본값 설정 및 입력창 업데이트
void SimulationScreen::initializeDefaultSK() { 
    float base_S=0.f,base_K=0.f; // 오염물질별 기본 S, K 값
    // 선택된 오염물질 인덱스에 따라 기본값 설정
    switch(m_selectedPollutantIndex){case 0:base_S=BASE_S_PM10;base_K=BASE_K_PM10;break; case 1:base_S=BASE_S_CO;base_K=BASE_K_CO;break; case 2:base_S=BASE_S_CL2;base_K=BASE_K_CL2;break; default:std::cerr<<"Warning: Unknown pollutant index "<<m_selectedPollutantIndex<<std::endl;base_S=10.f;base_K=0.1f;}
    // 통로 및 창문 개수에 따른 조정량 반영
    m_S_param=base_S+(m_numPassages*S_ADJUST_PASSAGE)+(m_numWindows*S_ADJUST_WINDOW); 
    m_K_param=base_K+(m_numPassages*K_ADJUST_PASSAGE)+(m_numWindows*K_ADJUST_WINDOW);
    if(m_K_param<0.0001f)m_K_param=0.0001f; // K값이 0 또는 음수가 되지 않도록 최소값 보장
    // 계산된 S, K 값을 입력창에 표시
    m_inputS.setText(sf::String(floatToWString(m_S_param,2)).toAnsiString()); 
    m_inputK.setText(sf::String(floatToWString(m_K_param,3)).toAnsiString());
}

// 사용자 입력 처리 함수 (메인 루프에서 호출)
void SimulationScreen::handleInput() {
    sf::Event event; // SFML 이벤트 객체
    // 창에서 발생한 모든 이벤트 폴링
    while (m_window.pollEvent(event)) {
        // 창 닫기 이벤트
        if (event.type == sf::Event::Closed) {
            m_running = false; m_nextState = ScreenState::EXIT;
        }
        // ESC 키 누름 이벤트
        if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
            if (m_activeInputBox) { // 활성 입력창이 있으면 비활성화
                m_activeInputBox->setActive(false); m_activeInputBox = nullptr;
            } else { // 없으면 시작 화면으로
                m_running = false; m_nextState = ScreenState::START; 
            }
        }

        // 활성 입력창의 텍스트 입력 또는 키 입력 이벤트 처리
        if (m_activeInputBox && (event.type == sf::Event::TextEntered || event.type == sf::Event::KeyPressed) ) {
            handleInputBoxEvents(event); // 해당 입력창으로 이벤트 전달
            // 시뮬레이션 비활성 시에만 파라미터 업데이트
            if (!m_simulationActive) { 
                if (m_activeInputBox == &m_inputS) {
                    m_S_param = m_inputS.getFloatValue();
                } else if (m_activeInputBox == &m_inputK) {
                     m_K_param = m_inputK.getFloatValue();
                     if (m_K_param < 0.0001f) m_K_param = 0.0001f; // K 최소값 보장
                } else if (m_activeInputBox == &m_inputC0 && !m_simulationStartedOnce) { // C0는 시뮬레이션 시작 전에만 변경 가능
                    m_C0 = m_inputC0.getFloatValue();
                    if (m_C0 < 0.f) { // C0 음수 방지
                        m_C0 = 0.f; 
                        m_inputC0.setText(sf::String(floatToWString(m_C0,1)).toAnsiString()); // 입력창 값도 수정
                    }
                    m_currentConcentration_Ct = m_C0; // 현재 농도도 C0로 즉시 반영
                    m_targetConcentration_Ct_for_particles = m_C0; // 파티클 목표 농도도 C0로 즉시 반영
                }
            }
            continue; // 이벤트 처리 완료
        }
        
        // 마우스 버튼 누름 이벤트 처리
        if (event.type == sf::Event::MouseButtonPressed) { 
            if(event.mouseButton.button==sf::Mouse::Left){ // 왼쪽 마우스 버튼
                sf::Vector2f mousePosWindow=static_cast<sf::Vector2f>(sf::Mouse::getPosition(m_window)); // 창 기준 마우스 위치
                sf::Vector2f mousePosUI=m_window.mapPixelToCoords(sf::Mouse::getPosition(m_window),m_uiView); // UI 뷰 기준 마우스 위치
                InputBox* previouslyActive=m_activeInputBox; InputBox* clickedBox=nullptr; // 이전/새로 클릭된 입력창 포인터
                // 어떤 입력창이 클릭되었는지 확인 (C0는 시뮬레이션 시작 전만)
                if(m_inputC0.getGlobalBounds().contains(mousePosUI)&&!m_simulationStartedOnce)clickedBox=&m_inputC0;
                else if(m_inputS.getGlobalBounds().contains(mousePosUI))clickedBox=&m_inputS;
                else if(m_inputK.getGlobalBounds().contains(mousePosUI))clickedBox=&m_inputK;
                // 활성 입력창 변경 처리
                if(previouslyActive&&previouslyActive!=clickedBox){previouslyActive->setActive(false);}
                m_activeInputBox=clickedBox;
                if(m_activeInputBox){m_activeInputBox->setActive(true);}
                else{ // 입력창이 아닌 곳을 클릭했다면 버튼 클릭 확인
                    if(m_shapeRun.getGlobalBounds().contains(mousePosUI))runSimulation();
                    else if(m_shapeStop.getGlobalBounds().contains(mousePosUI))stopSimulation();
                    else if(m_shapeReset.getGlobalBounds().contains(mousePosUI))resetSimulationState();
                    else if(m_shapeBack.getGlobalBounds().contains(mousePosUI)){m_running=false;m_nextState=ScreenState::START;}
                }
                // 3D 뷰 영역 클릭 시 드래그 시작
                sf::FloatRect view3DR(m_3dView.getViewport().left*m_window.getSize().x,m_3dView.getViewport().top*m_window.getSize().y,m_3dView.getViewport().width*m_window.getSize().x,m_3dView.getViewport().height*m_window.getSize().y);
                if(view3DR.contains(mousePosWindow)&&!m_activeInputBox){m_isDragging=true;m_lastMousePos=sf::Mouse::getPosition(m_window);}
            }
        }
        // 마우스 버튼 뗌 이벤트 처리
        if (event.type == sf::Event::MouseButtonReleased) { if(event.mouseButton.button==sf::Mouse::Left)m_isDragging=false;}
        // 마우스 이동 이벤트 처리 (3D 뷰 회전)
        if (event.type == sf::Event::MouseMoved) { 
            if(m_isDragging&&!m_activeInputBox){ // 드래그 중이고 입력창 비활성 시
                sf::Vector2i currMPos=sf::Mouse::getPosition(m_window); float dx=static_cast<float>(currMPos.x-m_lastMousePos.x);
                m_rotationY+=dx*0.005f; m_lastMousePos=currMPos; projectVertices(); // 회전각 업데이트 및 3D 모델 재투영
            }
        }
    }
}

// 활성화된 입력 상자의 이벤트를 내부적으로 처리하는 함수
void SimulationScreen::handleInputBoxEvents(sf::Event event) { if(m_activeInputBox){m_activeInputBox->handleEvent(event);}}
// 마우스 위치에 따라 버튼의 호버 스타일을 업데이트하는 함수
void SimulationScreen::updateButtonHovers(const sf::Vector2f& mousePos) { 
    auto updateVis=[&](sf::Text&t,sf::RectangleShape&s,bool h){t.setFillColor(h?m_buttonTextColorHover:m_buttonTextColorNormal);s.setFillColor(h?m_buttonBgColorHover:m_buttonBgColorNormal);s.setOutlineColor(m_buttonOutlineColor);};
    updateVis(m_buttonRun,m_shapeRun,m_shapeRun.getGlobalBounds().contains(mousePos)); updateVis(m_buttonStop,m_shapeStop,m_shapeStop.getGlobalBounds().contains(mousePos));
    updateVis(m_buttonReset,m_shapeReset,m_shapeReset.getGlobalBounds().contains(mousePos)); updateVis(m_buttonBack,m_shapeBack,m_shapeBack.getGlobalBounds().contains(mousePos));
}

// 화면 상태 업데이트 함수 (매 프레임 호출)
void SimulationScreen::update(sf::Time dt) {
    // 입력창들 상태 업데이트 (커서 등)
    m_inputC0.update();
    m_inputS.update();
    m_inputK.update();

    // 시뮬레이션 시간 진행 및 농도 계산
    if (m_simulationActive) { // 시뮬레이션 실행 중일 때
        m_simulationTimeStepAccumulator += dt.asSeconds(); // 실제 시간 누적
        if (m_simulationTimeStepAccumulator >= 1.0f) { // 1초 경과 시
            m_currentTime_t += 1.0f; // 시뮬레이션 시간 1분 증가
            calculateCurrentConcentration(); // 현재 농도 재계산
            m_targetConcentration_Ct_for_particles = m_currentConcentration_Ct; // 파티클 목표 농도 업데이트
            m_simulationTimeStepAccumulator -= 1.0f; // 누적 시간에서 1초 차감
        }
    } else { // 시뮬레이션 비실행 시 (예: C0 입력 중)
        // 파티클 목표 농도는 현재 입력/설정된 농도(m_currentConcentration_Ct, 시뮬레이션 전에는 C0와 동일)를 따름
        m_targetConcentration_Ct_for_particles = m_currentConcentration_Ct;
    }
    
    adjustParticleCount();    // 목표 농도에 맞춰 파티클 수 점진적 조절
    updateParticleSystem(dt); // 파티클 이동, 수명, 알파 등 업데이트

    // UI 정보 표시 텍스트 업데이트
    m_displayVolume.setString(floatToWString(m_volumeV)); // 부피 표시
    sf::FloatRect volBounds = m_displayVolume.getLocalBounds();
    m_displayVolume.setOrigin(std::round(volBounds.left + volBounds.width), std::round(volBounds.top + volBounds.height / 2.f)); // 우측 정렬
    
    m_displayTime.setString(floatToWString(m_currentTime_t, 0)); // 시간 표시 (정수)
    sf::FloatRect timeBounds = m_displayTime.getLocalBounds();
    m_displayTime.setOrigin(std::round(timeBounds.left + timeBounds.width), std::round(timeBounds.top + timeBounds.height / 2.f));

    m_displayConcentration.setString(floatToWString(m_currentConcentration_Ct)); // 현재 농도 표시
    sf::FloatRect concBounds = m_displayConcentration.getLocalBounds();
    m_displayConcentration.setOrigin(std::round(concBounds.left + concBounds.width), std::round(concBounds.top + concBounds.height / 2.f));

    // 버튼 호버 효과 업데이트
    sf::Vector2f mousePosUI = m_window.mapPixelToCoords(sf::Mouse::getPosition(m_window), m_uiView);
    updateButtonHovers(mousePosUI);
}

// 농도 공식에 따라 현재 시간 t에서의 농도 C(t) 계산
void SimulationScreen::calculateCurrentConcentration() { 
    // 0으로 나누기 방지 (K 또는 V가 0일 경우). 현재는 K와 V의 최소값이 설정되어 있어 이 조건은 거의 발생 안 함.
    if (m_K_param * m_volumeV == 0) { 
        if (m_S_param == 0) { 
            // S=0, K=0이면 농도 변화 없음. C(t) = C0
        } else { 
            // S!=0, K=0이면 C(t) = C0 + (S/V)*t. 이 공식은 K!=0 가정.
        }
    }
    // 농도 공식 적용: C(t) = (C0 - S/(k*V)) * exp(-k*t) + S/(k*V)
    float term_S_kV = m_S_param / (m_K_param * m_volumeV);
    m_currentConcentration_Ct = (m_C0 - term_S_kV) * std::exp(-m_K_param * m_currentTime_t) + term_S_kV;
    if (m_currentConcentration_Ct < 0.f) m_currentConcentration_Ct = 0.f; // 농도는 음수가 될 수 없음
}

// "실행" 버튼 클릭 시 호출: 시뮬레이션 시작
void SimulationScreen::runSimulation() {
    if (!m_simulationStartedOnce) { // 최초 실행 시
        m_C0 = m_inputC0.getFloatValue(); // C0 입력창에서 값 확정
        if (m_C0 < 0.f) m_C0 = 0.f; // 음수 방지
        m_inputC0.setText(sf::String(floatToWString(m_C0,1)).toAnsiString()); // 입력창 값 보정 표시
        m_simulationStartedOnce = true; // 실행 플래그 설정 (C0 고정)
        m_currentConcentration_Ct = m_C0; // 현재 농도를 C0로
        m_targetConcentration_Ct_for_particles = m_C0; // 파티클 목표 농도도 C0로
    }
    // 현재 S, K 입력창 값으로 파라미터 업데이트
    m_S_param = m_inputS.getFloatValue();
    m_K_param = m_inputK.getFloatValue();
    if (m_K_param < 0.0001f) { // K 최소값 보장
        m_K_param = 0.0001f; 
        m_inputK.setText(sf::String(floatToWString(m_K_param,3)).toAnsiString()); // 입력창 값 보정 표시
    }

    m_simulationActive = true; // 시뮬레이션 활성화
    // 만약 시뮬레이션이 초기 상태(시간 0)에서 시작하거나 리셋 후 첫 실행이면
    if (m_currentTime_t == 0.0f) { 
         m_currentConcentration_Ct = m_C0; // 현재 농도는 C0
         m_targetConcentration_Ct_for_particles = m_C0; // 파티클 목표 농도도 C0
    } else { // 중단했다가 재개하는 경우
        m_targetConcentration_Ct_for_particles = m_currentConcentration_Ct; // 현재 농도를 파티클 목표 농도로
    }
}

// "중단" 버튼 클릭 시 호출: 시뮬레이션 일시 중지
void SimulationScreen::stopSimulation() {
    m_simulationActive = false; // 시뮬레이션 비활성화
}

// "초기화" 버튼 클릭 시 호출: 시뮬레이션 상태 전체 초기화
void SimulationScreen::resetSimulationState() {
    stopSimulation(); // 시뮬레이션 중단
    m_simulationStartedOnce = false; // C0 다시 입력 가능하도록 플래그 리셋
    m_currentTime_t = 0.0f; // 시간 초기화
    m_simulationTimeStepAccumulator = 0.0f; // 시간 누적기 초기화
    
    initializeDefaultSK(); // S, K 값을 오염물질 및 개구부 기본값으로 되돌림

    // C0 입력창 및 값 초기화 (기본값 100.0)
    m_inputC0.setText(sf::String(floatToWString(100.0f, 1)).toAnsiString());
    m_C0 = m_inputC0.getFloatValue(); // setText 후 getFloatValue로 m_C0도 업데이트
    m_currentConcentration_Ct = m_C0; // 현재 농도도 C0로
    m_targetConcentration_Ct_for_particles = m_C0; // 파티클 목표 농도도 C0로

    m_particles.clear(); // 모든 파티클 제거
    adjustParticleCount(); // 초기 C0에 맞는 파티클 다시 생성 (점진적)

    projectVertices(); // 3D 모델 재투영 (필요시)
}

// 새로운 단일 파티클 생성 및 초기화
void SimulationScreen::spawnNewParticle() {
    // 난수 생성기 (static으로 선언하여 매번 재생성 방지)
    static std::random_device rd;
    static std::mt19937 gen(rd());
    // 파티클 초기 위치, 속도, 수명 다양성을 위한 분포 정의
    std::uniform_real_distribution<float> distrib_pos(-0.49f, 0.49f); // 정규화된 위치 (-0.5 ~ 0.5 안쪽)
    std::uniform_real_distribution<float> distrib_vel(-0.02f, 0.02f); // 정규화된 속도
    std::uniform_real_distribution<float> distrib_lifetime_factor(0.5f, 1.0f); // 수명 계수 (0.5 ~ 1.0)

    Particle p; // 새 파티클 객체
    p.position3D = {distrib_pos(gen), distrib_pos(gen), distrib_pos(gen)}; // 랜덤 3D 위치
    p.velocity = {distrib_vel(gen), distrib_vel(gen), distrib_vel(gen)};   // 랜덤 3D 속도
    p.shape.setRadius(2.f); // 파티클 시각적 크기
    p.shape.setOrigin(p.shape.getRadius(), p.shape.getRadius()); // 원점 중앙 설정
    p.currentAlpha = 255.f; // 초기 투명도 (불투명)
    p.lifetime = PARTICLE_MAX_LIFETIME * distrib_lifetime_factor(gen); // 랜덤 수명 설정
    
    m_particles.push_back(p); // 파티클 리스트에 추가
}

// 목표 농도에 맞춰 파티클 수를 점진적으로 조절하는 함수
void SimulationScreen::adjustParticleCount() {
    // 파티클 수 계산을 위한 기준 농도 설정 (C0 또는 현재 목표 농도 중 더 적절한 값)
    float C_ref = std::max(m_C0, 1.0f); // C0가 0일 경우 대비 최소 1.0 사용
    // 만약 목표 농도가 C0보다 훨씬 높다면, 기준 농도를 목표 농도로 업데이트 (스케일링 왜곡 방지)
    if (m_targetConcentration_Ct_for_particles > C_ref * 1.5f) { 
        C_ref = m_targetConcentration_Ct_for_particles; 
    }

    // 목표 파티클 수 계산 (최대 파티클 수에 비례)
    int targetParticleCount = 0;
    if (C_ref > 0.001f) { // 0으로 나누기 방지
        targetParticleCount = static_cast<int>(static_cast<float>(m_maxParticles) * (m_targetConcentration_Ct_for_particles / C_ref));
    }
    targetParticleCount = std::min(targetParticleCount, m_maxParticles); // 최대값 제한
    targetParticleCount = std::max(targetParticleCount, 0);             // 최소값 0 보장

    // 현재 파티클 수와 목표 파티클 수의 차이 계산
    int currentParticleCount = m_particles.size();
    int diff = targetParticleCount - currentParticleCount;

    if (diff > 0) { // 파티클 추가 필요
        // 프레임당 조절량(PARTICLES_PER_FRAME_ADJUST)만큼, 또는 필요한 만큼만 추가
        for (int i = 0; i < std::min(diff, PARTICLES_PER_FRAME_ADJUST); ++i) {
            if (m_particles.size() < static_cast<size_t>(m_maxParticles)) { // 최대 파티클 수 넘지 않도록
                spawnNewParticle(); // 새 파티클 생성
            }
        }
    } else if (diff < 0) { // 파티클 제거 필요
        // 프레임당 조절량만큼, 또는 필요한 만큼만 제거 (가장 오래된 파티클의 수명을 줄여 빠르게 소멸 유도)
        for (int i = 0; i < std::min(-diff, PARTICLES_PER_FRAME_ADJUST); ++i) {
            if (!m_particles.empty()) {
                // 리스트의 맨 앞에 있는 (가장 먼저 생성된) 파티클의 수명을 짧게 만듦
                m_particles.front().lifetime = std::min(m_particles.front().lifetime, 0.1f); 
            }
        }
    }
}

// 파티클 시스템 업데이트 (이동, 수명, 알파값 등)
void SimulationScreen::updateParticleSystem(sf::Time dt) {
    float w_half_norm = 0.5f; float h_half_norm = 0.5f; float d_half_norm = 0.5f; // 정규화된 방 경계
    float deltaTime = dt.asSeconds(); // 경과 시간 (초)

    // 모든 파티클에 대해 반복 (리스트 순회 중 삭제 가능하도록 반복자 사용)
    for (auto it = m_particles.begin(); it != m_particles.end(); ) {
        Particle& p = *it; // 현재 파티클 참조
        // 속도에 따라 위치 업데이트
        p.position3D.x += p.velocity.x * deltaTime;
        p.position3D.y += p.velocity.y * deltaTime;
        p.position3D.z += p.velocity.z * deltaTime;

        // 방 경계 처리 (순환)
        if (p.position3D.x > w_half_norm) p.position3D.x -= 2.f * w_half_norm;
        else if (p.position3D.x < -w_half_norm) p.position3D.x += 2.f * w_half_norm;
        if (p.position3D.y > h_half_norm) p.position3D.y -= 2.f * h_half_norm;
        else if (p.position3D.y < -h_half_norm) p.position3D.y += 2.f * h_half_norm;
        if (p.position3D.z > d_half_norm) p.position3D.z -= 2.f * d_half_norm;
        else if (p.position3D.z < -d_half_norm) p.position3D.z += 2.f * d_half_norm;

        // 파티클 수명 감소
        p.lifetime -= deltaTime;
        
        // 수명이 다 되면 알파값(투명도) 감소 시작
        if (p.lifetime <= 0.f) {
            p.currentAlpha -= PARTICLE_FADE_RATE * deltaTime; 
        }
        p.currentAlpha = std::max(0.f, p.currentAlpha); // 알파값은 0 이상으로 유지

        // 알파값이 0 이하가 되면 파티클 제거
        if (p.currentAlpha <= 0.f) { 
            it = m_particles.erase(it); // 리스트에서 제거하고 다음 유효한 반복자 반환
        } else {
            ++it; // 다음 파티클로 이동
        }
    }
}

// 화면 렌더링 함수
void SimulationScreen::render() {
    m_window.clear(sf::Color::Black); // 검은색으로 화면 지우기

    // 3D 뷰 렌더링
    m_window.setView(m_3dView);      // 3D 뷰 활성화
    drawCuboidEdges(m_window);       // 육면체 모서리 그리기
    drawOpeningsVisual(m_window);    // 통로/창문 그리기

    // 3D 파티클 렌더링
    float maxDim = std::max({m_roomWidth, m_roomDepth, m_roomHeight, 1.f}); // 방의 최대 차원
    float scaleFactor3D = 350.f / maxDim; // 3D 뷰 스케일 팩터

    for (const auto& p : m_particles) { // 모든 파티클에 대해
        // if (p.currentAlpha == 0.f) continue; // 이미 updateParticleSystem에서 제거되므로 불필요

        // 파티클의 정규화된 3D 위치를 실제 방 크기 기준으로 변환
        Vec3D v_world_scaled;
        v_world_scaled.x = p.position3D.x * m_roomWidth;
        v_world_scaled.y = p.position3D.y * m_roomHeight; 
        v_world_scaled.z = p.position3D.z * m_roomDepth;

        // 시점 변환 (회전) 적용
        float x_rot_y = v_world_scaled.x * std::cos(m_rotationY) - v_world_scaled.z * std::sin(m_rotationY);
        float z_rot_y = v_world_scaled.x * std::sin(m_rotationY) + v_world_scaled.z * std::cos(m_rotationY);
        float y_final = v_world_scaled.y * std::cos(m_rotationX) - z_rot_y * std::sin(m_rotationX);
        float z_final = v_world_scaled.y * std::sin(m_rotationX) + z_rot_y * std::cos(m_rotationX);
        
        // 3D 뷰 스케일 적용
        Vec3D v_transformed_for_projection = {
            x_rot_y * scaleFactor3D, y_final * scaleFactor3D, z_final * scaleFactor3D
        };
        
        // 2D 화면 좌표로 투영
        sf::Vector2f screenPos = project(v_transformed_for_projection);
        
        sf::CircleShape particleShape = p.shape; // 파티클 모양 복사 (색상, 알파 등 변경 위함)
        particleShape.setPosition(screenPos);    // 화면 위치 설정
        
        // 깊이에 따른 원근 효과 (크기 및 투명도 조절)
        float depthPerspectiveFactor = 500.f / (500.f + v_transformed_for_projection.z); // 깊이 계수
        depthPerspectiveFactor = std::max(0.2f, std::min(1.f, depthPerspectiveFactor)); // 계수 범위 제한 (0.2 ~ 1.0)
        
        particleShape.setScale(depthPerspectiveFactor, depthPerspectiveFactor); // 크기 조절
        
        sf::Color finalColor = m_particleColor; // 기본 오염물질 색상
        // 최종 알파값 = 현재 파티클 알파 * 깊이 계수
        finalColor.a = static_cast<sf::Uint8>(p.currentAlpha * depthPerspectiveFactor); 
        particleShape.setFillColor(finalColor); // 최종 색상(투명도 포함) 설정

        m_window.draw(particleShape); // 파티클 그리기
    }

    // UI 뷰 렌더링
    m_window.setView(m_uiView); // UI 뷰 활성화
    // UI 요소들 그리기
    m_window.draw(m_titleText);
    m_inputC0.render(m_window); m_window.draw(m_labelC0);
    m_inputS.render(m_window); m_window.draw(m_labelS);
    m_inputK.render(m_window); m_window.draw(m_labelK);
    m_window.draw(m_labelVolume); m_window.draw(m_displayVolume);
    m_window.draw(m_labelTime); m_window.draw(m_displayTime);
    m_window.draw(m_labelConcentration); m_window.draw(m_displayConcentration);
    m_window.draw(m_shapeRun); m_window.draw(m_buttonRun);
    m_window.draw(m_shapeStop); m_window.draw(m_buttonStop);
    m_window.draw(m_shapeReset); m_window.draw(m_buttonReset);
    m_window.draw(m_shapeBack); m_window.draw(m_buttonBack);

    m_window.setView(m_window.getDefaultView()); // 뷰를 기본값으로 복원
    m_window.display(); // 그려진 내용 화면에 최종 표시
}

// 3D 육면체 모서리 그리기 함수
void SimulationScreen::drawCuboidEdges(sf::RenderWindow& window) { 
    for(const auto& edge : m_cubeEdges){ // 모든 모서리에 대해
        // 모서리의 두 끝점 투영
        sf::Vector2f p1_s=project(m_transformedVertices[edge.start]); 
        sf::Vector2f p2_s=project(m_transformedVertices[edge.end]);
        // 선분으로 그리기
        sf::Vertex line[]={sf::Vertex(p1_s,sf::Color::White),sf::Vertex(p2_s,sf::Color::White)}; 
        window.draw(line,2,sf::Lines);
    }
}
// 3D 개구부(통로/창문) 그리기 함수
void SimulationScreen::drawOpeningsVisual(sf::RenderWindow& window) { 
    // 단일 개구부 그리기 람다
    auto draw_one_s=[&](const SettingScreen::OpeningDefinition& def){
        std::array<sf::Vector2f,4>scr_pts; // 화면 좌표 저장 배열
        // 방 크기 및 뷰 스케일 팩터
        float w=(m_roomWidth>0.1f)?m_roomWidth:0.1f;float d=(m_roomDepth>0.1f)?m_roomDepth:0.1f;float h=(m_roomHeight>0.1f)?m_roomHeight:0.1f;
        float maxD=std::max({w,d,h,1.f});float vSF=350.f/maxD;
        // 개구부의 4개 정점 변환 및 투영
        for(size_t i=0;i<4;++i){
            const Vec3D& vln=def.local_coords[i];Vec3D vws;vws.x=vln.x*w;vws.y=vln.y*h;vws.z=vln.z*d;
            float xry=vws.x*std::cos(m_rotationY)-vws.z*std::sin(m_rotationY);float zry=vws.x*std::sin(m_rotationY)+vws.z*std::cos(m_rotationY);
            float yr=vws.y*std::cos(m_rotationX)-zry*std::sin(m_rotationX);float zrf=vws.y*std::sin(m_rotationX)+zry*std::cos(m_rotationX);
            Vec3D vtfp={xry*vSF,yr*vSF,zrf*vSF};scr_pts[i]=project(vtfp);
        }
        // 투영된 점들로 선분(모서리) 그리기
        for(size_t i=0;i<4;++i){sf::Vertex line[]={sf::Vertex(scr_pts[i],sf::Color::Cyan),sf::Vertex(scr_pts[(i+1)%4],sf::Color::Cyan)};window.draw(line,2,sf::Lines);}
    };
    // 모든 통로 및 창문 그리기
    for(const auto&pass_def:m_passages_vis){draw_one_s(pass_def);} 
    for(const auto&win_def:m_windows_vis){draw_one_s(win_def);}
}

// 다음 화면 상태 반환
ScreenState SimulationScreen::getNextState() const { return m_nextState; }
// 현재 화면 실행 여부 반환
bool SimulationScreen::isRunning() const { return m_running; }
// 다음 화면 상태 설정
void SimulationScreen::setNextState(ScreenState state) { m_nextState = state; }