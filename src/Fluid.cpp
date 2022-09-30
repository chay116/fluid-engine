//
// Created by Chaeyoung Lim on 2022/04/24.
//

#include "Fluid.h"
#include <barrier>


static void Finished() {};

std::barrier s_barrier(4, Finished);
std::barrier sync_point(2, Finished);

Fluid::Fluid() :
    old_velocity(Grid<vec2f>(nx, ny)), new_velocity(Grid<vec2f>(nx, ny)),
    old_density(Grid<float>(nx, ny)), new_density(Grid<float>(nx, ny)),
    m_threadNum(std::thread::hardware_concurrency()),
    pixels(Grid<uint32_t>(nx, ny)), abs_curl(Grid<float>(nx, ny)),
    p(Grid<float>(nx, ny)), p2(Grid<float>(nx, ny)), div(Grid<float>(nx, ny))
    {
    m_finished = false;
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
    {
        std::thread worker_thread(Fluid::ThreadSwapping, this, 0, ny/4);
        m_threads.push_back(std::move(worker_thread));
    }
    {
        std::thread worker_thread(Fluid::ThreadCalculating, this, (ny)/4, (ny * 2)/4);
        m_threads.push_back(std::move(worker_thread));
    }
    {
        std::thread worker_thread(Fluid::ThreadCalculating, this, (ny * 2)/4, (ny * 3)/4);
        m_threads.push_back(std::move(worker_thread));
    }
    {
        std::thread worker_thread(Fluid::ThreadCalculating, this, (ny * 3)/4, ny);
        m_threads.push_back(std::move(worker_thread));
    }
}

Fluid::~Fluid()
{
    m_finished = true;
    for (auto& m_thread : m_threads)
        m_thread.join();
}

void Fluid::ThreadSwapping(Fluid* target, int start, int end){
    while(!target->m_finished)
    {
        sync_point.arrive_and_wait();
        s_barrier.arrive_and_wait();
        target->VorticityConfinement(start, end);

        s_barrier.arrive_and_wait();
        target->old_velocity.swap(target->new_velocity);

        s_barrier.arrive_and_wait();
        target->DiffuseVelocity(start, end);

        s_barrier.arrive_and_wait();
        target->old_velocity.swap(target->new_velocity);


        s_barrier.arrive_and_wait();
        target->InitializeP(start, end);

        for (int k = 0; k < iterations; k++)
        {
            s_barrier.arrive_and_wait();
            target->ProjectDiv(start, end);
        }

        s_barrier.arrive_and_wait();
        target->ProjectVelocity(start, end);

        s_barrier.arrive_and_wait();
        target->AdvectVelocity(start, end);

        s_barrier.arrive_and_wait();
        target->old_velocity.swap(target->new_velocity);


        s_barrier.arrive_and_wait();
        target->InitializeP(start, end);

        for (int k = 0; k < iterations; k++)
        {
            s_barrier.arrive_and_wait();
            target->ProjectDiv(start, end);
        }

        s_barrier.arrive_and_wait();
        target->ProjectVelocity(start, end);

        s_barrier.arrive_and_wait();
        target->DiffuseDensity(start, end);

        s_barrier.arrive_and_wait();
        target->old_density.swap(target->new_density);

        s_barrier.arrive_and_wait();
        target->AdvectDensity(start, end);

        s_barrier.arrive_and_wait();
        target->old_density.swap(target->new_density);

        s_barrier.arrive_and_wait();
        target->ExtractZero(start, end);
    }

}

void Fluid::ThreadCalculating(Fluid* target, int start, int end){
    while(!target->m_finished)
    {
        s_barrier.arrive_and_wait();
        target->VorticityConfinement(start, end);

        s_barrier.arrive_and_wait();
        s_barrier.arrive_and_wait();
        target->DiffuseVelocity(start, end);

        s_barrier.arrive_and_wait();
        s_barrier.arrive_and_wait();
        target->InitializeP(start, end);

        for (int k = 0; k < iterations; k++)
        {
            s_barrier.arrive_and_wait();
            target->ProjectDiv(start, end);
        }

        s_barrier.arrive_and_wait();
        target->ProjectVelocity(start, end);

        s_barrier.arrive_and_wait();
        target->AdvectVelocity(start, end);

        s_barrier.arrive_and_wait();
        s_barrier.arrive_and_wait();
        target->InitializeP(start, end);

        for (int k = 0; k < iterations; k++)
        {
            s_barrier.arrive_and_wait();
            target->ProjectDiv(start, end);
        }

        s_barrier.arrive_and_wait();
        target->ProjectVelocity(start, end);

        s_barrier.arrive_and_wait();
        target->DiffuseDensity(start, end);

        s_barrier.arrive_and_wait();
        s_barrier.arrive_and_wait();
        target->AdvectDensity(start, end);

        s_barrier.arrive_and_wait();
        s_barrier.arrive_and_wait();
        target->ExtractZero(start, end);
    }

}

void Fluid::SimulationStep(){

    VorticityConfinement(0, ny);
    old_velocity.swap(new_velocity);
    DiffuseVelocity(0, ny);
    old_velocity.swap(new_velocity);

    InitializeP(0, ny);
    for (int k = 0; k < iterations; k++)
        ProjectDiv(0, ny);
    ProjectVelocity(0, ny);

    AdvectVelocity(0, ny);
    old_velocity.swap(new_velocity);

    InitializeP(0, ny);
    for (int k = 0; k < iterations; k++)
        ProjectDiv(0, ny);
    ProjectVelocity(0, ny);

    DiffuseDensity(0, ny);
    old_density.swap(new_density);
    AdvectDensity(0, ny);
    old_density.swap(new_density);
    ExtractZero(0, ny);
}

float Fluid::randf(float a, float b){
    float u = rand()*(1.0f/RAND_MAX);
    return lerp(a, b, u);
}


void Fluid::AddingPoint(float x, float y) {
    m_addPoint.push_back({x, y});
}

void Fluid::MoveOldProperty(int start, int end){
    for (int y = start; y < end; y++) {
        for (int x = 0; x < nx; x++) {
            if (x <= nx*0.5f) {
                float r = 10.0f;
                old_velocity(x, y).x += randf(-r, +r);
                old_velocity(x, y).y += randf(-r, +r);
            }
            // dense regions rise up
            old_velocity(x, y).y += (old_density(x, y) * 20.0f - 5.0f) * dt;
            // fast movement is dampened
            old_velocity(x, y) *= 0.999f;
            // fade away
            old_density(x, y) *= 0.99f;
        }
    }
}

void Fluid::AdvectDensity(int start, int end){
    for (int y = start; y < end; y++) {
        for (int x = 0; x < nx; x++) {
            vec2f pos = v2f(x, y) - dt * old_velocity(x, y);
            new_density(x, y) = interpolate(old_density, pos);
        }
    }
}

void Fluid::AdvectVelocity(int start, int end){
    for (int y = start; y < end; y++) {
        for (int x = 0; x < nx; x++) {
            vec2f pos = v2f(x, y) - dt * old_velocity(x, y);
            new_velocity(x, y) = interpolate(old_velocity, pos);
        }
    }
}

void Fluid::DiffuseDensity(int start, int end){
    for (int y = start; y < end; y++) {
        for (int x = 0; x < nx; x++) {
            float sum =
                    diffusion * (
                            + old_density(x - 1, y + 0)
                            + old_density(x + 1, y + 0)
                            + old_density(x + 0, y - 1)
                            + old_density(x + 0, y + 1)
                    )
                    + old_density(x + 0, y + 0);
            new_density(x, y) = 1.0f / (1.0f + 4.0f * diffusion) * sum;
        }
    }
}

void Fluid::DiffuseVelocity(int start, int end){
    for (int y = start; y < end; y++) {
        for (int x = 0; x < nx; x++) {
            vec2f sum =
                    viscosity * (
                            + old_velocity(x - 1, y + 0)
                            + old_velocity(x + 1, y + 0)
                            + old_velocity(x + 0, y - 1)
                            + old_velocity(x + 0, y + 1)
                    )
                    + old_velocity(x + 0, y + 0);
            new_velocity(x, y) = 1.0f / (1.0f + 4.0f * viscosity) * sum;
        }
    }
}

void Fluid::InitializeP(int start, int end){
    for (int y = start; y < end; y++) {
        for (int x = 0; x < nx; x++) {
            float dx = old_velocity(x + 1, y + 0).x - old_velocity(x - 1, y + 0).x;
            float dy = old_velocity(x + 0, y + 1).y - old_velocity(x + 0, y - 1).y;
            div(x, y) = dx + dy;
            p(x, y) = 0.0f;
        }
    }
}

void Fluid::ProjectDiv(int start, int end){
    for (int y = 0; y < ny; y++)
        for (int x = 0; x < nx; x++) {
            float sum = -div(x, y)
                        + p(x + 1, y + 0)
                        + p(x - 1, y + 0)
                        + p(x + 0, y + 1)
                        + p(x + 0, y - 1);
            p2(x, y) = 0.25f*sum;
        }
    p.swap(p2);
}

void Fluid::ProjectVelocity(int start, int end){
    for (int y = start; y < end; y++) {
        for (int x = 0; x < nx; x++) {
            old_velocity(x, y).x -= 0.5f * (p(x + 1, y + 0) - p(x - 1, y + 0));
            old_velocity(x, y).y -= 0.5f * (p(x + 0, y + 1) - p(x + 0, y - 1));
        }
    }
}

void Fluid::VorticityConfinement(int start, int end){
    for (int y = start; y < end; y++) {
        for (int x = 0; x < nx; x++) {
            abs_curl(x, y) = fabsf(Curl(x, y));
        }
    }

    for (int y = start; y < end; y++) {
        for (int x = 0; x < nx; x++) {
            vec2f direction;
            direction.x = abs_curl(x + 0, y - 1) - abs_curl(x + 0, y + 1);
            direction.y = abs_curl(x + 1, y + 0) - abs_curl(x - 1, y + 0);

            direction = vorticity / (length(direction) + 1e-5f) * direction;

            if (x < nx / 2) direction *= 0.0f;

            new_velocity(x, y) = old_velocity(x, y) + dt * Curl(x, y) * direction;
        }
    }
}

void Fluid::ExtractZero(int start, int end){
    // zero out stuff at bottom
    for (int y = start; y < end; y++) {
        for (int x = 0; x < nx; x++) {
            if (y < 10) {
                old_density(x, y) = 0.0f;
                old_velocity(x, y) = vec2f{0.0f, 0.0f};
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


float Fluid::Curl(int x, int y){
    return  old_velocity(x, y + 1).x - old_velocity(x, y - 1).x +
            old_velocity(x - 1, y).y - old_velocity(x + 1, y).y;
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
    MoveOldProperty(0, ny);
    for (auto point : m_addPoint)
        AddDensity(point.x, point.y);

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

    sync_point.arrive_and_wait();
    // upload pixels to texture
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, nx, ny, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
}

