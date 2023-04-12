//------------------------------------------------------------------------------
// <copyright file="Utils.h" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------

#pragma once

namespace KinectEvolution {
    namespace Xaml {
        namespace Controls {
            namespace Base {

                typedef DWORD(*D3DColorConverter)(int a, int r, int g, int b);

                inline DWORD D3DCOLOR_ARGB(int a, int r, int g, int b)
                {
                    return (((a) & 0xff) << 24) | (((r) & 0xff) << 16) | (((g) & 0xff) << 8) | ((b) & 0xff);
                }

                inline DWORD D3DCOLOR_ABGR(int a, int r, int g, int b)
                {
                    return (((a) & 0xff) << 24) | (((b) & 0xff) << 16) | (((g) & 0xff) << 8) | ((r) & 0xff);
                }

                template<D3DColorConverter ColorConverter>
                DWORD ColorRamp(float normalizedInput)
                {
                    // use HSV to RGB conversion for (H, S=1, V=1)
                    float h = normalizedInput * 6.0f;
                    float hi = floorf(h);
                    float f = h - hi;
                    int   p = 0;
                    int   q = (BYTE) ((1.0f - f) * 255);
                    int   t = (BYTE) (f * 255);
                    int   v = 255;

                    switch ((int) hi)
                    {
                    case 0: return ColorConverter(0xff, v, t, p);
                    case 1: return ColorConverter(0xff, q, v, p);
                    case 2: return ColorConverter(0xff, p, v, t);
                    case 3: return ColorConverter(0xff, p, q, v);
                    case 4: return ColorConverter(0xff, t, p, v);
                    case 5: return ColorConverter(0xff, v, p, q);
                    }
                    return 0;
                }
            }
        }
    }
}
