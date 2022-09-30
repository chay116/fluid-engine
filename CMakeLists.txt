cmake_minimum_required(VERSION 3.20)
project(fluid-engine)

set(CMAKE_CXX_STANDARD 20)

set(WINDOW_NAME )
set(WINDOW_WIDTH 512)
set(WINDOW_HEIGHT 512)

if (APPLE)
    set(CMAKE_CXX_FLAGS "-framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo")
endif()

project(${PROJECT_NAME})
add_executable(${PROJECT_NAME} src/main.cpp src/common.cpp src/common.h src/Context.cpp src/Context.h src/Shader.cpp src/Shader.h src/Vertex_layout.cpp src/Vertex_layout.h src/Program.cpp src/Program.h src/Buffer.cpp src/Buffer.h src/Grid.h src/Fluid.cpp src/Fluid.h)

include(Dependency.cmake)

# 우리 프로젝트에 include / lib 관련 옵션 추가
target_include_directories(${PROJECT_NAME} PUBLIC ${DEP_INCLUDE_DIR})
target_link_directories(${PROJECT_NAME} PUBLIC ${DEP_LIB_DIR})
target_link_libraries(${PROJECT_NAME} PUBLIC ${DEP_LIBS})

target_compile_definitions(${PROJECT_NAME} PUBLIC
        WINDOW_NAME="${WINDOW_NAME}"
        WINDOW_WIDTH=${WINDOW_WIDTH}
        WINDOW_HEIGHT=${WINDOW_HEIGHT}
        )

# Dependency들이 먼저 build 될 수 있게 관계 설정
add_dependencies(${PROJECT_NAME} ${DEP_LIST})
