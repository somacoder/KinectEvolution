//------------------------------------------------------------------------------
// <copyright file="MainPage.xaml.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------

namespace KinectEvolution
{
    using System;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
    using System.Linq;
    using KinectEvolution.Xaml.Controls;
    using KinectEvolution.Xaml.Controls.DepthMap;
    using Windows.UI.ViewManagement;
    using Windows.UI.Xaml;
    using Windows.UI.Xaml.Controls;
    using Windows.UI.Xaml.Input;
    using Windows.UI.Xaml.Navigation;
    using WindowsPreview.Kinect;
    using Windows.UI.Popups;

    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class MainPage : Page
    {
        /// <summary>
        /// last view state name
        /// </summary>
        private string lastState;

        /// <summary>
        /// local instance of the KinectSensor
        /// </summary>
        private KinectSensor kinectSSensor;

        /// <summary>
        /// List of the available camera modes for the visualizer
        /// </summary>
        private List<DEPTH_PANEL_MODE> cameraDataMode;

        /// <summary>
        /// Tracks the currently selected panel mode for the camera
        /// </summary>
        private DEPTH_PANEL_MODE lastCameraPanel;

        /// <summary>
        /// Instance of the DepthMapPanel control
        /// </summary>
        private DepthMapPanel lastSwapChainPanel;

        /// <summary>
        /// tracking instance of the last selected tech panel
        /// </summary>
        private DXSwapPanel lastSelectedTech;

        /// <summary>
        /// tracking index of the last index to compare against
        /// </summary>
        private int lastSelectedTechIndex;

        /// <summary>
        /// tracking view state for camera panel view
        /// </summary>
        private bool isCameraFullScreen;

        /// <summary>
        /// tracking view state for tech panel view
        /// </summary>
        private bool isTechFullScreen;
        
        /// <summary>
        /// Initializes a new instance of the <see cref="MainPage"/> class
        /// </summary>
        public MainPage()
        {
            this.Loaded += this.MainPage_Loaded;
            this.Unloaded += this.MainPage_Unloaded;

            // register for size change
            this.SizeChanged += this.MainPage_SizeChanged;

            this.InitializeComponent();
        }

        /// <summary>
        /// possible directions for navigation
        /// </summary>
        private enum Direction
        {
            /// <summary>
            /// navigation going ahead
            /// </summary>
            Forward = 0,

            /// <summary>
            /// navigation going backward
            /// </summary>
            Back
        }

        /// <summary>
        /// register for key events when the page is setup
        /// </summary>
        /// <param name="e">event parameters</param>
        protected override void OnNavigatedTo(NavigationEventArgs e)
        {
            base.OnNavigatedTo(e);

            // focus on this control for keyboard registration
            this.Focus(Windows.UI.Xaml.FocusState.Programmatic);

            Window.Current.CoreWindow.KeyUp += this.CoreWindow_KeyUp;
        }

        /// <summary>
        /// unregister for key events
        /// </summary>
        /// <param name="e">event parameters</param>
        protected override void OnNavigatingFrom(NavigatingCancelEventArgs e)
        {
            Window.Current.CoreWindow.KeyUp -= this.CoreWindow_KeyUp;

            base.OnNavigatingFrom(e);
        }

        /// <summary>
        /// Initialization of the main camera panel and other properties of the window
        /// </summary>
        /// <param name="sender">sender of the event</param>
        /// <param name="e">event parameters</param>
        private void MainPage_Loaded(object sender, RoutedEventArgs e)
        {
            // configure each camera with appropriate Kinect Source
            var modeLst = (DEPTH_PANEL_MODE[])Enum.GetValues(typeof(DEPTH_PANEL_MODE));
            this.cameraDataMode = modeLst.OfType<DEPTH_PANEL_MODE>().ToList();

            // remove depth ramp w/color as an option in the camera view
            this.cameraDataMode.RemoveAt(this.cameraDataMode.IndexOf(DEPTH_PANEL_MODE.DEPTH_RAMP));

            // set last panel
            this.lastCameraPanel = this.cameraDataMode[0];

            // get the sensor
            this.kinectSSensor = KinectSensor.GetDefault();
            this.kinectSSensor.IsAvailableChanged += this.IsAvailableChanged;
            this.kinectSSensor.Open();

            // create depth swappanel
            this.lastSwapChainPanel = new DepthMapPanel() 
            { 
                CoordinateMapper = this.kinectSSensor.CoordinateMapper,
                DepthSource = this.kinectSSensor.DepthFrameSource,
                InfraredSource = this.kinectSSensor.InfraredFrameSource,
                ColorSource = this.kinectSSensor.ColorFrameSource,
                PanelMode = this.lastCameraPanel
            };

            // set the camera on the panel
            this.CameraPreview.Child = this.lastSwapChainPanel;

            // get the tech collection
            ThumbnailViewModel viewModel = new ThumbnailViewModel(this.kinectSSensor);
            var result = from t in viewModel.Panels
                         where t.PanelType == DXSwapPanel.Type.Tech
                         group t by t.PanelType into g
                         orderby g.Key
                         select new { Key = g.Key, Items = g };

            ThumbnailsSource.Source = result;

            // select the first item from the collection
            this.lastSelectedTechIndex = -1;
        }

        /// <summary>
        /// fires when the window is unloaded
        /// </summary>
        /// <param name="sender">sender of the parameter</param>
        /// <param name="e">event parameters</param>
        private void MainPage_Unloaded(object sender, RoutedEventArgs e)
        {
            this.SizeChanged -= this.MainPage_SizeChanged;
        }

        /// <summary>
        /// changes the view state based on window size
        /// </summary>
        /// <param name="sender">sender of the event</param>
        /// <param name="e">event parameters</param>
        private void MainPage_SizeChanged(object sender, SizeChangedEventArgs e)
        {
            this.SetViewState();
        } 

        /// <summary>
        /// register for key up events
        /// </summary>
        /// <param name="sender">event sender</param>
        /// <param name="args">event parameters</param>
        private void CoreWindow_KeyUp(Windows.UI.Core.CoreWindow sender, Windows.UI.Core.KeyEventArgs args)
        {
            switch (args.VirtualKey)
            {
                case Windows.System.VirtualKey.N:
                    this.SwitchCameraView(Direction.Back);
                    break;
                case Windows.System.VirtualKey.M:
                    this.SwitchCameraView(Direction.Forward);
                    break;
                case Windows.System.VirtualKey.Right:
                    this.SwitchTechView(Direction.Forward);
                    break;
                case Windows.System.VirtualKey.Left:
                    this.SwitchTechView(Direction.Back);
                    break;
                case Windows.System.VirtualKey.Enter:
                    this.SwitchToTechFullscreen();
                    break;
                case Windows.System.VirtualKey.Space:
                    this.SwitchToCameraFullscreen();
                    break;
                default:
                    break;
            }
        }

        /// <summary>
        /// changes the state of the UI based on whether a sensor is available
        /// </summary>
        /// <param name="sender">sender of the parameter</param>
        /// <param name="args">event parameters</param>
        private void IsAvailableChanged(KinectSensor sender, IsAvailableChangedEventArgs args)
        {
            this.SetViewState();

            if(args.IsAvailable)
            {
                // trigger controls to start rendering
                this.lastSwapChainPanel.Start();

                foreach (DXSwapPanel panel in Thumbnails.Items)
                {
                    if (panel != null)
                    {
                        KinectEvolution.Xaml.Controls.Base.Panel dx
                            = panel.DXPanel as KinectEvolution.Xaml.Controls.Base.Panel;
                        dx.Start();
                    }
                }

                if(this.lastSelectedTechIndex == -1)
                {
                    this.lastSelectedTechIndex = 0;
                }

                Thumbnails.SelectedIndex = this.lastSelectedTechIndex;
            }
        }

        /// <summary>
        /// event from thumbnail list to swap out selected tech panel
        /// </summary>
        /// <param name="sender">sender of the event</param>
        /// <param name="e">event parameter</param>
        private void OnSelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            // ensure the tech is no longer parented and selected property is false
            if (this.lastSelectedTech != null)
            {
                var parent = this.lastSelectedTech.DXPanel.Parent as Panel;
                if (parent != null)
                {
                    parent.Children.Remove(this.lastSelectedTech.DXPanel);
                }
                else
                {
                    var border = this.lastSelectedTech.DXPanel.Parent as Border;
                    border.Child = null;
                }

                // trigger the property change
                this.lastSelectedTech.IsSelected = false;
            }

            DXSwapPanel tech = Thumbnails.SelectedItem as DXSwapPanel;
            if (tech != null)
            {
                // trigger the property change
                tech.IsSelected = true;

                // parent the selected control to the main UI
                var parent = tech.DXPanel.Parent as Panel;
                if (parent != null)
                {
                    parent.Children.Remove(tech.DXPanel);
                }
                else
                {
                    var border = tech.DXPanel.Parent as Border;
                    if (border != null)
                    {
                        border.Child = null;
                    }
                }

                // parent the panel to large view
                if (DXSwapPanel.Type.Tech == tech.PanelType)
                {
                    TechPreview.Child = tech.DXPanel;
                }
            }

            // track this as the last selected item
            if(tech != null)
            {
                this.lastSelectedTech = tech;
            }
            else
            {
                this.Thumbnails.SelectedIndex = this.lastSelectedTechIndex;
            }
        }

        /// <summary>
        /// resets the view to default
        /// </summary>
        /// <param name="sender">sender of the event</param>
        /// <param name="e">event parameter</param>
        private void DefaultView_Click(object sender, RoutedEventArgs e)
        {
            this.SetViewState();

            AppBar.IsOpen = false;
        }

        /// <summary>
        /// makes the camera view full screen
        /// </summary>
        /// <param name="sender">sender of the event</param>
        /// <param name="e">event parameter</param>
        private void ToggleCameraFullScreen_Click(object sender, RoutedEventArgs e)
        {
            this.SwitchToCameraFullscreen();

            AppBar.IsOpen = false;
        }

        /// <summary>
        /// makes the tech view full screen
        /// </summary>
        /// <param name="sender">sender of the event</param>
        /// <param name="e">event parameter</param>
        private void ToggleTechFullscreen_Click(object sender, RoutedEventArgs e)
        {
            this.SwitchToTechFullscreen();

            AppBar.IsOpen = false;
        }

        /// <summary>
        /// Launches the help
        /// </summary>
        /// <param name="sender">sender of the event</param>
        /// <param name="e">event parameter</param>
        private void HelpButton_Click(object sender, RoutedEventArgs e)
        {
            HelpSettingsFlyout helpSF = new HelpSettingsFlyout();

            helpSF.ShowIndependent();

            AppBar.IsOpen = false;
        }
        
        /// <summary>
        /// switch camera view
        /// </summary>
        /// <param name="sender">sender of the event</param>
        /// <param name="e">event parameter</param>
        private void CameraPreview_Tapped(object sender, TappedRoutedEventArgs e)
        {
            this.SwitchCameraView();
        }

        /// <summary>
        /// switch tech view
        /// </summary>
        /// <param name="sender">sender of the event</param>
        /// <param name="e">event parameter</param>
        private void TechPreview_Tapped(object sender, TappedRoutedEventArgs e)
        {
            this.SwitchTechView();
        }

        /// <summary>
        /// switches the camera panel to next source view
        /// </summary>
        /// <param name="direction">navigate forward or back</param>
        private void SwitchCameraView(Direction direction = Direction.Forward)
        {
            int oldID = this.cameraDataMode.IndexOf(this.lastCameraPanel);
            int newID = oldID + ((direction == Direction.Forward) ? 1 : -1);

            if (newID >= this.cameraDataMode.Count)
            {
                newID = 0;
            }
            else if (newID < 0)
            {
                newID = this.cameraDataMode.Count - 1;
            }

            this.lastCameraPanel = this.cameraDataMode[newID];

            DepthMapPanel view = this.lastSwapChainPanel as DepthMapPanel;
            if (view != null)
            {
                view.PanelMode = this.lastCameraPanel;
            }
        }

        /// <summary>
        /// changes the selected tech panel based on direction
        /// </summary>
        /// <param name="direction">navigate forward or back</param>
        private void SwitchTechView(Direction direction = Direction.Forward)
        {
            int newID = (direction == Direction.Forward) ? 1 : -1;

            // make sure we only advance when the selection matches what is selected
            if (this.lastSelectedTechIndex != Thumbnails.SelectedIndex)
            {
                newID = Thumbnails.SelectedIndex;
            }
            else
            {
                newID += Thumbnails.SelectedIndex;
            }

            // clamp the selection to the count bounds
            if (newID >= Thumbnails.Items.Count)
            {
                newID = 0;
            }
            else if (newID < 0)
            {
                newID = Thumbnails.Items.Count - 1;
            }

            // set this as the last selected index
            // select the item from the list
            Thumbnails.SelectedIndex = this.lastSelectedTechIndex = newID;
        }

        /// <summary>
        /// control view state of window for camera panel
        /// </summary>
        private void SwitchToCameraFullscreen()
        {
            if (!this.isCameraFullScreen)
            {
                this.isCameraFullScreen = true;
                this.isTechFullScreen = false;

                this.lastState = "CameraFullscreen";
                VisualStateManager.GoToState(this, this.lastState, true);
            }
            else
            {
                this.SetViewState();
            }
        }

        /// <summary>
        /// control view state of window for tech panel
        /// </summary>
        private void SwitchToTechFullscreen()
        {
            if (!this.isTechFullScreen)
            {
                this.isCameraFullScreen = false;
                this.isTechFullScreen = true;

                this.lastState = "TechFullscreen";
                VisualStateManager.GoToState(this, this.lastState, true);
            }
            else
            {
                this.SetViewState();
            }
        }

        /// <summary>
        /// set the view state based on screen size and sensor IsAvailable property
        /// </summary>
        private void SetViewState()
        {
            var loader = new Windows.ApplicationModel.Resources.ResourceLoader();

            var applicationView = ApplicationView.GetForCurrentView();

            if (applicationView.IsFullScreen) 
            { 
                if (this.kinectSSensor != null && this.kinectSSensor.IsAvailable)
                {
                    this.lastState = "Default";
                }
                else
                {
                    DisplayMessage.Text = loader.GetString("NO_SENSOR");
                    this.lastState = "NoSensor";
                }
            } 
            else 
            {
                DisplayMessage.Text = loader.GetString("MSG_FULL_SCREEN");
                this.lastState = "Snapped";
            }

            this.isCameraFullScreen = false;
            this.isTechFullScreen = false;

            VisualStateManager.GoToState(this, this.lastState, true);
        }
    }
}