//------------------------------------------------------------------------------
// <copyright file="BlockManPanel.h" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------

#pragma once

#include "Panel.h"
#include "BlockManMesh.h"

namespace KinectEvolution {
    namespace Xaml {
        namespace Controls {
            namespace BlockMan {

                using namespace KinectEvolution::Xaml::Controls::Base;

#define QUATERNION_SMOOTHNESS 0.7f

                struct SkeletonBlock
                {
                    DirectX::XMVECTOR Scale;
                    DirectX::XMVECTOR CenterFromParentCenter;
                    WRK::JointType SkeletonJoint;
                    int ParentBlockIndex;
                };

                const SkeletonBlock g_SkeletonBlocks [] = {
                        { { 0.75f, 1.0f, 0.5f, 0.0f }, { 0.0f, 0.0f, 0.0f, 0.0f }, WRK::JointType::SpineMid, -1 },      //  0 - lower torso
                        { { 0.75f, 1.0f, 0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f, 0.0f }, WRK::JointType::SpineShoulder, 0 },  //  1 - upper torso

                        { { 0.25f, 0.25f, 0.25f, 0.0f }, { 0.0f, 1.0f, 0.0f, 0.0f }, WRK::JointType::Neck, 1 },         //  2 - neck
                        { { 0.5f, 0.5f, 0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f, 0.0f }, WRK::JointType::Head, 2 },            //  3 - head

                        { { 0.35f, 0.4f, 0.35f, 0.0f }, { 0.5f, 1.0f, 0.0f, 0.0f }, WRK::JointType::ShoulderLeft, 1 },  //  4 - Left shoulderblade
                        { { 0.25f, 1.0f, 0.25f, 0.0f }, { 0.0f, 1.0f, 0.0f, 0.0f }, WRK::JointType::ElbowLeft, 4 },     //  5 - Left upper arm
                        { { 0.15f, 1.0f, 0.15f, 0.0f }, { 0.0f, 1.0f, 0.0f, 0.0f }, WRK::JointType::WristLeft, 5 },     //  6 - Left forearm
                        { { 0.10f, 0.4f, 0.30f, 0.0f }, { 0.0f, 1.0f, 0.0f, 0.0f }, WRK::JointType::HandLeft, 6 },      //  7 - Left hand

                        { { 0.35f, 0.4f, 0.35f, 0.0f }, { -0.5f, 1.0f, 0.0f, 0.0f }, WRK::JointType::ShoulderRight, 1 },//   8 - Right shoulderblade
                        { { 0.25f, 1.0f, 0.25f, 0.0f }, { 0.0f, 1.0f, 0.0f, 0.0f }, WRK::JointType::ElbowRight, 8 },    //   9 - Right upper arm
                        { { 0.15f, 1.0f, 0.15f, 0.0f }, { 0.0f, 1.0f, 0.0f, 0.0f }, WRK::JointType::WristRight, 9 },    //  10 - Right forearm
                        { { 0.10f, 0.4f, 0.30f, 0.0f }, { 0.0f, 1.0f, 0.0f, 0.0f }, WRK::JointType::HandRight, 10 },    //  11 - Right hand

                        { { 0.35f, 0.4f, 0.35f, 0.0f }, { 0.5f, 0.0f, 0.0f, 0.0f }, WRK::JointType::HipLeft, 0 },       //  12 - Left hipblade
                        { { .4f, 1.0f, .4f, 0.0f }, { 0.5f, 0.0f, 0.0f, 0.0f }, WRK::JointType::KneeLeft, 12 },         //  13 - Left thigh
                        { { .3f, 1.0f, .3f, 0.0f }, { 0.0f, 1.0f, 0.0f, 0.0f }, WRK::JointType::AnkleLeft, 13 },        //  14 - Left calf
                        //{ { .35f, .6f, .20f, 0.0f }, { 0.0f, 1.0f, 0.0f, 0.0f }, WRK::JointType::FootLeft , 14 },     //     - Left foot

                        { { 0.35f, 0.4f, 0.35f, 0.0f }, { -0.5f, 0.0f, 0.0f, 0.0f }, WRK::JointType::HipRight, 0 },     //  15  - Right hipblade
                        { { .4f, 1.0f, .4f, 0.0f }, { -0.5f, 0.0f, 0.0f, 0.0f }, WRK::JointType::KneeRight, 15 },       //  16  - Right thigh
                        { { .3f, 1.0f, .3f, 0.0f }, { 0.0f, 1.0f, 0.0f, 0.0f }, WRK::JointType::AnkleRight, 16 },       //  17  - Right calf
                        //{ { .35f, .6f, .20f, 0.0f }, { 0.0f, 1.0f, 0.0f, 0.0f }, WRK::JointType::FootRight, 18 },     //      - Right foot

                };

                const UINT BLOCK_COUNT = _countof(g_SkeletonBlocks);
                const UINT BODY_COUNT = 6;
                const UINT JOINT_COUNT = 25;

                inline DirectX::XMVECTOR SmoothQuaternion(DirectX::XMVECTOR accumulatedQuaternion, DirectX::XMVECTOR newQuaternion)
                {
                    if (!DirectX::XMVector4Equal(newQuaternion, DirectX::XMVectorZero()))
                    {
                        DirectX::XMVECTOR normalizedNewQuaternion = DirectX::XMQuaternionNormalizeEst(newQuaternion);
                        if (DirectX::XMVector4Equal(accumulatedQuaternion, DirectX::XMVectorZero()))
                        {
                            return normalizedNewQuaternion;
                        }
                        else
                        {
                            return DirectX::XMQuaternionSlerp(normalizedNewQuaternion, accumulatedQuaternion, QUATERNION_SMOOTHNESS);
                        }
                    }
                    else
                    {
                        return accumulatedQuaternion;
                    }
                }

                struct BlockManBody
                {
                    DirectX::XMVECTOR _JointOrientations[JOINT_COUNT];
                    DirectX::XMVECTOR _HeadOrientation;
                    DirectX::XMVECTOR _position;

                    DirectX::XMVECTOR _BlockTranslations[BLOCK_COUNT];
                    DirectX::XMVECTOR _BlockOrientations[BLOCK_COUNT];

                    DirectX::XMMATRIX _BlockTransforms[BLOCK_COUNT];

                    DirectX::XMMATRIX _mWorld;
                    DirectX::XMMATRIX _FrontView;
                    DirectX::XMMATRIX _BackView;

                    BlockManBody()
                    {
                        // Initialize the world matrices
                        DirectX::XMVECTOR origin = DirectX::XMVectorZero();
                        DirectX::XMVECTOR planeNormal = DirectX::XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f);
                        DirectX::XMVECTOR plane = DirectX::XMPlaneFromPointNormal(origin, planeNormal);

                        _mWorld = DirectX::XMMatrixReflect(plane);

                        // Initialize the view matrix
                        DirectX::XMVECTOR eye = DirectX::XMVectorSet(0.0f, 0.0f, -10.0f, 0.0f);
                        DirectX::XMVECTOR neye = DirectX::XMVectorSet(0.0f, 0.0f, 10.0f, 0.0f);
                        DirectX::XMVECTOR at = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
                        DirectX::XMVECTOR up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

                        _BackView = DirectX::XMMatrixLookAtLH(eye, at, up);
                        _FrontView = DirectX::XMMatrixLookAtLH(neye, at, up);

                        Reset();
                    }

                    void Reset()
                    {
                        _HeadOrientation = DirectX::XMVectorZero();

                        ZeroMemory(_JointOrientations, sizeof(_JointOrientations));

                        _position = DirectX::XMVectorZero();
                    }
                };

                [Windows::Foundation::Metadata::WebHostHidden]
                public ref class BlockManPanel sealed
                    : public Panel
                {
                public:
                    BlockManPanel();

                    property WRK::BodyFrameSource^ BodySource
                    {
                        WRK::BodyFrameSource^ get();
                        void set(WRK::BodyFrameSource^ value);
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

                private:
                    ~BlockManPanel();

                    virtual void OnBodyFrame(_In_ WRK::BodyFrame^ frame);

                protected private:
                    BlockManBody                                _blockMen[BODY_COUNT];

                    BlockManMesh^                               _blockManMesh;

                    DirectX::XMMATRIX                           _mProjection;

                    BOOL                                        _bFront;

                    float                                       _curRotationAngleRad;

                    float                                       _scale;
                    float                                       _offsetX;
                    float                                       _offsetY;
                    float                                       _offsetZ;

                    WRK::BodyFrameSource^                       _frameSource;
                    WRK::BodyFrameReader^                       _frameReader;
                    Windows::Foundation::Collections::IVector<WRK::Body^>^ _bodies;
                };

            }
        }
    }
}
