//------------------------------------------------------------------------------
// <copyright file="DXSwapPanel.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------

namespace KinectEvolution
{
    using System;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;
    using System.ComponentModel;
    using System.Linq;
    using System.Text;
    using System.Threading.Tasks;
    using Windows.UI.Xaml.Controls;
    using WindowsPreview.Kinect;

    /// <summary>
    /// Wrapper class for view model for the SwapChainPanel
    /// </summary>
    public class DXSwapPanel : BindableBase
    {
        /// <summary>
        /// flag to set when this is the selected item in a list
        /// </summary>
        private bool isSelected;

        /// <summary>
        /// property to define the panel type
        /// </summary>
        private Type panelType;

        /// <summary>
        /// title property for the control
        /// </summary>
        private string title;

        /// <summary>
        /// description property for the control
        /// </summary>
        private string description;

        /// <summary>
        /// instance of the control that is wrapped
        /// </summary>
        private SwapChainPanel swapChainPanel;

        /// <summary>
        /// Wrapper Panel type 
        /// </summary>
        public enum Type
        {
            /// <summary>
            /// data from the sensor
            /// </summary>
            Data,

            /// <summary>
            /// technology that comes from analyzing sensor data
            /// </summary>
            Tech
        }

        /// <summary>
        /// Gets or sets a value indicating whether the instance is selected
        /// </summary>
        public bool IsSelected 
        { 
            get { return this.isSelected; }
            set { this.SetProperty(ref this.isSelected, value, "IsSelected"); }
        }

        /// <summary>
        /// Gets or sets the type
        /// </summary>
        public Type PanelType
        {
            get { return this.panelType; }
            set { this.SetProperty(ref this.panelType, value, "Type"); }
        }

        /// <summary>
        /// Gets or sets the title property
        /// </summary>
        public string Title 
        { 
            get { return this.title; }
            set { this.SetProperty(ref this.title, value, "Title"); }
        }
        
        /// <summary>
        /// Gets or sets the description property
        /// </summary>
        public string Description
        { 
            get { return this.description; }
            set { this.SetProperty(ref this.description, value, "Description"); }
        }
        
        /// <summary>
        /// Gets or sets the instance of the control
        /// </summary>
        public SwapChainPanel DXPanel 
        {
            get { return this.swapChainPanel; }
            set { this.SetProperty(ref this.swapChainPanel, value, "DXPanel"); }
        }
    }
}