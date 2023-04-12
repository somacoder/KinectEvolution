//------------------------------------------------------------------------------
// <copyright file="SkeletonPanel.h" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------

#pragma once

#include "Panel.h"
#include "PrimitiveMesh.h"
#include "PrimitiveEffect.h"

namespace KinectEvolution {
    namespace Xaml {
        namespace Controls {
            namespace Skeleton {

                using namespace KinectEvolution::Xaml::Controls::Base;
                using namespace WindowsPreview::Kinect;

                enum class Body2DMode
                {
                    DepthIR,
                    Color,
                };

                struct Bone
                {
                    JointType JointA;
                    JointType JointB;
                };

                static const struct Bone BodyBones [] =
                {
                    { JointType::SpineBase, JointType::SpineMid },
                    { JointType::SpineMid, JointType::SpineShoulder },
                    { JointType::SpineShoulder, JointType::Neck },
                    { JointType::Neck, JointType::Head },
                    { JointType::SpineShoulder, JointType::ShoulderLeft },
                    { JointType::ShoulderLeft, JointType::ElbowLeft },
                    { JointType::ElbowLeft, JointType::WristLeft },
                    { JointType::WristLeft, JointType::HandLeft },
                    { JointType::HandLeft, JointType::HandTipLeft },
                    { JointType::WristLeft, JointType::ThumbLeft },
                    { JointType::SpineShoulder, JointType::ShoulderRight },
                    { JointType::ShoulderRight, JointType::ElbowRight },
                    { JointType::ElbowRight, JointType::WristRight },
                    { JointType::WristRight, JointType::HandRight },
                    { JointType::HandRight, JointType::HandTipRight },
                    { JointType::WristRight, JointType::ThumbRight },
                    { JointType::SpineBase, JointType::HipLeft },
                    { JointType::HipLeft, JointType::KneeLeft },
                    { JointType::KneeLeft, JointType::AnkleLeft },
                    { JointType::AnkleLeft, JointType::FootLeft },
                    { JointType::SpineBase, JointType::HipRight },
                    { JointType::HipRight, JointType::KneeRight },
                    { JointType::KneeRight, JointType::AnkleRight },
                    { JointType::AnkleRight, JointType::FootRight },
                };

                // following the hand state color table in NuiView
                static const DirectX::XMVECTOR HandStateColorTable [] =
                {
                    { 0.0f, 1.0f, 0.0f, 0.7f }, // open
                    { 1.0f, 0.0f, 0.0f, 0.7f }, // closed
                    { 0.0f, 0.0f, 1.0f, 0.7f }, // lasso
                    { 0.5f, 0.5f, 0.5f, 0.5f }, // unknown
                };

                static const int BODY_COUNT = 6;
                static const int JOINT_COUNT = 25;

                static const XMVECTOR NotTrackedColor = { 0.0f, 0.0f, 0.0f, 0.3f };

                static const float DefaultJointSize = 0.05f; // diameter in world space (meters)
                static const float DefaultBoneSize = 0.03f; // diameter in world space
                static const float NonTrackedScale = 0.3f; // render non-tracked joints different than tracked ones

                static const float DefaultNormalHandleSize = 0.01f; // diameter of the handle
                static const float DefaultNormalHandleLength = 0.05f; // length of the handle
                static const float DefaultNormalArrowLength = 0.05f;

                static const float DefaultHandStateSize = 4 * DefaultJointSize;

                [Windows::Foundation::Metadata::WebHostHidden]
                public ref class SkeletonPanel sealed
                    : public Panel
                {
                public:
                    SkeletonPanel();

                    property WRK::BodyFrameSource^ BodySource
                    {
                        WRK::BodyFrameSource^ get();
                        void set(_In_ WRK::BodyFrameSource^ value);
                    }

                    property bool RenderBones;
                    property bool RenderJoints;
                    property bool RenderHandStates;
                    property bool RenderJointOrientations;

                protected private:
                    virtual event Windows::UI::Xaml::Data::PropertyChangedEventHandler^ PropertyChanged;
                    void NotifyPropertyChanged(Platform::String^ prop);

                    virtual void Update(double elapsedTime) override;
                    virtual void Render() override;

                    virtual bool IsLoadingComplete() override { return Panel::IsLoadingComplete() && _loadingComplete; }

                    virtual void ResetDeviceResources() override;
                    virtual void CreateDeviceResources() override;
                    virtual void CreateSizeDependentResources() override;

                    // override to add mouse/touch/pen input
                    virtual void StartRenderLoop() override;
                    virtual void StopRenderLoop() override;

                private:
                    ~SkeletonPanel();

                    void DoRenderJoints(UINT bodyIndex, _In_opt_ const PrimitiveParameter* pEffect, float pJointSizeMeters);

                    void DoRenderBones(UINT bodyIndex, _In_opt_ const PrimitiveParameter* pEffect, float pBoneSizeMeters);

                    void RenderBodyHandStates(UINT bodyIndex);

                    void RenderBodyJointOrientations(UINT bodyIndex, _In_opt_ const PrimitiveParameter* pEffect);

                    void RenderBodyJoints(UINT bodyIndex, _In_opt_ const PrimitiveParameter* pEffect)
                    {
                        DoRenderJoints(bodyIndex, pEffect, DefaultJointSize);
                    }

                    void RenderBodyJoints(UINT bodyIndex, _In_opt_ const PrimitiveParameter* pEffect, float pJointSizeMeters)
                    {
                        DoRenderJoints(bodyIndex, pEffect, pJointSizeMeters);
                    }

                    void RenderBodyBones(UINT bodyIndex, _In_opt_ const PrimitiveParameter* pEffect)
                    {
                        DoRenderBones(bodyIndex, pEffect, DefaultBoneSize);
                    }

                    void RenderBodyBones(UINT bodyIndex, _In_opt_ const PrimitiveParameter* pEffect, float pBoneSizeMeters)
                    {
                        DoRenderBones(bodyIndex, pEffect, pBoneSizeMeters);
                    }

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
                        _bodyMeshEffect->ApplyTransformAll(_d3dContext.Get(), worldViewMatrix, wvp);
                    }

                    void ApplyTransformWithWorldMatrix(_In_ const DirectX::XMMATRIX& worldMatrix)
                    {
                        DirectX::XMMATRIX wv = worldMatrix * GetViewMatrix();
                        ApplyTransformWithWorldViewMatrix(wv);
                    }

                private:
                    BOOL                                        _loadingComplete;

                    Microsoft::WRL::ComPtr<ID3D11BlendState>    _blendState;

                    PrimitiveEffect^                            _bodyMeshEffect;

                    DirectX::XMMATRIX                           _viewMatrix;
                    DirectX::XMMATRIX                           _projectionMatrix;

                    WRK::BodyFrameSource^                       _bodySource;
                    WRK::BodyFrameReader^                       _bodyReader;

                    Windows::Foundation::Collections::IVector<Body^>^   _bodies;
                    WRK::Vector4                                        _floorPlane;

                };

            }
        }
    }
}
