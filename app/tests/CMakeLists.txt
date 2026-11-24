# Pruebas automatizadas de FarmaciaApp (Qt Test, plataforma "offscreen": no abren ventanas).
# Se compilan aparte de la app:   cmake -S app/tests -B app/tests/build && cmake --build app/tests/build -j
# Se ejecutan con:                app/tests/run_tests.sh
cmake_minimum_required(VERSION 3.16)
project(FarmaciaTests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Sql Svg Network PrintSupport Test)

set(APP ${CMAKE_CURRENT_SOURCE_DIR}/..)
file(GLOB_RECURSE SRCS CONFIGURE_DEPENDS ${APP}/src/*.cpp ${APP}/src/*.ui ${APP}/src/*.h)
list(REMOVE_ITEM SRCS ${APP}/src/main.cpp)

# Todo el código de la app menos main(), compilado una sola vez y compartido por los tests.
add_library(farmacia_core OBJECT ${SRCS} ${APP}/resources/resources.qrc)
target_include_directories(farmacia_core PUBLIC ${APP}/src)
target_link_libraries(farmacia_core PUBLIC Qt6::Widgets Qt6::Sql Qt6::Svg Qt6::Network Qt6::PrintSupport Qt6::Test)

foreach(t validacion humo ventas bd formularios acceso)
    add_executable(test_${t} test_${t}.cpp)
    target_include_directories(test_${t} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_link_libraries(test_${t} PRIVATE farmacia_core)
endforeach()
