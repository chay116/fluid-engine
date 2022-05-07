//
// Created by Chaeyoung Lim on 2022/02/24.
//

#ifndef CHA_ENGINE_SHADER_H
#define CHA_ENGINE_SHADER_H

#include "common.h"


CLASS_PTR(Shader);

class Shader {
public:
    static ShaderUPtr CreateFromFile(const std::string &filename, GLenum shaderType);

    ~Shader();

    uint32_t Get() const { return m_shader; };

private:
    Shader() {};

    bool LoadFile(const std::string &filename, GLenum shaderType);

    uint32_t m_shader{0};
};


#endif //FLUID_ENGINE_SHADER_H
