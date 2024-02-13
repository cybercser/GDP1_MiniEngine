#pragma once

#include <string>
#include <vector>

#include <glad/glad.h>

namespace gdp1 {
class Texture {
public:
    static GLuint LoadTexture(const std::string& texPath, bool flipY = false);
    static GLuint LoadCubeMap(const std::vector<std::string>& faces, bool flipY = false);
    static GLuint LoadHdrCubeMap(const std::string& baseName);
    static unsigned char* LoadPixels(const std::string& texPath, int& w, int& h, int& numChannels, bool flip);
    static void DeletePixels(unsigned char*);
};
}  // namespace gdp1
