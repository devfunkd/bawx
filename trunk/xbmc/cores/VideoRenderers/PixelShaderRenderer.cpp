/*
 *      Copyright (C) 2005-2008 Team XBMC
 *      http://www.xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */
 
#include "PixelShaderRenderer.h"

CPixelShaderRenderer::CPixelShaderRenderer(LPDIRECT3DDEVICE8 pDevice)
    : CXBoxRenderer(pDevice)
{
}

bool CPixelShaderRenderer::Configure(unsigned int width, unsigned int height, unsigned int d_width, unsigned int d_height, float fps, unsigned flags)
{
  if(!CXBoxRenderer::Configure(width, height, d_width, d_height, fps, flags))
    return false;

  m_bConfigured = true;
  return true;
}


void CPixelShaderRenderer::Render(DWORD flags)
{
  // this is the low memory renderer
  RenderLowMem(flags);

  CXBoxRenderer::Render(flags);
}
