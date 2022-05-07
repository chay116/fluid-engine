//
// Created by Chaeyoung Lim on 2022/04/24.
//

#ifndef FLUID_ENGINE_FLUID_H
#define FLUID_ENGINE_FLUID_H

#include "Grid.h"
#include <vector>
#include <thread>
#define FOR_EACH_CELL

CLASS_PTR(Fluid)

class Fluid {
public:
    Fluid();
    ~Fluid() = default;
    void SimulationStep();
    void OnFrame();
    void AddingPoint(float x, float y);
    GLuint texture{0};

    enum class SwapT{
        NOTHING,
        DENSITY,
        VELOCITY
    };
    static constexpr int nx = 512;
    static constexpr int ny = 512;

private:
    void MultiThreading(void (Fluid *, int, int), SwapT flag);
    static void MoveOldProperty(Fluid *target, int start, int end);
    static void VorticityConfinement(Fluid *target, int start, int end);
    static void AdvectDensity(Fluid *target, int start, int end);
    static void AdvectVelocity(Fluid *target, int start, int end);
    static void DiffuseDensity(Fluid *target, int start, int end);
    static void InitializeP(Fluid *target, int start, int end);
    static void ProjectDiv(Fluid *target, int start, int end);
    static void ProjectVelocity(Fluid *target, int start, int end);
    static void DiffuseVelocity(Fluid *target, int start, int end);
    static void ExtractZero(Fluid *target, int start, int end);

    void AddDensity(int px, int py, int r = 10, float value = 0.5f);
    void Draw(const vec2f *data, int n, GLenum mode);
    void DrawCircle(float x, float y, float r, int n = 100);

    static float randf(float a, float b);
    static float sign(float x);
    static float Curl(Fluid *target, int x, int y);
    static uint32_t rgba32(uint32_t r, uint32_t g, uint32_t b, uint32_t a);
    static uint32_t rgba(float r, float g, float b, float a);

    unsigned int m_threadNum;
    Grid<vec2f> old_velocity;
    Grid<vec2f> new_velocity;
    Grid<float> old_density;
    Grid<float> new_density;
    Grid<uint32_t> pixels;
    std::vector<vec2f> m_addPoint;

    Grid<float> p;
    Grid<float> div;
    Grid<float> abs_curl;


    static constexpr float vorticity = 10.0f;
    static constexpr float dt = 0.02f;
    static constexpr int iterations = 5;
    static constexpr float viscosity = dt*0.000001f;
    static constexpr float diffusion = dt*100.01f;
};


#endif //FLUID_ENGINE_FLUID_H
