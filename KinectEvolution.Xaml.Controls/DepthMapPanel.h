//------------------------------------------------------------------------------
// <copyright file="DepthMapPanel.h" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------

#pragma once

#include "Panel.h"
#include <DirectXMath.h>

#include "DepthMeshEffect.h"
#include "DepthMesh.h"
#include "Texture.h"
#include "InfraredRenderer.h"

namespace KinectEvolution {
    namespace Xaml {
        namespace Controls {
            namespace DepthMap {

                using namespace KinectEvolution::Xaml::Controls::Base;

                public enum class DEPTH_PANEL_MODE
                {
                    DEPTH,
                    IR,
                    COLOR,
                    COLOR_AND_IR,
                    COLOR_REGISTRATION,
                    DEPTH_RAMP
                };

                enum class DEPTH_RENDER_MODE
                {
                    SURFACE_WITH_BODY_INDEX,
                    POINT_CLOUD,
                    SURFACE_WITH_IR_TEXTURE,
                    SURFACE_WITH_COLOR_TEXTURE,
                    POINTSPRITES,
                };

                [Windows::Foundation::Metadata::WebHostHidden]
                public ref class DepthMapPanel sealed
                    : public Panel
                {
                public:
                    DepthMapPanel();

                    property DEPTH_PANEL_MODE PanelMode;

                    property WRK::CoordinateMapper^ CoordinateMapper
                    {
                        WRK::CoordinateMapper^ get();
                        void set(_In_ WRK::CoordinateMapper^ value);
                    }

                    property WRK::DepthFrameSource^ DepthSource
                    {
                        WRK::DepthFrameSource^ get();
                        void set(_In_ WRK::DepthFrameSource^ value);
                    }

                    property WRK::InfraredFrameSource^ InfraredSource
                    {
                        WRK::InfraredFrameSource^ get();
                        void set(_In_ WRK::InfraredFrameSource^ value);
                    }

                    property WRK::ColorFrameSource^ ColorSource
                    {
                        WRK::ColorFrameSource^ get();
                        void set(_In_ WRK::ColorFrameSource^ value);
                    }

                protected private:
                    virtual event Windows::UI::Xaml::Data::PropertyChangedEventHandler^ PropertyChanged;
                    void NotifyPropertyChanged(Platform::String^ prop);

                    virtual void Update(double elapsedTime) override;
                    virtual void Render() override;

                    virtual void ResetDeviceResources() override;
                    virtual void CreateDeviceResources() override;
                    virtual void CreateSizeDependentResources() override;

                    // override to add mouse/touch/pen input
                    virtual void StartRenderLoop() override;
                    virtual void StopRenderLoop() override;

                    void OnMapperChanged(WRK::CoordinateMapper^ sender, WRK::CoordinateMappingChangedEventArgs^ args);

                private:
                    ~DepthMapPanel();

                    void CopyXYTableToDepthMap();

                    void OnDepthFrame(_In_ WRK::DepthFrame^ frame);
                    void OnInfraredFrame(_In_ WRK::InfraredFrame^ frame);
                    void OnColorFrame(_In_ WRK::ColorFrame^ frame);

                    void UpdateDepthTexture(_In_reads_(pixels) const UINT16* pZ, UINT pixels);
                    void FillInDefaultXYTable(_Out_writes_(pitch * DEPTH_FRAME_HEIGHT) float* pTable, UINT pitch);

                    void SetMode(VertexMode vertexMode, RampMode rampMode);

                    void UpdateData(_In_reads_(pixels) const UINT16* pZ, UINT pixels);
                    void UpdateData(_In_reads_(pixels) const float* pX, _In_reads_(pixels) const float* pY, _In_reads_(pixels) const UINT16* pZ, UINT pixels);
                    void UpdateXYTable(_In_reads_(2 * pixels) const float* pXYTable, UINT pixels);
                    void UpdateUVTable(_In_reads_(2 * pixels) const float* pUVTable, UINT pixels);
                    void Render(_In_opt_ const PrimitiveParameter* pBasicEffectParameter, _In_opt_ Texture^ pSurfaceTexture);

                    XMMATRIX GetViewMatrix()
                    {
                        return _viewMatrix;
                    }

                    XMMATRIX GetProjectionMatrix()
                    {
                        return _projectionMatrix;
                    }

                    void ApplyTransformWithWorldViewMatrix(_In_ const DirectX::XMMATRIX& worldViewMatrix)
                    {
                        DirectX::XMMATRIX wvp = worldViewMatrix * GetProjectionMatrix();
                        _meshEffect->ApplyTransformAll(_d3dContext.Get(), worldViewMatrix, wvp);
                    }

                    void ApplyTransformWithWorldMatrix(_In_ const DirectX::XMMATRIX& worldMatrix)
                    {
                        DirectX::XMMATRIX wv = worldMatrix * GetViewMatrix();
                        ApplyTransformWithWorldViewMatrix(wv);
                    }

                private:
                    const UINT COLOR_FRAME_WIDTH = 1920;
                    const UINT COLOR_FRAME_HEIGHT = 1080;

                    const UINT DEPTH_FRAME_WIDTH = 512;
                    const UINT DEPTH_FRAME_HEIGHT = 424;

                    const float DEPTH_MINMM = 500.0f;
                    const float DEPTH_MAXMM = 8000.0f;

                    const float DEPTH_FRAME_HFOV = DirectX::XM_PI * 70.6f / 180.0f;
                    const float DEPTH_FRAME_VFOV = DirectX::XM_PI * 60.0f / 180.0f;

                    Microsoft::WRL::ComPtr<ID3D11SamplerState>  _uvSamplerState;

                    // depth map props
                    DepthMeshEffect^            _meshEffect;
                    DepthMesh^                  _mesh;
                    Texture^                    _xyTexture;
                    Texture^                    _uvTexture;

                    Texture^                    _depthTexture;
                    DepthPointEffect^           _pointEffect;

                    VertexMode                  _vertexMode;
                    RampMode                    _rampMode;

                    PrimitiveParameter          _depthEffect;

                    DirectX::XMMATRIX           _viewMatrix;
                    DirectX::XMMATRIX           _projectionMatrix;

                    Platform::WriteOnlyArray<WRK::ColorSpacePoint>^  _uvTable;
                    Platform::Array<UINT16>^    _depthFrame;

                    // texture for raw image frame from Kinect
                    Texture^                    _colorFrame;

                    // IR renderer to get final image
                    InfraredRenderer^           _irRenderer;

                    WRK::CoordinateMapper^      _coordinateMapper;
                    BOOL                        _mapperChanged;

                    WRK::DepthFrameSource^      _depthSource;
                    WRK::DepthFrameReader^      _depthReader;

                    WRK::ColorFrameSource^      _colorSource;
                    WRK::ColorFrameReader^      _colorReader;

                    WRK::InfraredFrameSource^   _irSource;
                    WRK::InfraredFrameReader^   _irReader;

                    Windows::Foundation::EventRegistrationToken _mapperChangedEventToken;
                };

            }
        }
    }
}
