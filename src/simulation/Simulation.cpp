#include "Simulation.hpp"
#include <cmath>
#include <iomanip>
#include <sstream>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <random>

const float SimulationScreen::BASE_S_PM10 = 10.0f; const float SimulationScreen::BASE_K_PM10 = 0.05f;
const float SimulationScreen::BASE_S_CO = 20.0f;   const float SimulationScreen::BASE_K_CO = 0.02f;
const float SimulationScreen::BASE_S_CL2 = 5.0f;   const float SimulationScreen::BASE_K_CL2 = 0.1f;
const float SimulationScreen::S_ADJUST_PASSAGE = 2.0f; const float SimulationScreen::K_ADJUST_PASSAGE = 0.01f;
const float SimulationScreen::S_ADJUST_WINDOW = 1.0f;  const float SimulationScreen::K_ADJUST_WINDOW = 0.03f;

const float SimulationScreen::PASSAGE_RELATIVE_HEIGHT_FACTOR_SIM = 0.7f;
const float SimulationScreen::PASSAGE_RELATIVE_WIDTH_FACTOR_SIM = 0.25f;
const float SimulationScreen::WINDOW_RELATIVE_HEIGHT_FACTOR_SIM = 0.5f;
const float SimulationScreen::WINDOW_RELATIVE_WIDTH_FACTOR_SIM = 0.4f;

std::wstring floatToWString(float value, int precision = 2) {
    std::wstringstream wss;
    wss << std::fixed << std::setprecision(precision) << value;
    return wss.str();
}

SimulationScreen::SimulationScreen(sf::RenderWindow& window, sf::Font& font)
    : m_window(window), m_font(font),
      m_nextState(ScreenState::SIMULATION), m_running(true),
      m_rotationX(25.f * PI / 180.f),
      m_rotationY(-35.f * PI / 180.f),
      m_isDragging(false), m_activeInputBox(nullptr),
      m_simulationActive(false), m_simulationStartedOnce(false),
      m_currentTime_t(0.0f), m_currentConcentration_Ct(0.0f),
      m_C0(100.0f), m_S_param(0.0f), m_K_param(0.0f),
      m_roomWidth(5.f), m_roomDepth(5.f), m_roomHeight(3.f),
      m_selectedPollutantIndex(0), m_numPassages(0), m_numWindows(0),
      m_maxParticles(500), m_particleUpdateTimer(0.0f) {

    m_buttonTextColorNormal = sf::Color::White;
    m_buttonTextColorHover = sf::Color::Black;
    m_buttonBgColorNormal = sf::Color(80,80,80);
    m_buttonBgColorHover = sf::Color::White;
    m_buttonOutlineColor = sf::Color::White;

    m_3dView.setSize(static_cast<float>(m_window.getSize().x) * 0.6f, static_cast<float>(m_window.getSize().y));
    m_3dView.setCenter(m_3dView.getSize().x / 2.f, m_3dView.getSize().y / 2.f - 50.f);
    m_3dView.setViewport(sf::FloatRect(0.f, 0.f, 0.6f, 1.f));

    m_uiView.setSize(static_cast<float>(m_window.getSize().x) * 0.4f, static_cast<float>(m_window.getSize().y));
    m_uiView.setCenter(m_uiView.getSize().x / 2.f, m_uiView.getSize().y / 2.f);
    m_uiView.setViewport(sf::FloatRect(0.6f, 0.f, 0.4f, 1.f));

    loadSettingsFromFile("Setting_values.text");
    setupUI();
    setup3D();
    reconstructOpenings();
    projectVertices();
    initializeDefaultSK();
    
    m_inputC0.setText(sf::String(floatToWString(m_C0,1)).toAnsiString());
    m_C0 = m_inputC0.getFloatValue(); 
    m_currentConcentration_Ct = m_C0;

    spawnParticles();
}

SimulationScreen::~SimulationScreen() {}

void SimulationScreen::reset() {
    m_running = true;
    m_nextState = ScreenState::SIMULATION;
    loadSettingsFromFile("Setting_values.text");
    setup3D();
    reconstructOpenings();
    projectVertices();
    resetSimulationState();
}

void SimulationScreen::loadSettingsFromFile(const std::string& filename) {
    std::ifstream inFile(filename);
    if (inFile.is_open()) {
        std::string line;
        while (std::getline(inFile, line)) {
            std::stringstream ss(line);
            std::string key, value;
            if (std::getline(ss, key, ':') && std::getline(ss, value)) {
                try {
                    if (key == "width") m_roomWidth = std::stof(value);
                    else if (key == "depth") m_roomDepth = std::stof(value);
                    else if (key == "height") m_roomHeight = std::stof(value);
                    else if (key == "pollutant_index") m_selectedPollutantIndex = std::stoi(value);
                    else if (key == "passages_count") m_numPassages = std::stoi(value);
                    else if (key == "windows_count") m_numWindows = std::stoi(value);
                } catch (const std::invalid_argument& ia) {
                    std::cerr << "Invalid argument parsing setting: " << key << ":" << value << " - " << ia.what() << std::endl;
                } catch (const std::out_of_range& oor) {
                    std::cerr << "Out of range parsing setting: " << key << ":" << value << " - " << oor.what() << std::endl;
                }
            }
        }
        inFile.close();
    } else {
        std::cerr << "Error: Could not open file to load settings: " << filename << ". Using defaults." << std::endl;
    }
    m_volumeV = m_roomWidth * m_roomDepth * m_roomHeight;
    if (m_volumeV < 0.001f) m_volumeV = 0.001f;

    if (m_selectedPollutantIndex == 0) {
        m_particleColor = sf::Color(200, 200, 200, 150);
    } else if (m_selectedPollutantIndex == 1) {
        m_particleColor = sf::Color(100, 100, 100, 180);
    } else {
        m_particleColor = sf::Color(70, 70, 180, 150);
    }
}

void SimulationScreen::reconstructOpenings() {
    m_passages_vis.clear();
    m_windows_vis.clear();

    float p_h_norm = PASSAGE_RELATIVE_HEIGHT_FACTOR_SIM * 0.5f;
    float p_w_norm = PASSAGE_RELATIVE_WIDTH_FACTOR_SIM * 0.5f;
    float w_h_norm = WINDOW_RELATIVE_HEIGHT_FACTOR_SIM * 0.5f;
    float w_d_norm = WINDOW_RELATIVE_WIDTH_FACTOR_SIM * 0.5f;

    if (m_numPassages >= 1) {
        SettingScreen::OpeningDefinition passage1;
        passage1.local_coords = {{
            {-p_w_norm, -p_h_norm, -0.5f}, { p_w_norm, -p_h_norm, -0.5f},
            { p_w_norm,  p_h_norm, -0.5f}, {-p_w_norm,  p_h_norm, -0.5f}
        }};
        m_passages_vis.push_back(passage1);
    }
    if (m_numPassages >= 2) {
        SettingScreen::OpeningDefinition passage2;
        passage2.local_coords = {{
            {-p_w_norm, -p_h_norm,  0.5f}, { p_w_norm, -p_h_norm,  0.5f},
            { p_w_norm,  p_h_norm,  0.5f}, {-p_w_norm,  p_h_norm,  0.5f}
        }};
        m_passages_vis.push_back(passage2);
    }

    if (m_numWindows >= 1) {
        SettingScreen::OpeningDefinition window1;
         window1.local_coords = {{
            {-0.5f, -w_h_norm, -w_d_norm}, {-0.5f, -w_h_norm,  w_d_norm},
            {-0.5f,  w_h_norm,  w_d_norm}, {-0.5f,  w_h_norm, -w_d_norm}
        }};
        m_windows_vis.push_back(window1);
    }
    if (m_numWindows >= 2) {
        SettingScreen::OpeningDefinition window2;
        window2.local_coords = {{
            { 0.5f, -w_h_norm, -w_d_norm}, { 0.5f, -w_h_norm,  w_d_norm},
            { 0.5f,  w_h_norm,  w_d_norm}, { 0.5f,  w_h_norm, -w_d_norm}
        }};
        m_windows_vis.push_back(window2);
    }
}


void SimulationScreen::setupUI() {
    float currentY = 20.f;
    float inputHeight = 28.f;
    float spacing = 35.f;
    unsigned int charSize = 18;
    float titleCharSize = 28;

    float labelWidth = 130.f; 
    float inputBoxWidth = 140.f; 
    float gapBetweenLabelInput = 10.f;
    float maxUiElementWidth = labelWidth + gapBetweenLabelInput + inputBoxWidth;
    float rightPadding = 30.f; 
    float uiX = m_uiView.getSize().x - maxUiElementWidth - rightPadding; 

    m_titleText.setFont(m_font);
    m_titleText.setString(L"시뮬레이션");
    m_titleText.setCharacterSize(titleCharSize);
    m_titleText.setFillColor(sf::Color::White);
    sf::FloatRect titleBounds = m_titleText.getLocalBounds();
    m_titleText.setOrigin(std::round(titleBounds.left + titleBounds.width / 2.f), std::round(titleBounds.top));
    m_titleText.setPosition(std::round(uiX + maxUiElementWidth / 2.f), std::round(currentY));
    currentY += titleCharSize + spacing * 0.8f; 

    auto setupInputField = [&](sf::Text& label, InputBox& inputBox, const std::wstring& labelText, const std::string& defaultVal, const std::wstring& placeholder) {
        label.setFont(m_font);
        label.setString(labelText);
        label.setCharacterSize(charSize);
        label.setFillColor(sf::Color::White);
        sf::FloatRect labelBounds = label.getLocalBounds();
        label.setOrigin(std::round(labelBounds.left), std::round(labelBounds.top + labelBounds.height / 2.f));
        label.setPosition(std::round(uiX), std::round(currentY + inputHeight / 2.f));

        inputBox.setup(m_font, sf::Vector2f(uiX + labelWidth + gapBetweenLabelInput, currentY), sf::Vector2f(inputBoxWidth, inputHeight), placeholder);
        inputBox.setText(defaultVal);
        currentY += spacing;
    };
    
    setupInputField(m_labelC0, m_inputC0, L"초기 농도 C0:", "100.0", L"예: 100.0");
    setupInputField(m_labelS, m_inputS, L"유입 속도 S:", "10.0", L"예: 10.0");
    setupInputField(m_labelK, m_inputK, L"제거 상수 K:", "0.1", L"예: 0.1");
    currentY += spacing * 0.2f;

    auto setupDisplayField = [&](sf::Text& label, sf::Text& display, const std::wstring& labelText, const std::wstring& initialValText) {
        label.setFont(m_font);
        label.setString(labelText);
        label.setCharacterSize(charSize);
        label.setFillColor(sf::Color::White);
        sf::FloatRect labelBounds = label.getLocalBounds();
        label.setOrigin(std::round(labelBounds.left), std::round(labelBounds.top + labelBounds.height / 2.f));
        label.setPosition(std::round(uiX), std::round(currentY + inputHeight / 2.f));
        
        display.setFont(m_font);
        display.setString(initialValText);
        display.setCharacterSize(charSize);
        display.setFillColor(sf::Color::White);
        sf::FloatRect displayBounds = display.getLocalBounds();
        display.setOrigin(std::round(displayBounds.left + displayBounds.width), std::round(displayBounds.top + displayBounds.height / 2.f));
        display.setPosition(std::round(uiX + labelWidth + gapBetweenLabelInput + inputBoxWidth), std::round(currentY + inputHeight / 2.f));
        currentY += spacing;
    };

    setupDisplayField(m_labelVolume, m_displayVolume, L"공간 부피 V (m³):", floatToWString(m_volumeV));
    setupDisplayField(m_labelTime, m_displayTime, L"시간 t (min):", floatToWString(m_currentTime_t, 0));
    setupDisplayField(m_labelConcentration, m_displayConcentration, L"현재 농도 C(t):", floatToWString(m_currentConcentration_Ct));
    currentY += spacing * 0.5f; 

    float buttonWidth = (maxUiElementWidth - 10.f) / 2.f;
    float buttonY1 = currentY;
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

    setupButtonLambda(m_buttonRun, m_shapeRun, L"실행", buttonY1, buttonWidth);
    setupButtonLambda(m_buttonStop, m_shapeStop, L"중단", buttonY1, buttonWidth, buttonWidth + 10.f);
    currentY += spacing;
    
    float buttonY2 = currentY;
    setupButtonLambda(m_buttonReset, m_shapeReset, L"초기화", buttonY2, buttonWidth);
    setupButtonLambda(m_buttonBack, m_shapeBack, L"돌아가기", buttonY2, buttonWidth, buttonWidth + 10.f);
}

void SimulationScreen::setup3D() {
    m_cubeVertices = {{
        {-0.5f, -0.5f, -0.5f}, {0.5f, -0.5f, -0.5f}, {0.5f, 0.5f, -0.5f}, {-0.5f, 0.5f, -0.5f}, 
        {-0.5f, -0.5f, 0.5f}, {0.5f, -0.5f, 0.5f}, {0.5f, 0.5f, 0.5f}, {-0.5f, 0.5f, 0.5f}    
    }};
    m_cubeEdges = {
        {0,1}, {1,2}, {2,3}, {3,0}, {4,5}, {5,6}, {6,7}, {7,4}, 
        {0,4}, {1,5}, {2,6}, {3,7}  
    };
    m_transformedVertices.fill({});
}

void SimulationScreen::projectVertices() {
    float w = (m_roomWidth > 0.01f) ? m_roomWidth : 0.01f;
    float d = (m_roomDepth > 0.01f) ? m_roomDepth : 0.01f;
    float h = (m_roomHeight > 0.01f) ? m_roomHeight : 0.01f;

    float maxDim = std::max({w, d, h, 1.f}); 
    float scaleFactor = 350.f / maxDim;

    for (size_t i = 0; i < m_cubeVertices.size(); ++i) {
        const Vec3D& v_orig = m_cubeVertices[i];
        Vec3D v_scaled;
        v_scaled.x = v_orig.x * w;
        v_scaled.y = v_orig.y * h; 
        v_scaled.z = v_orig.z * d;

        float x_rot_y = v_scaled.x * std::cos(m_rotationY) - v_scaled.z * std::sin(m_rotationY);
        float z_rot_y = v_scaled.x * std::sin(m_rotationY) + v_scaled.z * std::cos(m_rotationY);
        
        float y_final = v_scaled.y * std::cos(m_rotationX) - z_rot_y * std::sin(m_rotationX);
        float z_final = v_scaled.y * std::sin(m_rotationX) + z_rot_y * std::cos(m_rotationX); 
        
        m_transformedVertices[i] = {
            x_rot_y * scaleFactor, 
            y_final * scaleFactor, 
            z_final * scaleFactor 
        };
    }
}

sf::Vector2f SimulationScreen::project(const Vec3D& p) const {
    sf::Vector2f viewCenter = m_3dView.getCenter();
    float perspectiveFactor = 500.f / (500.f + p.z); 
    
    return sf::Vector2f(
        std::round(p.x * perspectiveFactor + viewCenter.x), 
        std::round(p.y * perspectiveFactor + viewCenter.y)
    );
}

void SimulationScreen::initializeDefaultSK() {
    float base_S = 0.f, base_K = 0.f;
    switch (m_selectedPollutantIndex) {
        case 0:
            base_S = BASE_S_PM10; base_K = BASE_K_PM10;
            break;
        case 1:
            base_S = BASE_S_CO; base_K = BASE_K_CO;
            break;
        case 2:
            base_S = BASE_S_CL2; base_K = BASE_K_CL2;
            break;
        default:
            std::cerr << "Warning: Unknown pollutant index " << m_selectedPollutantIndex << std::endl;
            base_S = 10.f; base_K = 0.1f;
    }

    m_S_param = base_S + (m_numPassages * S_ADJUST_PASSAGE) + (m_numWindows * S_ADJUST_WINDOW);
    m_K_param = base_K + (m_numPassages * K_ADJUST_PASSAGE) + (m_numWindows * K_ADJUST_WINDOW);
    if (m_K_param < 0.0001f) m_K_param = 0.0001f;

    m_inputS.setText(sf::String(floatToWString(m_S_param, 2)).toAnsiString());
    m_inputK.setText(sf::String(floatToWString(m_K_param, 3)).toAnsiString());
}


void SimulationScreen::handleInput() {
    sf::Event event;
    while (m_window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            m_running = false;
            m_nextState = ScreenState::EXIT;
        }
        if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
            if (m_activeInputBox) {
                m_activeInputBox->setActive(false);
                m_activeInputBox = nullptr;
            } else {
                m_running = false;
                m_nextState = ScreenState::START; 
            }
        }

        if (m_activeInputBox && (event.type == sf::Event::TextEntered || event.type == sf::Event::KeyPressed) ) {
            handleInputBoxEvents(event);
            if (!m_simulationActive) {
                if (m_activeInputBox == &m_inputS) {
                    m_S_param = m_inputS.getFloatValue();
                } else if (m_activeInputBox == &m_inputK) {
                     m_K_param = m_inputK.getFloatValue();
                     if (m_K_param < 0.0001f) m_K_param = 0.0001f;
                } else if (m_activeInputBox == &m_inputC0 && !m_simulationStartedOnce) {
                    m_C0 = m_inputC0.getFloatValue();
                    if (!m_simulationActive) m_currentConcentration_Ct = m_C0;
                }
            }
            continue;
        }
        
        if (event.type == sf::Event::MouseButtonPressed) {
            if (event.mouseButton.button == sf::Mouse::Left) {
                sf::Vector2f mousePosWindow = static_cast<sf::Vector2f>(sf::Mouse::getPosition(m_window));
                sf::Vector2f mousePosUI = m_window.mapPixelToCoords(sf::Mouse::getPosition(m_window), m_uiView);

                InputBox* previouslyActive = m_activeInputBox;
                InputBox* clickedBox = nullptr;

                if (m_inputC0.getGlobalBounds().contains(mousePosUI) && !m_simulationStartedOnce) clickedBox = &m_inputC0;
                else if (m_inputS.getGlobalBounds().contains(mousePosUI)) clickedBox = &m_inputS;
                else if (m_inputK.getGlobalBounds().contains(mousePosUI)) clickedBox = &m_inputK;

                if (previouslyActive && previouslyActive != clickedBox) {
                    previouslyActive->setActive(false);
                }
                m_activeInputBox = clickedBox;
                if (m_activeInputBox) {
                    m_activeInputBox->setActive(true);
                } else {
                    if (m_shapeRun.getGlobalBounds().contains(mousePosUI)) runSimulation();
                    else if (m_shapeStop.getGlobalBounds().contains(mousePosUI)) stopSimulation();
                    else if (m_shapeReset.getGlobalBounds().contains(mousePosUI)) resetSimulationState();
                    else if (m_shapeBack.getGlobalBounds().contains(mousePosUI)) {
                        m_running = false;
                        m_nextState = ScreenState::START;
                    }
                }
                
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
            if (event.mouseButton.button == sf::Mouse::Left) m_isDragging = false;
        }
        if (event.type == sf::Event::MouseMoved) {
            if (m_isDragging && !m_activeInputBox) {
                sf::Vector2i currentMousePos = sf::Mouse::getPosition(m_window);
                float dx = static_cast<float>(currentMousePos.x - m_lastMousePos.x);
                m_rotationY += dx * 0.005f;
                m_lastMousePos = currentMousePos;
                projectVertices();
            }
        }
    }
}

void SimulationScreen::handleInputBoxEvents(sf::Event event) {
    if (m_activeInputBox) {
        m_activeInputBox->handleEvent(event);
    }
}

void SimulationScreen::updateButtonHovers(const sf::Vector2f& mousePos) {
    auto updateVisuals = [&](sf::Text& text, sf::RectangleShape& shape, bool isHovered) {
        text.setFillColor(isHovered ? m_buttonTextColorHover : m_buttonTextColorNormal);
        shape.setFillColor(isHovered ? m_buttonBgColorHover : m_buttonBgColorNormal);
        shape.setOutlineColor(m_buttonOutlineColor);
    };
    updateVisuals(m_buttonRun, m_shapeRun, m_shapeRun.getGlobalBounds().contains(mousePos));
    updateVisuals(m_buttonStop, m_shapeStop, m_shapeStop.getGlobalBounds().contains(mousePos));
    updateVisuals(m_buttonReset, m_shapeReset, m_shapeReset.getGlobalBounds().contains(mousePos));
    updateVisuals(m_buttonBack, m_shapeBack, m_shapeBack.getGlobalBounds().contains(mousePos));
}


void SimulationScreen::update(sf::Time dt) {
    m_inputC0.update();
    m_inputS.update();
    m_inputK.update();

    if (m_simulationActive) {
        m_particleUpdateTimer += dt.asSeconds();
        if (m_particleUpdateTimer >= 1.0f) {
            m_currentTime_t += 1.0f;
            calculateCurrentConcentration();
            spawnParticles();
            m_particleUpdateTimer -= 1.0f;
        }
    }
    updateParticleSystem(dt);

    m_displayVolume.setString(floatToWString(m_volumeV));
    sf::FloatRect volBounds = m_displayVolume.getLocalBounds();
    m_displayVolume.setOrigin(std::round(volBounds.left + volBounds.width), std::round(volBounds.top + volBounds.height / 2.f));
    
    m_displayTime.setString(floatToWString(m_currentTime_t, 0));
    sf::FloatRect timeBounds = m_displayTime.getLocalBounds();
    m_displayTime.setOrigin(std::round(timeBounds.left + timeBounds.width), std::round(timeBounds.top + timeBounds.height / 2.f));

    m_displayConcentration.setString(floatToWString(m_currentConcentration_Ct));
    sf::FloatRect concBounds = m_displayConcentration.getLocalBounds();
    m_displayConcentration.setOrigin(std::round(concBounds.left + concBounds.width), std::round(concBounds.top + concBounds.height / 2.f));

    sf::Vector2f mousePosUI = m_window.mapPixelToCoords(sf::Mouse::getPosition(m_window), m_uiView);
    updateButtonHovers(mousePosUI);
}

void SimulationScreen::calculateCurrentConcentration() {
    if (std::abs(m_K_param) < 0.00001f || std::abs(m_volumeV) < 0.00001f) {
        if (std::abs(m_S_param) < 0.00001f) {
             m_currentConcentration_Ct = m_C0;
        }
    }
    float term_S_kV = m_S_param / (m_K_param * m_volumeV);
    m_currentConcentration_Ct = (m_C0 - term_S_kV) * std::exp(-m_K_param * m_currentTime_t) + term_S_kV;
    
    if (m_currentConcentration_Ct < 0.f) m_currentConcentration_Ct = 0.f; 
}

void SimulationScreen::runSimulation() {
    if (!m_simulationStartedOnce) {
        m_C0 = m_inputC0.getFloatValue();
        m_simulationStartedOnce = true;
    }
    m_S_param = m_inputS.getFloatValue();
    m_K_param = m_inputK.getFloatValue();
    if (m_K_param < 0.0001f) {
        m_K_param = 0.0001f; 
        m_inputK.setText(sf::String(floatToWString(m_K_param,3)).toAnsiString());
    }

    m_simulationActive = true;
    if (m_currentTime_t == 0.0f) {
         m_currentConcentration_Ct = m_C0;
         spawnParticles();
    }
}

void SimulationScreen::stopSimulation() {
    m_simulationActive = false;
}

void SimulationScreen::resetSimulationState() {
    stopSimulation();
    m_simulationStartedOnce = false;
    
    m_currentTime_t = 0.0f;
    
    initializeDefaultSK();

    m_inputC0.setText(sf::String(floatToWString(100.0f, 1)).toAnsiString());
    m_C0 = m_inputC0.getFloatValue();
    m_currentConcentration_Ct = m_C0;

    m_particles.clear();
    spawnParticles();

    projectVertices();
}

void SimulationScreen::spawnParticles() {
    m_particles.clear();

    float max_conc_for_scaling = std::max(m_C0, 1.0f);
    if (m_currentConcentration_Ct > max_conc_for_scaling * 1.5f) {
        max_conc_for_scaling = m_currentConcentration_Ct;
    }

    int numParticlesToSpawn = 0;
    if (max_conc_for_scaling > 0.001f) {
       numParticlesToSpawn = static_cast<int>(static_cast<float>(m_maxParticles) * (m_currentConcentration_Ct / max_conc_for_scaling));
    }
    numParticlesToSpawn = std::min(numParticlesToSpawn, m_maxParticles);
    numParticlesToSpawn = std::max(numParticlesToSpawn, 0);

    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<float> distrib_pos(-0.49f, 0.49f);
    std::uniform_real_distribution<float> distrib_vel(-0.02f, 0.02f);

    for (int i = 0; i < numParticlesToSpawn; ++i) {
        Particle p;
        p.position3D = {distrib_pos(gen), distrib_pos(gen), distrib_pos(gen)};
        p.velocity = {distrib_vel(gen), distrib_vel(gen), distrib_vel(gen)};
        
        p.shape.setRadius(2.f);
        p.shape.setFillColor(m_particleColor);
        p.shape.setOrigin(p.shape.getRadius(), p.shape.getRadius());
        m_particles.push_back(p);
    }
}

void SimulationScreen::updateParticleSystem(sf::Time dt) {
    float w_half_norm = 0.5f;
    float h_half_norm = 0.5f;
    float d_half_norm = 0.5f;

    float deltaTime = dt.asSeconds();

    for (auto& p : m_particles) {
        p.position3D.x += p.velocity.x * deltaTime;
        p.position3D.y += p.velocity.y * deltaTime;
        p.position3D.z += p.velocity.z * deltaTime;

        if (p.position3D.x > w_half_norm) p.position3D.x -= 2.f * w_half_norm;
        else if (p.position3D.x < -w_half_norm) p.position3D.x += 2.f * w_half_norm;

        if (p.position3D.y > h_half_norm) p.position3D.y -= 2.f * h_half_norm;
        else if (p.position3D.y < -h_half_norm) p.position3D.y += 2.f * h_half_norm;

        if (p.position3D.z > d_half_norm) p.position3D.z -= 2.f * d_half_norm;
        else if (p.position3D.z < -d_half_norm) p.position3D.z += 2.f * d_half_norm;
    }
}


void SimulationScreen::render() {
    m_window.clear(sf::Color::Black);

    m_window.setView(m_3dView);
    drawCuboidEdges(m_window);
    drawOpeningsVisual(m_window);

    float maxDim = std::max({m_roomWidth, m_roomDepth, m_roomHeight, 1.f});
    float scaleFactor3D = 350.f / maxDim;

    for (const auto& p : m_particles) {
        Vec3D v_world_scaled;
        v_world_scaled.x = p.position3D.x * m_roomWidth;
        v_world_scaled.y = p.position3D.y * m_roomHeight; 
        v_world_scaled.z = p.position3D.z * m_roomDepth;

        float x_rot_y = v_world_scaled.x * std::cos(m_rotationY) - v_world_scaled.z * std::sin(m_rotationY);
        float z_rot_y = v_world_scaled.x * std::sin(m_rotationY) + v_world_scaled.z * std::cos(m_rotationY);
        float y_final = v_world_scaled.y * std::cos(m_rotationX) - z_rot_y * std::sin(m_rotationX);
        float z_final = v_world_scaled.y * std::sin(m_rotationX) + z_rot_y * std::cos(m_rotationX);
        
        Vec3D v_transformed_for_projection = {
            x_rot_y * scaleFactor3D,
            y_final * scaleFactor3D,
            z_final * scaleFactor3D
        };
        
        sf::Vector2f screenPos = project(v_transformed_for_projection);
        
        sf::CircleShape particleShape = p.shape;
        particleShape.setPosition(screenPos);
        
        float depthPerspectiveFactor = 500.f / (500.f + v_transformed_for_projection.z);
        depthPerspectiveFactor = std::max(0.2f, std::min(1.f, depthPerspectiveFactor));
        
        particleShape.setScale(depthPerspectiveFactor, depthPerspectiveFactor);
        
        sf::Color c = particleShape.getFillColor();
        c.a = static_cast<sf::Uint8>(static_cast<float>(m_particleColor.a) * depthPerspectiveFactor); 
        particleShape.setFillColor(c);

        m_window.draw(particleShape);
    }

    m_window.setView(m_uiView);
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

    m_window.setView(m_window.getDefaultView());
    m_window.display();
}

void SimulationScreen::drawCuboidEdges(sf::RenderWindow& window) {
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

void SimulationScreen::drawOpeningsVisual(sf::RenderWindow& window) {
    auto draw_one_opening_shape = [&](const SettingScreen::OpeningDefinition& def) {
        std::array<sf::Vector2f, 4> screen_points;
        
        float w = (m_roomWidth > 0.1f) ? m_roomWidth : 0.1f;
        float d = (m_roomDepth > 0.1f) ? m_roomDepth : 0.1f;
        float h = (m_roomHeight > 0.1f) ? m_roomHeight : 0.1f;
        float maxDim = std::max({w, d, h, 1.f});
        float viewScaleFactor = 350.f / maxDim;

        for (size_t i = 0; i < 4; ++i) {
            const Vec3D& v_local_norm = def.local_coords[i]; 
            Vec3D v_world_scaled;
            v_world_scaled.x = v_local_norm.x * w;
            v_world_scaled.y = v_local_norm.y * h; 
            v_world_scaled.z = v_local_norm.z * d;

            float x_rot_y = v_world_scaled.x * std::cos(m_rotationY) - v_world_scaled.z * std::sin(m_rotationY);
            float z_rot_y = v_world_scaled.x * std::sin(m_rotationY) + v_world_scaled.z * std::cos(m_rotationY);
            float y_rotated = v_world_scaled.y * std::cos(m_rotationX) - z_rot_y * std::sin(m_rotationX);
            float z_rotated_final = v_world_scaled.y * std::sin(m_rotationX) + z_rot_y * std::cos(m_rotationX);
            
            Vec3D v_transformed_for_projection = {
                x_rot_y * viewScaleFactor, y_rotated * viewScaleFactor, z_rotated_final * viewScaleFactor
            };
            screen_points[i] = project(v_transformed_for_projection);
        }

        for (size_t i = 0; i < 4; ++i) {
            sf::Vertex line[] = {
                sf::Vertex(screen_points[i], sf::Color::Cyan),
                sf::Vertex(screen_points[(i + 1) % 4], sf::Color::Cyan) 
            };
            window.draw(line, 2, sf::Lines);
        }
    };

    for (const auto& passage_def : m_passages_vis) {
        draw_one_opening_shape(passage_def);
    }
    for (const auto& window_def : m_windows_vis) {
        draw_one_opening_shape(window_def);
    }
}

ScreenState SimulationScreen::getNextState() const { return m_nextState; }
bool SimulationScreen::isRunning() const { return m_running; }
void SimulationScreen::setNextState(ScreenState state) { m_nextState = state; }