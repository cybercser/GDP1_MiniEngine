#include "texture.h"

#include "stb_image.h"
#include "Core/logger.h"

#include <codecvt>

namespace gdp1 {

std::wstring GetPath() {
    TCHAR buffer[MAX_PATH] = {0};
    GetModuleFileName(NULL, buffer, MAX_PATH);
    std::wstring::size_type pos = std::wstring(buffer).find_last_of(L"\\/");
    return std::wstring(buffer).substr(0, pos);
}

std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;

std::string ws_to_string(const std::wstring& wideStr) { return converter.to_bytes(wideStr); }

GLuint Texture::LoadTexture(const std::string& texPath, bool flipY) {
    int width, height, numChannels;
    std::string texturePath;
    size_t lastSlashPosition = texPath.find_last_of("/\\");

    // If no directory separator found, return the whole string
    if (lastSlashPosition == std::string::npos) {
        texturePath = texPath;
    } else {
        texturePath = ws_to_string(GetPath()) + "/Assets/Textures/" + texPath.substr(lastSlashPosition + 1);
    }

    unsigned char* data = Texture::LoadPixels(texturePath, width, height, numChannels, flipY);
    GLuint tex = 0;
    if (data != nullptr) {
        GLenum format = GL_RGBA, internalFormat = GL_RGBA8;
        if (numChannels == 1) {
            format = GL_R8;
            internalFormat = GL_RED;
        } else if (numChannels == 3) {
            format = GL_RGB;
            internalFormat = GL_RGB8;
        } else if (numChannels == 4) {
            format = GL_RGBA;
            internalFormat = GL_RGBA8;
        }

        glCreateTextures(GL_TEXTURE_2D, 1, &tex);
        glTextureStorage2D(tex, 1, internalFormat, width, height);
        glTextureSubImage2D(tex, 0, 0, 0, width, height, format, GL_UNSIGNED_BYTE, data);

        glTextureParameteri(tex, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTextureParameteri(tex, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTextureParameteri(tex, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTextureParameteri(tex, GL_TEXTURE_WRAP_T, GL_REPEAT);

        Texture::DeletePixels(data);
    } else {
        LOG_ERROR("Failed to load texture: {}", texturePath);
    }

    return tex;
}

void Texture::DeletePixels(unsigned char* data) { stbi_image_free(data); }

unsigned char* Texture::LoadPixels(const std::string& fName, int& width, int& height, int& numChannels, bool flip) {
    stbi_set_flip_vertically_on_load(flip);
    stbi_uc* data = stbi_load(fName.c_str(), &width, &height, &numChannels, STBI_default);

    if (!data) {
        LOG_ERROR("Reason: ", stbi_failure_reason());
    }

    return data;
}

// GLuint Texture::LoadCubeMap(const std::vector<std::string> &faces) {
//     GLuint tex;
//
//     glGenTextures(1, &tex);
//     glBindTexture(GL_TEXTURE_CUBE_MAP, tex);
//
//     int width, height, nChannels;
//     for (unsigned int i = 0; i < faces.size(); i++) {
//         unsigned char *data = Texture::LoadPixels(faces[i], width, height, nChannels, 0);
//         if (data) {
//             glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
//                          data);
//         } else {
//             LOG_ERROR("Cubemap texture failed to load at path: {}", faces[i]);
//             return false;
//         }
//         Texture::DeletePixels(data);
//     }
//
//     // set the texture wrapping/filtering options
//     glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//     glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//     glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//     glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//     glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
//
//     return tex;
// }

// the DSA (Direct State Access) enabled version of LoadCubeMap, requires OpenGL 4.5+
GLuint Texture::LoadCubeMap(const std::vector<std::string>& faces, bool flipY) {
    GLuint cubemap;

    glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &cubemap);

    int width, height, nChannels;
    stbi_uc* data = Texture::LoadPixels(faces[0], width, height, nChannels, flipY);
    glTextureStorage2D(cubemap, 1, GL_RGBA8, width, height);

    for (unsigned int i = 0; i < faces.size(); i++) {
        data = Texture::LoadPixels(faces[i], width, height, nChannels, flipY);
        if (data) {
            if (nChannels == 3) {
                glTextureSubImage3D(cubemap, 0, 0, 0, i, width, height, 1, GL_RGB, GL_UNSIGNED_BYTE, data);

            } else if (nChannels == 4) {
                glTextureSubImage3D(cubemap, 0, 0, 0, i, width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE, data);
            }
        } else {
            LOG_ERROR("Cubemap texture failed to load at path: {}", faces[i]);
            return false;
        }
        Texture::DeletePixels(data);
    }

    // set the texture wrapping/filtering options
    glTextureParameteri(cubemap, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(cubemap, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(cubemap, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(cubemap, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTextureParameteri(cubemap, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return cubemap;
}

GLuint Texture::LoadHdrCubeMap(const std::string& baseName) {
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_CUBE_MAP, tex);

    const char* suffixes[] = {"posx", "negx", "posy", "negy", "posz", "negz"};
    GLint w, h;

    // Load the first one to get width/height
    std::string texName = baseName + "_" + suffixes[0] + ".hdr";
    float* data = stbi_loadf(texName.c_str(), &w, &h, NULL, 3);

    // Allocate immutable storage for the whole cube map texture
    glTexStorage2D(GL_TEXTURE_CUBE_MAP, 1, GL_RGB32F, w, h);
    glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, 0, 0, w, h, GL_RGB, GL_FLOAT, data);
    stbi_image_free(data);

    // Load the other 5 cube-map faces
    for (int i = 1; i < 6; i++) {
        std::string texName = baseName + "_" + suffixes[i] + ".hdr";
        data = stbi_loadf(texName.c_str(), &w, &h, NULL, 3);
        glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, 0, 0, w, h, GL_RGB, GL_FLOAT, data);
        stbi_image_free(data);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return tex;
}

}  // namespace gdp1
