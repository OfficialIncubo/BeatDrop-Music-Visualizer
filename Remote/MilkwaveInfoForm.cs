using DarkModeForms;
using System.Diagnostics;
using System.Runtime.InteropServices;

namespace MilkwaveRemote {
  public partial class MilkwaveInfoForm : Form {

    [DllImport("user32.dll")]
    static extern bool HideCaret(IntPtr hWnd);

    public MilkwaveInfoForm(bool darkMode) {
      InitializeComponent();
      dm = new DarkModeCS(this) {
        ColorMode = darkMode ? DarkModeCS.DisplayMode.DarkMode : DarkModeCS.DisplayMode.ClearMode,
      };
    }

    public void ShowDialog(string title, string text) {

      Text = title;
      textBox.Text = text;
      ShowDialog();
    }

    private void MilkwaveInfoForm_Shown(object sender, EventArgs e) {
      btnClose.Focus();
    }

    private void btnClose_Click(object sender, EventArgs e) {
      Close();
    }

    private void textBox_LinkClicked(object sender, LinkClickedEventArgs e) {
      string? url = e.LinkText;
      if (url != null) {
        Process.Start(new ProcessStartInfo(url) { UseShellExecute = true });
      }
    }
  }

}
