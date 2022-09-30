//
// Created by Chaeyoung Lim on 2022/04/24.
//

#ifndef FLUID_ENGINE_FLUID_H
#define FLUID_ENGINE_FLUID_H

#include "Grid.h"
#include <vector>
#include <thread>
#include <list>

CLASS_PTR(Fluid)

class Fluid {
public:
    Fluid();
    ~Fluid();
    void SimulationStep();
    void OnFrame();
    void AddingPoint(float x, float y);
    GLuint texture{0};
    static constexpr int nx = 256;
    static constexpr int ny = 256;

private:
    void MoveOldProperty(int start, int end);
    void VorticityConfinement(int start, int end);
    void AdvectDensity(int start, int end);
    void AdvectVelocity(int start, int end);
    void DiffuseDensity(int start, int end);
    void InitializeP(int start, int end);
    void ProjectDiv(int start, int end);
    void ProjectVelocity(int start, int end);
    void DiffuseVelocity(int start, int end);
    void ExtractZero(int start, int end);
    void AddDensity(int px, int py, int r = 10, float value = 0.5f);

    static void ThreadSwapping(Fluid* target, int start, int end);
    static void ThreadCalculating(Fluid* target, int start, int end);
    float Curl(int x, int y);
    static float randf(float a, float b);
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
    Grid<float> p2;
    Grid<float> div;
    Grid<float> abs_curl;
    std::list<std::thread> m_threads;

    bool m_finished;
    static constexpr float vorticity = 10.0f;
    static constexpr float dt = 0.02f;
    static constexpr int iterations = 5;
    static constexpr float viscosity = dt*0.000001f;
    static constexpr float diffusion = dt*100.01f;
};


#endif //FLUID_ENGINE_FLUID_H
