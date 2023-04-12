//------------------------------------------------------------------------------
// <copyright file="pch.h" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------

#pragma once

#include <wrl/client.h>

#include <collection.h>
#include <ppltasks.h>

#include <d3d11_2.h>
#pragma comment(lib, "d3d11")
#pragma comment(lib, "dxgi")

#include <d2d1_2.h>
#pragma comment(lib, "d2d1")

// SSE and XMath headers
#include <mmintrin.h>
#include <emmintrin.h>
#include <smmintrin.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>

using namespace DirectX;

#include <assert.h>

#ifndef ASSERT
#if defined(DBG) && (DBG == 1)
#define ASSERT( x ) assert( x )
#else
#define ASSERT( x )
#endif
#endif

namespace WRK = WindowsPreview::Kinect;
#pragma warning(disable:4250) // disable bogus inherits via dominance warning for pure virtual functions

#include "DirectXHelper.h"
