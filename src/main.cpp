#include <spdlog/spdlog.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "Context.h"

void OnFramebufferSizeChange(GLFWwindow* window, int width, int height) {
    SPDLOG_INFO("framebuffer size changed: ({} x {})", width, height);
    auto context = (Context*)glfwGetWindowUserPointer(window);
    context->Reshape(width, height);
}


void OnMouseButton(GLFWwindow* window, int button, int action, int modifier) {
    auto context = (Context*)glfwGetWindowUserPointer(window);
    double x, y;
    glfwGetCursorPos(window, &x, &y);
    context->MouseButton(button, action, x, y);
}


int main(int argc, const char **argv) {
    // Logging start of program
    SPDLOG_INFO("Start program");

    // Initializing glfw library
    SPDLOG_INFO("Initialize glfw");
    if (!glfwInit()) {
        const char *description = nullptr;
        glfwGetError(&description);
        SPDLOG_ERROR("failed to initialize glfw: {}", description);
        return -1;
    }

    // Setup GLFW window properties
    // OpenGL version
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    // Core Profile = No Backwards Compatibility
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // Allow Forward Compatibility
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_SAMPLES, 4);
    // Create GLFW window
    SPDLOG_INFO("Create glfw window");
    GLFWwindow *window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_NAME, nullptr, nullptr);
    if (!window) {
        SPDLOG_ERROR("failed to create glfw window");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // OpenGL function loading with glad
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        SPDLOG_ERROR("failed to initialize glad");
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // OpenGL Version check
    auto glVersion = glGetString(GL_VERSION);
    SPDLOG_INFO("OpenGL context version: {}", glVersion);

    auto context = Context::Create();
    if (!context) {
        SPDLOG_ERROR("failed to create context");
        glfwTerminate();
        return -1;
    }
    glfwSetWindowUserPointer(window, context.get());

    glfwSetFramebufferSizeCallback(window, OnFramebufferSizeChange);
    glfwSetMouseButtonCallback(window, OnMouseButton);
    OnFramebufferSizeChange(window, WINDOW_WIDTH * 2, WINDOW_HEIGHT * 2);


    // glfw 루프 실행, 윈도우 close 버튼을 누르면 정상 종료
    SPDLOG_INFO("Start main loop");
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        context->Render();
        glfwSwapBuffers(window);
    }
    glfwTerminate();
    return 0;
}

