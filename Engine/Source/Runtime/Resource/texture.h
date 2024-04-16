#pragma once

#include <string>
#include <vector>
#include <map>

#include <glad/glad.h>

namespace gdp1 {

struct TextureInfo {
    unsigned int id;
    std::string type;
    std::string path;
    bool hasFBO;
};

class Texture {
public:
    static GLuint LoadTexture(const std::string& texPath, bool flipY = false);
    static GLuint LoadCubeMap(const std::vector<std::string>& faces, bool flipY = false);
    static GLuint LoadHdrCubeMap(const std::string& baseName);
    static unsigned char* LoadPixels(const std::string& texPath, int& w, int& h, int& numChannels, bool flip);
    static void DeletePixels(unsigned char*);

    static void AddTexture(TextureInfo* texture);
    static TextureInfo* GetTexture(std::string textureName);

    static std::map<std::string, TextureInfo*> m_TextureMap;
};
}  // namespace gdp1
