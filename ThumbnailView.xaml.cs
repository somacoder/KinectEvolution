//------------------------------------------------------------------------------
// <copyright file="ThumbnailView.xaml.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------

namespace KinectEvolution
{
    using System;
    using System.Collections.Generic;
    using System.IO;
    using System.Linq;
    using Windows.Foundation;
    using Windows.Foundation.Collections;
    using Windows.UI.Xaml;
    using Windows.UI.Xaml.Controls;
    using Windows.UI.Xaml.Controls.Primitives;
    using Windows.UI.Xaml.Data;
    using Windows.UI.Xaml.Input;
    using Windows.UI.Xaml.Media;
    using Windows.UI.Xaml.Navigation;

    /// <summary>
    /// User control to handle state changes for re-parenting
    /// </summary>
    public sealed partial class ThumbnailView : UserControl
    {
        /// <summary>
        /// local instance of the control wrapper class
        /// </summary>
        private DXSwapPanel swapPanel = null;

        /// <summary>
        /// Initializes a new instance of the <see cref="ThumbnailView" /> class
        /// </summary>
        public ThumbnailView()
        {
            this.InitializeComponent();

            // setup notification when context has been updated
            this.DataContextChanged += this.OnDataContextChanged;

            var loader = new Windows.ApplicationModel.Resources.ResourceLoader();
            SelectedText.Text = loader.GetString("SELECTED_TEXT");
        }

        /// <summary>
        /// handles data context changes
        /// </summary>
        /// <param name="sender">sender of the event</param>
        /// <param name="args">passed parameter values for the event</param>
        private void OnDataContextChanged(FrameworkElement sender, DataContextChangedEventArgs args)
        {
            DXSwapPanel panel = args.NewValue as DXSwapPanel;

            if (this.swapPanel != panel)
            {
                // unsubscribe to previous event changes
                if (panel != null && this.swapPanel != null)
                {
                    this.swapPanel.PropertyChanged -= this.OnPropertyChanged;
                }

                this.swapPanel = panel;

                // subscribe to the n
                if (this.swapPanel != null)
                {
                    this.swapPanel.PropertyChanged += this.OnPropertyChanged;
                }

                if (!this.swapPanel.IsSelected)
                {
                    // ensure control is not parented
                    var parent = this.swapPanel.DXPanel.Parent as Panel;
                    if (parent != null)
                    {
                        parent.Children.Remove(this.swapPanel.DXPanel);
                    }
                    else
                    {
                        var border = this.swapPanel.DXPanel.Parent as Border;
                        if (border != null)
                        {
                            border.Child = null;
                        }
                    }

                    ParentGrid.Children.Add(this.swapPanel.DXPanel);
                }
            }
        }

        /// <summary>
        ///  handles property changes to the wrapper class
        /// </summary>
        /// <param name="sender">sender of the event</param>
        /// <param name="args">passed parameter values for the event</param>
        private void OnPropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs args)
        {
            if (args.PropertyName == "IsSelected")
            {
                if (this.swapPanel.IsSelected)
                {
                    VisualStateManager.GoToState(this, "Selected", true);
                }
                else
                {
                    VisualStateManager.GoToState(this, "Normal", true);

                    // for a non-selected item, should not have a parent of Border
                    var parent = this.swapPanel.DXPanel.Parent as Panel;
                    if (parent != null)
                    {
                        parent.Children.Remove(this.swapPanel.DXPanel);
                    }
                    else
                    {
                        var border = this.swapPanel.DXPanel.Parent as Border;
                        if (border != null && !this.swapPanel.IsSelected)
                        {
                            this.swapPanel.IsSelected = true;

                            return;
                        }
                    }

                    ParentGrid.Children.Add(this.swapPanel.DXPanel);
                }
            }
        }
    }
}
