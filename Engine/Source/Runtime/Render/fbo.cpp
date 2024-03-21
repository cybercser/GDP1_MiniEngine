#include "fbo.h"

#include <common.h>

namespace gdp1 {

FBO::FBO(int width, int height) {
    this->width = width;
    this->height = height;

    this->fboID = 0;
    this->colorTextureId = 0;
    this->depthTextureId = 0;

    std::string error;
    if (!SetupFBO(width, height, error)) {
        LOG_ERROR("Failed setting up FBO: ", error);
    }
}

FBO::~FBO() {
    this->fboID = 0;
    this->colorTextureId = 0;
    this->depthTextureId = 0;

    DeleteFBO();
}

bool FBO::SetupFBO(int width, int height, std::string& error) {
    glGenFramebuffers(1, &fboID);
    glBindFramebuffer(GL_FRAMEBUFFER, fboID);

    glGenTextures(1, &colorTextureId);
    glBindTexture(GL_TEXTURE_2D, this->colorTextureId);

    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB8, this->width, this->height);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glGenTextures(1, &depthTextureId);
    glBindTexture(GL_TEXTURE_2D, depthTextureId);

    glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH24_STENCIL8, this->width, this->height);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, this->colorTextureId, 0);

    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, this->depthTextureId, 0);

    static const GLenum drawBuffers[] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, drawBuffers);

    bool isInitialized = true;

    switch (glCheckFramebufferStatus(GL_FRAMEBUFFER)) {
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
            error = "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT";
            isInitialized = false;
            break;
        case GL_FRAMEBUFFER_COMPLETE:
            isInitialized = true;
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
        case GL_FRAMEBUFFER_UNSUPPORTED:
        default:
            isInitialized = false;
            break;
    }

    // Point back to default frame buffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return isInitialized;
}

bool FBO::Reset(int width, int height, std::string& error) {
    if (width <= 0 || height <= 0) {
        error = "Invalid dimensions for FBO reset.";
        return false;
    }

    this->width = width;
    this->height = height;

    // Additional implementation as needed
    return true;
}

void FBO::ClearBuffers(bool bClearColour, bool bClearDepth) {
    GLbitfield clearMask = 0;
    if (bClearColour) clearMask |= GL_COLOR_BUFFER_BIT;
    if (bClearDepth) clearMask |= GL_DEPTH_BUFFER_BIT;
    glClear(clearMask);
}

int FBO::GetMaxColourAttachments(void) {
    int maxColourAttach = 0;
    glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &maxColourAttach);

    return maxColourAttach;
}

int FBO::GetMaxDrawBuffers(void) {
    int maxDrawBuffers = 0;
    glGetIntegerv(GL_MAX_DRAW_BUFFERS, &maxDrawBuffers);

    return maxDrawBuffers;
}

bool FBO::DeleteFBO(void) {
    glDeleteTextures(1, &(this->colorTextureId));
    glDeleteTextures(1, &(this->depthTextureId));
    glDeleteFramebuffers(1, &(this->fboID));

    return true;
}

void FBO::Bind() {
    glBindFramebuffer(GL_FRAMEBUFFER, this->fboID);
    glViewport(0, 0, (float)this->width, (float)this->height);
    ClearBuffers(true, true);
}

}  // namespace gdp1