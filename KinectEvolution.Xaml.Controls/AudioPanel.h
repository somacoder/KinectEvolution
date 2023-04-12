//------------------------------------------------------------------------------
// <copyright file="AudioPanel.h" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------

#pragma once

#include "Panel.h"
#include "StepTimer.h"

using namespace KinectEvolution::Xaml::Controls::Base;

namespace KinectEvolution {
    namespace Xaml {
        namespace Controls {

            [Windows::Foundation::Metadata::WebHostHidden]
            public ref class AudioPanel sealed
                : public Panel
            {
            public:
                AudioPanel();

                property WRK::AudioSource^ AudioSource
                {
                    WRK::AudioSource^ get();
                    void set(_In_ WRK::AudioSource^ value);
                }

            protected private:
                virtual event Windows::UI::Xaml::Data::PropertyChangedEventHandler^ PropertyChanged;

                void NotifyPropertyChanged(_In_ Platform::String^ prop);

                // override to add mouse/touch/pen input
                virtual void StartRenderLoop() override;
                virtual void StopRenderLoop() override;

                virtual void Update(double elapsedTime) override;
                virtual void Render() override;

                virtual bool IsLoadingComplete() override { return Panel::IsLoadingComplete() && _loadingComplete; }

                virtual void ResetDeviceResources() override;
                virtual void CreateSizeDependentResources() override;

            private:
                ~AudioPanel();

                void ProcessAudio();
                void UpdateBeamAngle();
                void UpdateEnergy();
                void UpdateEnergyDisplay(const float *pEnergy, const UINT energyLength);

                HRESULT CreateEnergyDisplay();
                HRESULT CreateBeamGauge();
                HRESULT CreateBeamGaugeNeedle();
                HRESULT CreateConfidenceGauge();
                HRESULT CreateConfidenceGaugeFill(const float & width);
                HRESULT CreatePanelOutline();

            private:
                BOOL                                        _loadingComplete;
                BOOL                                        _processingAudio;
                Concurrency::critical_section               _processingLock;

                // audio thread
                DX::StepTimer                               _audioTimer;
                Windows::Foundation::IAsyncAction^          _audioLoopWorker;


                WRK::AudioSource^                           _audioSource;
                WRK::AudioBeam^                             _audioBeam;
                Windows::Storage::Streams::IInputStream^    _audioStream;

                // Data buffer for audio stream
                Windows::Storage::Streams::Buffer^          _dataBuffer;

                /// Width/Height of engery visualization.
                UINT                        _uiEnergyDisplayWidth;
                UINT                        _uiEnergyDisplayHeight;

                D2D_MATRIX_3X2_F            _renderTargetTransform;
                BYTE*                       _pbEnergyBackground;
                UINT                        _uiEnergyBackgroundStride;
                BYTE*                       _pbEnergyForeground;
                UINT                        _uiEnergyForegroundStride;
                Microsoft::WRL::ComPtr<ID2D1Bitmap>                _energyDisplay;

                D2D1_RECT_F                 _energyDisplayPosition;
                Microsoft::WRL::ComPtr<ID2D1PathGeometry>          _beamGauge;
                Microsoft::WRL::ComPtr<ID2D1RadialGradientBrush>   _beamGaugeFill;
                Microsoft::WRL::ComPtr<ID2D1PathGeometry>          _beamNeedle;
                Microsoft::WRL::ComPtr<ID2D1LinearGradientBrush>   _beamNeedleFill;
                Microsoft::WRL::ComPtr<ID2D1PathGeometry>          _confidenceGauge;
                Microsoft::WRL::ComPtr<ID2D1LinearGradientBrush>   _confidenceGaugeFill;

                D2D_MATRIX_3X2_F            _beamNeedleTransform;
                Microsoft::WRL::ComPtr<ID2D1PathGeometry>          _panelOutline;
                Microsoft::WRL::ComPtr<ID2D1SolidColorBrush>       _panelOutlineStroke;

            private:
                // Time interval, in milliseconds, for timer that drives audio capture.
                static const int        cAudioReadTimerInterval = 50;

                // Audio samples per second in Kinect audio stream
                static const int        cAudioSamplesPerSecond = 16000;

                // Number of float samples in the audio beffer we allocate for reading every time the audio capture timer fires
                // (should be larger than the amount of audio corresponding to cAudioReadTimerInterval msec).
                static const int        cAudioBufferLength = (int) (2 * cAudioReadTimerInterval * cAudioSamplesPerSecond * .001);

                // Number of audio samples captured from Kinect audio stream accumulated into a single
                // energy measurement that will get displayed.
                static const int        cAudioSamplesPerEnergySample = 15;

                // Number of energy samples that will be visible in display at any given time.
                static const int        cEnergySamplesToDisplay = 2100;

                // Number of energy samples that will be stored in the circular buffer.
                // Always keep it higher than the energy display length to avoid overflow.
                static const int        cEnergyBufferLength = 2300;

                // Minimum energy of audio to display (in dB value, where 0 dB is full scale)
                static const int        cMinEnergy = -90;

                const FLOAT             cVisTop = 0.052f;

                // Buffer used to store audio stream energy data as we read audio.
                Platform::Array<float>^ _fEnergyBuffer;

                // Buffer used to store audio stream energy data ready to be displayed.
                Platform::Array<float>^ _fEnergyDisplayBuffer;

                // Sum of squares of audio samples being accumulated to compute the next energy value.
                float                   _fAccumulatedSquareSum;

                // Error between time slice we wanted to display and time slice that we ended up
                // displaying, given that we have to display in integer pixels.
                float                   _fEnergyError;

                // Number of audio samples accumulated so far to compute the next energy value.
                int                     _nAccumulatedSampleCount;

                // Index of next element available in audio energy buffer.
                int                     _nEnergyIndex;

                // Number of newly calculated audio stream energy values that have not yet been displayed.
                int                     _nNewEnergyAvailable;

                // Index of first energy element that has never (yet) been displayed to screen.
                int                     _nEnergyRefreshIndex;

                // Last time energy visualization was rendered to screen.
                ULONGLONG               _nLastEnergyRefreshTime;

            };

        }
    }
}
