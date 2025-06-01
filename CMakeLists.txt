set(CMAKE_CXX_STANDARD 20)
cmake_minimum_required(VERSION 3.22 FATAL_ERROR)

set(NAME iaps)
project(${NAME})
set(CMAKE_SKIP_BUILD_RPATH  FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_INSTALL_RPATH "${ORIGIN}")

if(APPLE)
    list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew/Cellar/sfml@2/2.6.2_1/")
endif()

find_package(SFML 2.6.2 COMPONENTS system window graphics audio REQUIRED)

if(DEFINED TORCH_CXX_FLAGS)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TORCH_CXX_FLAGS}")
else()
    message(WARNING "TORCH_CXX_FLAGS is not defined. CXX flags will not be modified by it.")
endif()


include_directories(include)

add_executable(${NAME}
    src/main.cpp
    src/screen/Screen.cpp
    src/setting/Setting.cpp
    src/simulation/Simulation.cpp
)

target_link_libraries(${NAME} PRIVATE sfml-system sfml-window sfml-graphics sfml-audio)