//
// Created by Chaeyoung Lim on 2022/02/10.
//

#include "Context.h"
#include <imgui.h>
#include <algorithm>

void Context::Screenshot(const char *path){
    std::vector<uint32_t> rgba(m_width * m_height);
    std::vector<uint8_t> rgb(m_width * m_height * 3);

    glReadPixels(0, 0, m_width, m_height, GL_RGBA, GL_UNSIGNED_BYTE, rgba.data());

    int i = 0;
    for (int y = m_height - 1; y >= 0; y--) for (int x = 0; x < m_width; x++){
            uint32_t c = rgba[x + y * m_width];
            rgb[i++] = (c >> 0*8) & 255;
            rgb[i++] = (c >> 1*8) & 255;
            rgb[i++] = (c >> 2*8) & 255;
        }

    FILE *fp = fopen(path, "wb");
    assert(fp);
    fprintf(fp, "P6\n%i %i\n255\n", m_width, m_height);
    fwrite(rgb.data(), 1, rgb.size(), fp);
    fclose(fp);
}

ContextUPtr Context::Create() {
    auto context = ContextUPtr(new Context());
    if (!context->Init())
        return nullptr;
    return std::move(context);
}

void Context::Reshape(int width, int height) {
    m_width = width;
    m_height = height;
    glViewport(0, 0, m_width, m_height);
}

void Context::MouseMove(double x, double y) {
    return ;
}
#include <iostream>
void Context::MouseButton(int button, int action, double x, double y) {
    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        if (action == GLFW_PRESS) {
            std::cout << m_width << " " << m_height << std::endl;
            std::cout << x << " " << y << std::endl;
            m_fluid->AddingPoint(x * m_fluid->nx * 2 / (float)m_width, (m_height - y) * m_fluid->ny * 2 / (float)m_height);
//            m_prevMousePos = glm::vec2((float)x, (float)y);
//            m_cameraControl = true;
        }
    }
}

void Context::Render() {
//    glClearColor(0.1f, 0.2f, 0.3f, 0.0f);

//    if (ImGui::Begin("ui window")) {
//        ImGui::ColorEdit3("##RingColor", m_color);
//        if (ImGui::CollapsingHeader("fluid", ImGuiTreeNodeFlags_DefaultOpen)) {
//            ImGui::DragFloat3("number", glm::value_ptr(m_particle.number), 0.01f);
//            ImGui::DragFloat3("viscocity", glm::value_ptr(m_particle.viscocity), 100);
//            ImGui::ColorEdit3("color", glm::value_ptr(m_particle.color));
//        }
//
//        ImGui::Button("Start", &m_start);
//        ImGui::Separator();
//    }
//    ImGui::End();

    glClear(GL_COLOR_BUFFER_BIT);

    m_program->Use();
    m_fluid->OnFrame();
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindTexture( GL_TEXTURE_2D, 0);
//    glUniform1i(glGetUniformLocation(m_program->Get(), "tex"), 0);
}

bool Context::Init() {
    m_program = Program::Create("../assets/shaders/texture.vs", "../assets/shaders/texture.fs");
    if (!m_program)
        return false;
    m_fluid = FluidUPtr(new Fluid());

    float vertices[] = {
            1.f, 1.f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
            1.f, -1.f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
            -1.f, -1.f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
            -1.f, 1.f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
    };
    uint32_t indices[] = { // note that we start from 0!
            0, 1, 3, // first triangle
            1, 2, 3, // second triangle
    };

    m_vertexLayout = VertexLayout::Create();
    m_vertexBuffer = Buffer::CreateWithData(
            GL_ARRAY_BUFFER, GL_STATIC_DRAW,
            vertices, sizeof(float), 32);

    m_vertexLayout->SetAttrib(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, 0);
    m_vertexLayout->SetAttrib(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, sizeof(float) * 3);
    m_vertexLayout->SetAttrib(2, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 8, sizeof(float) * 6);

    m_indexBuffer = Buffer::CreateWithData(
            GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW,
            indices, sizeof(uint32_t), 6);

    glClearColor(0.1f, 0.2f, 0.3f, 0.0f);
    return true;
}
