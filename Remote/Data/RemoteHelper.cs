using NAudio.CoreAudioApi;
using System.Runtime.InteropServices;
using System.Text;

namespace MilkwaveRemote {

  public class RemoteHelper {
    public RemoteHelper() {
    }

    [DllImport("kernel32")]
    private static extern int GetPrivateProfileString(
       string section, string key, string defaultValue,
       StringBuilder returnValue, int size, string filePath);

    public string ReadMilkwaveAudioDevice() {
      StringBuilder returnValue = new StringBuilder(256);
      string iniFile = Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "settings.ini");
      if (File.Exists(iniFile)) {
        int result = GetPrivateProfileString("Settings", "MilkwaveAudioDevice", string.Empty, returnValue, 256, iniFile);
      }
      return returnValue.ToString();
    }

    public void FillAudioDevices(ComboBox cbo) {
      cbo.Items.Clear(); // Clear existing items

      MMDevice defaultMDevice;
      string iniMilkwaveAudioDevice = ReadMilkwaveAudioDevice();

      using (var enumerator = new MMDeviceEnumerator()) {
        var devices = enumerator.EnumerateAudioEndPoints(DataFlow.Render, DeviceState.Active);
        foreach (var device in devices) {
          cbo.Items.Add(new ComboBoxItem(device.FriendlyName, device)); // Add device names to ComboBox
        }
        defaultMDevice = enumerator.GetDefaultAudioEndpoint(DataFlow.Render, Role.Multimedia);
      }

      // Sort items alphabetically
      var sortedItems = cbo.Items.Cast<ComboBoxItem>().OrderBy(item => item.Text).ToList();
      cbo.Items.Clear();
      foreach (var item in sortedItems) {
        cbo.Items.Add(item);
      }

      if (cbo.Items.Count > 0) {
        if (defaultMDevice != null) {
          foreach (ComboBoxItem item in cbo.Items) {
            if (iniMilkwaveAudioDevice.Length > 0) {
              if (item.Device.FriendlyName.Equals(iniMilkwaveAudioDevice)) {
                cbo.SelectedItem = item;
                break;
              }
            } else if (item.Device.ID == defaultMDevice.ID) {
              cbo.SelectedItem = item;
              break;
            }
          }
        }
      }
    }

    protected class ComboBoxItem {

      public ComboBoxItem(string text, MMDevice device) {
        Text = text;
        Device = device;
      }

      public string Text { get; set; }
      public MMDevice Device { get; set; }

      public override string ToString() {
        return Text;
      }

    }
  }
}

