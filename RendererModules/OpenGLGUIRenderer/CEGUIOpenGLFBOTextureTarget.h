/***********************************************************************
    filename:   CEGUIOpenGLFBOTextureTarget.h
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
#ifndef _CEGUIOpenGLFBOTextureTarget_h_
#define _CEGUIOpenGLFBOTextureTarget_h_

#include "CEGUIOpenGLRenderTarget.h"
#include "CEGUITextureTarget.h"
#include "CEGUIRect.h"
#include "CEGUIOpenGL.h"

#if defined(_MSC_VER)
#   pragma warning(push)
#   pragma warning(disable : 4250)
#endif

// Start of CEGUI namespace section
namespace CEGUI
{
class OpenGLTexture;

//! OpenGLFBOTextureTarget - allows rendering to an OpenGL texture via FBO.
class OPENGL_GUIRENDERER_API OpenGLFBOTextureTarget : public OpenGLRenderTarget,
                                                      public TextureTarget
{
public:
    OpenGLFBOTextureTarget(OpenGLRenderer& owner);
    virtual ~OpenGLFBOTextureTarget();

    // overrides from OpenGLRenderTarget
    void activate();
    void deactivate();
    // implementation of RenderTarget interface
    bool isImageryCache() const;
    // implementation of TextureTarget interface
    void clear();
    Texture& getTexture() const;
    void declareRenderSize(const Size& sz);

protected:
    //! default size of created texture objects
    static const int DEFAULT_SIZE = 128;

    //! allocate and set up the texture used with the FBO.
    void initialiseRenderTexture();
    //! resize the texture
    void resizeRenderTexture();

    //! Frame buffer object.
    GLuint d_frameBuffer;
    //! depth buffer
    GLuint d_depthBuffer;
    //! Associated OpenGL texture ID
    GLuint d_texture;
    //! we use this to wrap d_texture so it can be used by the core CEGUI lib.
    OpenGLTexture* d_CEGUITexture;
};

} // End of  CEGUI namespace section

#if defined(_MSC_VER)
#   pragma warning(pop)
#endif

#endif  // end of guard _CEGUIOpenGLFBOTextureTarget_h_