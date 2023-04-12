//------------------------------------------------------------------------------
// <copyright file="ThumbnailViewModel.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------

namespace KinectEvolution
{
    using System;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
    using System.Linq;
    using System.Text;
    using System.Threading.Tasks;
    using KinectEvolution.Xaml.Controls;
    using KinectEvolution.Xaml.Controls.BlockMan;
    using KinectEvolution.Xaml.Controls.DepthMap;
    using KinectEvolution.Xaml.Controls.Skeleton;
    using Windows.UI.Xaml.Controls;
    using WindowsPreview.Kinect;

    /// <summary>
    /// View model for generating the controls to use in the main application
    /// </summary>
    public class ThumbnailViewModel
    {
        /// <summary>
        /// Instance of the KinectSensor object
        /// </summary>
        private KinectSensor kinectSensor;

        /// <summary>
        /// Initializes a new instance of the <see cref="ThumbnailViewModel" /> class
        /// </summary>
        /// <param name="sensor">takes an instance of the KinectSensor instance.</param>
        public ThumbnailViewModel(KinectSensor sensor)
        {
            this.CreateAll(sensor);
        }

        /// <summary>
        /// Gets or sets the collection of the wrapped swap panel control
        /// </summary>
        public ObservableCollection<DXSwapPanel> Panels { get; set; }

        /// <summary>
        /// Creates an instance of the DepthWithColor viewer control
        /// </summary>
        /// <param name="sensor">Instance of the KinectSensor object</param>
        /// <returns>Returns an instance of the control</returns>
        private static SwapChainPanel CreateDepthwithColor(KinectSensor sensor)
        {
            DepthMapPanel control = new DepthMapPanel();
            control.DepthSource = sensor.DepthFrameSource;
            control.PanelMode = DEPTH_PANEL_MODE.DEPTH_RAMP;

            return control;
        }

        /// <summary>
        /// Creates an instance of the BlockMan viewer control 
        /// </summary>
        /// <param name="sensor">Instance of the KinectSensor object</param>
        /// <returns>Returns an instance of the control</returns>
        private static SwapChainPanel CreateRotation(KinectSensor sensor)
        {
            BlockManPanel control = new BlockManPanel();
            control.BodySource = sensor.BodyFrameSource;

            return control;
        }

        /// <summary>
        /// Creates an instance Body viewer control 
        /// </summary>
        /// <param name="sensor">Instance of the KinectSensor object</param>
        /// <returns>Returns an instance of the control</returns>
        private static SwapChainPanel CreateBody(KinectSensor sensor)
        {
            SkeletonPanel control = new SkeletonPanel();
            control.BodySource = sensor.BodyFrameSource;
            control.RenderBones = true;
            control.RenderHandStates = true;
            control.RenderJointOrientations = false;
            control.RenderJoints = true;

            return control;
        }

        /// <summary>
        /// Creates an instance Audio control 
        /// </summary>
        /// <param name="sensor">Instance of the KinectSensor object</param>
        /// <returns>Returns an instance of the control</returns>
        private static SwapChainPanel CreateAudio(KinectSensor sensor)
        {
            AudioPanel control = new AudioPanel();
            control.AudioSource = sensor.AudioSource;

            return control;
        }

        /// <summary>
        /// method to create all the known tech panels
        /// </summary>
        /// <param name="sensor">Instance of the KinectSensor object</param>
        private void CreateAll(KinectSensor sensor)
        {
            if (this.kinectSensor == sensor)
            {
                return;
            }

            if (sensor == null)
            {
                if (this.kinectSensor != null && this.kinectSensor.IsOpen)
                {
                    this.kinectSensor.Close();
                }

                this.Panels.Clear();
            }

            this.kinectSensor = sensor;

            if (this.kinectSensor == null)
            {
                return;
            }

            var loader = new Windows.ApplicationModel.Resources.ResourceLoader();

            this.Panels = new ObservableCollection<DXSwapPanel>();
            this.Panels.Add(new DXSwapPanel { PanelType = DXSwapPanel.Type.Tech, Title = loader.GetString("TECH_AUDIO"), Description = loader.GetString("TECH_AUDIO_DESC"), DXPanel = CreateAudio(this.kinectSensor) });
            this.Panels.Add(new DXSwapPanel { PanelType = DXSwapPanel.Type.Tech, Title = loader.GetString("TECH_BODY"), Description = loader.GetString("TECH_BODY_DESC"), DXPanel = CreateBody(this.kinectSensor) });
            this.Panels.Add(new DXSwapPanel { PanelType = DXSwapPanel.Type.Tech, Title = loader.GetString("TECH_ROTATION"), Description = loader.GetString("TECH_ROTATION_DESC"), DXPanel = CreateRotation(this.kinectSensor) });
            this.Panels.Add(new DXSwapPanel { PanelType = DXSwapPanel.Type.Tech, Title = loader.GetString("TECH_DEPTH_WITH_COLOR"), Description = loader.GetString("TECH_DEPTH_WITH_COLOR_DESC"), DXPanel = CreateDepthwithColor(this.kinectSensor) });
        }
    }
}
