//
// Created by Chaeyoung Lim on 2022/04/24.
//

#include "Fluid.h"

Fluid::Fluid() :
    old_velocity(Grid<vec2f>(nx, ny)), new_velocity(Grid<vec2f>(nx, ny)),
    old_density(Grid<float>(nx, ny)), new_density(Grid<float>(nx, ny)),
    m_threadNum(std::thread::hardware_concurrency()),
    pixels(Grid<uint32_t>(nx, ny)), abs_curl(Grid<float>(nx, ny)),
    p(Grid<float>(nx, ny)), div(Grid<float>(nx, ny))
    {
    for (int y = 0; y < ny; y++) {
        for (int x = 0; x < nx; x++) {
            old_density(x, y) = 0.0f;
            old_velocity(x, y) = vec2f{0.0f, 0.0f};
        }
    }
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, nx, ny, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
}

void Fluid::SimulationStep(){
    MultiThreading(Fluid::MoveOldProperty, SwapT::NOTHING);
    for (auto point : m_addPoint)
        AddDensity(point.x, point.y);
    MultiThreading(Fluid::VorticityConfinement, SwapT::VELOCITY);
    MultiThreading(Fluid::DiffuseVelocity, SwapT::VELOCITY);
    MultiThreading(Fluid::InitializeP, SwapT::NOTHING);
    MultiThreading(Fluid::ProjectDiv, SwapT::NOTHING);
    MultiThreading(Fluid::ProjectVelocity, SwapT::VELOCITY);
    MultiThreading(Fluid::AdvectVelocity, SwapT::VELOCITY);
    MultiThreading(Fluid::InitializeP, SwapT::NOTHING);
    MultiThreading(Fluid::ProjectDiv, SwapT::NOTHING);
    MultiThreading(Fluid::ProjectVelocity, SwapT::VELOCITY);
    MultiThreading(Fluid::DiffuseDensity, SwapT::DENSITY);
    MultiThreading(Fluid::AdvectDensity, SwapT::DENSITY);
    MultiThreading(Fluid::ExtractZero, SwapT::DENSITY);
}

float Fluid::randf(float a, float b){
    float u = rand()*(1.0f/RAND_MAX);
    return lerp(a, b, u);
}

float Fluid::sign(float x){
    return
            x > 0.0f ? +1.0f :
            x < 0.0f ? -1.0f :
            0.0f;
}

void Fluid::AddingPoint(float x, float y) {
    m_addPoint.push_back({x, y});
}

void Fluid::MultiThreading(void func(Fluid *, int, int), SwapT flag) {
    std::thread thr1(func, this, 0, ny / 8);
    std::thread thr2(func, this, ny / 8, ny * 2 / 8);
    std::thread thr3(func, this, ny * 2 / 8, ny * 3 / 8);
    std::thread thr4(func, this, ny * 3 / 8, ny * 4 / 8);
    std::thread thr5(func, this, ny * 4 / 8, ny * 5 / 8);
    std::thread thr6(func, this, ny * 5 / 8, ny * 6 / 8);
    std::thread thr7(func, this, ny * 6 / 8, ny * 7 / 8);
    std::thread thr8(func, this, ny * 7 / 8, ny);
    thr1.join();
    thr2.join();
    thr3.join();
    thr4.join();
    thr5.join();
    thr6.join();
    thr7.join();
    thr8.join();
    switch (flag)
    {
        case SwapT::DENSITY:
            old_density.swap(new_density);
            break;
        case SwapT::VELOCITY:
            old_velocity.swap(new_velocity);
            break;
        default:
            break;
    }
}

void Fluid::MoveOldProperty(Fluid *target, int start, int end){
    for (int y = start; y < end; y++) {
        for (int x = 0; x < nx; x++) {
            if (x <= nx*0.5f) {
                float r = 10.0f;
                target->old_velocity(x, y).x += randf(-r, +r);
                target->old_velocity(x, y).y += randf(-r, +r);
            }
            // dense regions rise up
            target->old_velocity(x, y).y += (target->old_density(x, y) * 20.0f - 5.0f) * dt;
            // fast movement is dampened
            target->old_velocity(x, y) *= 0.999f;
            // fade away
            target->old_density(x, y) *= 0.99f;
        }
    }
}

void Fluid::AdvectDensity(Fluid *target, int start, int end){
    for (int y = start; y < end; y++) {
        for (int x = 0; x < nx; x++) {
            vec2f pos = v2f(x, y) - dt * target->old_velocity(x, y);
            target->new_density(x, y) = interpolate(target->old_density, pos);
        }
    }
}

void Fluid::AdvectVelocity(Fluid *target, int start, int end){
    for (int y = start; y < end; y++) {
        for (int x = 0; x < nx; x++) {
            vec2f pos = v2f(x, y) - dt * target->old_velocity(x, y);
            target->new_velocity(x, y) = interpolate(target->old_velocity, pos);
        }
    }
}

void Fluid::DiffuseDensity(Fluid *target, int start, int end){
    for (int y = start; y < end; y++) {
        for (int x = 0; x < nx; x++) {
            float sum =
                    diffusion * (
                            + target->old_density(x - 1, y + 0)
                            + target->old_density(x + 1, y + 0)
                            + target->old_density(x + 0, y - 1)
                            + target->old_density(x + 0, y + 1)
                    )
                    + target->old_density(x + 0, y + 0);
            target->new_density(x, y) = 1.0f / (1.0f + 4.0f * diffusion) * sum;
        }
    }
}

void Fluid::DiffuseVelocity(Fluid *target, int start, int end){
    for (int y = start; y < end; y++) {
        for (int x = 0; x < nx; x++) {
            vec2f sum =
                    viscosity * (
                            + target->old_velocity(x - 1, y + 0)
                            + target->old_velocity(x + 1, y + 0)
                            + target->old_velocity(x + 0, y - 1)
                            + target->old_velocity(x + 0, y + 1)
                    )
                    + target->old_velocity(x + 0, y + 0);
            target->new_velocity(x, y) = 1.0f / (1.0f + 4.0f * viscosity) * sum;
        }
    }
}

void Fluid::InitializeP(Fluid *target, int start, int end){
    for (int y = start; y < end; y++) {
        for (int x = 0; x < nx; x++) {
            target->div(x, y) = target->old_velocity(x + 1, y + 0).x - target->old_velocity(x - 1, y + 0).x
                    + target->old_velocity(x + 0, y + 1).y - target->old_velocity(x + 0, y - 1).y;
            target->p(x, y) = 0.0f;
        }
    }
}

void Fluid::ProjectDiv(Fluid *target, int start, int end){
    for (int k = 0; k < iterations; k++){
        for (int y = start; y < end; y++) {
            for (int x = 0; x < nx; x++) {
                target->p(x, y) = -0.25f * target->div(x, y);
            }
        }
    }
}

void Fluid::ProjectVelocity(Fluid *target, int start, int end){
    for (int y = start; y < end; y++) {
        for (int x = 0; x < nx; x++) {
            target->old_velocity(x, y).x -= 0.5f * (target->div(x + 1, y + 0) - target->div(x - 1, y + 0));
            target->old_velocity(x, y).y -= 0.5f * (target->div(x + 0, y + 1) - target->div(x + 0, y - 1));
        }
    }
}

void Fluid::VorticityConfinement(Fluid *target, int start, int end){
    for (int y = start; y < end; y++) {
        for (int x = 0; x < nx; x++) {
            target->abs_curl(x, y) = fabsf(Curl(target, x, y));
        }
    }

    for (int y = start; y < end; y++) {
        for (int x = 0; x < nx; x++) {
            vec2f direction;
            direction.x = target->abs_curl(x + 0, y - 1) - target->abs_curl(x + 0, y + 1);
            direction.y = target->abs_curl(x + 1, y + 0) - target->abs_curl(x - 1, y + 0);

            direction = vorticity / (length(direction) + 1e-5f) * direction;

            if (x < nx / 2) direction *= 0.0f;

            target->new_velocity(x, y) = target->old_velocity(x, y) + dt * Curl(target, x, y) * direction;
        }
    }
}

void Fluid::ExtractZero(Fluid *target, int start, int end){
    // zero out stuff at bottom
    for (int y = start; y < end; y++) {
        for (int x = 0; x < nx; x++) {
            if (y < 10) {
                target->old_density(x, y) = 0.0f;
                target->old_velocity(x, y) = vec2f{0.0f, 0.0f};
            }
        }
    }
}

void Fluid::AddDensity(int px, int py, int r, float value){
    for (int y = -r; y <= r; y++) {
        for (int x = -r; x <= r; x++){
            float d = sqrtf(x*x + y*y);
            float u = smoothstep(float(r), 0.0f, d);
            old_density(px + x, py + y) += u*value;
        }
    }
}

void Fluid::Draw(const vec2f *data, int n, GLenum mode){
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, data);
    glDrawArrays(mode, 0, n);
}

void Fluid::DrawCircle(float x, float y, float r, int n){
    vec2f pos[n];
    for (int i = 0; i < n; i++){
        float angle = 2.0f*3.14159f*i/n;
        pos[i] = vec2f{x, y} + r*polar(angle);
    }
    Draw(pos, n, GL_LINE_LOOP);
}

float Fluid::Curl(Fluid *target, int x, int y){
    return
            target->old_velocity(x, y + 1).x - target->old_velocity(x, y - 1).x +
            target->old_velocity(x - 1, y).y - target->old_velocity(x + 1, y).y;
}

uint32_t Fluid::rgba32(uint32_t r, uint32_t g, uint32_t b, uint32_t a){
    r = clamp(r, 0u, 255u);
    g = clamp(g, 0u, 255u);
    b = clamp(b, 0u, 255u);
    a = clamp(a, 0u, 255u);
    return (a << 24) | (b << 16) | (g << 8) | r;
}

uint32_t Fluid::rgba(float r, float g, float b, float a){
    return rgba32(r*256, g*256, b*256, a*256);
}

void Fluid::OnFrame(){
    SimulationStep();
    // density field to pixels
    for (int y = 0; y < ny; y++) {
        for (int x = 0; x < nx; x++) {
            float f = old_density(x, y);
            f = log2f(f * 0.25f + 1.0f);
            float f3 = f * f * f;
            float r = 1.5f * f;
            float g = 1.5f * f3;
            float b = f3 * f3;
            pixels(x, y) = rgba(r, g, b, 1.0);
        }
    }
    // upload pixels to texture
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, nx, ny, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
}

