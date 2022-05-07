//
// Created by Chaeyoung Lim on 2022/02/24.
//

#ifndef FLUID_ENGINE_COMMON_H
#define FLUID_ENGINE_COMMON_H

#include <string>
#include <optional>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <memory>
#include <spdlog/spdlog.h>

#define CLASS_PTR(klassName) \
class klassName; \
using klassName ## UPtr = std::unique_ptr<klassName>; \
using klassName ## SPtr = std::shared_ptr<klassName>; \
using klassName ## WPtr = std::weak_ptr<klassName>;

std::optional<std::string> LoadTextFile(const std::string& filename);

inline void check_gl(int line){
    int error = glGetError();
    if (error != GL_NO_ERROR){
        printf("OpenGL ERROR line %i: %i, %s\n", line, error, glGetString(error));
    }
}

#define CHECK_GL check_gl(__LINE__);

#endif //FLUID_ENGINE_COMMON_H
