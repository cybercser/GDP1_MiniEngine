// Adapted from https://github.com/PacktPublishing/OpenGL-4-Shading-Language-Cookbook-Third-Edition
#include "shader.h"

#include "Utils/glutils.h"

#include <fstream>

using std::ifstream;
using std::ios;
using std::string;

#include <sstream>
#include <sys/stat.h>

namespace gdp1 {

std::unordered_map<std::string, GLSLShaderType> s_Extensions = {{".vs", GLSLShaderType::VERTEX},
                                                                {".vert", GLSLShaderType::VERTEX},
                                                                {"_vert.glsl", GLSLShaderType::VERTEX},
                                                                {".vert.glsl", GLSLShaderType::VERTEX},
                                                                {".gs", GLSLShaderType::GEOMETRY},
                                                                {".geom", GLSLShaderType::GEOMETRY},
                                                                {".geom.glsl", GLSLShaderType::GEOMETRY},
                                                                {".tcs", GLSLShaderType::TESS_CONTROL},
                                                                {".tcs.glsl", GLSLShaderType::TESS_CONTROL},
                                                                {".tes", GLSLShaderType::TESS_EVALUATION},
                                                                {".tes.glsl", GLSLShaderType::TESS_EVALUATION},
                                                                {".fs", GLSLShaderType::FRAGMENT},
                                                                {".frag", GLSLShaderType::FRAGMENT},
                                                                {"_frag.glsl", GLSLShaderType::FRAGMENT},
                                                                {".frag.glsl", GLSLShaderType::FRAGMENT},
                                                                {".cs", GLSLShaderType::COMPUTE},
                                                                {".cs.glsl", GLSLShaderType::COMPUTE}};

Shader::Shader()
    : m_Handle(0)
    , m_Linked(false) {}

Shader::~Shader() {
    if (m_Handle == 0) return;
    DetachAndDeleteShaderObjects();
    // Delete the program
    glDeleteProgram(m_Handle);
}

void Shader::DetachAndDeleteShaderObjects() {
    // Detach and delete the shader_ptr_ objects (if they are not already removed)
    GLint numShaders = 0;
    glGetProgramiv(m_Handle, GL_ATTACHED_SHADERS, &numShaders);
    std::vector<GLuint> shaderNames(numShaders);
    glGetAttachedShaders(m_Handle, numShaders, NULL, shaderNames.data());
    for (GLuint shader : shaderNames) {
        glDetachShader(m_Handle, shader);
        glDeleteShader(shader);
    }
}

void Shader::CompileShader(const std::string& fileName) {
    // Check the file name's extension to determine the shader_ptr_ type
    string ext = GetExtension(fileName);
    GLSLShaderType type = GLSLShaderType::VERTEX;
    auto it = s_Extensions.find(ext);
    if (it != s_Extensions.end()) {
        type = it->second;
    } else {
        string msg = "Unrecognized extension: " + ext;
        throw GLSLProgramException(msg);
    }

    // Pass the discovered shader_ptr_ type along
    CompileShader(fileName, type);
}

string Shader::GetExtension(const std::string& fileName) {
    size_t dotLoc = fileName.find_last_of('.');
    if (dotLoc != string::npos) {
        std::string ext = fileName.substr(dotLoc);
        if (ext == ".glsl") {
            size_t loc = fileName.find_last_of('.', dotLoc - 1);
            if (loc == string::npos) {
                loc = fileName.find_last_of('_', dotLoc - 1);
            }
            if (loc != string::npos) {
                return fileName.substr(loc);
            }
        } else {
            return ext;
        }
    }
    return "";
}

void Shader::CompileShader(const std::string& fileName, GLSLShaderType type) {
    if (!FileExists(fileName)) {
        string message = string("Shader: ") + fileName + " not found.";
        throw GLSLProgramException(message);
    }

    if (m_Handle <= 0) {
        m_Handle = glCreateProgram();
        if (m_Handle == 0) {
            throw GLSLProgramException("Unable to create shader program.");
        }
    }

    ifstream inFile(fileName, ios::in);
    if (!inFile) {
        string message = string("Unable to open: ") + fileName;
        throw GLSLProgramException(message);
    }

    // Get file contents
    std::stringstream code;
    code << inFile.rdbuf();
    inFile.close();

    CompileShader(code.str(), type, fileName);
}

void Shader::CompileShader(const string& source, GLSLShaderType type, const std::string& fileName) {
    if (m_Handle <= 0) {
        m_Handle = glCreateProgram();
        if (m_Handle == 0) {
            throw GLSLProgramException("Unable to create shader program.");
        }
    }

    GLuint shaderHandle = glCreateShader(type);

    const char* c_code = source.c_str();
    glShaderSource(shaderHandle, 1, &c_code, NULL);

    // Compile the shader_ptr_
    glCompileShader(shaderHandle);

    // Check for errors
    int result;
    glGetShaderiv(shaderHandle, GL_COMPILE_STATUS, &result);
    if (GL_FALSE == result) {
        // Compile failed, get log
        std::string msg;
        if (fileName.empty()) {
            msg = string(fileName) + ": shader compilation failed\n";
        } else {
            msg = "Shader compilation failed.\n";
        }

        int length = 0;
        glGetShaderiv(shaderHandle, GL_INFO_LOG_LENGTH, &length);
        if (length > 0) {
            std::string log(length, ' ');
            int written = 0;
            glGetShaderInfoLog(shaderHandle, length, &written, &log[0]);
            msg += log;
        }
        throw GLSLProgramException(msg);
    } else {
        // Compile succeeded, attach shader_ptr_
        glAttachShader(m_Handle, shaderHandle);
    }
}

void Shader::Link() {
    if (m_Linked) return;
    if (m_Handle <= 0) throw GLSLProgramException("Program has not been compiled.");

    glLinkProgram(m_Handle);
    int status = 0;
    std::string errString;
    glGetProgramiv(m_Handle, GL_LINK_STATUS, &status);
    if (GL_FALSE == status) {
        // Store log and return false
        int length = 0;
        glGetProgramiv(m_Handle, GL_INFO_LOG_LENGTH, &length);
        errString += "Program link failed:\n";
        if (length > 0) {
            std::string log(length, ' ');
            int written = 0;
            glGetProgramInfoLog(m_Handle, length, &written, &log[0]);
            errString += log;
        }
    } else {
        FindUniformLocations();
        m_Linked = true;
    }

    DetachAndDeleteShaderObjects();

    if (GL_FALSE == status) throw GLSLProgramException(errString);
}

void Shader::FindUniformLocations() {
    m_UniformLocations.clear();

    GLint numUniforms = 0;

    // For OpenGL 4.3 and above, use glGetProgramResource
    glGetProgramInterfaceiv(m_Handle, GL_UNIFORM, GL_ACTIVE_RESOURCES, &numUniforms);

    GLenum properties[] = {GL_NAME_LENGTH, GL_TYPE, GL_LOCATION, GL_BLOCK_INDEX};

    for (GLint i = 0; i < numUniforms; ++i) {
        GLint results[4];
        glGetProgramResourceiv(m_Handle, GL_UNIFORM, i, 4, properties, 4, NULL, results);

        if (results[3] != -1) continue;  // Skip uniforms in blocks
        GLint nameBufSize = results[0] + 1;
        char* name = new char[nameBufSize];
        glGetProgramResourceName(m_Handle, GL_UNIFORM, i, nameBufSize, NULL, name);
        m_UniformLocations[name] = results[2];
        delete[] name;
    }
}

void Shader::Use() {
    if (m_Handle <= 0 || (!m_Linked)) throw GLSLProgramException("Shader has not been linked");
    glUseProgram(m_Handle);
}

int Shader::GetHandle() { return m_Handle; }

bool Shader::IsLinked() { return m_Linked; }

void Shader::BindAttribLocation(GLuint location, const char* name) { glBindAttribLocation(m_Handle, location, name); }

void Shader::BindFragDataLocation(GLuint location, const char* name) {
    glBindFragDataLocation(m_Handle, location, name);
}

void Shader::SetUniform(const char* name, float x, float y, float z) {
    GLint loc = GetUniformLocation(name);
    glUniform3f(loc, x, y, z);
}

void Shader::SetUniform(const char* name, const glm::vec3& v) { this->SetUniform(name, v.x, v.y, v.z); }

void Shader::SetUniform(const char* name, const glm::vec4& v) {
    GLint loc = GetUniformLocation(name);
    glUniform4f(loc, v.x, v.y, v.z, v.w);
}

void Shader::SetUniform(const char* name, const glm::vec2& v) {
    GLint loc = GetUniformLocation(name);
    glUniform2f(loc, v.x, v.y);
}

void Shader::SetUniform(const char* name, const glm::mat4& m) {
    GLint loc = GetUniformLocation(name);
    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(m));
}

void Shader::SetUniform(const char* name, size_t size, GLboolean transpose, const glm::mat4& m) {
    GLint loc = GetUniformLocation(name);
    glUniformMatrix4fv(loc, size, transpose, glm::value_ptr(m));
}

void Shader::SetUniform(const char* name, const glm::mat3& m) {
    GLint loc = GetUniformLocation(name);
    glUniformMatrix3fv(loc, 1, GL_FALSE, glm::value_ptr(m));
}

void Shader::SetUniform(const char* name, float val) {
    GLint loc = GetUniformLocation(name);
    glUniform1f(loc, val);
}

void Shader::SetUniform(const char* name, int val) {
    GLint loc = GetUniformLocation(name);
    glUniform1i(loc, val);
}

void Shader::SetUniform(const char* name, GLuint val) {
    GLint loc = GetUniformLocation(name);
    glUniform1ui(loc, val);
}

void Shader::SetUniform(const char* name, bool val) {
    int loc = GetUniformLocation(name);
    glUniform1i(loc, val);
}

void Shader::SetUniform(const std::string& name, float x, float y, float z) { SetUniform(name.c_str(), x, y, z); }

void Shader::SetUniform(const std::string& name, const glm::vec2& v) { SetUniform(name.c_str(), v); }

void Shader::SetUniform(const std::string& name, const glm::vec3& v) { SetUniform(name.c_str(), v); }

void Shader::SetUniform(const std::string& name, const glm::vec4& v) { SetUniform(name.c_str(), v); }

void Shader::SetUniform(const std::string& name, const glm::mat4& m) { SetUniform(name.c_str(), m); }

void Shader::SetUniform(const std::string& name, const glm::mat3& m) { SetUniform(name.c_str(), m); }

void Shader::SetUniform(const std::string& name, float val) { SetUniform(name.c_str(), val); }

void Shader::SetUniform(const std::string& name, int val) { SetUniform(name.c_str(), val); }

void Shader::SetUniform(const std::string& name, bool val) { SetUniform(name.c_str(), val); }

void Shader::SetUniform(const std::string& name, GLuint val) { SetUniform(name.c_str(), val); }

void Shader::PrintActiveUniforms() {
    // For OpenGL 4.3 and above, use glGetProgramResource
    GLint numUniforms = 0;
    glGetProgramInterfaceiv(m_Handle, GL_UNIFORM, GL_ACTIVE_RESOURCES, &numUniforms);

    GLenum properties[] = {GL_NAME_LENGTH, GL_TYPE, GL_LOCATION, GL_BLOCK_INDEX};

    printf("Active uniforms:\n");
    for (int i = 0; i < numUniforms; ++i) {
        GLint results[4];
        glGetProgramResourceiv(m_Handle, GL_UNIFORM, i, 4, properties, 4, NULL, results);

        if (results[3] != -1) continue;  // Skip uniforms in blocks
        GLint nameBufSize = results[0] + 1;
        char* name = new char[nameBufSize];
        glGetProgramResourceName(m_Handle, GL_UNIFORM, i, nameBufSize, NULL, name);
        printf("%-5d %s (%s)\n", results[2], name, GetTypeString(results[1]));
        delete[] name;
    }
}

void Shader::PrintActiveUniformBlocks() {
    GLint numBlocks = 0;

    glGetProgramInterfaceiv(m_Handle, GL_UNIFORM_BLOCK, GL_ACTIVE_RESOURCES, &numBlocks);
    GLenum blockProps[] = {GL_NUM_ACTIVE_VARIABLES, GL_NAME_LENGTH};
    GLenum blockIndex[] = {GL_ACTIVE_VARIABLES};
    GLenum props[] = {GL_NAME_LENGTH, GL_TYPE, GL_BLOCK_INDEX};

    for (int block = 0; block < numBlocks; ++block) {
        GLint blockInfo[2];
        glGetProgramResourceiv(m_Handle, GL_UNIFORM_BLOCK, block, 2, blockProps, 2, NULL, blockInfo);
        GLint numUnis = blockInfo[0];

        char* blockName = new char[blockInfo[1] + 1];
        glGetProgramResourceName(m_Handle, GL_UNIFORM_BLOCK, block, blockInfo[1] + 1, NULL, blockName);
        printf("Uniform block \"%s\":\n", blockName);
        delete[] blockName;

        GLint* unifIndexes = new GLint[numUnis];
        glGetProgramResourceiv(m_Handle, GL_UNIFORM_BLOCK, block, 1, blockIndex, numUnis, NULL, unifIndexes);

        for (int unif = 0; unif < numUnis; ++unif) {
            GLint uniIndex = unifIndexes[unif];
            GLint results[3];
            glGetProgramResourceiv(m_Handle, GL_UNIFORM, uniIndex, 3, props, 3, NULL, results);

            GLint nameBufSize = results[0] + 1;
            char* name = new char[nameBufSize];
            glGetProgramResourceName(m_Handle, GL_UNIFORM, uniIndex, nameBufSize, NULL, name);
            printf("    %s (%s)\n", name, GetTypeString(results[1]));
            delete[] name;
        }

        delete[] unifIndexes;
    }
}

void Shader::PrintActiveAttribs() {
    // >= OpenGL 4.3, use glGetProgramResource
    GLint numAttribs;
    glGetProgramInterfaceiv(m_Handle, GL_PROGRAM_INPUT, GL_ACTIVE_RESOURCES, &numAttribs);

    GLenum properties[] = {GL_NAME_LENGTH, GL_TYPE, GL_LOCATION};

    printf("Active attributes:\n");
    for (int i = 0; i < numAttribs; ++i) {
        GLint results[3];
        glGetProgramResourceiv(m_Handle, GL_PROGRAM_INPUT, i, 3, properties, 3, NULL, results);

        GLint nameBufSize = results[0] + 1;
        char* name = new char[nameBufSize];
        glGetProgramResourceName(m_Handle, GL_PROGRAM_INPUT, i, nameBufSize, NULL, name);
        printf("%-5d %s (%s)\n", results[2], name, GetTypeString(results[1]));
        delete[] name;
    }
}

const char* Shader::GetTypeString(GLenum type) {
    // There are many more types than are covered here, but
    // these are the most common in these examples.
    switch (type) {
        case GL_FLOAT:
            return "float";
        case GL_FLOAT_VEC2:
            return "vec2";
        case GL_FLOAT_VEC3:
            return "vec3";
        case GL_FLOAT_VEC4:
            return "vec4";
        case GL_DOUBLE:
            return "double";
        case GL_INT:
            return "int";
        case GL_UNSIGNED_INT:
            return "unsigned int";
        case GL_BOOL:
            return "bool";
        case GL_FLOAT_MAT2:
            return "mat2";
        case GL_FLOAT_MAT3:
            return "mat3";
        case GL_FLOAT_MAT4:
            return "mat4";
        default:
            return "?";
    }
}

void Shader::Validate() {
    if (!IsLinked()) throw GLSLProgramException("Program is not linked");

    GLint status;
    glValidateProgram(m_Handle);
    glGetProgramiv(m_Handle, GL_VALIDATE_STATUS, &status);

    if (GL_FALSE == status) {
        // Store log and return false
        int length = 0;
        string logString;

        glGetProgramiv(m_Handle, GL_INFO_LOG_LENGTH, &length);

        if (length > 0) {
            char* c_log = new char[length];
            int written = 0;
            glGetProgramInfoLog(m_Handle, length, &written, c_log);
            logString = c_log;
            delete[] c_log;
        }

        throw GLSLProgramException(string("Program failed to validate\n") + logString);
    }
}

bool Shader::FileExists(const string& fileName) {
    struct stat info;
    int ret = -1;

    ret = stat(fileName.c_str(), &info);
    return 0 == ret;
}

int Shader::GetUniformLocation(const std::string& name) {
    auto pos = m_UniformLocations.find(name);

    if (pos == m_UniformLocations.end()) {
        GLint loc = glGetUniformLocation(m_Handle, name.c_str());
        m_UniformLocations[name] = loc;
        return loc;
    }

    return pos->second;
}

}  // namespace gdp1
