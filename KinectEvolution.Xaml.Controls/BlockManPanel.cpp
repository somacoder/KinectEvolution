//------------------------------------------------------------------------------
// <copyright file="BlockManPanel.cpp" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------

#include "pch.h"
#include "BlockManPanel.h"

using namespace KinectEvolution::Xaml::Controls::BlockMan;

using namespace Concurrency;
using namespace DirectX;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Data;

using namespace WindowsPreview::Kinect;

BlockManPanel::BlockManPanel()
    : Panel()
    , _scale(4.0f)
    , _offsetX(0)
    , _offsetY(-0.5f)
    , _offsetZ(-8.0f)
    , _bFront(FALSE)
    , _curRotationAngleRad(0.0f)
{
    critical_section::scoped_lock lock(_criticalSection);

    CreateDeviceResources();
    CreateSizeDependentResources();
}

BlockManPanel::~BlockManPanel()
{
    Stop();
}

void BlockManPanel::StartRenderLoop()
{
    DirectXPanel::StartRenderLoop();
}

void BlockManPanel::StopRenderLoop()
{
    DirectXPanel::StopRenderLoop();
}

/// <summary>
/// Sets the source for this control
/// </summary>
void BlockManPanel::BodySource::set(BodyFrameSource^ value)
{
    if (_frameSource == value)
    {
        return;
    }

    // close the previous reader
    _frameReader = nullptr;

    // set the new source
    _frameSource = value;

    if (nullptr != _frameSource)
    {
        _frameReader = _frameSource->OpenReader();
    }

    NotifyPropertyChanged("BodySource");
}

BodyFrameSource^ BlockManPanel::BodySource::get()
{
    return _frameSource;
}

void BlockManPanel::NotifyPropertyChanged(Platform::String^ prop)
{
    PropertyChangedEventArgs^ args = ref new PropertyChangedEventArgs(prop);
    PropertyChanged(this, args);
}

void BlockManPanel::Update(double elapsedTime)
{
    // if everything is loaded, then we can create the texture
    if (nullptr == _frameReader)
    {
        return;
    }

    // check for body ddata
    BodyFrame^ frame = _frameReader->AcquireLatestFrame();
    if (nullptr != frame)
    {
        OnBodyFrame(frame);
    }

}

void BlockManPanel::OnBodyFrame(_In_ BodyFrame^ bodyFrame)
{
    if (nullptr == _bodies)
    {
        _bodies = ref new Platform::Collections::Vector<Body^>(bodyFrame->BodyCount);;
    }

    bodyFrame->GetAndRefreshBodyData(_bodies);

    for (int iBody = 0; iBody < bodyFrame->BodyCount; ++iBody)
    {
        IBody^ body = _bodies->GetAt(iBody);
        if (nullptr == body)
        {
            continue;
        }

        BOOLEAN bTracked = false;
        if (!body->IsTracked)
        {
            _blockMen[iBody].Reset();

            continue;
        }

        IMapView<JointType, Joint>^ joints = body->Joints;
        IMapView<JointType, JointOrientation>^ jointOrientations = body->JointOrientations;
        {
            for (UINT i = 0; i < joints->Size; i++)
            {
                JointType jt = static_cast<JointType>(i);

                // TODO: update when head orientation is fixed
                if (jt == JointType::Head)
                {
                    continue;
                }

                JointOrientation jo = jointOrientations->Lookup(jt);
                _blockMen[iBody]._JointOrientations[i] = SmoothQuaternion(_blockMen[iBody]._JointOrientations[i], XMLoadFloat4((XMFLOAT4*) &jo.Orientation));
            }

            for (int blockIndex = 0; blockIndex < BLOCK_COUNT; ++blockIndex)
            {
                JointType jointIndex = g_SkeletonBlocks[blockIndex].SkeletonJoint;
                int parentIndex = g_SkeletonBlocks[blockIndex].ParentBlockIndex;

                XMVECTOR translateFromParent = g_SkeletonBlocks[blockIndex].CenterFromParentCenter;
                XMMATRIX parentTransform = (parentIndex > -1) ? _blockMen[iBody]._BlockTransforms[parentIndex] : XMMatrixIdentity();
                XMVECTOR position = XMVector3Transform(translateFromParent, parentTransform);

                XMVECTOR rotationQuat = XMVectorZero();

                if (XMVector4Equal(_blockMen[iBody]._JointOrientations[(int)jointIndex], XMVectorZero()))
                {
                    if (parentIndex > -1)
                    {
                        if (XMVector4Equal(_blockMen[iBody]._JointOrientations[parentIndex], XMVectorZero()))
                        {
                            rotationQuat = XMQuaternionIdentity();
                        }
                        else
                        {
                            rotationQuat = _blockMen[iBody]._JointOrientations[parentIndex];
                        }
                    }
                }
                else
                {
                    rotationQuat = _blockMen[iBody]._JointOrientations[(int) jointIndex];
                }

                XMMATRIX translation = XMMatrixTranslationFromVector(position);
                XMMATRIX scale = XMMatrixScalingFromVector(g_SkeletonBlocks[blockIndex].Scale);
                XMMATRIX rotation = XMMatrixRotationQuaternion(rotationQuat);

                _blockMen[iBody]._BlockTransforms[blockIndex] = scale * rotation * translation;
            }

            Joint joint = joints->Lookup(JointType::SpineBase);

            _blockMen[iBody]._position = XMLoadFloat4((XMFLOAT4*) &(joint.Position));
        }
    }
}

void BlockManPanel::Render()
{
    if (!IsLoadingComplete())
    {
        return;
    }

    // Set render targets to the screen.
    BeginRender(nullptr, nullptr, nullptr);

    // bind viewport the size of the control
    const D3D11_VIEWPORT backBufferView = CD3D11_VIEWPORT(0.0f, 0.0f, _renderTargetWidth, _renderTargetHeight);
    _d3dContext->RSSetViewports(1, &backBufferView);

    // Initialize the projection matrix
    _mProjection = XMMatrixPerspectiveFovLH(DirectX::XM_PI * 60.0f / 180.0f, _renderTargetWidth / _renderTargetHeight, 0.01f, 100.0f);

    // Setup our lighting parameters
    XMFLOAT4 lightDirs[2] =
    {
        XMFLOAT4(-.707f, 0.0f, -.707f, 1.0f),
        XMFLOAT4(.707f, 0.0f, -.707f, 1.0f),
    };

    XMFLOAT4 lightColors[2] =
    {
        XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f),
        XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f)
    };

    // Rotate the second light around the origin
    XMMATRIX mRotate = XMMatrixRotationY(-2.0f * _curRotationAngleRad);
    XMVECTOR lightDir = XMLoadFloat4(&lightDirs[1]);
    lightDir = XMVector3Transform(lightDir, mRotate);
    XMStoreFloat4(&lightDirs[1], lightDir);

    BlockManEffect::BlockManParams fxParams;

    for (int iBody = 0; iBody < BODY_COUNT; ++iBody)
    {
        if (XMVector3Equal(_blockMen[iBody]._position, XMVectorZero()))
        {
            continue;
        }

        // Update matrix variables and lighting variables
        fxParams.mView = _bFront ? _blockMen[iBody]._FrontView : _blockMen[iBody]._BackView;
        fxParams.mProjection = _mProjection;
        fxParams.lightDir[0] = lightDirs[0];
        fxParams.lightDir[1] = lightDirs[1];
        fxParams.lightColor[0] = lightColors[0];
        fxParams.lightColor[1] = lightColors[1];
        fxParams.outputColor = XMFLOAT4(0, 0, 0, 0);

        //
        // Render the cubes
        //
        for (int blockIndex = 0; blockIndex < BLOCK_COUNT; ++blockIndex)
        {
            fxParams.mWorld = _blockMen[iBody]._mWorld * _blockMen[iBody]._BlockTransforms[blockIndex] * XMMatrixTranslationFromVector(_blockMen[iBody]._position * XMVectorSet(_scale, _scale, _scale, 1.0f) + XMVectorSet(_offsetX, _offsetY, _offsetZ, 0));

            _blockManMesh->Render(_d3dContext.Get(), TRUE, &fxParams);
        }

        ID3D11Buffer* buffer = nullptr;
        _d3dContext->VSSetConstantBuffers(0, 1, &buffer);
        _d3dContext->PSSetConstantBuffers(0, 1, &buffer);

        ID3D11ShaderResourceView* nullSRV = nullptr;
        _d3dContext->VSSetShaderResources(0, 1, &nullSRV);
        _d3dContext->PSSetShaderResources(0, 1, &nullSRV);
    }

    // finalize the render
    Present();
}

void BlockManPanel::ResetDeviceResources()
{
    Panel::ResetDeviceResources();
    _blockManMesh = nullptr;
}

void BlockManPanel::CreateDeviceResources()
{
    Panel::CreateDeviceResources();

    _blockManMesh = ref new BlockManMesh();
    _blockManMesh->Initialize(_d3dDevice.Get(), _d3dContext.Get());
}

void BlockManPanel::CreateSizeDependentResources()
{
    Panel::CreateSizeDependentResources();

    Render();
}
