#pragma once

#include <glad/glad.h>
#include <common.h>

namespace gdp1 {

class FBO {
public:
    FBO(int width, int height);
    ~FBO();

    unsigned int fboID = 0;
    unsigned int colorTextureId = 0;
    unsigned int depthTextureId = 0;

    unsigned int width = 0;
    unsigned int height = 0;

    bool SetupFBO(int width, int height, std::string& error);

    bool Reset(int width, int height, std::string& error);

    void ClearBuffers(bool bClearColour = true, bool bClearDepth = true);

    bool DeleteFBO(void);

    int GetMaxColourAttachments(void);

    int GetMaxDrawBuffers(void);

    void Bind();

};

}  // namespace gdp1
