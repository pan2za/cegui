/***********************************************************************
    filename:   CEGUIOpenGLFBOTextureTarget.cpp
    created:    Sun Jan 11 2009
    author:     Paul D Turner
*************************************************************************/
/***************************************************************************
 *   Copyright (C) 2004 - 2009 Paul D Turner & The CEGUI Development Team
 *
 *   Permission is hereby granted, free of charge, to any person obtaining
 *   a copy of this software and associated documentation files (the
 *   "Software"), to deal in the Software without restriction, including
 *   without limitation the rights to use, copy, modify, merge, publish,
 *   distribute, sublicense, and/or sell copies of the Software, and to
 *   permit persons to whom the Software is furnished to do so, subject to
 *   the following conditions:
 *
 *   The above copyright notice and this permission notice shall be
 *   included in all copies or substantial portions of the Software.
 *
 *   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 *   IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 *   OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 *   ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 *   OTHER DEALINGS IN THE SOFTWARE.
 ***************************************************************************/
#include <GL/glew.h>
#include "CEGUIOpenGLFBOTextureTarget.h"
#include "CEGUIExceptions.h"
#include "CEGUIRenderQueue.h"
#include "CEGUIGeometryBuffer.h"

#include "CEGUIOpenGLRenderer.h"
#include "CEGUIOpenGLTexture.h"

// Start of CEGUI namespace section
namespace CEGUI
{
//----------------------------------------------------------------------------//
OpenGLFBOTextureTarget::OpenGLFBOTextureTarget(OpenGLRenderer& owner) :
    OpenGLRenderTarget(owner),
    d_texture(0)
{
    if (!GLEW_EXT_framebuffer_object)
        throw InvalidRequestException("Hardware does not support FBO");

    // this essentially creates a 'null' CEGUI::Texture
    d_CEGUITexture = &static_cast<OpenGLTexture&>(
        d_owner.createTexture(d_texture, d_area.getSize()));

    initialiseRenderTexture();

    // setup area and cause the initial texture to be generated.
    declareRenderSize(Size(DEFAULT_SIZE, DEFAULT_SIZE));
    clear();
}

//----------------------------------------------------------------------------//
OpenGLFBOTextureTarget::~OpenGLFBOTextureTarget()
{
    glDeleteFramebuffersEXT(1, &d_frameBuffer);
    d_owner.destroyTexture(*d_CEGUITexture);
}

//----------------------------------------------------------------------------//
void OpenGLFBOTextureTarget::declareRenderSize(const Size& sz)
{
    // exit if current size is enough
    if ((d_area.getWidth() >= sz.d_width) && (d_area.getHeight() >=sz.d_height))
        return;

    setArea(Rect(d_area.getPosition(), sz));
    resizeRenderTexture();
}

//----------------------------------------------------------------------------//
bool OpenGLFBOTextureTarget::isImageryCache() const
{
    return true;
}

//----------------------------------------------------------------------------//
void OpenGLFBOTextureTarget::activate()
{
    // switch to rendering to the texture
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, d_frameBuffer);

    OpenGLRenderTarget::activate();
}

//----------------------------------------------------------------------------//
void OpenGLFBOTextureTarget::deactivate()
{
    OpenGLRenderTarget::deactivate();

    // switch back to rendering to default buffer
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

//----------------------------------------------------------------------------//
void OpenGLFBOTextureTarget::clear()
{
    // switch to our FBO
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, d_frameBuffer);
    // Clear it.
    glClearColor(0,0,0,0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // switch back to rendering to view port
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

//----------------------------------------------------------------------------//
Texture& OpenGLFBOTextureTarget::getTexture() const
{
    return *d_CEGUITexture;
}

//----------------------------------------------------------------------------//
void OpenGLFBOTextureTarget::initialiseRenderTexture()
{
    // create FBO
    glGenFramebuffersEXT(1, &d_frameBuffer);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, d_frameBuffer);

    // set up the texture the FBO will draw to
    glGenTextures(1, &d_texture);
    glBindTexture(GL_TEXTURE_2D, d_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8,
                 static_cast<GLsizei>(d_area.d_right),
                 static_cast<GLsizei>(d_area.d_bottom),
                 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
                              GL_TEXTURE_2D, d_texture, 0);

    // setup depth buffer
    glGenRenderbuffersEXT(1, &d_depthBuffer);
    glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, d_depthBuffer);
    glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24,
                             static_cast<GLsizei>(d_area.d_right),
                             static_cast<GLsizei>(d_area.d_bottom));
    glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,
                                 GL_RENDERBUFFER_EXT, d_depthBuffer);

    // TODO: Check for completeness and then maybe try some alternative stuff?

    // switch from our frame buffer back to using default buffer.
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

    // ensure the CEGUI::Texture is wrapping the gl texture and has correct size
    d_CEGUITexture->setOpenGLTexture(d_texture, d_area.getSize());
}

//----------------------------------------------------------------------------//
void OpenGLFBOTextureTarget::resizeRenderTexture()
{
    const Size sz(d_area.getSize());

    // set the texture to the required size
    glBindTexture(GL_TEXTURE_2D, d_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8,
                 static_cast<GLsizei>(sz.d_width),
                 static_cast<GLsizei>(sz.d_height),
                 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

    // set depth buffer to required size
    glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, d_depthBuffer);
    glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24,
                             static_cast<GLsizei>(d_area.d_right),
                             static_cast<GLsizei>(d_area.d_bottom));

    // ensure the CEGUI::Texture is wrapping the gl texture and has correct size
    d_CEGUITexture->setOpenGLTexture(d_texture, sz);
}

//----------------------------------------------------------------------------//

} // End of  CEGUI namespace section