//------------------------------------------------------------------------------
// <copyright file="DepthMesh.h" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------

#pragma once

#include "Mesh.h"

namespace KinectEvolution {
    namespace Xaml {
        namespace Controls {
            namespace DepthMap {

                using namespace KinectEvolution::Xaml::Controls::Base;

                ref class DepthMesh sealed
                    : Mesh
                {
                internal:
                    DepthMesh() : Mesh(), _loadingComplete(FALSE) {}

                    BOOL IsLoadingComplete() override { return _loadingComplete; }

                private:

                    BOOL _loadingComplete;
                };

            }
        }
    }
}
