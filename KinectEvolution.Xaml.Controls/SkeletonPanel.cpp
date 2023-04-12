//------------------------------------------------------------------------------
// <copyright file="SkeletonPanel.cpp" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------

#include "pch.h"
#include "SkeletonPanel.h"

using namespace KinectEvolution::Xaml::Controls::Base;
using namespace KinectEvolution::Xaml::Controls::Skeleton;

using namespace Concurrency;
using namespace DirectX;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Data;

using namespace WindowsPreview::Kinect;

SkeletonPanel::SkeletonPanel()
    : Panel()
    , _loadingComplete(FALSE)
{
    critical_section::scoped_lock lock(_criticalSection);

    CreateDeviceResources();
    CreateSizeDependentResources();

    RenderBones = TRUE;
    RenderJoints = TRUE;
    RenderHandStates = TRUE;
}

SkeletonPanel::~SkeletonPanel()
{
    Stop();
}

void SkeletonPanel::StartRenderLoop()
{
    DirectXPanel::StartRenderLoop();
}

void SkeletonPanel::StopRenderLoop()
{
    DirectXPanel::StopRenderLoop();
}

void SkeletonPanel::NotifyPropertyChanged(Platform::String^ prop)
{
    PropertyChangedEventArgs^ args = ref new PropertyChangedEventArgs(prop);
    PropertyChanged(this, args);
}

void SkeletonPanel::BodySource::set(_In_ WRK::BodyFrameSource^ value)
{
    if (_bodySource == value)
    {
        return;
    }

    // close the previous reader
    _bodyReader = nullptr;

    // set the new source
    _bodySource = value;

    if (nullptr != _bodySource)
    {
        _bodyReader = _bodySource->OpenReader();
    }

    NotifyPropertyChanged("BodySource");
}

BodyFrameSource^ SkeletonPanel::BodySource::get()
{
    return _bodySource;
}


void SkeletonPanel::Update(double elapsedTime)
{
    BodyFrame^ frame = _bodyReader->AcquireLatestFrame();
    if (nullptr != frame)
    {
        if (_bodies == nullptr)
        {
            _bodies = ref new Platform::Collections::Vector<Body^>(frame->BodyCount);
        }

        frame->GetAndRefreshBodyData(_bodies);

        _floorPlane = frame->FloorClipPlane;

    }
}

void SkeletonPanel::Render()
{
    // create effect
    PrimitiveParameter effectBone;
    ZeroMemory(&effectBone, sizeof(effectBone));
    effectBone._enableLighting = TRUE;
    effectBone._specular = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
    effectBone._power = 22.0f;

    static const XMVECTOR BodyColor [] =
    {
        { 1, 0, 0, 1 },
        { 0, 1, 0, 1 },
        { 0.25f, 1, 1, 1 },
        { 1, 1, 0.25f, 1 },
        { 1.0, 0.25f, 1, 1 },
        { 0.5f, 0.5f, 1, 1 }
    };

    // Set render targets to the screen.
    BeginRender(nullptr, nullptr, nullptr);

    // set the blend state
    _d3dContext->OMSetBlendState(_blendState.Get(), nullptr, 0xFFFFFF);

    // bind viewport the size of the control
    const D3D11_VIEWPORT backBufferView = CD3D11_VIEWPORT(0.0f, 0.0f, _renderTargetWidth, _renderTargetHeight);
    _d3dContext->RSSetViewports(1, &backBufferView);

    for (int i = 0; i < BODY_COUNT; ++i)
    {
        if (RenderBones)
        {
            effectBone._ambient = BodyColor[i];
            RenderBodyBones(i, &effectBone);
        }

        if (RenderJoints)
        {
            effectBone._ambient = XMVectorSet(0.3f, 0.3f, 0.3f, 1.0f);
            effectBone._diffuse = XMVectorSet(0.8f, 0.8f, 0.8f, 1.0f);
            RenderBodyJoints(i, &effectBone);
        }

        if (RenderJointOrientations)
        {
            effectBone._ambient = XMVectorSet(0.3f, 0.3f, 0.3f, 1.0f);
            effectBone._diffuse = XMVectorSet(0.8f, 0.8f, 0.8f, 1.0f);

            RenderBodyJointOrientations(i, &effectBone);
        }
    }

    for (int i = 0; i < BODY_COUNT; ++i)
    {
        if (RenderHandStates)
        {
            RenderBodyHandStates(i);
        }
    }

    EndRender();
}

void SkeletonPanel::ResetDeviceResources()
{
    _loadingComplete = false;

    Panel::ResetDeviceResources();
}

void SkeletonPanel::CreateDeviceResources()
{
    Panel::CreateDeviceResources();

    // create blend state
    D3D11_BLEND_DESC blendStateDesc;
    ZeroMemory(&blendStateDesc, sizeof(D3D11_BLEND_DESC));
    blendStateDesc.AlphaToCoverageEnable = FALSE;
    blendStateDesc.IndependentBlendEnable = FALSE;
    blendStateDesc.RenderTarget[0].BlendEnable = TRUE;
    blendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blendStateDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blendStateDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
    blendStateDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_DEST_ALPHA;
    blendStateDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendStateDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    DX::ThrowIfFailed(
        _d3dDevice->CreateBlendState(&blendStateDesc, &_blendState)
        );

    // create effects
    _bodyMeshEffect = ref new PrimitiveEffect();
    _bodyMeshEffect->Initialize(_d3dDevice.Get());
}

void SkeletonPanel::CreateSizeDependentResources()
{
    Panel::CreateSizeDependentResources();

    _viewMatrix = XMMatrixIdentity();

    _projectionMatrix = XMMatrixPerspectiveFovLH(DirectX::XM_PI * 60.0f / 180.0f, _renderTargetWidth / _renderTargetHeight, 0.01f, 100.0f);

    _loadingComplete = true;

    Render();
}

void SkeletonPanel::DoRenderBones(UINT bodyIndex, _In_opt_ const PrimitiveParameter* pEffect, float pBoneSizeMeters)
{
    if (nullptr == _bodies)
    {
        return;
    }

    Body^ body = _bodies->GetAt(bodyIndex);
    if (nullptr == body || !body->IsTracked)
    {
        return;
    }

    if (!_bodyMeshEffect->Apply(_d3dContext.Get(), pEffect))
    {
        return;
    };

    PrimitiveMesh^ mesh = PrimitiveMesh::GetDefaultPrimitive(_d3dDevice.Get(), _d3dContext.Get(), DefaultPrimitive::Cylinder);
    if (nullptr == mesh)
    {
        return;
    }

    for (UINT i = 0; i < _countof(BodyBones); i++)
    {
        Joint jointA = body->Joints->Lookup(BodyBones[i].JointA);
        Joint jointB = body->Joints->Lookup(BodyBones[i].JointB);

        XMVECTOR positionA = XMLoadFloat3(&XMFLOAT3(jointA.Position.X, jointA.Position.Y, jointA.Position.Z));
        XMVECTOR positionB = XMLoadFloat3(&XMFLOAT3(jointB.Position.X, jointB.Position.Y, jointB.Position.Z));
        if (XMVector3Equal(positionA, XMVectorZero()) || XMVector3Equal(positionB, XMVectorZero()))
        {
            continue;
        }
        float boneScale = pBoneSizeMeters / 2; // cylinder has a radius of 1.0f so div 2 to scale correctly

        if (jointA.TrackingState != TrackingState::Tracked || jointB.TrackingState != TrackingState::Tracked)
        {
            boneScale *= NonTrackedScale;
        }

        XMMATRIX m = XMMatrixScaling(boneScale, 1.0f, boneScale) * PrimitiveEffect::TransformUnitUpVector(positionA, positionB, FALSE);

        ApplyTransformWithWorldMatrix(m);

        mesh->RenderTriangleList(_d3dContext.Get(), TRUE);
    }
}

void SkeletonPanel::DoRenderJoints(UINT bodyIndex, _In_opt_ const PrimitiveParameter* pEffect, float pJointSizeMeters)
{
    if (nullptr == _bodies)
    {
        return;
    }

    Body^ body = _bodies->GetAt(bodyIndex);
    if (nullptr == body || !body->IsTracked)
    {
        return;
    }

    if (!_bodyMeshEffect->Apply(_d3dContext.Get(), pEffect))
    {
        return;
    };

    for (auto pair : body->Joints)
    {
        Joint joint = pair->Value;
        JointOrientation rotation = body->JointOrientations->Lookup(pair->Key);

        PrimitiveMesh^ mesh = nullptr;
        float jointSize = pJointSizeMeters;
        switch (pair->Key)
        {
        case JointType::SpineBase:
        case JointType::SpineMid:
        case JointType::SpineShoulder:
        case JointType::Neck:
        case JointType::Head:
            mesh = PrimitiveMesh::GetDefaultPrimitive(_d3dDevice.Get(), _d3dContext.Get(), DefaultPrimitive::Pyramid);
            jointSize *= 1.15f; // make pyramid joints a little bigger
            break;
        case JointType::ShoulderLeft:
        case JointType::ElbowLeft:
        case JointType::WristLeft:
        case JointType::HandLeft:
        case JointType::HandTipLeft:
        case JointType::ThumbLeft:
        case JointType::HipLeft:
        case JointType::KneeLeft:
        case JointType::AnkleLeft:
        case JointType::FootLeft:
            mesh = PrimitiveMesh::GetDefaultPrimitive(_d3dDevice.Get(), _d3dContext.Get(), DefaultPrimitive::Sphere);
            break;
        case JointType::ShoulderRight:
        case JointType::ElbowRight:
        case JointType::WristRight:
        case JointType::HandRight:
        case JointType::HandTipRight:
        case JointType::ThumbRight:
        case JointType::HipRight:
        case JointType::KneeRight:
        case JointType::AnkleRight:
        case JointType::FootRight:
            mesh = PrimitiveMesh::GetDefaultPrimitive(_d3dDevice.Get(), _d3dContext.Get(), DefaultPrimitive::Cube);
            jointSize *= 0.85f; // make cube joints a little smaller
            break;
        }

        if (nullptr == mesh)
        {
            return;
        }

        XMVECTOR position = XMLoadFloat3(&XMFLOAT3(joint.Position.X, joint.Position.Y, joint.Position.Z));
        XMVECTOR orientation = XMLoadFloat4(&XMFLOAT4(rotation.Orientation.X, rotation.Orientation.Y, rotation.Orientation.Z, rotation.Orientation.W));

        if (XMVector3Equal(position, DirectX::XMVectorZero()))
            continue;


        // render inferred and not tracked joints smaller
        switch (joint.TrackingState)
        {
        case TrackingState::Inferred:
            jointSize *= NonTrackedScale;
            break;
        case TrackingState::NotTracked:
            jointSize *= NonTrackedScale;
            break;
        }

        ApplyTransformWithWorldMatrix(XMMatrixScaling(jointSize, jointSize, jointSize) * XMMatrixTranslationFromVector(position));

        mesh->RenderTriangleList(_d3dContext.Get(), TRUE);
    }
}

void SkeletonPanel::RenderBodyHandStates(UINT bodyIndex)
{
    if (nullptr == _bodies)
    {
        return;
    }

    Body^ body = _bodies->GetAt(bodyIndex);
    if (nullptr == body || !body->IsTracked)
    {
        return;
    }

    // note that hand state are not rendered in the body bracket because it needs to blend with depth
    PrimitiveMesh^ pSphereMesh = PrimitiveMesh::GetDefaultPrimitive(_d3dDevice.Get(), _d3dContext.Get(), DefaultPrimitive::Sphere);
    if (nullptr == pSphereMesh)
    {
        return;
    }

    PrimitiveParameter effect = { 0 };
    effect._enableLighting = TRUE;

    Joint hands [] =
    {
        body->Joints->Lookup(JointType::HandLeft),
        body->Joints->Lookup(JointType::HandRight)
    };

    for (UINT i = 0; i < 2; i++)
    {
        if (hands[i].TrackingState != TrackingState::Tracked)
        {
            effect._ambient = NotTrackedColor;
        }
        else
        {
            UINT index = 0;

            if (hands[i].JointType == JointType::HandLeft)
            {
                index = static_cast<int>(body->HandLeftState);
            }
            else
            {
                index = static_cast<int>(body->HandRightState);
            }

            effect._ambient = HandStateColorTable[min(index - 2, _countof(HandStateColorTable) - 1)];
        }

        if (!_bodyMeshEffect->Apply(_d3dContext.Get(), &effect))
        {
            return;
        };

        XMVECTOR handPosition = XMLoadFloat3(&XMFLOAT3(hands[i].Position.X, hands[i].Position.Y, hands[i].Position.Z));
        if (XMVector3Equal(handPosition, XMVectorZero()))
        {
            continue; // don't render when hand position is not filled in
        }

        XMMATRIX matWorld =
            XMMatrixScaling(DefaultHandStateSize, DefaultHandStateSize, DefaultHandStateSize) *
            XMMatrixTranslationFromVector(handPosition);

        ApplyTransformWithWorldMatrix(matWorld);

        pSphereMesh->RenderTriangleList(_d3dContext.Get(), TRUE);
    }
}

void SkeletonPanel::RenderBodyJointOrientations(UINT bodyIndex, _In_opt_ const PrimitiveParameter* pEffect)
{
    if (nullptr == _bodies)
    {
        return;
    }

    Body^ body = _bodies->GetAt(bodyIndex);
    if (nullptr == body || !body->IsTracked)
    {
        return;
    }

    if (!_bodyMeshEffect->Apply(_d3dContext.Get(), pEffect))
    {
        return;
    };

    PrimitiveMesh^ pArrowMesh = PrimitiveMesh::GetDefaultPrimitive(_d3dDevice.Get(), _d3dContext.Get(), DefaultPrimitive::Cone);
    PrimitiveMesh^ pHandleMesh = PrimitiveMesh::GetDefaultPrimitive(_d3dDevice.Get(), _d3dContext.Get(), DefaultPrimitive::Cylinder);
    if (nullptr == pArrowMesh || nullptr == pHandleMesh)
    {
        return;
    }

    for (auto pair : body->Joints)
    {
        Joint joint = pair->Value;
        JointOrientation rotation = body->JointOrientations->Lookup(pair->Key);

        XMVECTOR position = XMLoadFloat3(&XMFLOAT3(joint.Position.X, joint.Position.Y, joint.Position.Z));
        XMVECTOR orientation = XMLoadFloat4(&XMFLOAT4(rotation.Orientation.X, rotation.Orientation.Y, rotation.Orientation.Z, rotation.Orientation.W));
        if (XMVector3Equal(position, XMVectorZero()) || XMVector4Equal(orientation, XMVectorZero()))
        {
            continue;
        }

        XMVECTOR normal = XMVector3Rotate(XMVectorSet(0, 0, 1, 0), orientation);
        if (XMVector3Equal(normal, XMVectorZero()))
        {
            continue;
        }

        float handleLength = DefaultNormalHandleLength;
        float handleSize = DefaultNormalHandleSize;
        float arrowLength = DefaultNormalArrowLength;

        XMVECTOR handleEnd = position + handleLength * normal;
        XMVECTOR arrowEnd = handleEnd + arrowLength * normal;

        XMMATRIX matHandle = XMMatrixScaling(handleSize, 1.0f, handleSize) * PrimitiveEffect::TransformUnitUpVector(position, handleEnd, FALSE);
        XMMATRIX matArrow = XMMatrixScaling(arrowLength, arrowLength, arrowLength) * PrimitiveEffect::TransformUnitUpVector(handleEnd, arrowEnd, TRUE);

        ApplyTransformWithWorldMatrix(matHandle);
        pHandleMesh->RenderTriangleList(_d3dContext.Get(), TRUE);

        ApplyTransformWithWorldMatrix(matArrow);
        pArrowMesh->RenderTriangleList(_d3dContext.Get(), TRUE);
    }
}
