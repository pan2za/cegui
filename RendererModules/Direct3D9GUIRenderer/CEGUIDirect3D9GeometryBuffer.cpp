/***********************************************************************
    filename:   CEGUIDirect3D9GeometryBuffer.cpp
    created:    Mon Feb 9 2009
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
#include "CEGUIDirect3D9GeometryBuffer.h"
#include "CEGUIDirect3D9Texture.h"
#include "CEGUIRenderEffect.h"
#include <d3d9.h>

// Start of CEGUI namespace section
namespace CEGUI
{
//----------------------------------------------------------------------------//
Direct3D9GeometryBuffer::Direct3D9GeometryBuffer(LPDIRECT3DDEVICE9 device) :
    d_activeTexture(0),
    d_hasCustomMatrix(false),
    d_translation(0, 0, 0),
    d_rotation(0, 0, 0),
    d_pivot(0, 0, 0),
    d_effect(0),
    d_device(device),
    d_matrixValid(false)
{
}

//----------------------------------------------------------------------------//
void Direct3D9GeometryBuffer::draw() const
{
    // setup clip region
    RECT clip;
    clip.left   = static_cast<LONG>(d_clipRect.d_left);
    clip.top    = static_cast<LONG>(d_clipRect.d_top);
    clip.right  = static_cast<LONG>(d_clipRect.d_right);
    clip.bottom = static_cast<LONG>(d_clipRect.d_bottom);
    d_device->SetScissorRect(&clip);

    // apply the transformations we need to use.
    if (!d_matrixValid)
        updateMatrix();

    d_device->SetTransform(D3DTS_WORLD, &d_matrix);

    // set up RenderEffect
    if (d_effect)
        d_effect->performPreRenderFunctions();

    // draw the batches
    BatchList::const_iterator i = d_batches.begin();
    for ( ; i != d_batches.end(); ++i)
        (*i).draw();

    // clean up RenderEffect
    if (d_effect)
        d_effect->performPostRenderFunctions();
}

//----------------------------------------------------------------------------//
void Direct3D9GeometryBuffer::setTransform(const float* matrix)
{
    for (int i = 0; i < 16; ++i)
        d_xform[i] = matrix[i];

    d_hasCustomMatrix = true;
    d_matrixValid = false;
}

//----------------------------------------------------------------------------//
void Direct3D9GeometryBuffer::setTranslation(const Vector3& t)
{
    d_translation = t;
    d_matrixValid = false;
}

//----------------------------------------------------------------------------//
void Direct3D9GeometryBuffer::setRotation(const Vector3& r)
{
    d_rotation = r;
    d_matrixValid = false;
}

//----------------------------------------------------------------------------//
void Direct3D9GeometryBuffer::setPivot(const Vector3& p)
{
    d_pivot = p;
    d_matrixValid = false;
}

//----------------------------------------------------------------------------//
void Direct3D9GeometryBuffer::setClippingRegion(const Rect& region)
{
    d_clipRect = region;
}

//----------------------------------------------------------------------------//
void Direct3D9GeometryBuffer::appendVertex(const Vertex& vertex)
{
    appendGeometry(&vertex, 1);
}

//----------------------------------------------------------------------------//
void Direct3D9GeometryBuffer::appendGeometry(const Vertex* const vbuff,
                                             uint vertex_count)
{
    performBatchManagement();
    d_batches.back().appendGeometry(vbuff, vertex_count);
}

//----------------------------------------------------------------------------//
void Direct3D9GeometryBuffer::setActiveTexture(Texture* texture)
{
    d_activeTexture = static_cast<Direct3D9Texture*>(texture);
}

//----------------------------------------------------------------------------//
void Direct3D9GeometryBuffer::reset()
{
    d_batches.clear();
    d_activeTexture = 0;
}

//----------------------------------------------------------------------------//
Texture* Direct3D9GeometryBuffer::getActiveTexture() const
{
    return d_activeTexture;
}

//----------------------------------------------------------------------------//
uint Direct3D9GeometryBuffer::getVertexCount() const
{
    uint count = 0;
    BatchList::const_iterator i = d_batches.begin();

    for ( ; i != d_batches.end(); ++i)
        count += (*i).getVertexCount();

    return count;
}

//----------------------------------------------------------------------------//
uint Direct3D9GeometryBuffer::getBatchCount() const
{
    return d_batches.size();
}

//----------------------------------------------------------------------------//
void Direct3D9GeometryBuffer::setRenderEffect(RenderEffect* effect)
{
    d_effect = effect;
}

//----------------------------------------------------------------------------//
RenderEffect* Direct3D9GeometryBuffer::getRenderEffect()
{
    return d_effect;
}

//----------------------------------------------------------------------------//
void Direct3D9GeometryBuffer::performBatchManagement()
{
    const LPDIRECT3DTEXTURE9 t = d_activeTexture ?
                                 d_activeTexture->getDirect3D9Texture() : 0;

    // create a new batch if there are no batches yet, or if the active texture
    // differs from that used by the current batch.
    if (d_batches.empty() || (t != d_batches.back().getTexture()))
        d_batches.push_back(Direct3D9GeometryBatch(d_device, t));
}

//----------------------------------------------------------------------------//
void Direct3D9GeometryBuffer::updateMatrix() const
{
    if (d_hasCustomMatrix)
    {
        // TODO!
    }
    else
    {
        const D3DXVECTOR3 p(d_pivot.d_x, d_pivot.d_y, d_pivot.d_z);
        const D3DXVECTOR3 t(d_translation.d_x, d_translation.d_y, d_translation.d_z);

        D3DXQUATERNION r;
        D3DXQuaternionRotationYawPitchRoll(&r,
            D3DXToRadian(d_rotation.d_y),
            D3DXToRadian(d_rotation.d_x),
            D3DXToRadian(d_rotation.d_z));

        D3DXMatrixTransformation(&d_matrix, 0, 0, 0, &p, &r, &t);
    }

    d_matrixValid = true;
}

//----------------------------------------------------------------------------//
const D3DXMATRIX* Direct3D9GeometryBuffer::getMatrix() const
{
    if (!d_matrixValid)
        updateMatrix();

    return &d_matrix;
}

//----------------------------------------------------------------------------//

} // End of  CEGUI namespace section