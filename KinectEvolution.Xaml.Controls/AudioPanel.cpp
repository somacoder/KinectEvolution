//------------------------------------------------------------------------------
// <copyright file="AudioPanel.cpp" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------

#include "pch.h"
#include "AudioPanel.h"

using namespace KinectEvolution::Xaml::Controls;
using namespace KinectEvolution::Xaml::Controls::Base;

using namespace Concurrency;
using namespace DirectX;
using namespace Windows::Foundation;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::System::Threading;
using namespace Microsoft::WRL;

using namespace WindowsPreview::Kinect;

static XMVECTOR vAudioBeamColor = { 0, 1.0f, 0, 0.7f };
static XMVECTOR vAudioTextColor = { 0, 1.0f, 0, 1.0f };

AudioPanel::AudioPanel()
    : Panel()
    , _fAccumulatedSquareSum(0.0)
    , _fEnergyError(0.0)
    , _nAccumulatedSampleCount(0)
    , _nEnergyIndex(0)
    , _nEnergyRefreshIndex(0)
    , _nNewEnergyAvailable(0)
    , _nLastEnergyRefreshTime(0)
    , _loadingComplete(FALSE)
{
    critical_section::scoped_lock lock(_criticalSection);

    CreateDeviceResources();
    CreateSizeDependentResources();

    _fEnergyBuffer = ref new Platform::Array<float>(cEnergyBufferLength);
    _fEnergyDisplayBuffer = ref new Platform::Array<float>(cEnergySamplesToDisplay);
}

AudioPanel::~AudioPanel()
{
    Stop();
}

void AudioPanel::StartRenderLoop()
{
    // If the animation render loop is already running then do not start another thread.
    if (nullptr != _audioLoopWorker && _audioLoopWorker->Status == AsyncStatus::Started)
    {
        return;
    }

    _audioTimer.SecondsToTicks(cAudioReadTimerInterval / 1000.0);

    auto audioHandler = ref new WorkItemHandler([this](IAsyncAction ^ action)
    {
        while (action->Status == AsyncStatus::Started)
        {
            _audioTimer.Tick([&]()
            {
                critical_section::scoped_lock lock(_processingLock);

                ProcessAudio();
            });

        }
    });

    //Run task on a dedicated high priority background thread.
    _audioLoopWorker = ThreadPool::RunAsync(audioHandler, WorkItemPriority::High, WorkItemOptions::TimeSliced);

    DirectXPanel::StartRenderLoop();
}

void AudioPanel::StopRenderLoop()
{
    // Cancel the asynchronous task and let the render thread exit.
    if (nullptr != _audioLoopWorker)
    {
        _audioLoopWorker->Cancel();
    }

    DirectXPanel::StopRenderLoop();
}

void AudioPanel::Update(double elapsedTime)
{
    UpdateBeamAngle();

    UpdateEnergy();
}

void AudioPanel::Render()
{
    if (!IsLoadingComplete())
    {
        return;
    }

    _d2dContext->BeginDraw();
    _d2dContext->SetTransform(_renderTargetTransform);

    _d2dContext->Clear(D2D1::ColorF(D2D1::ColorF::White));

    // Draw energy display
    _d2dContext->DrawBitmap(_energyDisplay.Get(), _energyDisplayPosition);

    // Draw audio beam gauge
    _d2dContext->FillGeometry(_beamGauge.Get(), _beamGaugeFill.Get(), nullptr);
    _d2dContext->SetTransform(_beamNeedleTransform * _renderTargetTransform);
    _d2dContext->FillGeometry(_beamNeedle.Get(), _beamNeedleFill.Get(), nullptr);
    _d2dContext->SetTransform(_renderTargetTransform);

    // Draw audio beam confidence gauge
    _confidenceGaugeFill->SetTransform(_beamNeedleTransform);
    _d2dContext->FillGeometry(_confidenceGauge.Get(), _confidenceGaugeFill.Get(), nullptr);

    // Draw panel outline
    _d2dContext->DrawGeometry(_panelOutline.Get(), _panelOutlineStroke.Get(), 0.001f);

    _d2dContext->EndDraw();

    // present
    DirectXPanel::Present();
}


// audio source property
AudioSource^ AudioPanel::AudioSource::get()
{
    return _audioSource;
}

void AudioPanel::AudioSource::set(_In_ WRK::AudioSource^ value)
{
    if (_audioSource == value)
    {
        return;
    }

    // set the new source
    _audioSource = value;

    if (nullptr != _audioSource)
    {
        Windows::Foundation::Collections::IVectorView<AudioBeam^>^ audioBeamList = _audioSource->AudioBeams;
        _audioBeam = audioBeamList->GetAt(0);
        _audioStream = _audioBeam->OpenInputStream();
        _dataBuffer = ref new Windows::Storage::Streams::Buffer(cEnergyBufferLength);
    }
    else
    {
        _audioBeam = nullptr;
        _audioStream = nullptr;
        _dataBuffer = nullptr;
    }

    NotifyPropertyChanged("AudioSource");
}

void AudioPanel::NotifyPropertyChanged(_In_ Platform::String^ prop)
{
    PropertyChangedEventArgs^ args = ref new PropertyChangedEventArgs(prop);
    PropertyChanged(this, args);
}


// process the avaiable audio data
void AudioPanel::ProcessAudio()
{
    if (nullptr == _audioSource)
    {
        return;
    }

    if (_audioSource->AudioBeams->Size > 0)
    {
        if (!_processingAudio)
        {
            _processingAudio = true;

            // create async task to get buffer from stream
            auto asyncReadOp = _audioStream->ReadAsync(_dataBuffer, cEnergyBufferLength, Windows::Storage::Streams::InputStreamOptions::None);

            Concurrency::create_task(asyncReadOp).then([this](Windows::Storage::Streams::IBuffer^ audioBuffer)
            {
                critical_section::scoped_lock lock(_processingLock);

                // if there is data in the buffer
                if (audioBuffer->Length > 0)
                {
                    // extract audio samples from buffer
                    byte* pData = DX::GetPointerToPixelData(audioBuffer);

                    if (nullptr != pData)
                    {
                        // Calculate energy from audio
                        UINT64 nSampleCount = audioBuffer->Length / sizeof(FLOAT);

                        for (UINT i = 0; i < nSampleCount; ++i)
                        {
                            // compute the sum of squares of audio samples that will get accumulated
                            // into a single energy value.
                            float fData = *reinterpret_cast<float*>(&pData[i*sizeof(float)]);

                            _fAccumulatedSquareSum += fData * fData;
                            ++_nAccumulatedSampleCount;

                            if (_nAccumulatedSampleCount < cAudioSamplesPerEnergySample)
                            {
                                continue;
                            }

                            // Each energy value will represent the logarithm of the mean of the
                            // sum of squares of a group of audio samples.
                            float fMeanSquare = _fAccumulatedSquareSum / cAudioSamplesPerEnergySample;
                            float fEnergy = cMinEnergy;
                            if (fMeanSquare > 0.f)
                            {
                                // Convert to dB
                                fEnergy = 10.0f * log10(fMeanSquare);
                            }

                            // Renormalize signal above noise floor to [0,1] range for visualization.
                            _fEnergyBuffer[_nEnergyIndex] = (cMinEnergy - fEnergy) / cMinEnergy;
                            _nNewEnergyAvailable++;
                            _nEnergyIndex = (_nEnergyIndex + 1) % cEnergyBufferLength;

                            _fAccumulatedSquareSum = 0.f;
                            _nAccumulatedSampleCount = 0;
                        }

                    }
                }

                // process more audio
                _processingAudio = false;
            });

        }

    }
}

void AudioPanel::UpdateBeamAngle()
{
    if (nullptr == _audioSource)
    {
        return;
    }

    if (_audioSource->AudioBeams->Size > 0)
    {
        // Get most recent audio beam angle and confidence
        FLOAT fBeamAngle = _audioBeam->BeamAngle;
        FLOAT fBeamAngleConfidence = _audioBeam->BeamAngleConfidence;

        // Convert angles to degrees and set values in audio panel
        float beamAngle = 180.0f * fBeamAngle / XM_PI;

        // Maximum possible confidence corresponds to this gradient width
        const float cMinGradientWidth = 0.04f;

        // Set width of mark based on confidence.
        // A confidence of 0 would give us a gradient that fills whole area diffusely.
        // A confidence of 1 would give us the narrowest allowed gradient width.
        float width = max((1 - fBeamAngleConfidence), cMinGradientWidth);

        // Update the gradient representing to reflect confidence
        CreateConfidenceGaugeFill(width);

        _beamNeedleTransform = D2D1::Matrix3x2F::Rotation(-beamAngle, D2D1::Point2F(0.5f, cVisTop + (-0.033f)));
    }
}

void AudioPanel::UpdateEnergy()
{
    // Calculate how many energy samples we need to advance since the last update in order to
    // have a smooth animation effect.
    ULONGLONG previousRefreshTime = _nLastEnergyRefreshTime;
    ULONGLONG now = GetTickCount64();

    _nLastEnergyRefreshTime = now;

    // No need to refresh if there is no new energy available to render
    if (0 >= _nNewEnergyAvailable)
    {
        return;
    }

    if (0 != previousRefreshTime)
    {
        float energyToAdvance = _fEnergyError + (((now - previousRefreshTime) * cAudioSamplesPerSecond / 1000.0f) / cAudioSamplesPerEnergySample);
        int energySamplesToAdvance = min(_nNewEnergyAvailable, (int)(energyToAdvance));
        _fEnergyError = energyToAdvance - energySamplesToAdvance;
        _nEnergyRefreshIndex = (_nEnergyRefreshIndex + energySamplesToAdvance) % cEnergyBufferLength;
        _nNewEnergyAvailable -= energySamplesToAdvance;
    }

    // Copy energy samples into buffer to be displayed, taking into account that energy
    // wraps around in a circular buffer.
    int baseIndex = (_nEnergyRefreshIndex + cEnergyBufferLength - cEnergySamplesToDisplay) % cEnergyBufferLength;
    int samplesUntilEnd = cEnergyBufferLength - baseIndex;
    if (samplesUntilEnd > cEnergySamplesToDisplay)
    {
        critical_section::scoped_lock lock(_processingLock);

        memcpy_s(_fEnergyDisplayBuffer->Data, cEnergySamplesToDisplay * sizeof(float), _fEnergyBuffer->Data + baseIndex, cEnergySamplesToDisplay * sizeof(float));
    }
    else
    {
        critical_section::scoped_lock lock(_processingLock);

        int samplesFromBeginning = cEnergySamplesToDisplay - samplesUntilEnd;

        memcpy_s(_fEnergyDisplayBuffer->Data, cEnergySamplesToDisplay * sizeof(float), _fEnergyBuffer->Data + baseIndex, samplesUntilEnd * sizeof(float));
        memcpy_s(_fEnergyDisplayBuffer->Data + samplesUntilEnd, (cEnergySamplesToDisplay - samplesUntilEnd) * sizeof(float), _fEnergyBuffer->Data, samplesFromBeginning * sizeof(float));
    }

    //update the energy to display in the rendered output panel
    UpdateEnergyDisplay(_fEnergyDisplayBuffer->Data, cEnergySamplesToDisplay);

}

void AudioPanel::UpdateEnergyDisplay(const float *pEnergy, const UINT energyLength)
{
    // Clear whole display to background color
    _energyDisplay->CopyFromMemory(nullptr, _pbEnergyBackground, _uiEnergyBackgroundStride);

    const int cHalfImageHeight = _uiEnergyDisplayHeight / 2;

    for (UINT i = 0; i < min(energyLength, _uiEnergyDisplayWidth); ++i)
    {
        // Each bar has a minimum height of 1 (to get a steady signal down the middle) and a maximum height
        // equal to the bitmap height.
        int barHeight = static_cast<int>(max(1.0f, (pEnergy[i] * _uiEnergyDisplayHeight)));

        // Center bar vertically on image
        int top = cHalfImageHeight - (barHeight / 2);
        int bottom = top + barHeight;
        D2D1_RECT_U barRect = D2D1::RectU(i, top, i + 1, bottom);

        // Draw bar in foreground color
        _energyDisplay->CopyFromMemory(&barRect, _pbEnergyForeground, _uiEnergyForegroundStride);
    }
};


void AudioPanel::ResetDeviceResources()
{
    _loadingComplete = false;

    Panel::ResetDeviceResources();

    _energyDisplay.ReleaseAndGetAddressOf();
    _beamGauge.ReleaseAndGetAddressOf();
    _beamGaugeFill.ReleaseAndGetAddressOf();
    _beamNeedle.ReleaseAndGetAddressOf();
    _beamNeedleFill.ReleaseAndGetAddressOf();
    _confidenceGauge.ReleaseAndGetAddressOf();
    _confidenceGaugeFill.ReleaseAndGetAddressOf();
    _panelOutline.ReleaseAndGetAddressOf();
    _panelOutlineStroke.ReleaseAndGetAddressOf();
}

void AudioPanel::CreateSizeDependentResources()
{
    Panel::CreateSizeDependentResources();

    // set the Direct2D render target.
    _d2dContext->SetTarget(_d2dTargetBitmap.Get());

    _uiEnergyDisplayWidth = static_cast<UINT>(_renderTargetWidth * (_width / _renderTargetWidth));
    _uiEnergyDisplayHeight = static_cast<UINT>(max(_uiEnergyDisplayWidth * .25, 1.0f));

    _renderTargetTransform = D2D1::Matrix3x2F::Scale(D2D1::SizeF((FLOAT) _uiEnergyDisplayWidth, (FLOAT) _uiEnergyDisplayWidth));

    HRESULT hr = CreateEnergyDisplay();
    if (SUCCEEDED(hr))
    {
        hr = CreateBeamGauge();

        if (SUCCEEDED(hr))
        {
            hr = CreateConfidenceGauge();

            if (SUCCEEDED(hr))
            {
                hr = CreatePanelOutline();
            }
        }
    }

    _loadingComplete = true;

    Render(); // render one frame to force resize
}


HRESULT AudioPanel::CreateEnergyDisplay()
{
    const int cBytesPerPixel = 4;
    const int cMaxPixelChannelIntensity = 0xff;

    D2D1_SIZE_U size = D2D1::SizeU(_uiEnergyDisplayWidth, _uiEnergyDisplayHeight);
    HRESULT hr = S_OK;

    // Allocate background and set to white
    _uiEnergyBackgroundStride = cBytesPerPixel * _uiEnergyDisplayWidth;
    int numBackgroundBytes = _uiEnergyBackgroundStride * _uiEnergyDisplayHeight;
    _pbEnergyBackground = new BYTE[numBackgroundBytes];
    memset(_pbEnergyBackground, cMaxPixelChannelIntensity, numBackgroundBytes);

    // Allocate foreground and set to blue/violet color
    _uiEnergyForegroundStride = cBytesPerPixel;
    _pbEnergyForeground = new BYTE[cBytesPerPixel * _uiEnergyDisplayHeight];
    UINT *pPixels = reinterpret_cast<UINT*>(_pbEnergyForeground);
    for (UINT iPixel = 0; iPixel < _uiEnergyDisplayHeight; ++iPixel)
    {
        pPixels[iPixel] = 0x8A2BE2;
    }

    // Specify layout position for energy display
    _energyDisplayPosition = D2D1::RectF(0.13f, cVisTop, 0.87f, cVisTop + 0.185f);

    hr = _d2dContext->CreateBitmap(
        size,
        D2D1::BitmapProperties(D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE)),
        &_energyDisplay
        );

    if (SUCCEEDED(hr))
    {
        _energyDisplay->CopyFromMemory(nullptr, _pbEnergyBackground, _uiEnergyBackgroundStride);
    }

    return hr;
}

HRESULT AudioPanel::CreateBeamGauge()
{
    HRESULT hr = _d2dFactory->CreatePathGeometry(&_beamGauge);

    // Create gauge background shape
    if (SUCCEEDED(hr))
    {
        Microsoft::WRL::ComPtr<ID2D1GeometrySink> pGeometrySink;
        hr = _beamGauge->Open(&pGeometrySink);

        if (SUCCEEDED(hr))
        {
            pGeometrySink->BeginFigure(D2D1::Point2F(0.1503f, cVisTop + 0.2479f), D2D1_FIGURE_BEGIN_FILLED);
            pGeometrySink->AddLine(D2D1::Point2F(0.228f, cVisTop + 0.185f));
            pGeometrySink->AddArc(D2D1::ArcSegment(D2D1::Point2F(0.772f, cVisTop + 0.185f), D2D1::SizeF(0.35f, 0.35f), 102, D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE, D2D1_ARC_SIZE_SMALL));
            pGeometrySink->AddLine(D2D1::Point2F(0.8497f, cVisTop + 0.2479f));
            pGeometrySink->AddArc(D2D1::ArcSegment(D2D1::Point2F(0.1503f, cVisTop + 0.2479f), D2D1::SizeF(0.45f, 0.45f), 102, D2D1_SWEEP_DIRECTION_CLOCKWISE, D2D1_ARC_SIZE_SMALL));
            pGeometrySink->EndFigure(D2D1_FIGURE_END_CLOSED);
            hr = pGeometrySink->Close();

            // Create gauge background brush
            if (SUCCEEDED(hr))
            {
                Microsoft::WRL::ComPtr<ID2D1GradientStopCollection> pGradientStops;
                D2D1_GRADIENT_STOP gradientStops[4];
                gradientStops[0].color = D2D1::ColorF(D2D1::ColorF::LightGray, 1);
                gradientStops[0].position = 0.0f;
                gradientStops[1].color = D2D1::ColorF(D2D1::ColorF::LightGray, 1);
                gradientStops[1].position = cVisTop + 0.325f;
                gradientStops[2].color = D2D1::ColorF(D2D1::ColorF::WhiteSmoke, 1);
                gradientStops[2].position = cVisTop + 0.345f;
                gradientStops[3].color = D2D1::ColorF(D2D1::ColorF::WhiteSmoke, 1);
                gradientStops[3].position = 1.0f;
                hr = _d2dContext->CreateGradientStopCollection(
                    gradientStops,
                    4,
                    &pGradientStops
                    );

                if (SUCCEEDED(hr))
                {
                    hr = _d2dContext->CreateRadialGradientBrush(D2D1::RadialGradientBrushProperties(D2D1::Point2F(0.5f, 0.0f), D2D1::Point2F(0.0f, 0.0f), 1.0f, 1.0f), pGradientStops.Get(), &_beamGaugeFill);

                    if (SUCCEEDED(hr))
                    {
                        // Create gauge needle shape and fill brush
                        hr = CreateBeamGaugeNeedle();
                    }
                }
            }
        }

    }

    return hr;
}

HRESULT AudioPanel::CreateBeamGaugeNeedle()
{
    HRESULT hr = _d2dFactory->CreatePathGeometry(&_beamNeedle);

    // Create gauge needle shape
    if (SUCCEEDED(hr))
    {
        Microsoft::WRL::ComPtr<ID2D1GeometrySink> pGeometrySink;
        hr = _beamNeedle->Open(&pGeometrySink);

        if (SUCCEEDED(hr))
        {
            pGeometrySink->BeginFigure(D2D1::Point2F(0.495f, cVisTop + 0.3147f), D2D1_FIGURE_BEGIN_FILLED);
            pGeometrySink->AddLine(D2D1::Point2F(0.505f, cVisTop + 0.3147f));
            pGeometrySink->AddLine(D2D1::Point2F(0.5f, cVisTop + 0.4047f));
            pGeometrySink->EndFigure(D2D1_FIGURE_END_CLOSED);
            hr = pGeometrySink->Close();

            // Create gauge needle brush
            if (SUCCEEDED(hr))
            {
                Microsoft::WRL::ComPtr<ID2D1GradientStopCollection> pGradientStops;
                D2D1_GRADIENT_STOP gradientStops[4];
                gradientStops[0].color = D2D1::ColorF(D2D1::ColorF::LightGray, 1);
                gradientStops[0].position = 0.0f;
                gradientStops[1].color = D2D1::ColorF(D2D1::ColorF::LightGray, 1);
                gradientStops[1].position = 0.35f;
                gradientStops[2].color = D2D1::ColorF(D2D1::ColorF::BlueViolet, 1);
                gradientStops[2].position = 0.395f;
                gradientStops[3].color = D2D1::ColorF(D2D1::ColorF::BlueViolet, 1);
                gradientStops[3].position = 1.0f;
                hr = _d2dContext->CreateGradientStopCollection(
                    gradientStops,
                    4,
                    &pGradientStops
                    );

                if (SUCCEEDED(hr))
                {
                    hr = _d2dContext->CreateLinearGradientBrush(D2D1::LinearGradientBrushProperties(D2D1::Point2F(0.5f, 0.0f), D2D1::Point2F(0.5f, 1.0f)), pGradientStops.Get(), &_beamNeedleFill);
                }

            }
        }

    }

    return hr;
}

HRESULT AudioPanel::CreateConfidenceGauge()
{
    HRESULT hr = _d2dFactory->CreatePathGeometry(&_confidenceGauge);

    // Create gauge background shape
    if (SUCCEEDED(hr))
    {
        Microsoft::WRL::ComPtr<ID2D1GeometrySink> pGeometrySink;
        hr = _confidenceGauge->Open(&pGeometrySink);

        if (SUCCEEDED(hr))
        {
            pGeometrySink->BeginFigure(D2D1::Point2F(0.1270f, cVisTop + 0.2668f), D2D1_FIGURE_BEGIN_FILLED);
            pGeometrySink->AddLine(D2D1::Point2F(0.1503f, cVisTop + 0.2479f));
            pGeometrySink->AddArc(D2D1::ArcSegment(D2D1::Point2F(0.8497f, cVisTop + 0.2479f), D2D1::SizeF(0.45f, 0.45f), 102, D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE, D2D1_ARC_SIZE_SMALL));
            pGeometrySink->AddLine(D2D1::Point2F(0.8730f, cVisTop + 0.2668f));
            pGeometrySink->AddArc(D2D1::ArcSegment(D2D1::Point2F(0.1270f, cVisTop + 0.2668f), D2D1::SizeF(0.48f, 0.48f), 102, D2D1_SWEEP_DIRECTION_CLOCKWISE, D2D1_ARC_SIZE_SMALL));
            pGeometrySink->EndFigure(D2D1_FIGURE_END_CLOSED);
            hr = pGeometrySink->Close();

            // Create gauge background brush
            if (SUCCEEDED(hr))
            {
                hr = CreateConfidenceGaugeFill(0.1f);
            }
        }

    }

    return hr;
}

HRESULT AudioPanel::CreateConfidenceGaugeFill(const float & width)
{
    HRESULT hr = S_OK;

    float halfWidth = width / 2;
    Microsoft::WRL::ComPtr<ID2D1GradientStopCollection> pGradientStops;
    D2D1_GRADIENT_STOP gradientStops[5];
    gradientStops[0].color = D2D1::ColorF(D2D1::ColorF::White, 1);
    gradientStops[0].position = 0.0f;
    gradientStops[1].color = D2D1::ColorF(D2D1::ColorF::White, 1);
    gradientStops[1].position = max(0.5f - halfWidth, 0.0f);
    gradientStops[2].color = D2D1::ColorF(D2D1::ColorF::BlueViolet, 1);
    gradientStops[2].position = 0.5f;
    gradientStops[3].color = D2D1::ColorF(D2D1::ColorF::White, 1);
    gradientStops[3].position = min(0.5f + halfWidth, 1.0f);
    gradientStops[4].color = D2D1::ColorF(D2D1::ColorF::White, 1);
    gradientStops[4].position = 1.0f;
    hr = _d2dContext->CreateGradientStopCollection(
        gradientStops,
        5,
        &pGradientStops
        );

    if (SUCCEEDED(hr))
    {
        _confidenceGaugeFill.ReleaseAndGetAddressOf();
        hr = _d2dContext->CreateLinearGradientBrush(D2D1::LinearGradientBrushProperties(D2D1::Point2F(0.0f, 0.5f), D2D1::Point2F(1.0f, 0.5f)), pGradientStops.Get(), &_confidenceGaugeFill);
    }

    return hr;
}

HRESULT AudioPanel::CreatePanelOutline()
{
    HRESULT hr = _d2dFactory->CreatePathGeometry(&_panelOutline);

    // Create panel outline path
    if (SUCCEEDED(hr))
    {
        Microsoft::WRL::ComPtr<ID2D1GeometrySink> pGeometrySink;
        hr = _panelOutline->Open(&pGeometrySink);

        if (SUCCEEDED(hr))
        {
            /// Draw left wave display frame
            pGeometrySink->BeginFigure(D2D1::Point2F(0.15f, cVisTop), D2D1_FIGURE_BEGIN_FILLED);
            pGeometrySink->AddLine(D2D1::Point2F(0.13f, cVisTop));
            pGeometrySink->AddLine(D2D1::Point2F(0.13f, cVisTop + 0.185f));
            pGeometrySink->AddLine(D2D1::Point2F(0.2280f, cVisTop + 0.185f));

            // Draw gauge outline
            pGeometrySink->AddLine(D2D1::Point2F(0.1270f, cVisTop + 0.2668f));
            pGeometrySink->AddArc(D2D1::ArcSegment(D2D1::Point2F(0.8730f, cVisTop + 0.2668f), D2D1::SizeF(0.48f, 0.48f), 102, D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE, D2D1_ARC_SIZE_SMALL));
            pGeometrySink->AddLine(D2D1::Point2F(0.7720f, cVisTop + 0.185f));
            pGeometrySink->AddArc(D2D1::ArcSegment(D2D1::Point2F(0.2280f, cVisTop + 0.185f), D2D1::SizeF(0.35f, 0.35f), 102, D2D1_SWEEP_DIRECTION_CLOCKWISE, D2D1_ARC_SIZE_SMALL));

            // Reposition geometry without drawing
            pGeometrySink->SetSegmentFlags(D2D1_PATH_SEGMENT_FORCE_UNSTROKED);
            pGeometrySink->AddLine(D2D1::Point2F(0.7720f, cVisTop + 0.185f));
            pGeometrySink->SetSegmentFlags(D2D1_PATH_SEGMENT_NONE);

            // Draw right wave display frame
            pGeometrySink->AddLine(D2D1::Point2F(0.87f, cVisTop + 0.185f));
            pGeometrySink->AddLine(D2D1::Point2F(0.87f, cVisTop));
            pGeometrySink->AddLine(D2D1::Point2F(0.85f, cVisTop));
            pGeometrySink->EndFigure(D2D1_FIGURE_END_OPEN);
            hr = pGeometrySink->Close();

            // Create panel outline brush
            if (SUCCEEDED(hr))
            {
                hr = _d2dContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::LightGray), &_panelOutlineStroke);
            }
        }

    }

    return hr;
}
