#include "Setting.hpp"
#include "../screen/Screen.hpp" // For ScreenState enum
#include <cmath>
#include <iomanip>
#include <sstream>
#include <iostream> // For std::cout (and potentially std::max if not using <algorithm>)
#include <algorithm> // For std::max

const float PI = 3.1415926535f; // 이 줄의 주석을 제거하여 PI를 정의합니다.

// --- START: Added for Passages and Windows ---
// Define static const members
const float SettingScreen::PASSAGE_RELATIVE_HEIGHT_FACTOR = 0.7f;  // 방 높이의 70%
const float SettingScreen::PASSAGE_RELATIVE_WIDTH_FACTOR = 0.25f; // 면 너비의 25% (예: 앞/뒤 면의 경우 방 너비)

const float SettingScreen::WINDOW_RELATIVE_HEIGHT_FACTOR = 0.5f;   // 방 높이의 50%
const float SettingScreen::WINDOW_RELATIVE_WIDTH_FACTOR = 0.4f;    // 면 너비/깊이의 40% (예: 좌/우 면의 경우 방 깊이)
// --- END: Added for Passages and Windows ---


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
        if (event.text.unicode == '\b') { // Backspace
            if (!m_inputString.empty()) {
                m_inputString.pop_back();
            }
        } else if (event.text.unicode < 128 && event.text.unicode != '\r' && event.text.unicode != '\n') { // Printable ASCII
            char enteredChar = static_cast<char>(event.text.unicode);
            if (std::isdigit(enteredChar) ||
                (enteredChar == '.' && m_inputString.find('.') == std::string::npos) || // Allow one dot
                (enteredChar == '-' && m_inputString.empty())) { // Allow minus at the beginning
                 if (m_inputString.length() < 10) { // Limit input length
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
        sf::Text tempText = m_text; // Use a temporary to avoid modifying m_text's origin/position logic repeatedly
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
    // Ensure placeholder is correctly positioned when deactivated and empty
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
    // Update text origin and position after setting new text
    sf::FloatRect textBounds = m_text.getLocalBounds();
    m_text.setOrigin(std::round(textBounds.left), std::round(textBounds.top + textBounds.height / 2.0f));
    m_text.setPosition(std::round(m_shape.getPosition().x + 5.f), std::round(m_shape.getPosition().y + m_shape.getSize().y / 2.f));
}

float InputBox::getFloatValue() const {
    try {
        if (m_inputString == "-" || m_inputString == "." || m_inputString == "-.") return 0.0f; // Handle incomplete inputs
        if (m_inputString.empty()) return 0.0f; // Default or error
        return std::stof(m_inputString);
    } catch (const std::exception& e) {
        // std::cerr << "Error converting string to float: " << e.what() << std::endl;
        return 0.0f; // Or some other error indication / default value
    }
}
sf::FloatRect InputBox::getGlobalBounds() const {
    return m_shape.getGlobalBounds();
}


SettingScreen::SettingScreen(sf::RenderWindow& window, sf::Font& font)
    : m_window(window), m_font(font),
      m_nextState(ScreenState::SETTING), m_running(true),
      m_roomWidth(5.f), m_roomDepth(5.f), m_roomHeight(3.f),
      m_rotationX(25.f * PI / 180.f), // PI is now defined
      m_rotationY(-35.f * PI / 180.f),// PI is now defined
      m_isDragging(false), m_selectedPollutantIndex(0), m_activeInputBox(nullptr) {

    m_buttonTextColorNormal = sf::Color::White;
    m_buttonTextColorHover = sf::Color::Black;
    m_buttonBgColorNormal = sf::Color(80,80,80);
    m_buttonBgColorHover = sf::Color::White;
    m_buttonOutlineColor = sf::Color::White;

    // --- START: Added for Passages and Windows ---
    m_passages_defs.reserve(2); // Max 2 passages
    m_windows_defs.reserve(2);  // Max 2 windows
    // --- END: Added for Passages and Windows ---

    m_3dView.setSize(static_cast<float>(m_window.getSize().x) * 0.6f, static_cast<float>(m_window.getSize().y));
    m_3dView.setCenter(m_3dView.getSize().x / 2.f, m_3dView.getSize().y / 2.f - 50.f); // Move Y origin up by 50px
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

    float labelWidthForCalc = 90.f; // "가로 (m):"
    float inputBoxWidthForCalc = 180.f; // "0.0 - 100.0"
    float gapBetweenLabelInput = 10.f;
    float maxUiElementWidth = labelWidthForCalc + gapBetweenLabelInput + inputBoxWidthForCalc;

    float rightPadding = 30.f; // Padding from the right edge of the UI view
    float uiX = m_uiView.getSize().x - maxUiElementWidth - rightPadding; 


    m_titleText.setFont(m_font);
    m_titleText.setString(L"시뮬레이션 설정");
    m_titleText.setCharacterSize(titleCharSize);
    m_titleText.setFillColor(sf::Color::White);
    sf::FloatRect titleBounds = m_titleText.getLocalBounds();
    m_titleText.setOrigin(std::round(titleBounds.left + titleBounds.width / 2.f), std::round(titleBounds.top));
    m_titleText.setPosition(std::round(uiX + maxUiElementWidth / 2.f), std::round(currentY));
    currentY += titleCharSize + spacing * 0.8f; // Extra spacing after title

    auto setupInputField = [&](sf::Text& label, InputBox& inputBox, const std::wstring& labelText, const std::string& defaultVal, const std::wstring& placeholder) {
        label.setFont(m_font);
        label.setString(labelText);
        label.setCharacterSize(charSize);
        label.setFillColor(sf::Color::White);
        sf::FloatRect labelBounds = label.getLocalBounds();
        // Align origin for vertical centering with InputBox
        label.setOrigin(std::round(labelBounds.left), std::round(labelBounds.top + labelBounds.height / 2.f));
        label.setPosition(std::round(uiX), std::round(currentY + inputHeight / 2.f));

        inputBox.setup(m_font, sf::Vector2f(uiX + labelWidthForCalc + gapBetweenLabelInput, currentY), sf::Vector2f(inputBoxWidthForCalc, inputHeight), placeholder);
        inputBox.setText(defaultVal);
        currentY += spacing;
    };

    setupInputField(m_labelWidth, m_inputWidth, L"가로 (m):", "5.0", L"0.0 - 100.0");
    setupInputField(m_labelDepth, m_inputDepth, L"세로 (m):", "5.0", L"0.0 - 100.0");
    setupInputField(m_labelHeight, m_inputHeight, L"높이 (m):", "3.0", L"0.0 - 10.0");
    currentY += spacing * 0.5f; // Extra spacing before next section

    // Pollutant selection UI
    m_labelPollutant.setFont(m_font);
    m_labelPollutant.setString(L"오염 물질:");
    m_labelPollutant.setCharacterSize(charSize);
    m_labelPollutant.setFillColor(sf::Color::White);
    sf::FloatRect pollutantLabelBounds = m_labelPollutant.getLocalBounds();
    m_labelPollutant.setOrigin(std::round(pollutantLabelBounds.left), std::round(pollutantLabelBounds.top + pollutantLabelBounds.height / 2.f));
    m_labelPollutant.setPosition(std::round(uiX), std::round(currentY + (inputHeight * 0.9f) / 2.f)); // Slightly smaller height for options
    currentY += spacing * 0.8f;

    std::vector<std::wstring> pollutants = {L"미세먼지 (PM10)", L"일산화탄소 (CO)", L"염소가스 (Cl₂)"};
    m_pollutantOptions.resize(pollutants.size());
    m_pollutantOptionShapes.resize(pollutants.size());
    float pollutantOptionItemWidth = maxUiElementWidth; // Full width for selection items

    for (size_t i = 0; i < pollutants.size(); ++i) {
        m_pollutantOptionShapes[i].setSize(sf::Vector2f(pollutantOptionItemWidth, inputHeight * 0.9f));
        m_pollutantOptionShapes[i].setPosition(std::round(uiX), std::round(currentY));
        m_pollutantOptionShapes[i].setOutlineThickness(1.f);
        // Visuals updated in updateButtonHovers

        m_pollutantOptions[i].setFont(m_font);
        m_pollutantOptions[i].setString(pollutants[i]);
        m_pollutantOptions[i].setCharacterSize(charSize - 2); // Slightly smaller text for options
        
        sf::FloatRect textBounds = m_pollutantOptions[i].getLocalBounds();
        m_pollutantOptions[i].setOrigin(std::round(textBounds.left + textBounds.width / 2.f), std::round(textBounds.top + textBounds.height / 2.f));
        m_pollutantOptions[i].setPosition(std::round(uiX + pollutantOptionItemWidth / 2.f), std::round(currentY + (inputHeight * 0.9f) / 2.f));
        
        currentY += (inputHeight * 0.9f) + 5.f; // Spacing between options
    }
    currentY += spacing * 0.5f; // Extra spacing

    // Buttons
    auto setupButtonLambda = [&](sf::Text& text, sf::RectangleShape& shape, const std::wstring& str, float yPos, float btnWidth, float btnXOffset = 0.f) {
        shape.setSize(sf::Vector2f(btnWidth, inputHeight));
        shape.setPosition(std::round(uiX + btnXOffset), std::round(yPos));
        shape.setOutlineThickness(1.f);
        // Visuals updated in updateButtonHovers

        text.setFont(m_font);
        text.setString(str);
        text.setCharacterSize(charSize - 2); // Slightly smaller text for buttons
        
        sf::FloatRect textBounds = text.getLocalBounds();
        text.setOrigin(std::round(textBounds.left + textBounds.width / 2.f), std::round(textBounds.top + textBounds.height / 2.f));
        text.setPosition(std::round(shape.getPosition().x + btnWidth / 2.f), std::round(yPos + inputHeight / 2.f));
    };
    
    float singleButtonWidth = maxUiElementWidth;
    float pairedButtonWidth = (maxUiElementWidth - 10.f) / 2.f; // For two buttons in a row with 10px spacing

    setupButtonLambda(m_buttonCreatePassage, m_shapeCreatePassage, L"통로 생성", currentY, pairedButtonWidth);
    setupButtonLambda(m_buttonCreateWindow, m_shapeCreateWindow, L"창문 생성", currentY, pairedButtonWidth, pairedButtonWidth + 10.f);
    currentY += spacing;

    setupButtonLambda(m_buttonRemovePassage, m_shapeRemovePassage, L"통로 제거", currentY, pairedButtonWidth);
    setupButtonLambda(m_buttonRemoveWindow, m_shapeRemoveWindow, L"창문 제거", currentY, pairedButtonWidth, pairedButtonWidth + 10.f);
    currentY += spacing * 1.5f; // More spacing before start button

    // Start Simulation Button (at the bottom)
    float startButtonY = m_uiView.getSize().y - spacing - inputHeight; // Positioned from bottom
    setupButtonLambda(m_buttonStartSimulation, m_shapeStartSimulation, L"시뮬레이션 시작", startButtonY, singleButtonWidth);
}


void SettingScreen::setup3D() {
    // Normalized cube vertices (center at 0,0,0, dimensions -0.5 to 0.5)
    m_cubeVertices = {{
        {-0.5f, -0.5f, -0.5f}, {0.5f, -0.5f, -0.5f}, {0.5f, 0.5f, -0.5f}, {-0.5f, 0.5f, -0.5f}, // Front face
        {-0.5f, -0.5f, 0.5f}, {0.5f, -0.5f, 0.5f}, {0.5f, 0.5f, 0.5f}, {-0.5f, 0.5f, 0.5f}    // Back face
    }};
    m_cubeEdges = {
        {0,1}, {1,2}, {2,3}, {3,0}, // Front face edges
        {4,5}, {5,6}, {6,7}, {7,4}, // Back face edges
        {0,4}, {1,5}, {2,6}, {3,7}  // Connecting edges
    };
    m_transformedVertices.fill({}); // Initialize with zero vectors
}

void SettingScreen::projectVertices() {
    // Ensure dimensions are positive and non-zero for scaling
    float w = (m_roomWidth > 0.1f) ? m_roomWidth : 0.1f;
    float d = (m_roomDepth > 0.1f) ? m_roomDepth : 0.1f;
    float h = (m_roomHeight > 0.1f) ? m_roomHeight : 0.1f;

    // Determine scale factor to fit the room in the view
    float maxDim = std::max({w, d, h, 1.f}); // Compare with 1.f to avoid too large scaling for small rooms
    float scaleFactor = 350.f / maxDim; // Adjust 350.f to change overall size in view

    for (size_t i = 0; i < m_cubeVertices.size(); ++i) {
        const Vec3D& v_orig = m_cubeVertices[i]; // Original normalized vertex
        Vec3D v_scaled; // Vertex scaled by room dimensions
        v_scaled.x = v_orig.x * w;
        v_scaled.y = v_orig.y * h; // Y is up in SFML 2D, but typically up in 3D model
        v_scaled.z = v_orig.z * d;

        // Rotate around Y axis (yaw)
        float x_rot_y = v_scaled.x * std::cos(m_rotationY) - v_scaled.z * std::sin(m_rotationY);
        float z_rot_y = v_scaled.x * std::sin(m_rotationY) + v_scaled.z * std::cos(m_rotationY);
        
        // Rotate around X axis (pitch) - applied to Y-rotated coordinates
        float y_final = v_scaled.y * std::cos(m_rotationX) - z_rot_y * std::sin(m_rotationX);
        float z_final = v_scaled.y * std::sin(m_rotationX) + z_rot_y * std::cos(m_rotationX); // This is the depth after rotation
        
        // Apply view scale factor
        m_transformedVertices[i] = {
            x_rot_y * scaleFactor, 
            y_final * scaleFactor, 
            z_final * scaleFactor 
        };
    }
}


sf::Vector2f SettingScreen::project(const Vec3D& p) const {
    sf::Vector2f viewCenter = m_3dView.getCenter();
    // Simple perspective projection: farther objects appear smaller and closer to center
    // Adjust 500.f (focal length) to change perspective strength
    float perspectiveFactor = 500.f / (500.f + p.z); 
    
    return sf::Vector2f(
        std::round(p.x * perspectiveFactor + viewCenter.x), 
        std::round(p.y * perspectiveFactor + viewCenter.y) // p.y is already prepared for screen (Y-down if needed)
    );
}

void SettingScreen::handleInputBoxEvents(sf::Event event) {
    InputBox* currentActive = m_activeInputBox; // Store current active, in case it changes during handling
    if (currentActive) {
        currentActive->handleEvent(event);
        // If the active InputBox is still the same one after handling the event
        if (m_activeInputBox == currentActive) { 
            // Update room dimensions if corresponding input box changed
            if (currentActive == &m_inputWidth) m_roomWidth = m_inputWidth.getFloatValue();
            else if (currentActive == &m_inputDepth) m_roomDepth = m_inputDepth.getFloatValue();
            else if (currentActive == &m_inputHeight) m_roomHeight = m_inputHeight.getFloatValue();
            projectVertices(); // Re-project cube and openings if dimensions change
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
                m_activeInputBox->setActive(false); // Deactivate input box on Esc
                m_activeInputBox = nullptr;
            } else {
                m_running = false; // Exit to start screen if no input box active
                m_nextState = ScreenState::START;
            }
        }
        
        // Prioritize TextEntered for active InputBox
        if (m_activeInputBox && event.type == sf::Event::TextEntered) {
            handleInputBoxEvents(event);
            continue; // Event handled by InputBox, skip other checks for this event
        }
        // Handle KeyPressed for active InputBox (e.g., backspace not covered by TextEntered)
        if (m_activeInputBox && event.type == sf::Event::KeyPressed) {
             handleInputBoxEvents(event);
             // Potentially continue here too if certain keys should only affect input box
        }


        if (event.type == sf::Event::MouseButtonPressed) {
            if (event.mouseButton.button == sf::Mouse::Left) {
                sf::Vector2f mousePosWindow = static_cast<sf::Vector2f>(sf::Mouse::getPosition(m_window));
                // Map mouse to UI view coordinates for UI elements
                sf::Vector2f mousePosUI = m_window.mapPixelToCoords(sf::Mouse::getPosition(m_window), m_uiView);

                InputBox* previouslyActive = m_activeInputBox;
                InputBox* clickedBox = nullptr;

                // Check clicks on InputBoxes
                if (m_inputWidth.getGlobalBounds().contains(mousePosUI)) clickedBox = &m_inputWidth;
                else if (m_inputDepth.getGlobalBounds().contains(mousePosUI)) clickedBox = &m_inputDepth;
                else if (m_inputHeight.getGlobalBounds().contains(mousePosUI)) clickedBox = &m_inputHeight;

                if (previouslyActive && previouslyActive != clickedBox) {
                    previouslyActive->setActive(false); // Deactivate previous
                }
                m_activeInputBox = clickedBox; // Set new active (can be nullptr)
                if (m_activeInputBox) {
                    m_activeInputBox->setActive(true);
                }
                
                // Check clicks on Pollutant Options if no InputBox was clicked or became active
                if (!m_activeInputBox) {
                    for(size_t i=0; i<m_pollutantOptionShapes.size(); ++i) {
                        if (m_pollutantOptionShapes[i].getGlobalBounds().contains(mousePosUI)) {
                            m_selectedPollutantIndex = static_cast<int>(i);
                            break; 
                        }
                    }
                }
                
                // --- START: Added for Passages and Windows ---
                // Check button clicks only if no InputBox is active
                if (!m_activeInputBox) {
                    if (m_shapeCreatePassage.getGlobalBounds().contains(mousePosUI)) {
                        createPassage();
                    } else if (m_shapeRemovePassage.getGlobalBounds().contains(mousePosUI)) {
                        removePassage();
                    } else if (m_shapeCreateWindow.getGlobalBounds().contains(mousePosUI)) {
                        createWindow();
                    } else if (m_shapeRemoveWindow.getGlobalBounds().contains(mousePosUI)) {
                        removeWindow();
                    }
                }
                // --- END: Added for Passages and Windows ---


                // Check Start Simulation button (only if no InputBox active)
                if (m_shapeStartSimulation.getGlobalBounds().contains(mousePosUI) && !m_activeInputBox) {
                    m_nextState = ScreenState::SIMULATION;
                    m_running = false;
                }

                // Check for dragging in 3D view (only if no InputBox active and click is in 3D view)
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
            if (m_isDragging && !m_activeInputBox) { // Only drag if mouse button is held and no input box is active
                sf::Vector2i currentMousePos = sf::Mouse::getPosition(m_window);
                float dx = static_cast<float>(currentMousePos.x - m_lastMousePos.x);
                // float dy = static_cast<float>(currentMousePos.y - m_lastMousePos.y); // For X-axis rotation

                m_rotationY += dx * 0.005f; // Adjust sensitivity as needed
                // m_rotationX -= dy * 0.005f; // Uncomment for X-axis rotation
                                            // Ensure m_rotationX is clamped if necessary (e.g. -PI/2 to PI/2)

                m_lastMousePos = currentMousePos;
                projectVertices(); // Re-project cube and openings on rotation
            }
        }
    }
}

void SettingScreen::updateButtonHovers(const sf::Vector2f& mousePos) {
    // Lambda to update visuals of a button/selectable item
    auto updateVisuals = [&](sf::Text& text, sf::RectangleShape& shape, bool isHovered, bool isSelected = false) {
        text.setFillColor(isHovered || isSelected ? m_buttonTextColorHover : m_buttonTextColorNormal);
        shape.setFillColor(isHovered || isSelected ? m_buttonBgColorHover : m_buttonBgColorNormal);
        if (isSelected) {
            shape.setOutlineColor(sf::Color::Yellow); // Highlight selected pollutant
        } else {
            shape.setOutlineColor(m_buttonOutlineColor);
        }
    };
    
    // Update Pollutant Options
    for(size_t i=0; i<m_pollutantOptions.size(); ++i) {
        bool isSelected = (static_cast<int>(i) == m_selectedPollutantIndex);
        bool isHovered = m_pollutantOptionShapes[i].getGlobalBounds().contains(mousePos);
        updateVisuals(m_pollutantOptions[i], m_pollutantOptionShapes[i], isHovered, isSelected);
    }

    // Update other buttons
    updateVisuals(m_buttonCreatePassage, m_shapeCreatePassage, m_shapeCreatePassage.getGlobalBounds().contains(mousePos));
    updateVisuals(m_buttonCreateWindow, m_shapeCreateWindow, m_shapeCreateWindow.getGlobalBounds().contains(mousePos));
    updateVisuals(m_buttonRemovePassage, m_shapeRemovePassage, m_shapeRemovePassage.getGlobalBounds().contains(mousePos));
    updateVisuals(m_buttonRemoveWindow, m_shapeRemoveWindow, m_shapeRemoveWindow.getGlobalBounds().contains(mousePos));
    updateVisuals(m_buttonStartSimulation, m_shapeStartSimulation, m_shapeStartSimulation.getGlobalBounds().contains(mousePos));
}


void SettingScreen::update(sf::Time dt) {
    // Update InputBoxes (for cursor blinking)
    m_inputWidth.update();
    m_inputDepth.update();
    m_inputHeight.update();

    // Update button hover states based on mouse position in UI view
    sf::Vector2f mousePosUI = m_window.mapPixelToCoords(sf::Mouse::getPosition(m_window), m_uiView);
    updateButtonHovers(mousePosUI);
}

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

// --- START: Added for Passages and Windows ---
void SettingScreen::createPassage() {
    if (m_passages_defs.size() < 2) {
        OpeningDefinition new_passage;
        // Passage dimensions are relative to the cuboid's normalized size (-0.5 to 0.5)
        // For passages on front/back, width is along X, height along Y. Z is fixed.
        float p_h_norm = PASSAGE_RELATIVE_HEIGHT_FACTOR * 0.5f; // Half height for centering
        float p_w_norm = PASSAGE_RELATIVE_WIDTH_FACTOR * 0.5f;  // Half width for centering

        if (m_passages_defs.empty()) { // First passage: Front face (normalized z = -0.5)
            new_passage.local_coords = {{
                {-p_w_norm, -p_h_norm, -0.5f}, { p_w_norm, -p_h_norm, -0.5f},
                { p_w_norm,  p_h_norm, -0.5f}, {-p_w_norm,  p_h_norm, -0.5f}
            }};
        } else { // Second passage: Back face (normalized z = 0.5)
             new_passage.local_coords = {{
                {-p_w_norm, -p_h_norm,  0.5f}, { p_w_norm, -p_h_norm,  0.5f},
                { p_w_norm,  p_h_norm,  0.5f}, {-p_w_norm,  p_h_norm,  0.5f}
            }};
        }
        m_passages_defs.push_back(new_passage);
        projectVertices(); // Re-project to show changes (though openings are drawn separately, good for consistency if needed)
    }
}

void SettingScreen::removePassage() {
    if (!m_passages_defs.empty()) {
        m_passages_defs.pop_back(); // Removes the last added: Back then Front
        projectVertices(); 
    }
}

void SettingScreen::createWindow() {
    if (m_windows_defs.size() < 2) {
        OpeningDefinition new_window;
        // For windows on left/right, width is along Z, height along Y. X is fixed.
        float w_h_norm = WINDOW_RELATIVE_HEIGHT_FACTOR * 0.5f; // Half height
        float w_d_norm = WINDOW_RELATIVE_WIDTH_FACTOR * 0.5f;  // Half depth/width_on_side

        if (m_windows_defs.empty()) { // First window: Left face (normalized x = -0.5)
            new_window.local_coords = {{
                {-0.5f, -w_h_norm, -w_d_norm}, {-0.5f, -w_h_norm,  w_d_norm},
                {-0.5f,  w_h_norm,  w_d_norm}, {-0.5f,  w_h_norm, -w_d_norm}
            }};
        } else { // Second window: Right face (normalized x = 0.5)
            new_window.local_coords = {{
                { 0.5f, -w_h_norm, -w_d_norm}, { 0.5f, -w_h_norm,  w_d_norm},
                { 0.5f,  w_h_norm,  w_d_norm}, { 0.5f,  w_h_norm, -w_d_norm}
            }};
        }
        m_windows_defs.push_back(new_window);
        projectVertices();
    }
}

void SettingScreen::removeWindow() {
    if (!m_windows_defs.empty()) {
        m_windows_defs.pop_back(); // Removes the last added: Right then Left
        projectVertices();
    }
}

void SettingScreen::drawOpenings(sf::RenderWindow& window) {
    auto draw_one_opening_shape = [&](const OpeningDefinition& def) {
        std::array<sf::Vector2f, 4> screen_points;
        
        // Ensure dimensions are positive for scaling the opening
        float w = (m_roomWidth > 0.1f) ? m_roomWidth : 0.1f;
        float d = (m_roomDepth > 0.1f) ? m_roomDepth : 0.1f;
        float h = (m_roomHeight > 0.1f) ? m_roomHeight : 0.1f;

        // Scale factor for the 3D view (same as in projectVertices)
        float maxDim = std::max({w, d, h, 1.f});
        float viewScaleFactor = 350.f / maxDim;

        for (size_t i = 0; i < 4; ++i) {
            const Vec3D& v_local_norm = def.local_coords[i]; // Normalized (-0.5 to 0.5)

            // 1. Scale normalized local coords by actual room dimensions
            Vec3D v_world_scaled;
            v_world_scaled.x = v_local_norm.x * w;
            v_world_scaled.y = v_local_norm.y * h; 
            v_world_scaled.z = v_local_norm.z * d;

            // 2. Rotate (same rotation as the main cuboid)
            float x_rot_y = v_world_scaled.x * std::cos(m_rotationY) - v_world_scaled.z * std::sin(m_rotationY);
            float z_rot_y = v_world_scaled.x * std::sin(m_rotationY) + v_world_scaled.z * std::cos(m_rotationY);
            
            float y_rotated = v_world_scaled.y * std::cos(m_rotationX) - z_rot_y * std::sin(m_rotationX);
            float z_rotated_final = v_world_scaled.y * std::sin(m_rotationX) + z_rot_y * std::cos(m_rotationX);
            
            // 3. Apply view scale factor (same as for cuboid vertices)
            Vec3D v_transformed_for_projection = {
                x_rot_y * viewScaleFactor,
                y_rotated * viewScaleFactor,
                z_rotated_final * viewScaleFactor
            };
            
            // 4. Project to 2D screen space
            screen_points[i] = project(v_transformed_for_projection);
        }

        // Draw the 4 lines of the opening
        for (size_t i = 0; i < 4; ++i) {
            sf::Vertex line[] = {
                sf::Vertex(screen_points[i], sf::Color::White),
                sf::Vertex(screen_points[(i + 1) % 4], sf::Color::White) // Connect to next point, wrap around for last
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
// --- END: Added for Passages and Windows ---

void SettingScreen::render() {
    m_window.clear(sf::Color::Black);

    // Draw 3D View
    m_window.setView(m_3dView);
    drawCuboidEdges(m_window);
    // --- START: Added for Passages and Windows ---
    drawOpenings(m_window); // Draw passages and windows after the cuboid
    // --- END: Added for Passages and Windows ---

    // Draw UI View
    m_window.setView(m_uiView);
    m_window.draw(m_titleText);

    // Input fields and labels
    m_inputWidth.render(m_window); m_window.draw(m_labelWidth);
    m_inputDepth.render(m_window); m_window.draw(m_labelDepth);
    m_inputHeight.render(m_window); m_window.draw(m_labelHeight);

    // Pollutant selection
    m_window.draw(m_labelPollutant);
    for(size_t i=0; i<m_pollutantOptions.size(); ++i) {
        m_window.draw(m_pollutantOptionShapes[i]);
        m_window.draw(m_pollutantOptions[i]);
    }

    // Buttons
    m_window.draw(m_shapeCreatePassage); m_window.draw(m_buttonCreatePassage);
    m_window.draw(m_shapeCreateWindow);  m_window.draw(m_buttonCreateWindow);
    m_window.draw(m_shapeRemovePassage); m_window.draw(m_buttonRemovePassage);
    m_window.draw(m_shapeRemoveWindow);  m_window.draw(m_buttonRemoveWindow);
    m_window.draw(m_shapeStartSimulation);m_window.draw(m_buttonStartSimulation);

    m_window.setView(m_window.getDefaultView()); // Reset to default view before display
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