//
// Created by Chaeyoung Lim on 2022/04/23.
//

#ifndef FLUID_ENGINE_GRID_H
#define FLUID_ENGINE_GRID_H

#include "common.h"

template <typename T>
class Grid {
public:
    T *values;
    int nx, ny;
    inline Grid() : nx(256), ny(256) { values = new T[256 * 256]; };
    inline Grid(int x, int y) : nx(x), ny(y) { values = new T[nx*ny]; };
    Grid(const Grid&) = delete;
    Grid& operator = (const Grid&) = delete;

    inline ~Grid() { delete[] values; };

    inline void swap(Grid &other) {
        std::swap(values, other.values);
        std::swap(nx, other.nx);
        std::swap(ny, other.ny);
    };

    inline const T* data() const { return values; };

    inline int idx(int x, int y) const {
        //x = clamp(x, 0, nx - 1);
        //y = clamp(y, 0, ny - 1);

        // wrap around
        x = (x + nx) % nx;
        y = (y + ny) % ny;

        return x + y*nx;
    };

    inline T& operator () (int x, int y){
        return values[idx(x, y)];
    };

    inline const T& operator () (int x, int y) const {
        return values[idx(x, y)];
    };
};

struct vec2f {
    float x, y;

    inline vec2f& operator *= (float b){ x *= b  ; y *= b  ; return *this; }
    inline vec2f& operator += (vec2f b){ x += b.x; y += b.y; return *this; }
    inline vec2f& operator -= (vec2f b){ x -= b.x; y -= b.y; return *this; }
};

inline vec2f operator + (vec2f a, vec2f b){ return vec2f{a.x + b.x, a.y + b.y}; }
inline vec2f operator - (vec2f a, vec2f b){ return vec2f{a.x - b.x, a.y - b.y}; }
inline vec2f operator + (         vec2f b){ return vec2f{    + b.x,     + b.y}; }
inline vec2f operator - (         vec2f b){ return vec2f{    - b.x,     - b.y}; }
inline vec2f operator * (float a, vec2f b){ return vec2f{a   * b.x, a   * b.y}; }
inline vec2f operator * (vec2f a, float b){ return vec2f{a.x * b  , a.y * b  }; }
inline float dot        (vec2f a, vec2f b){ return float(a.x * b.x+ a.y * b.y); }

inline vec2f polar(float radians){
    return vec2f{cosf(radians), sinf(radians)};
}

inline float length(vec2f a){
    return sqrtf(dot(a, a));
}

inline vec2f normalize(vec2f a){
    return 1.0f/sqrtf(dot(a, a)) * a;
}

inline vec2f normalize(vec2f a, float eps){
    return 1.0f/(eps + sqrtf(dot(a, a))) * a;
}

inline vec2f v2f(float x, float y){
    return vec2f{x, y};
}

template <typename T, typename U>
inline T lerp(const T &a, const T &b, const U &u){
    return (U(1) - u)*a + u*b;
}

template <typename T>
inline const T& clamp(const T &x, const T &a, const T &b){
    return
            x < a ? a :
            x > b ? b :
            x;
}
template <typename T, typename U>
inline T smoothstep(const T &a, const T &b, const U &u){
    T t = clamp((u - a)/(b - a), U(0), U(1));
    return t*t*(U(3) - U(2)*t);
}

template <typename T>
T interpolate(const Grid<T> &grid, vec2f p){
    int ix = floorf(p.x);
    int iy = floorf(p.y);
    float ux = p.x - ix;
    float uy = p.y - iy;
    return lerp(
            lerp(grid(ix + 0, iy + 0), grid(ix + 1, iy + 0), ux),
            lerp(grid(ix + 0, iy + 1), grid(ix + 1, iy + 1), ux),
            uy
    );
}

#endif //FLUID_ENGINE_GRID_H
