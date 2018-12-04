using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.Diagnostics;

namespace Launcher
{
    /// <summary>
    /// Логика взаимодействия для MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        private int sampleCount = 1;

        private int shadowsDim = 1024;

        private int shadowsDistance = 10;

        private int ssaoEnabled = 0;

        private string lightingScheme = "Noon";

        private int mercedesModels = 0;

        private int supercarModels = 0;

        private int houseModels = 0;

        private int treeModels = 0;

        public MainWindow()
        {
            InitializeComponent();
        }

        private void Button_Click(object sender, RoutedEventArgs e)
        {
            string arguments = sampleCount + " "
                + shadowsDim + " "
                + shadowsDistance + " "
                + ssaoEnabled + " "
                + lightingScheme + " "
                + mercedesModels + " "
                + supercarModels + " "
                + houseModels + " "
                + treeModels;

            Process vulkanScene = new Process();

            vulkanScene.StartInfo = new ProcessStartInfo("VulkanScene.exe", arguments);
            vulkanScene.StartInfo.RedirectStandardOutput = true;
            vulkanScene.StartInfo.UseShellExecute = false;

            vulkanScene.OutputDataReceived += (s, eventArgs) =>
            {
                if (!string.IsNullOrEmpty(eventArgs.Data))
                {
                    MessageBox.Show(eventArgs.Data);
                }
            };

            vulkanScene.Start();

            vulkanScene.BeginOutputReadLine();
        }

        private void AntiAliasing_Checked(object sender, RoutedEventArgs e)
        {
            var radioButton = sender as RadioButton;
            if (radioButton == null)
            {
                return;
            }

            string content = radioButton.Content.ToString();

            if (content == "No")
            {
                sampleCount = 1;
            }
            else
            {
                sampleCount = int.Parse(content[0].ToString());
            }
        }

        private int getFromQuality(string quality, int low, int medium, int height, int ultra)
        {
            switch (quality)
            {
                case "Low":
                    return low;
                case "Medium":
                    return medium;
                case "Height":
                    return height;
                case "Ultra":
                    return ultra;
                default:
                    throw new ArgumentOutOfRangeException("quality");
            }
        }

        private void ShadowsQuality_Checked(object sender, RoutedEventArgs e)
        {
            var radioButton = sender as RadioButton;
            if (radioButton == null)
            {
                return;
            }

            string content = radioButton.Content.ToString();

            shadowsDim = getFromQuality(content, 1024, 2048, 4096, 8192);
        }

        private void ShadowsDistance_Checked(object sender, RoutedEventArgs e)
        {
            var radioButton = sender as RadioButton;
            if (radioButton == null)
            {
                return;
            }

            string content = radioButton.Content.ToString();

            shadowsDistance = getFromQuality(content, 10, 20, 40, 60);
        }

        private void AmbientOcclusion_Checked(object sender, RoutedEventArgs e)
        {
            var radioButton = sender as RadioButton;
            if (radioButton == null)
            {
                return;
            }

            string content = radioButton.Content.ToString();

            ssaoEnabled = Convert.ToInt32(content == "SSAO");
        }

        private void LightingScheme_Checked(object sender, RoutedEventArgs e)
        {

            var radioButton = sender as RadioButton;
            if (radioButton == null)
            {
                return;
            }

            lightingScheme = radioButton.Content.ToString();
        }


        private void MercedesModels_Checked(object sender, RoutedEventArgs e)
        {
            mercedesModels = 1;
        }
        private void MercedesModels_Unchecked(object sender, RoutedEventArgs e)
        {
            mercedesModels = 0;
        }

        private void SupercarModels_Checked(object sender, RoutedEventArgs e)
        {
            supercarModels = 1;
        }
        private void SupercarModels_Unchecked(object sender, RoutedEventArgs e)
        {
            supercarModels = 0;
        }

        private void HouseModels_Checked(object sender, RoutedEventArgs e)
        {
            houseModels = 1;
        }
        private void HouseModels_Unchecked(object sender, RoutedEventArgs e)
        {
            houseModels = 0;
        }

        private void TreeModels_Checked(object sender, RoutedEventArgs e)
        {
            treeModels = 1;
        }
        private void TreeModels_Unchecked(object sender, RoutedEventArgs e)
        {
            treeModels = 0;
        }
    }
}
