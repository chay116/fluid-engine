//
// Created by Chaeyoung Lim on 2022/02/24.
//

#ifndef FLUID_ENGINE_CONTEXT_H
#define FLUID_ENGINE_CONTEXT_H

#include "common.h"
#include "Shader.h"
#include "Program.h"
#include "Buffer.h"
#include "Vertex_layout.h"
#include <ctime>
#include "Fluid.h"

CLASS_PTR(Context)

class Context {
public:
    static ContextUPtr Create();
    void Render();
    void ProcessInput(GLFWwindow* window);
    void Reshape(int width, int height);
    void MouseMove(double x, double y);
    void MouseButton(int button, int action, double x, double y);
    void Screenshot(const char *path);

private:
    Context() {};

    bool Init();

    int m_width{WINDOW_WIDTH};
    int m_height{WINDOW_HEIGHT};
    float m_color[3] = {0.0, 0.0, 0.0};

    FluidUPtr m_fluid;
    ProgramUPtr m_program;
    VertexLayoutUPtr m_vertexLayout;
    BufferUPtr m_vertexBuffer;
    BufferUPtr m_indexBuffer;
};

#endif //FLUID_ENGINE_CONTEXT_H
