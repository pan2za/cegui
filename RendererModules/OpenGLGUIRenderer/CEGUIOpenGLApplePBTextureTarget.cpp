/***********************************************************************
    filename:   CEGUIOpenGLApplePBTextureTarget.cpp
    created:    Sun Feb 1 2009
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
#include "GL/glew.h"
#include "CEGUIOpenGLApplePBTextureTarget.h"
#include "CEGUIExceptions.h"
#include "CEGUIRenderQueue.h"
#include "CEGUIGeometryBuffer.h"

#include "CEGUIOpenGLRenderer.h"
#include "CEGUIOpenGLTexture.h"

// Start of CEGUI namespace section
namespace CEGUI
{
//----------------------------------------------------------------------------//
const float OpenGLApplePBTextureTarget::DEFAULT_SIZE = 128.0f;

//----------------------------------------------------------------------------//
static CGLPixelFormatAttribute fmtAttrs[] =
{
    kCGLPFAAccelerated,
    kCGLPFAPBuffer,
    kCGLPFAColorSize, static_cast<CGLPixelFormatAttribute>(24),
    kCGLPFAAlphaSize, static_cast<CGLPixelFormatAttribute>(8),
    static_cast<CGLPixelFormatAttribute>(0)
};

//----------------------------------------------------------------------------//
OpenGLApplePBTextureTarget::OpenGLApplePBTextureTarget(OpenGLRenderer& owner) :
    OpenGLRenderTarget(owner),
    d_pbuffer(0),
    d_context(0),
    d_texture(0)
{
    if (!GLEW_APPLE_pixel_buffer)
        throw RendererException("GL_APPLE_pixel_buffer extension is needed to "
            "use OpenGLApplePBTextureTarget!");

    initialiseTexture();

    // create CEGUI::Texture to wrap GL texture
    d_CEGUITexture = &static_cast<OpenGLTexture&>(
        d_owner.createTexture(d_texture, d_area.getSize()));

    CGLError err;
    CGLContextObj cctx = CGLGetCurrentContext();
    if (err = CGLGetVirtualScreen(cctx, &d_screen))
        throw RendererException("OpenGLApplePBTextureTarget - "
                                "CGLGetVirtualScreen failed: " +
                                String(CGLErrorString(err)));

    long fmt_count;
    CGLPixelFormatObj pix_fmt;
    if (err = CGLChoosePixelFormat(fmtAttrs, &pix_fmt, &fmt_count))
        throw RendererException("OpenGLApplePBTextureTarget - "
                                "CGLChoosePixelFormat failed: " +
                                String(CGLErrorString(err)));

    err = CGLCreateContext(pix_fmt, cctx, &d_context);
    CGLDestroyPixelFormat(pix_fmt);

    if (err)
        throw RendererException("OpenGLApplePBTextureTarget - "
                                "CGLCreateContext failed: " +
                                String(CGLErrorString(err)));

    // set default size (and cause initialisation of the pbuffer)
    try
    {
        declareRenderSize(Size(DEFAULT_SIZE, DEFAULT_SIZE));
    }
    catch(...)
    {
        CGLDestroyContext(d_context);
        throw;
    }

    // set these states one-time since we have our own context
    enablePBuffer();
    glEnable(GL_SCISSOR_TEST);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthFunc(GL_ALWAYS);
    disablePBuffer();
}

//----------------------------------------------------------------------------//
OpenGLApplePBTextureTarget::~OpenGLApplePBTextureTarget()
{
    if (d_context)
        CGLDestroyContext(d_context);

    if (d_pbuffer)
        CGLDestroyPBuffer(d_pbuffer);

    d_owner.destroyTexture(*d_CEGUITexture);
}

//----------------------------------------------------------------------------//
void OpenGLApplePBTextureTarget::activate()
{
    enablePBuffer();
    OpenGLRenderTarget::activate();
}

//----------------------------------------------------------------------------//
void OpenGLApplePBTextureTarget::deactivate()
{
    glFlush();
    OpenGLRenderTarget::deactivate();
    disablePBuffer();
}

//----------------------------------------------------------------------------//
bool OpenGLApplePBTextureTarget::isImageryCache() const
{
    return true;
}

//----------------------------------------------------------------------------//
void OpenGLApplePBTextureTarget::clear()
{
    enablePBuffer();
    glClearColor(0,0,0,0);
    glClear(GL_COLOR_BUFFER_BIT);
    disablePBuffer();
}

//----------------------------------------------------------------------------//
Texture& OpenGLApplePBTextureTarget::getTexture() const
{
    return *d_CEGUITexture;
}

//----------------------------------------------------------------------------//
void OpenGLApplePBTextureTarget::declareRenderSize(const Size& sz)
{
    // exit if current size is enough
    if ((d_area.getWidth() >= sz.d_width) &&
        (d_area.getHeight() >= sz.d_height))
            return;

    setArea(Rect(d_area.getPosition(), sz));

    // dump any previous pbuffer
    if (d_pbuffer)
    {
        CGLDestroyPBuffer(d_pbuffer);
        d_pbuffer = 0;
    }

    CGLError err;
    if (err = CGLCreatePBuffer(d_area.getWidth(), d_area.getHeight(),
                                GL_TEXTURE_2D, GL_RGBA, 0, &d_pbuffer))
    {
        throw RendererException("OpenGLApplePBTextureTarget::declareRenderSize "
                                "- CGLCreatePBuffer failed: " +
                                String(CGLErrorString(err)));
    }

    if (err = CGLSetPBuffer(d_context, d_pbuffer, 0, 0, d_screen))
        throw RendererException("OpenGLApplePBTextureTarget::declareRenderSize "
                                "- CGLSetPBuffer failed: " +
                                String(CGLErrorString(err)));

    // make d_texture use the pbuffer as it's data source
    enablePBuffer();
    glBindTexture(GL_TEXTURE_2D, d_texture);
    err = CGLTexImagePBuffer(d_context, d_pbuffer, GL_FRONT);
    disablePBuffer();

    if (err)
        throw RendererException("OpenGLApplePBTextureTarget::declareRenderSize "
                                "- CGLTexImagePBuffer failed: " +
                                String(CGLErrorString(err)));

    // ensure CEGUI::Texture is wrapping real GL texture and has correct size
    d_CEGUITexture->setOpenGLTexture(d_texture, d_area.getSize());
}

//----------------------------------------------------------------------------//
void OpenGLApplePBTextureTarget::initialiseTexture()
{
    // create and setup texture which pbuffer content will be loaded to
    glGenTextures(1, &d_texture);
    glBindTexture(GL_TEXTURE_2D, d_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}

//----------------------------------------------------------------------------//
void OpenGLApplePBTextureTarget::enablePBuffer() const
{
    if (CGLGetCurrentContext() != d_context)
    {
        d_prevContext = CGLGetCurrentContext();
        CGLSetCurrentContext(d_context);
    }
}

//----------------------------------------------------------------------------//
void OpenGLApplePBTextureTarget::disablePBuffer() const
{
    if (CGLGetCurrentContext() == d_context)
        CGLSetCurrentContext(d_prevContext);
}

//----------------------------------------------------------------------------//

} // End of  CEGUI namespace section