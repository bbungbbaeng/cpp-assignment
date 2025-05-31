#include "Setting.hpp"
#include "../screen/Screen.hpp"
#include <cmath>
#include <iomanip>
#include <sstream>
#include <iostream>

const float PI = 3.1415926535f;

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
    sf::FloatRect textBounds = m_text.getLocalBounds();
    m_text.setOrigin(std::round(textBounds.left), std::round(textBounds.top + textBounds.height / 2.0f));
    m_text.setPosition(std::round(position.x + 5.f), std::round(position.y + size.y / 2.f));

    m_placeholderText = m_text;
    m_placeholderText.setString(placeholder);
    m_placeholderText.setFillColor(sf::Color(150,150,150));
    sf::FloatRect placeholderBounds = m_placeholderText.getLocalBounds();
    m_placeholderText.setOrigin(std::round(placeholderBounds.left), std::round(placeholderBounds.top + placeholderBounds.height / 2.0f));
    m_placeholderText.setPosition(std::round(position.x + 5.f), std::round(position.y + size.y / 2.f));
}

void InputBox::handleEvent(sf::Event event) {
    if (!m_isActive) return;

    if (event.type == sf::Event::TextEntered) {
        if (event.text.unicode == '\b') {
            if (!m_inputString.empty()) {
                m_inputString.pop_back();
            }
        } else if (event.text.unicode < 128 && event.text.unicode != '\r' && event.text.unicode != '\n') {
            char enteredChar = static_cast<char>(event.text.unicode);
            if (std::isdigit(enteredChar) ||
                (enteredChar == '.' && m_inputString.find('.') == std::string::npos) ||
                (enteredChar == '-' && m_inputString.empty())) {
                 if (m_inputString.length() < 10) {
                    m_inputString += enteredChar;
                 }
            }
        }
        m_text.setString(m_inputString);
        sf::FloatRect textBounds = m_text.getLocalBounds();
        m_text.setOrigin(std::round(textBounds.left), std::round(textBounds.top + textBounds.height / 2.0f));
        m_text.setPosition(std::round(m_shape.getPosition().x + 5.f), std::round(m_shape.getPosition().y + m_shape.getSize().y / 2.f));
    }
}

void InputBox::update() {
    if (m_isActive) {
        if (m_cursorClock.getElapsedTime().asSeconds() > 0.5f) {
            m_showCursor = !m_showCursor;
            m_cursorClock.restart();
        }
    } else {
        m_showCursor = false;
    }
}

void InputBox::render(sf::RenderWindow& window) {
    window.draw(m_shape);
    if (m_inputString.empty() && !m_isActive) {
        window.draw(m_placeholderText);
    } else {
        std::string currentTextStr = m_inputString;
        if (m_isActive && m_showCursor) {
            currentTextStr += "|";
        }
        sf::Text tempText = m_text;
        tempText.setString(currentTextStr);
        sf::FloatRect tempBounds = tempText.getLocalBounds();
        tempText.setOrigin(std::round(tempBounds.left), std::round(tempBounds.top + tempBounds.height / 2.0f));
        tempText.setPosition(std::round(m_shape.getPosition().x + 5.f), std::round(m_shape.getPosition().y + m_shape.getSize().y / 2.f));
        window.draw(tempText);
    }
}

void InputBox::setActive(bool active) {
    m_isActive = active;
    if (m_isActive) {
        m_shape.setOutlineColor(sf::Color::White);
        m_cursorClock.restart();
        m_showCursor = true;
    } else {
        m_shape.setOutlineColor(sf::Color(100, 100, 100));
        m_showCursor = false;
    }
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
    sf::FloatRect textBounds = m_text.getLocalBounds();
    m_text.setOrigin(std::round(textBounds.left), std::round(textBounds.top + textBounds.height / 2.0f));
    m_text.setPosition(std::round(m_shape.getPosition().x + 5.f), std::round(m_shape.getPosition().y + m_shape.getSize().y / 2.f));
}

float InputBox::getFloatValue() const {
    try {
        if (m_inputString == "-" || m_inputString == "." || m_inputString == "-.") return 0.0f;
        if (m_inputString.empty()) return 0.0f;
        return std::stof(m_inputString);
    } catch (const std::exception& e) {
        return 0.0f;
    }
}
sf::FloatRect InputBox::getGlobalBounds() const {
    return m_shape.getGlobalBounds();
}


SettingScreen::SettingScreen(sf::RenderWindow& window, sf::Font& font)
    : m_window(window), m_font(font),
      m_nextState(ScreenState::SETTING), m_running(true),
      m_roomWidth(5.f), m_roomDepth(5.f), m_roomHeight(3.f),
      m_rotationX(25.f * PI / 180.f),
      m_rotationY(-35.f * PI / 180.f),
      m_isDragging(false), m_selectedPollutantIndex(0), m_activeInputBox(nullptr) {

    m_buttonTextColorNormal = sf::Color::White;
    m_buttonTextColorHover = sf::Color::Black;
    m_buttonBgColorNormal = sf::Color(80,80,80);
    m_buttonBgColorHover = sf::Color::White;
    m_buttonOutlineColor = sf::Color::White;

    m_3dView.setSize(static_cast<float>(m_window.getSize().x) * 0.6f, static_cast<float>(m_window.getSize().y));
    m_3dView.setCenter(m_3dView.getSize().x / 2.f, m_3dView.getSize().y / 2.f - 50.f); // Y 좌표를 위로 50픽셀 이동
    m_3dView.setViewport(sf::FloatRect(0.f, 0.f, 0.6f, 1.f));

    m_uiView.setSize(static_cast<float>(m_window.getSize().x) * 0.4f, static_cast<float>(m_window.getSize().y));
    m_uiView.setCenter(m_uiView.getSize().x / 2.f, m_uiView.getSize().y / 2.f);
    m_uiView.setViewport(sf::FloatRect(0.6f, 0.f, 0.4f, 1.f));

    setupUI();
    setup3D();
    projectVertices();
}

SettingScreen::~SettingScreen() {}

void SettingScreen::setupUI() {
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


    m_titleText.setFont(m_font);
    m_titleText.setString(L"시뮬레이션 설정");
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

        inputBox.setup(m_font, sf::Vector2f(uiX + labelWidthForCalc + gapBetweenLabelInput, currentY), sf::Vector2f(inputBoxWidthForCalc, inputHeight), placeholder);
        inputBox.setText(defaultVal);
        currentY += spacing;
    };

    setupInputField(m_labelWidth, m_inputWidth, L"가로 (m):", "5.0", L"0.0 - 100.0");
    setupInputField(m_labelDepth, m_inputDepth, L"세로 (m):", "5.0", L"0.0 - 100.0");
    setupInputField(m_labelHeight, m_inputHeight, L"높이 (m):", "3.0", L"0.0 - 10.0");
    currentY += spacing * 0.5f;

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
        
        currentY += (inputHeight * 0.9f) + 5.f;
    }
    currentY += spacing * 0.5f;

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
    float pairedButtonWidth = (maxUiElementWidth - 10.f) / 2.f;

    setupButtonLambda(m_buttonCreatePassage, m_shapeCreatePassage, L"통로 생성", currentY, pairedButtonWidth);
    setupButtonLambda(m_buttonCreateWindow, m_shapeCreateWindow, L"창문 생성", currentY, pairedButtonWidth, pairedButtonWidth + 10.f);
    currentY += spacing;

    setupButtonLambda(m_buttonRemovePassage, m_shapeRemovePassage, L"통로 제거", currentY, pairedButtonWidth);
    setupButtonLambda(m_buttonRemoveWindow, m_shapeRemoveWindow, L"창문 제거", currentY, pairedButtonWidth, pairedButtonWidth + 10.f);
    currentY += spacing * 1.5f;

    float startButtonY = m_uiView.getSize().y - spacing - inputHeight; 
    setupButtonLambda(m_buttonStartSimulation, m_shapeStartSimulation, L"시뮬레이션 시작", startButtonY, singleButtonWidth);
}


void SettingScreen::setup3D() {
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

void SettingScreen::projectVertices() {
    float w = (m_roomWidth > 0.1f) ? m_roomWidth : 0.1f;
    float d = (m_roomDepth > 0.1f) ? m_roomDepth : 0.1f;
    float h = (m_roomHeight > 0.1f) ? m_roomHeight : 0.1f;

    float maxDim = std::max({w, d, h, 1.f});
    float scaleFactor = 350.f / maxDim;

    for (size_t i = 0; i < m_cubeVertices.size(); ++i) {
        Vec3D v_orig = m_cubeVertices[i];
        Vec3D v;
        v.x = v_orig.x * w;
        v.y = v_orig.y * h; 
        v.z = v_orig.z * d;

        float x_rot_y = v.x * std::cos(m_rotationY) - v.z * std::sin(m_rotationY);
        float z_rot_y = v.x * std::sin(m_rotationY) + v.z * std::cos(m_rotationY);
        
        float y_final = v.y * std::cos(m_rotationX) - z_rot_y * std::sin(m_rotationX);
        float z_final = v.y * std::sin(m_rotationX) + z_rot_y * std::cos(m_rotationX);
        
        m_transformedVertices[i] = {x_rot_y * scaleFactor, y_final * scaleFactor, z_final * scaleFactor};
    }
}


sf::Vector2f SettingScreen::project(const Vec3D& p) const {
    sf::Vector2f viewCenter = m_3dView.getCenter(); // 뷰의 중심 사용
    float perspectiveFactor = 500.f / (500.f + p.z);
    // 뷰의 크기 대신 중심을 기준으로 투영
    return sf::Vector2f(std::round(p.x * perspectiveFactor + viewCenter.x), 
                        std::round(p.y * perspectiveFactor + viewCenter.y));
}

void SettingScreen::handleInputBoxEvents(sf::Event event) {
    InputBox* currentActive = m_activeInputBox;
    if (currentActive) {
        currentActive->handleEvent(event);
        if (m_activeInputBox == currentActive) { 
            if (currentActive == &m_inputWidth) m_roomWidth = m_inputWidth.getFloatValue();
            else if (currentActive == &m_inputDepth) m_roomDepth = m_inputDepth.getFloatValue();
            else if (currentActive == &m_inputHeight) m_roomHeight = m_inputHeight.getFloatValue();
            projectVertices();
        }
    }
}


void SettingScreen::handleInput() {
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
        
        if (m_activeInputBox && event.type == sf::Event::TextEntered) {
            handleInputBoxEvents(event);
            continue;
        }
        if (m_activeInputBox && event.type == sf::Event::KeyPressed) {
             handleInputBoxEvents(event);
        }


        if (event.type == sf::Event::MouseButtonPressed) {
            if (event.mouseButton.button == sf::Mouse::Left) {
                sf::Vector2f mousePosWindow = static_cast<sf::Vector2f>(sf::Mouse::getPosition(m_window));
                sf::Vector2f mousePosUI = m_window.mapPixelToCoords(sf::Mouse::getPosition(m_window), m_uiView);

                InputBox* previouslyActive = m_activeInputBox;
                InputBox* clickedBox = nullptr;

                if (m_inputWidth.getGlobalBounds().contains(mousePosUI)) clickedBox = &m_inputWidth;
                else if (m_inputDepth.getGlobalBounds().contains(mousePosUI)) clickedBox = &m_inputDepth;
                else if (m_inputHeight.getGlobalBounds().contains(mousePosUI)) clickedBox = &m_inputHeight;

                if (previouslyActive && previouslyActive != clickedBox) {
                    previouslyActive->setActive(false);
                }
                m_activeInputBox = clickedBox;
                if (m_activeInputBox) {
                    m_activeInputBox->setActive(true);
                }
                
                for(size_t i=0; i<m_pollutantOptionShapes.size(); ++i) {
                    if (m_pollutantOptionShapes[i].getGlobalBounds().contains(mousePosUI)) {
                        m_selectedPollutantIndex = static_cast<int>(i);
                        break; 
                    }
                }

                if (m_shapeStartSimulation.getGlobalBounds().contains(mousePosUI)) {
                    m_nextState = ScreenState::SIMULATION;
                    m_running = false;
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
            if (event.mouseButton.button == sf::Mouse::Left) {
                m_isDragging = false;
            }
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

void SettingScreen::updateButtonHovers(const sf::Vector2f& mousePos) {
    auto updateVisuals = [&](sf::Text& text, sf::RectangleShape& shape, bool isHovered, bool isSelected = false) {
        text.setFillColor(isHovered || isSelected ? m_buttonTextColorHover : m_buttonTextColorNormal);
        shape.setFillColor(isHovered || isSelected ? m_buttonBgColorHover : m_buttonBgColorNormal);
        if (isSelected) {
            shape.setOutlineColor(sf::Color::Yellow);
        } else {
            shape.setOutlineColor(m_buttonOutlineColor);
        }
    };
    
    for(size_t i=0; i<m_pollutantOptions.size(); ++i) {
        bool isSelected = (static_cast<int>(i) == m_selectedPollutantIndex);
        bool isHovered = m_pollutantOptionShapes[i].getGlobalBounds().contains(mousePos);
        updateVisuals(m_pollutantOptions[i], m_pollutantOptionShapes[i], isHovered, isSelected);
    }

    updateVisuals(m_buttonCreatePassage, m_shapeCreatePassage, m_shapeCreatePassage.getGlobalBounds().contains(mousePos));
    updateVisuals(m_buttonCreateWindow, m_shapeCreateWindow, m_shapeCreateWindow.getGlobalBounds().contains(mousePos));
    updateVisuals(m_buttonRemovePassage, m_shapeRemovePassage, m_shapeRemovePassage.getGlobalBounds().contains(mousePos));
    updateVisuals(m_buttonRemoveWindow, m_shapeRemoveWindow, m_shapeRemoveWindow.getGlobalBounds().contains(mousePos));
    updateVisuals(m_buttonStartSimulation, m_shapeStartSimulation, m_shapeStartSimulation.getGlobalBounds().contains(mousePos));
}


void SettingScreen::update(sf::Time dt) {
    m_inputWidth.update();
    m_inputDepth.update();
    m_inputHeight.update();

    sf::Vector2f mousePosUI = m_window.mapPixelToCoords(sf::Mouse::getPosition(m_window), m_uiView);
    updateButtonHovers(mousePosUI);
}

void SettingScreen::drawCuboidEdges(sf::RenderWindow& window) {
    for(const auto& edge : m_cubeEdges) {
        sf::Vector2f p1 = project(m_transformedVertices[edge.start]);
        sf::Vector2f p2 = project(m_transformedVertices[edge.end]);
        sf::Vertex line[] = { sf::Vertex(p1, sf::Color::White), sf::Vertex(p2, sf::Color::White) };
        window.draw(line, 2, sf::Lines);
    }
}

void SettingScreen::render() {
    m_window.clear(sf::Color::Black);

    m_window.setView(m_3dView);
    drawCuboidEdges(m_window);

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
    m_window.draw(m_shapeStartSimulation);m_window.draw(m_buttonStartSimulation);

    m_window.setView(m_window.getDefaultView());
    m_window.display();
}

ScreenState SettingScreen::getNextState() const {
    return m_nextState;
}
void SettingScreen::setNextState(ScreenState state) {
    m_nextState = state;
}

bool SettingScreen::isRunning() const {
    return m_running;
}