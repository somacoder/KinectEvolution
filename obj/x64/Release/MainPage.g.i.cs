﻿

#pragma checksum "D:\Dropbox\Active\KinectEvolution\KinectEvolution.Xaml\MainPage.xaml" "{406ea660-64cf-4c82-b6f0-42d48172a799}" "096D8A53D4368F5F9C9CAC972DCBAA09"
//------------------------------------------------------------------------------
// <auto-generated>
//     This code was generated by a tool.
//
//     Changes to this file may cause incorrect behavior and will be lost if
//     the code is regenerated.
// </auto-generated>
//------------------------------------------------------------------------------

namespace KinectEvolution
{
    partial class MainPage : global::Windows.UI.Xaml.Controls.Page
    {
        [global::System.CodeDom.Compiler.GeneratedCodeAttribute("Microsoft.Windows.UI.Xaml.Build.Tasks"," 4.0.0.0")]
        private global::Windows.UI.Xaml.Controls.CommandBar AppBar; 
        [global::System.CodeDom.Compiler.GeneratedCodeAttribute("Microsoft.Windows.UI.Xaml.Build.Tasks"," 4.0.0.0")]
        private global::Windows.UI.Xaml.Controls.AppBarButton DefaultView; 
        [global::System.CodeDom.Compiler.GeneratedCodeAttribute("Microsoft.Windows.UI.Xaml.Build.Tasks"," 4.0.0.0")]
        private global::Windows.UI.Xaml.Controls.AppBarButton ToggleCameraFullScreen; 
        [global::System.CodeDom.Compiler.GeneratedCodeAttribute("Microsoft.Windows.UI.Xaml.Build.Tasks"," 4.0.0.0")]
        private global::Windows.UI.Xaml.Controls.AppBarButton ToggleTechFullscreen; 
        [global::System.CodeDom.Compiler.GeneratedCodeAttribute("Microsoft.Windows.UI.Xaml.Build.Tasks"," 4.0.0.0")]
        private global::Windows.UI.Xaml.Controls.Grid MainLayout; 
        [global::System.CodeDom.Compiler.GeneratedCodeAttribute("Microsoft.Windows.UI.Xaml.Build.Tasks"," 4.0.0.0")]
        private global::Windows.UI.Xaml.VisualStateGroup VisualStateGroup; 
        [global::System.CodeDom.Compiler.GeneratedCodeAttribute("Microsoft.Windows.UI.Xaml.Build.Tasks"," 4.0.0.0")]
        private global::Windows.UI.Xaml.VisualState Default; 
        [global::System.CodeDom.Compiler.GeneratedCodeAttribute("Microsoft.Windows.UI.Xaml.Build.Tasks"," 4.0.0.0")]
        private global::Windows.UI.Xaml.VisualState NoSensor; 
        [global::System.CodeDom.Compiler.GeneratedCodeAttribute("Microsoft.Windows.UI.Xaml.Build.Tasks"," 4.0.0.0")]
        private global::Windows.UI.Xaml.VisualState Snapped; 
        [global::System.CodeDom.Compiler.GeneratedCodeAttribute("Microsoft.Windows.UI.Xaml.Build.Tasks"," 4.0.0.0")]
        private global::Windows.UI.Xaml.VisualState CameraFullscreen; 
        [global::System.CodeDom.Compiler.GeneratedCodeAttribute("Microsoft.Windows.UI.Xaml.Build.Tasks"," 4.0.0.0")]
        private global::Windows.UI.Xaml.VisualState TechFullscreen; 
        [global::System.CodeDom.Compiler.GeneratedCodeAttribute("Microsoft.Windows.UI.Xaml.Build.Tasks"," 4.0.0.0")]
        private global::Windows.UI.Xaml.Data.CollectionViewSource ThumbnailsSource; 
        [global::System.CodeDom.Compiler.GeneratedCodeAttribute("Microsoft.Windows.UI.Xaml.Build.Tasks"," 4.0.0.0")]
        private global::Windows.UI.Xaml.Controls.Border CameraPreview; 
        [global::System.CodeDom.Compiler.GeneratedCodeAttribute("Microsoft.Windows.UI.Xaml.Build.Tasks"," 4.0.0.0")]
        private global::Windows.UI.Xaml.Controls.Border TechPreview; 
        [global::System.CodeDom.Compiler.GeneratedCodeAttribute("Microsoft.Windows.UI.Xaml.Build.Tasks"," 4.0.0.0")]
        private global::Windows.UI.Xaml.Controls.GridView Thumbnails; 
        [global::System.CodeDom.Compiler.GeneratedCodeAttribute("Microsoft.Windows.UI.Xaml.Build.Tasks"," 4.0.0.0")]
        private global::Windows.UI.Xaml.Controls.Grid Message; 
        [global::System.CodeDom.Compiler.GeneratedCodeAttribute("Microsoft.Windows.UI.Xaml.Build.Tasks"," 4.0.0.0")]
        private global::Windows.UI.Xaml.Shapes.Rectangle rectangle; 
        [global::System.CodeDom.Compiler.GeneratedCodeAttribute("Microsoft.Windows.UI.Xaml.Build.Tasks"," 4.0.0.0")]
        private global::Windows.UI.Xaml.Controls.TextBlock DisplayMessage; 
        [global::System.CodeDom.Compiler.GeneratedCodeAttribute("Microsoft.Windows.UI.Xaml.Build.Tasks"," 4.0.0.0")]
        private bool _contentLoaded;

        [global::System.CodeDom.Compiler.GeneratedCodeAttribute("Microsoft.Windows.UI.Xaml.Build.Tasks"," 4.0.0.0")]
        [global::System.Diagnostics.DebuggerNonUserCodeAttribute()]
        public void InitializeComponent()
        {
            if (_contentLoaded)
                return;

            _contentLoaded = true;
            global::Windows.UI.Xaml.Application.LoadComponent(this, new global::System.Uri("ms-appx:///MainPage.xaml"), global::Windows.UI.Xaml.Controls.Primitives.ComponentResourceLocation.Application);
 
            AppBar = (global::Windows.UI.Xaml.Controls.CommandBar)this.FindName("AppBar");
            DefaultView = (global::Windows.UI.Xaml.Controls.AppBarButton)this.FindName("DefaultView");
            ToggleCameraFullScreen = (global::Windows.UI.Xaml.Controls.AppBarButton)this.FindName("ToggleCameraFullScreen");
            ToggleTechFullscreen = (global::Windows.UI.Xaml.Controls.AppBarButton)this.FindName("ToggleTechFullscreen");
            MainLayout = (global::Windows.UI.Xaml.Controls.Grid)this.FindName("MainLayout");
            VisualStateGroup = (global::Windows.UI.Xaml.VisualStateGroup)this.FindName("VisualStateGroup");
            Default = (global::Windows.UI.Xaml.VisualState)this.FindName("Default");
            NoSensor = (global::Windows.UI.Xaml.VisualState)this.FindName("NoSensor");
            Snapped = (global::Windows.UI.Xaml.VisualState)this.FindName("Snapped");
            CameraFullscreen = (global::Windows.UI.Xaml.VisualState)this.FindName("CameraFullscreen");
            TechFullscreen = (global::Windows.UI.Xaml.VisualState)this.FindName("TechFullscreen");
            ThumbnailsSource = (global::Windows.UI.Xaml.Data.CollectionViewSource)this.FindName("ThumbnailsSource");
            CameraPreview = (global::Windows.UI.Xaml.Controls.Border)this.FindName("CameraPreview");
            TechPreview = (global::Windows.UI.Xaml.Controls.Border)this.FindName("TechPreview");
            Thumbnails = (global::Windows.UI.Xaml.Controls.GridView)this.FindName("Thumbnails");
            Message = (global::Windows.UI.Xaml.Controls.Grid)this.FindName("Message");
            rectangle = (global::Windows.UI.Xaml.Shapes.Rectangle)this.FindName("rectangle");
            DisplayMessage = (global::Windows.UI.Xaml.Controls.TextBlock)this.FindName("DisplayMessage");
        }
    }
}



