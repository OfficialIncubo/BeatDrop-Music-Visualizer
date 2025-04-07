using DarkModeForms;
using System.Diagnostics;
using System.Drawing.Text;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Text;
using System.Text.Json;
using static DarkModeForms.DarkModeCS;

namespace MilkwaveRemote {
  public partial class MilkwaveRemoteForm : Form {
    [DllImport("user32.dll", SetLastError = true)]
    private static extern bool EnumWindows(EnumWindowsProc lpEnumFunc, IntPtr lParam);

    [DllImport("user32.dll", SetLastError = true)]
    private static extern int GetWindowText(IntPtr hWnd, StringBuilder lpString, int nMaxCount);

    [DllImport("user32.dll", SetLastError = true)]
    private static extern int GetWindowTextLength(IntPtr hWnd);

    [DllImport("user32.dll", SetLastError = true, CharSet = CharSet.Unicode)]
    private static extern IntPtr SendMessageW(IntPtr hWnd, uint Msg, IntPtr wParam, ref COPYDATASTRUCT lParam);

    [DllImport("user32.dll", SetLastError = true)]
    private static extern IntPtr PostMessage(IntPtr hWnd, uint Msg, IntPtr wParam, IntPtr lParam);

    [DllImport("user32.dll", SetLastError = true)]
    private static extern uint SendInput(uint nInputs, INPUT[] pInputs, int cbSize);

    [DllImport("user32.dll")]
    private static extern bool SetForegroundWindow(IntPtr hWnd);

    [DllImport("user32.dll")]
    private static extern IntPtr GetForegroundWindow();

    [DllImport("user32.dll", SetLastError = true)]
    private static extern bool GetWindowRect(IntPtr hWnd, out RECT lpRect);

    // Add the missing P/Invoke declaration for SetWindowPos
    [DllImport("user32.dll", SetLastError = true)]
    private static extern bool SetWindowPos(IntPtr hWnd, IntPtr hWndInsertAfter, int X, int Y, int cx, int cy, uint uFlags);

    // Add the missing constants for SWP_NOZORDER and SWP_NOACTIVATE
    private const uint SWP_NOZORDER = 0x0004;
    private const uint SWP_NOACTIVATE = 0x0010;

    private delegate bool EnumWindowsProc(IntPtr hWnd, IntPtr lParam);

    private const uint WM_COPYDATA = 0x004A;
    private const uint WM_KEYDOWN = 0x0100;

    private DarkModeCS dm;

    private System.Windows.Forms.Timer autoplayTimer;
    private int currentLineIndex = 0;
    private float autoplayRemainingBeats = 1;
    private int lastLineIndex = 0;

    private List<string> lines = new List<string>();

    private string lastFileName = "script-default.txt";
    private string windowNotFound = "Milkwave Visualizer window not found";
    private string foundWindowTitle = "";
    private string defaultFontName = "Segoe UI";

    Random rnd = new Random();
    private Settings Settings = new Settings();
    string milkwaveSettingsFile = "settings-milkwave.json";

    [StructLayout(LayoutKind.Sequential)]
    private struct COPYDATASTRUCT {
      public IntPtr dwData;
      public int cbData;
      public IntPtr lpData;
    }

    [StructLayout(LayoutKind.Sequential)]
    private struct INPUT {
      public uint type;
      public InputUnion u;
    }

    [StructLayout(LayoutKind.Explicit)]
    private struct InputUnion {
      [FieldOffset(0)]
      public MOUSEINPUT mi;
      [FieldOffset(0)]
      public KEYBDINPUT ki;
      [FieldOffset(0)]
      public HARDWAREINPUT hi;
    }

    [StructLayout(LayoutKind.Sequential)]
    private struct MOUSEINPUT {
      public int dx;
      public int dy;
      public uint mouseData;
      public uint dwFlags;
      public uint time;
      public IntPtr dwExtraInfo;
    }

    [StructLayout(LayoutKind.Sequential)]
    private struct KEYBDINPUT {
      public ushort wVk;
      public ushort wScan;
      public uint dwFlags;
      public uint time;
      public IntPtr dwExtraInfo;
    }

    [StructLayout(LayoutKind.Sequential)]
    private struct HARDWAREINPUT {
      public uint uMsg;
      public ushort wParamL;
      public ushort wParamH;
    }

    public MilkwaveRemoteForm() {
      InitializeComponent();

      if (FindVisualizerWindow() == IntPtr.Zero) {
        // Try to run MilkwaveVisualizer.exe from the same directory as the assembly
        string visualizerPath = Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "MilkwaveVisualizer.exe");
        if (File.Exists(visualizerPath)) {
          Process.Start(new ProcessStartInfo(visualizerPath) { UseShellExecute = true });
        }
      }

      FixNumericUpDownMouseWheel(this);

      Assembly executingAssembly = Assembly.GetExecutingAssembly();
      var fieVersionInfo = FileVersionInfo.GetVersionInfo(executingAssembly.Location);
      var version = fieVersionInfo.FileVersion;
      barVersion.Text = $"v{version}";

      dm = new DarkModeCS(this) {
        ColorMode = DarkModeCS.DisplayMode.SystemDefault
      };

      cboParameters.DropDownStyle = ComboBoxStyle.DropDown;
      cboParameters.AutoCompleteMode = AutoCompleteMode.SuggestAppend;

      try {
        string jsonString = File.ReadAllText(milkwaveSettingsFile);
        Settings? loadedSettings = JsonSerializer.Deserialize<Settings>(jsonString, new JsonSerializerOptions {
          PropertyNameCaseInsensitive = true
        });
        if (loadedSettings != null) {
          Settings = loadedSettings;
        }
      } catch (Exception ex) {
        Settings = new Settings();
      }

      if (Settings.Presets?.Count > 0) {
        ReloadPresetList();
      } else {
        cboParameters.Text = "size=25|time=5.0|x=0.5|y=0.5|growth=1.5";
      }

      // Fill cboFonts with available system fonts and add a blank first entry
      cboFonts.Items.Add(""); // Add a blank first entry
      using (InstalledFontCollection fontsCollection = new InstalledFontCollection()) {
        foreach (FontFamily font in fontsCollection.Families) {
          cboFonts.Items.Add(font.Name);
        }
        if (cboFonts.Items.Contains(defaultFontName)) {
          cboFonts.SelectedItem = defaultFontName;
        }
      }

      LoadMessages(lastFileName);

      autoplayTimer = new System.Windows.Forms.Timer();
      autoplayTimer.Tick += AutoplayTimer_Tick;

      Location = Settings.RemoteWindowLocation;
      Size = Settings.RemoteWindowSize;

      int maxWait = 30; // 3 seconds
      while (FindVisualizerWindow() == IntPtr.Zero && maxWait > 0) {
        // Wait for the visualizer window to be found
        Thread.Sleep(100);
        maxWait--;
      }
      SetVisualizerWindowSizeAndPosition();
    }

    private void SetVisualizerWindowSizeAndPosition() {

      Point VisualizerLocation = Settings.VisualizerWindowLocation;
      Size VisualizerSize = Settings.VisualizerWindowSize;

      if (VisualizerSize.Height > 0 && VisualizerSize.Width > 0) {
        IntPtr foundWindow = FindVisualizerWindow();
        if (foundWindow != IntPtr.Zero) {
          SetWindowPos(foundWindow, IntPtr.Zero, VisualizerLocation.X, VisualizerLocation.Y, VisualizerSize.Width, VisualizerSize.Height, SWP_NOZORDER | SWP_NOACTIVATE);
        }
      }
    }

    private nint FindVisualizerWindow() {
      IntPtr foundWindow = IntPtr.Zero;
      EnumWindows((hWnd, lParam) => {
        int length = GetWindowTextLength(hWnd);
        if (length == 0) return true;

        StringBuilder windowTitle = new StringBuilder(length + 1);
        GetWindowText(hWnd, windowTitle, windowTitle.Capacity);

        if (windowTitle.ToString().Contains(txtWindowTitle.Text)) {
          foundWindow = hWnd;
          foundWindowTitle = windowTitle.ToString();
          return false; // Stop enumeration
        }

        return true; // Continue enumeration
      }, IntPtr.Zero);
      return foundWindow;
    }

    private void btnSend_Click(object sender, EventArgs e) {
      SendToMilkwaveVisualizer(txtMessage.Text);
      txtMessage.Focus();
      txtMessage.SelectAll();
    }

    private void SendToMilkwaveVisualizer(string messageToSend) {
      statusBar.Text = "";
      string partialTitle = txtWindowTitle.Text;

      IntPtr foundWindow = FindVisualizerWindow();
      if (foundWindow != IntPtr.Zero) {
        string message = "MSG|text=" + messageToSend;
        if (cboParameters.Text.Length > 0) {
          message += "|" + cboParameters.Text;
        }
        if (!message.Contains("font=")) {
          message += "|font=" + cboFonts.Text;
        }
        if (!message.Contains("r=") && !message.Contains("g=") && !message.Contains("b=")) {
          message += "|r=" + pnlColor.BackColor.R;
          message += "|g=" + pnlColor.BackColor.G;
          message += "|b=" + pnlColor.BackColor.B;
        }
        if (!message.Contains("size=")) {
          message += "|size=" + txtSize.Value;
        }
        byte[] messageBytes = Encoding.Unicode.GetBytes(message);
        IntPtr messagePtr = Marshal.AllocHGlobal(messageBytes.Length);
        Marshal.Copy(messageBytes, 0, messagePtr, messageBytes.Length);

        COPYDATASTRUCT cds = new COPYDATASTRUCT {
          dwData = 1,
          cbData = messageBytes.Length,
          lpData = messagePtr
        };

        SendMessageW(foundWindow, WM_COPYDATA, IntPtr.Zero, ref cds);
        statusBar.Text = ($"Sent '{messageToSend}' to '{foundWindowTitle}'");

        Marshal.FreeHGlobal(messagePtr);

      } else {
        statusBar.Text = windowNotFound;
      }
    }

    private void MainForm_Shown(object sender, EventArgs e) {
      txtMessage.Focus();
      txtMessage.SelectAll();
      pnlColor.BackColor = Color.FromArgb(230, 0, 120);
      colorDialog1.Color = pnlColor.BackColor;
      if (cboParameters.Items.Count > 0) {
        cboParameters.SelectedIndex = 0;
      }
      SetFormattedMessage();
    }

    private void btnSaveParam_Click(object sender, EventArgs e) {
      if (txtPreset.Text.Length == 0) {
        txtPreset.Text = "Preset A";
      }

      var newPreset = new Preset {
        Name = txtPreset.Text,
        Value = cboParameters.Text
      };

      int index = Settings.Presets.FindIndex(item => item.Name == newPreset.Name);
      if (index >= 0) {
        Settings.Presets[index] = newPreset;
      } else {
        Settings.Presets.Add(newPreset);
      }

      ReloadPresetList();

      statusBar.Text = $"Saved preset '{txtPreset.Text}'";
    }

    private void chkAutoplay_CheckedChanged(object sender, EventArgs e) {
      if (chkAutoplay.Checked) {
        ResetAndStartTimer(true);
      } else {
        autoplayTimer.Stop();
        statusBar.Text = "";
        autoplayRemainingBeats = 0;
      }
    }

    private void ResetAndStartTimer(bool startInstant) {
      if (float.TryParse(txtBeats.Text, out float interval)) {
        autoplayRemainingBeats = 0;
        setTimerInterval();
        if (startInstant) {
          AutoplayTimer_Tick(null, null);
        }
        autoplayTimer.Start();
        start = DateTime.Now.Ticks;
      } else {
        statusBar.Text = "Invalid wait value";
      }
    }

    long start;

    private void setTimerInterval() {
      if (autoplayTimer != null) {
        float bpm = 120;
        try {
          bpm = float.Parse(txtBPM.Text);
        } catch (Exception) {
          bpm = 120;
        }
        autoplayTimer.Interval = (int)((float)60 / bpm * 1000) - 15; // Timer inaccuracy compensation
      }
    }

    private void AutoplayTimer_Tick(object? sender, EventArgs? e) {

      Debug.Print("" + ((start - DateTime.Now.Ticks) / 1000));

      if (lines.Count > 0) {
        if (autoplayRemainingBeats == 0) {
          string[] strings = lines[currentLineIndex].Split('|');
          foreach (string s in strings) {
            string token = s.Trim();
            string tokenUpper = token.ToUpper();
            if (tokenUpper.Equals("NEXT")) {
              btnSpace.PerformClick();
            } else if (tokenUpper.Equals("PREV")) {
              btnBackspace.PerformClick();
            } else if (tokenUpper.Equals("STOP")) {
              chkAutoplay.CheckState = CheckState.Unchecked;
            } else if (tokenUpper.Equals("RESET")) {
              ResetAndStartTimer(false);
            } else if (tokenUpper.StartsWith("BPM=")) {
              string BPM = tokenUpper.Substring(4);
              if (int.TryParse(BPM, out int bpm)) {
                txtBPM.Text = BPM;
              }
            } else if (tokenUpper.StartsWith("BEATS=")) {
              string beats = tokenUpper.Substring(6);
              if (int.TryParse(beats, out int b)) {
                txtBeats.Text = beats;
              }
            } else if (tokenUpper.StartsWith("FONT=")) {
              string font = token.Substring(5);
              cboFonts.Text = font;
            } else if (tokenUpper.StartsWith("PRESET=")) {
              string preset = tokenUpper.Substring(7);
              var foundItem = from item in cboParameters.Items.Cast<Preset>()
                              where item.Name.ToUpper() == preset
                              select item;
              if (foundItem != null && foundItem.Any()) {
                cboParameters.SelectedItem = foundItem.First();
              } else {
                statusBar.Text = $"Preset '{preset}' not found";
              }
            } else if (tokenUpper.StartsWith("SEND=")) {
              string sendString = token.Substring(5);
              if (sendString.Length > 0) {
                SendUnicodeChars(sendString);
              }
            } else { // no known command, send as message
              SendToMilkwaveVisualizer(token);
            }
          }


          try {
            autoplayRemainingBeats = int.Parse(txtBeats.Text) - 1;
          } catch (Exception) {
            autoplayRemainingBeats = 1;
          }

          if (autoplayRemainingBeats < 1) {
            autoplayRemainingBeats = 1;
          }
          lastLineIndex = currentLineIndex;

          if (chkFileRandom.Checked && lines.Count > 1) {
            while (currentLineIndex == lastLineIndex) {
              currentLineIndex = rnd.Next(0, lines.Count);
            }
          } else {
            currentLineIndex = (currentLineIndex + 1) % lines.Count;
          }
        } else {
          txtAutoplay.Text = lines[currentLineIndex];
          statusBar.Text = $"Next line in {autoplayRemainingBeats} beats";
          autoplayRemainingBeats--;
        }
      }
    }

    private void btnFontAppend_Click(object sender, EventArgs e) {
      RemoveParam("font");
      AppendParam("font=" + cboFonts.Text);
    }

    private void pnlColor_Click(object sender, EventArgs e) {
      if (colorDialog1.ShowDialog() == DialogResult.OK) {
        pnlColor.BackColor = colorDialog1.Color;
        SetFormattedMessage();
      }
    }

    private void btnAppendColor_Click(object sender, EventArgs e) {
      RemoveParam("r");
      RemoveParam("g");
      RemoveParam("b");
      AppendParam("r=" + pnlColor.BackColor.R);
      AppendParam("g=" + pnlColor.BackColor.G);
      AppendParam("b=" + pnlColor.BackColor.B);
    }

    private void LoadMessages(string fileName) {
      currentLineIndex = 0;
      lines.Clear();
      string filePath = fileName;
      if (!fileName.Contains("\\")) {
        filePath = Path.Combine(AppDomain.CurrentDomain.BaseDirectory, fileName);
      }
      if (File.Exists(filePath)) {
        string[] strings = File.ReadAllLines(filePath);
        foreach (string line in strings) {
          if (line.Length > 0 && !line.StartsWith("#")) {
            lines.Add(line);
          }
        }
      }

      if (lines?.Count > 0) {
        if (lines.Count > 1 && chkFileRandom.Checked) {
          currentLineIndex = rnd.Next(0, lines.Count);
        }
        txtAutoplay.Text = lines[currentLineIndex];
        toolTip1.SetToolTip(txtAutoplay, "Next line from file " + fileName);
        chkAutoplay.Enabled = true;
      } else {
        txtAutoplay.Text = "No messages in " + fileName;
        chkAutoplay.Enabled = false;
      }
    }

    private void SendPostMessage(int VKKey, string keyName) {
      IntPtr foundWindow = FindVisualizerWindow();

      if (foundWindow != IntPtr.Zero) {
        PostMessage(foundWindow, WM_KEYDOWN, (IntPtr)VKKey, IntPtr.Zero);
        statusBar.Text = $"Pressed {keyName} in '{foundWindowTitle}'";
      } else {
        statusBar.Text = windowNotFound;
      }
    }

    private const int VK_SHIFT = 0x10;
    private const int VK_ALT = 0x12;

    private void SendInput(int VKKey, string keyName, bool doShift, bool doAlt) {
      IntPtr currentWindow = GetForegroundWindow();
      IntPtr foundWindow = FindVisualizerWindow();
      if (foundWindow != IntPtr.Zero) {
        SetForegroundWindow(foundWindow);

        INPUT[] inputs;

        if (doShift) {
          inputs = new INPUT[4];

          inputs[0] = new INPUT {
            type = 1, // Keyboard input
            u = new InputUnion {
              ki = new KEYBDINPUT {
                wVk = VK_SHIFT,
                dwFlags = 0 // Key down
              }
            }
          };

          inputs[1] = new INPUT {
            type = 1, // Keyboard input
            u = new InputUnion {
              ki = new KEYBDINPUT {
                wVk = (ushort)VKKey,
                dwFlags = 0 // Key down
              }
            }
          };

          inputs[2] = new INPUT {
            type = 1, // Keyboard input
            u = new InputUnion {
              ki = new KEYBDINPUT {
                wVk = (ushort)VKKey,
                dwFlags = 2 // Key up
              }
            }
          };

          inputs[3] = new INPUT {
            type = 1, // Keyboard input
            u = new InputUnion {
              ki = new KEYBDINPUT {
                wVk = VK_SHIFT,
                dwFlags = 2 // Key up
              }
            }
          };
        } else if (doAlt) {
          inputs = new INPUT[4];

          inputs[0] = new INPUT {
            type = 1, // Keyboard input
            u = new InputUnion {
              ki = new KEYBDINPUT {
                wVk = VK_ALT,
                dwFlags = 0 // Key down
              }
            }
          };

          inputs[1] = new INPUT {
            type = 1, // Keyboard input
            u = new InputUnion {
              ki = new KEYBDINPUT {
                wVk = (ushort)VKKey,
                dwFlags = 0 // Key down
              }
            }
          };

          inputs[2] = new INPUT {
            type = 1, // Keyboard input
            u = new InputUnion {
              ki = new KEYBDINPUT {
                wVk = (ushort)VKKey,
                dwFlags = 2 // Key up
              }
            }
          };

          inputs[3] = new INPUT {
            type = 1, // Keyboard input
            u = new InputUnion {
              ki = new KEYBDINPUT {
                wVk = VK_ALT,
                dwFlags = 2 // Key up
              }
            }
          };
        } else {
          inputs = new INPUT[2];

          inputs[0] = new INPUT {
            type = 1, // Keyboard input
            u = new InputUnion {
              ki = new KEYBDINPUT {
                wVk = (ushort)VKKey,
                dwFlags = 0 // Key down
              }
            }
          };

          inputs[1] = new INPUT {
            type = 1, // Keyboard input
            u = new InputUnion {
              ki = new KEYBDINPUT {
                wVk = (ushort)VKKey,
                dwFlags = 2 // Key up
              }
            }
          };
        }

        SendInput((uint)inputs.Length, inputs, Marshal.SizeOf(typeof(INPUT)));
        statusBar.Text = $"Pressed {keyName} in '{foundWindowTitle}'";

        SetForegroundWindow(currentWindow);
      } else {
        statusBar.Text = windowNotFound;
      }
    }

    private const int VK_F3 = 0x72;

    private void btnF3_Click(object sender, EventArgs e) {
      SendPostMessage(VK_F3, "F3");
    }

    private const int VK_F4 = 0x73;

    private void btnF4_Click(object sender, EventArgs e) {
      SendPostMessage(VK_F4, "F4");
    }

    private const int VK_F7 = 0x76;

    private void btnF7_Click(object sender, EventArgs e) {
      SendPostMessage(VK_F7, "F7");
    }

    private const int VK_SPACE = 0x20;

    private void btnSpace_Click(object sender, EventArgs e) {
      SendPostMessage(VK_SPACE, "Space");
    }

    private const int VK_BACKSPACE = 0x08;

    private void btnBackspace_Click(object sender, EventArgs e) {
      SendPostMessage(VK_BACKSPACE, "Backspace");
    }

    private void SendUnicodeChars(string inputString) {
      IntPtr currentWindow = GetForegroundWindow();
      IntPtr foundWindow = FindVisualizerWindow();

      if (foundWindow != IntPtr.Zero) {
        SetForegroundWindow(foundWindow);

        for (int i = 0; i < inputString.Length; i++) {
          INPUT[] inputs = new INPUT[1];
          inputs[0] = new INPUT {
            type = 1, // Keyboard input
            u = new InputUnion {
              ki = new KEYBDINPUT {
                wVk = 0,
                wScan = (ushort)inputString[i],
                dwFlags = 4, // KEYEVENTF_UNICODE
                time = 0,
                dwExtraInfo = IntPtr.Zero
              }
            }
          };
          SendInput((uint)inputs.Length, inputs, Marshal.SizeOf(typeof(INPUT)));
          Thread.Sleep(50);
        }

        statusBar.Text = $"Pressed {inputString.ToUpper()} in '{foundWindowTitle}'";

        SetForegroundWindow(currentWindow);

      } else {
        statusBar.Text = windowNotFound;
      }
    }

    private void btnTilde_Click(object sender, EventArgs e) {
      SendUnicodeChars("~");
    }

    private const int VK_DELETE = 0x2E;

    private void btnDelete_Click(object sender, EventArgs e) {
      SendPostMessage(VK_DELETE, "Delete");
    }

    private const int VK_ENTER = 0x0D;

    private void btnAltEnter_Click(object sender, EventArgs e) {
      SendInput(VK_ENTER, "Alt+Enter", false, true);
    }

    private void btnN_Click(object sender, EventArgs e) {
      SendUnicodeChars("n");
    }

    private const int VK_F2 = 0x71;

    private void btnF2_Click(object sender, EventArgs e) {
      SendPostMessage(VK_F2, "F2");
    }

    private void btnK_Click(object sender, EventArgs e) {
      SendUnicodeChars("k");
    }

    private void btnY_Click(object sender, EventArgs e) {
      SendUnicodeChars("y");
    }

    private void btn00_Click(object sender, EventArgs e) {
      SendUnicodeChars("00");
    }

    private void btn11_Click(object sender, EventArgs e) {
      SendUnicodeChars("11");
    }

    private void btnm22_Click(object sender, EventArgs e) {
      SendUnicodeChars("22");
    }

    private void btn33_Click(object sender, EventArgs e) {
      SendUnicodeChars("33");
    }

    private void btn44_Click(object sender, EventArgs e) {
      SendUnicodeChars("44");
    }

    private void btn55_Click(object sender, EventArgs e) {
      SendUnicodeChars("55");
    }

    private void btn66_Click(object sender, EventArgs e) {
      SendUnicodeChars("66");
    }

    private void btn77_Click(object sender, EventArgs e) {
      SendUnicodeChars("77");
    }

    private void lblFromFile_DoubleClick(object sender, EventArgs e) {
      LoadMessages(lastFileName);
    }

    private void MainForm_KeyDown(object sender, KeyEventArgs e) {
      if (e.KeyCode == Keys.F1) {
        txtMessage.Focus();
        txtMessage.SelectAll();
      } else if (e.KeyCode == Keys.F2) {
        SendToMilkwaveVisualizer(txtMessage.Text);
      } else if (e.KeyCode == Keys.F3) {
        btnSpace.PerformClick();
      } else if (e.KeyCode == Keys.F4) {
        btnBackspace.PerformClick();
      } else if (e.KeyCode == Keys.F12) {
        chkAutoplay.Checked = !chkAutoplay.Checked;
      }
    }

    private void txtBPM_TextChanged(object sender, EventArgs e) {
      setTimerInterval();
    }

    private void lblBPM(object sender, EventArgs e) {
      ResetAndStartTimer(true);
    }

    private void cboParameters_SelectedIndexChanged(object sender, EventArgs e) {
      if (cboParameters.SelectedItem is Preset selectedPreset) {
        txtPreset.Text = selectedPreset.Name;
        BeginInvoke(new Action(() => cboParameters.Text = selectedPreset.Value));
      }
    }

    private void lblParameters_DoubleClick(object sender, EventArgs e) {
      Settings.Presets.Clear();
      ReloadPresetList();
      statusBar.Text = $"Saved presets cleared";
    }

    private void lblPreset_DoubleClick(object sender, EventArgs e) {
      var foundItem = from item in Settings.Presets
                      where item.Name == txtPreset.Text
                      select item;
      if (foundItem != null && foundItem.Any()) {
        Settings.Presets.Remove(foundItem.First());
        ReloadPresetList();
      }
    }

    private void ReloadPresetList() {
      cboParameters.Items.Clear();
      cboParameters.Items.AddRange(Settings.Presets.OrderBy(x => x.Name).ToArray());
      cboParameters.Refresh();
    }

    private void txtPreset_KeyDown(object sender, KeyEventArgs e) {
      if (e.KeyCode == Keys.Enter) {
        e.SuppressKeyPress = true; // Prevent the beep sound on Enter key press
        btnSaveParam.PerformClick();
      }
    }

    private void MainForm_FormClosing(object sender, FormClosingEventArgs e) {

      if (WindowState == FormWindowState.Normal) {
        Settings.RemoteWindowLocation = Location;
        Settings.RemoteWindowSize = Size;
      } else {
        Settings.RemoteWindowLocation = RestoreBounds.Location;
        Settings.RemoteWindowSize = RestoreBounds.Size;
      }

      IntPtr foundWindow = FindVisualizerWindow();
      if (foundWindow != IntPtr.Zero) {
        RECT savedWindowRect;
        GetWindowRect(foundWindow, out savedWindowRect);

        Settings.VisualizerWindowLocation = new Point(savedWindowRect.Left, savedWindowRect.Top);
        Settings.VisualizerWindowSize = new Size(savedWindowRect.Right - savedWindowRect.Left, savedWindowRect.Bottom - savedWindowRect.Top);

        // Close the Visualizer window
        PostMessage(foundWindow, 0x0010, IntPtr.Zero, IntPtr.Zero); // WM_CLOSE message
      }

      // Hold the Ctrl key while closing the form to reset local settings to default
      if ((Control.ModifierKeys & Keys.Control) == Keys.Control) {
        Settings = new Settings();
      }

      string jsonString = JsonSerializer.Serialize(Settings, new JsonSerializerOptions { WriteIndented = true });
      File.WriteAllText(milkwaveSettingsFile, jsonString);
    }

    private void cboParameters_KeyDown(object sender, KeyEventArgs e) {
      if (e.KeyCode == Keys.Enter) {
        e.SuppressKeyPress = true; // Prevent the beep sound on Enter key press
        btnSaveParam.PerformClick();
      }
    }

    private void txtMessage_KeyDown(object sender, KeyEventArgs e) {
      if (e.KeyCode == Keys.Enter) {
        e.SuppressKeyPress = true; // Prevent the beep sound on Enter key press
        btnSend.PerformClick();
      }
    }

    private void lblFont_DoubleClick(object sender, EventArgs e) {
      RemoveParam("font");
    }

    private void RemoveParam(string param) {
      try {
        int rIndex = cboParameters.Text.IndexOf("|" + param + "=", StringComparison.CurrentCultureIgnoreCase);
        if (rIndex == -1) {
          rIndex = cboParameters.Text.IndexOf(param + "=", StringComparison.CurrentCultureIgnoreCase);
        }
        if (rIndex > -1) {
          int rIndex2 = cboParameters.Text.IndexOf("|", rIndex + 1);
          if (rIndex2 > -1) {
            cboParameters.Text = cboParameters.Text.Remove(rIndex, rIndex2 - rIndex + 1);
          } else {
            cboParameters.Text = cboParameters.Text.Remove(rIndex);
          }
        }
      } catch (Exception ex) {
        statusBar.Text = "Error: " + ex.Message.Replace(Environment.NewLine, " ");
      }
    }

    private string GetParam(string param) {
      string result = "";
      try {
        int rIndex = cboParameters.Text.IndexOf("|" + param + "=", StringComparison.CurrentCultureIgnoreCase);
        if (rIndex == -1) {
          rIndex = cboParameters.Text.IndexOf(param + "=", StringComparison.CurrentCultureIgnoreCase);
        }
        if (rIndex > -1) {
          int rIndex2 = cboParameters.Text.IndexOf("|", rIndex + 1);
          if (rIndex2 > -1) {
            result = cboParameters.Text.Substring(rIndex, rIndex2 - rIndex);
          } else {
            result = cboParameters.Text.Substring(rIndex);
          }
        }
        if (result.Length > 0) {
          result = result.Substring(result.IndexOf("=") + 1);
        }
      } catch (Exception ex) {
        statusBar.Text = "Error: " + ex.Message.Replace(Environment.NewLine, " ");
      }
      return result;
    }

    private void lblColor_DoubleClick(object sender, EventArgs e) {
      RemoveParam("r");
      RemoveParam("g");
      RemoveParam("b");
    }

    private void lblSize_DoubleClick(object sender, EventArgs e) {
      RemoveParam("size");
    }

    private void btnAppendSize_Click(object sender, EventArgs e) {
      RemoveParam("size");
      AppendParam("size=" + txtSize.Text);
    }

    private void AppendParam(string param) {
      if (cboParameters.Text.Length > 0) {
        cboParameters.Text += "|";
      }
      cboParameters.Text += param;
    }

    private void txtSize_KeyDown(object sender, KeyEventArgs e) {
      if (e.KeyCode == Keys.Enter) {
        e.SuppressKeyPress = true; // Prevent the beep sound on Enter key press
        btnAppendSize.PerformClick();
      }
    }

    private void cboFonts_KeyDown(object sender, KeyEventArgs e) {
      if (e.KeyCode == Keys.Enter) {
        e.SuppressKeyPress = true; // Prevent the beep sound on Enter key press
        btnFontAppend.PerformClick();
      }
    }

    private void SetFormattedMessage() {
      if (!chkPreview.Checked) {
        return;
      }
      try {
        string fontName = GetParam("font");
        if (fontName.Length == 0) {
          fontName = cboFonts.Text;
        }

        Color fontColor;
        string colorR = GetParam("r");
        string colorG = GetParam("g");
        string colorB = GetParam("b");
        if (colorR.Length == 0 || colorG.Length == 0 || colorB.Length == 0) {
          fontColor = pnlColor.BackColor;
        } else {
          fontColor = Color.FromArgb(int.Parse(colorR), int.Parse(colorG), int.Parse(colorB));
        }

        int fontSize;
        string size = GetParam("size");
        if (size.Length == 0 || !int.TryParse(size, out fontSize)) {
          fontSize = int.Parse(txtSize.Text);
        }

        FontStyle style = cboParameters.Text.ToUpper().Contains("bold=1") ? FontStyle.Bold : FontStyle.Regular;
        txtMessage.Font = new Font(fontName, fontSize, style);
        txtMessage.ForeColor = fontColor;

        txtMessage.Refresh();
      } catch (Exception e) {
        // ignore
      }
    }


    private void txtSize_TextChanged(object sender, EventArgs e) {
      SetFormattedMessage();
    }

    private void cboFonts_SelectedIndexChanged(object sender, EventArgs e) {
      SetFormattedMessage();
    }

    private void cboParameters_TextChanged(object sender, EventArgs e) {
      SetFormattedMessage();
    }

    private void chkPreview_CheckedChanged(object sender, EventArgs e) {
      if (chkPreview.Checked) {
        SetFormattedMessage();
      } else {
        txtMessage.Font = txtAutoplay.Font;
        txtMessage.ForeColor = txtAutoplay.ForeColor;
      }
    }

    static void FixNumericUpDownMouseWheel(Control c) {
      foreach (var num in c.Controls.OfType<NumericUpDown>())
        num.MouseWheel += FixNumericUpDownMouseWheelHandler;

      foreach (var child in c.Controls.OfType<Control>())
        FixNumericUpDownMouseWheel(child);
    }

    static private void FixNumericUpDownMouseWheelHandler(object? sender, MouseEventArgs e) {
      ((HandledMouseEventArgs)e).Handled = true;
      var self = ((NumericUpDown)sender!);
      self.Value = Math.Max(Math.Min(
          self.Value + ((e.Delta > 0) ? self.Increment : -self.Increment), self.Maximum), self.Minimum);
    }

    private void lblFromFile_MouseClick(object sender, MouseEventArgs e) {
      if (e.Button == MouseButtons.Right) {
        OpenFileDialog ofd = new OpenFileDialog();
        ofd.Filter = "Text files (*.txt)|*.txt|All files (*.*)|*.*";
        ofd.InitialDirectory = AppDomain.CurrentDomain.BaseDirectory;
        if (ofd.ShowDialog() == DialogResult.OK) {
          lastFileName = ofd.FileName;
          LoadMessages(lastFileName);
        }
      }
    }

    private void lblPreset_MouseClick(object sender, MouseEventArgs e) {
      if (e.Button == MouseButtons.Right) {
        string fontName = GetParam("font");
        if (fontName.Length > 0) {
          cboFonts.Text = fontName;
        }

        string colorR = GetParam("r");
        string colorG = GetParam("g");
        string colorB = GetParam("b");
        if (colorR.Length > 0 && colorG.Length > 0 && colorB.Length > 0) {
          pnlColor.BackColor = Color.FromArgb(int.Parse(colorR), int.Parse(colorG), int.Parse(colorB));
        }

        int fontSize;
        string size = GetParam("size");
        if (size.Length > 0 && int.TryParse(size, out fontSize)) {
          txtSize.Value = fontSize;
        }
      }
    }

    private void lblWindow_DoubleClick(object sender, EventArgs e) {
      SetVisualizerWindowSizeAndPosition();
    }

    private void statusHelp_Click(object sender, EventArgs e) {
      string dialogtext =
        "There are many tooltips explaining all features when you hover over the form elements." + Environment.NewLine +
        "" + Environment.NewLine +
        "More help resources:" + Environment.NewLine +
        "" + Environment.NewLine +
        "GitHub homepage: https://github.com/IkeC/Milkwave" + Environment.NewLine +
        "GitHub issues: https://github.com/IkeC/Milkwave/issues" + Environment.NewLine +
        "Ikes Discord: https://bit.ly/Ikes-Discord" + Environment.NewLine +
        "README.txt in the program folder";
      new MilkwaveInfoForm().ShowDialog("Milkwave Help", dialogtext);
    }

    private void statusSupporters_Click(object sender, EventArgs e) {
      string dialogtext =
        "Such emptiness..." + Environment.NewLine +
        "Your name could be here!" + Environment.NewLine +
        "" + Environment.NewLine +
        "Support my work: https://www.paypal.com/ncp/payment/5XMP3S69PJLCU" + Environment.NewLine +
        "" + Environment.NewLine +
        "Any amount appreciated. Thank you very much!";
      new MilkwaveInfoForm().ShowDialog("Milkwave Supporters", dialogtext);
    }

    private void barVersion_Click(object sender, EventArgs e) {
      string url = "https://github.com/IkeC/Milkwave/releases";
      Process.Start(new ProcessStartInfo(url) { UseShellExecute = true });
    }
  }
}