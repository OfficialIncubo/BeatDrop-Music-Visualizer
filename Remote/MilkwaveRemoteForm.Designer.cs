using System.Windows.Forms;

namespace MilkwaveRemote
{
    partial class MilkwaveRemoteForm
    {
        /// <summary>
        ///  Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        ///  Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            } 
            base.Dispose(disposing);
        }

    #region Windows Form Designer generated code

    /// <summary>
    ///  Required method for Designer support - do not modify
    ///  the contents of this method with the code editor.
    /// </summary>
    private void InitializeComponent() {
      components = new System.ComponentModel.Container();
      System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(MilkwaveRemoteForm));
      statusStrip1 = new StatusStrip();
      statusBar = new ToolStripStatusLabel();
      toolStripDropDownButton = new ToolStripDropDownButton();
      toolStripMenuItemHelp = new ToolStripMenuItem();
      toolStripMenuItemHomepage = new ToolStripMenuItem();
      toolStripMenuItemSupporters = new ToolStripMenuItem();
      toolStripSeparator1 = new ToolStripSeparator();
      toolStripMenuItemDarkMode = new ToolStripMenuItem();
      toolStripSeparator2 = new ToolStripSeparator();
      toolStripMenuItemMessagePanel = new ToolStripMenuItem();
      toolStripMenuItemPresetPanel = new ToolStripMenuItem();
      toolStripMenuItemButtonPanel = new ToolStripMenuItem();
      toolTip1 = new ToolTip(components);
      chkPreview = new CheckBox();
      btnAppendSize = new Button();
      numSize = new NumericUpDown();
      lblSize = new Label();
      lblStyle = new Label();
      numBPM = new NumericUpDown();
      lblBPM = new Label();
      chkFileRandom = new CheckBox();
      pnlColorMessage = new Panel();
      btnAppendColor = new Button();
      lblColor = new Label();
      btnFontAppend = new Button();
      lblFont = new Label();
      cboFonts = new ComboBox();
      numBeats = new NumericUpDown();
      label7 = new Label();
      lblFromFile = new Label();
      chkAutoplay = new CheckBox();
      txtAutoplay = new TextBox();
      btnSaveParam = new Button();
      lblParameters = new Label();
      cboParameters = new ComboBox();
      lblWindow = new Label();
      txtWindowTitle = new TextBox();
      btnSend = new Button();
      txtMessage = new TextBox();
      btnSpace = new Button();
      btnBackspace = new Button();
      lblPreset = new Label();
      btnPresetLoadFile = new Button();
      btnPresetSend = new Button();
      btnPresetLoadDirectory = new Button();
      txtVisRunning = new TextBox();
      cboPresets = new ComboBox();
      txtDirFilter = new TextBox();
      numAmpLeft = new NumericUpDown();
      lblAmp = new Label();
      lblAmpRight = new Label();
      numAmpRight = new NumericUpDown();
      numWavemode = new NumericUpDown();
      btnSendWave = new Button();
      lblWaveColor = new Label();
      lblWavemode = new Label();
      pnlColorWave = new Panel();
      lblCurrentPreset = new Label();
      chkAmpLinked = new CheckBox();
      chkWrap = new CheckBox();
      numWrap = new NumericUpDown();
      btnSetAudioDevice = new Button();
      cboAudioDevice = new ComboBox();
      numAlpha = new NumericUpDown();
      colorDialogMessage = new ColorDialog();
      splitContainer1 = new SplitContainer();
      txtStyle = new TextBox();
      label2 = new Label();
      splitContainer2 = new SplitContainer();
      lblAudioDevice = new Label();
      tableLayoutPanel1 = new TableLayoutPanel();
      btn77 = new Button();
      btn66 = new Button();
      btn55 = new Button();
      btn44 = new Button();
      btn33 = new Button();
      btn22 = new Button();
      btnK = new Button();
      btnF2 = new Button();
      btnN = new Button();
      btnAltEnter = new Button();
      btnY = new Button();
      btn11 = new Button();
      btnTilde = new Button();
      btn00 = new Button();
      btnF7 = new Button();
      btnF4 = new Button();
      btnF3 = new Button();
      btnDelete = new Button();
      colorDialogWave = new ColorDialog();
      tabControl1 = new TabControl();
      tabRemote = new TabPage();
      statusStrip1.SuspendLayout();
      ((System.ComponentModel.ISupportInitialize)numSize).BeginInit();
      ((System.ComponentModel.ISupportInitialize)numBPM).BeginInit();
      ((System.ComponentModel.ISupportInitialize)numBeats).BeginInit();
      ((System.ComponentModel.ISupportInitialize)numAmpLeft).BeginInit();
      ((System.ComponentModel.ISupportInitialize)numAmpRight).BeginInit();
      ((System.ComponentModel.ISupportInitialize)numWavemode).BeginInit();
      ((System.ComponentModel.ISupportInitialize)numWrap).BeginInit();
      ((System.ComponentModel.ISupportInitialize)numAlpha).BeginInit();
      ((System.ComponentModel.ISupportInitialize)splitContainer1).BeginInit();
      splitContainer1.Panel1.SuspendLayout();
      splitContainer1.Panel2.SuspendLayout();
      splitContainer1.SuspendLayout();
      ((System.ComponentModel.ISupportInitialize)splitContainer2).BeginInit();
      splitContainer2.Panel1.SuspendLayout();
      splitContainer2.Panel2.SuspendLayout();
      splitContainer2.SuspendLayout();
      tableLayoutPanel1.SuspendLayout();
      tabControl1.SuspendLayout();
      tabRemote.SuspendLayout();
      SuspendLayout();
      // 
      // statusStrip1
      // 
      statusStrip1.ImageScalingSize = new Size(20, 20);
      statusStrip1.Items.AddRange(new ToolStripItem[] { statusBar, toolStripDropDownButton });
      statusStrip1.Location = new Point(0, 543);
      statusStrip1.Name = "statusStrip1";
      statusStrip1.Padding = new Padding(1, 0, 12, 0);
      statusStrip1.Size = new Size(702, 26);
      statusStrip1.TabIndex = 5;
      statusStrip1.Text = "statusStrip1";
      // 
      // statusBar
      // 
      statusBar.Margin = new Padding(7, 4, 0, 2);
      statusBar.Name = "statusBar";
      statusBar.Size = new Size(601, 20);
      statusBar.Spring = true;
      statusBar.TextAlign = ContentAlignment.TopLeft;
      // 
      // toolStripDropDownButton
      // 
      toolStripDropDownButton.DropDownItems.AddRange(new ToolStripItem[] { toolStripMenuItemHelp, toolStripMenuItemHomepage, toolStripMenuItemSupporters, toolStripSeparator1, toolStripMenuItemDarkMode, toolStripSeparator2, toolStripMenuItemMessagePanel, toolStripMenuItemPresetPanel, toolStripMenuItemButtonPanel });
      toolStripDropDownButton.Image = (Image)resources.GetObject("toolStripDropDownButton.Image");
      toolStripDropDownButton.ImageTransparentColor = Color.Magenta;
      toolStripDropDownButton.Name = "toolStripDropDownButton";
      toolStripDropDownButton.ShowDropDownArrow = false;
      toolStripDropDownButton.Size = new Size(81, 24);
      toolStripDropDownButton.Text = "Milkwave";
      // 
      // toolStripMenuItemHelp
      // 
      toolStripMenuItemHelp.Name = "toolStripMenuItemHelp";
      toolStripMenuItemHelp.Size = new Size(152, 22);
      toolStripMenuItemHelp.Text = "Help";
      toolStripMenuItemHelp.Click += toolStripMenuItemHelp_Click;
      // 
      // toolStripMenuItemHomepage
      // 
      toolStripMenuItemHomepage.Name = "toolStripMenuItemHomepage";
      toolStripMenuItemHomepage.Size = new Size(152, 22);
      toolStripMenuItemHomepage.Text = "Releases";
      toolStripMenuItemHomepage.Click += toolStripMenuItemReleases_Click;
      // 
      // toolStripMenuItemSupporters
      // 
      toolStripMenuItemSupporters.Name = "toolStripMenuItemSupporters";
      toolStripMenuItemSupporters.Size = new Size(152, 22);
      toolStripMenuItemSupporters.Text = "Supporters";
      toolStripMenuItemSupporters.Click += toolStripMenuItemSupporters_Click;
      // 
      // toolStripSeparator1
      // 
      toolStripSeparator1.Name = "toolStripSeparator1";
      toolStripSeparator1.Size = new Size(149, 6);
      // 
      // toolStripMenuItemDarkMode
      // 
      toolStripMenuItemDarkMode.Checked = true;
      toolStripMenuItemDarkMode.CheckState = CheckState.Checked;
      toolStripMenuItemDarkMode.Name = "toolStripMenuItemDarkMode";
      toolStripMenuItemDarkMode.Size = new Size(152, 22);
      toolStripMenuItemDarkMode.Text = "Dark Mode";
      toolStripMenuItemDarkMode.Click += toolStripMenuItemDarkMode_Click;
      // 
      // toolStripSeparator2
      // 
      toolStripSeparator2.Name = "toolStripSeparator2";
      toolStripSeparator2.Size = new Size(149, 6);
      // 
      // toolStripMenuItemMessagePanel
      // 
      toolStripMenuItemMessagePanel.Checked = true;
      toolStripMenuItemMessagePanel.CheckState = CheckState.Checked;
      toolStripMenuItemMessagePanel.Name = "toolStripMenuItemMessagePanel";
      toolStripMenuItemMessagePanel.Size = new Size(152, 22);
      toolStripMenuItemMessagePanel.Text = "Message Panel";
      toolStripMenuItemMessagePanel.Click += toolStripMenuItemMessagePanel_Click;
      // 
      // toolStripMenuItemPresetPanel
      // 
      toolStripMenuItemPresetPanel.Checked = true;
      toolStripMenuItemPresetPanel.CheckState = CheckState.Checked;
      toolStripMenuItemPresetPanel.Name = "toolStripMenuItemPresetPanel";
      toolStripMenuItemPresetPanel.Size = new Size(152, 22);
      toolStripMenuItemPresetPanel.Text = "Preset Panel";
      toolStripMenuItemPresetPanel.Click += toolStripMenuItemPresetPanel_Click;
      // 
      // toolStripMenuItemButtonPanel
      // 
      toolStripMenuItemButtonPanel.Checked = true;
      toolStripMenuItemButtonPanel.CheckState = CheckState.Checked;
      toolStripMenuItemButtonPanel.Name = "toolStripMenuItemButtonPanel";
      toolStripMenuItemButtonPanel.Size = new Size(152, 22);
      toolStripMenuItemButtonPanel.Text = "Button Panel";
      toolStripMenuItemButtonPanel.Click += toolStripMenuItemButtonPanel_Click;
      // 
      // chkPreview
      // 
      chkPreview.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      chkPreview.Appearance = Appearance.Button;
      chkPreview.Checked = true;
      chkPreview.CheckState = CheckState.Checked;
      chkPreview.FlatStyle = FlatStyle.System;
      chkPreview.Location = new Point(413, 66);
      chkPreview.Margin = new Padding(3, 2, 3, 2);
      chkPreview.Name = "chkPreview";
      chkPreview.Size = new Size(64, 23);
      chkPreview.TabIndex = 73;
      chkPreview.Text = "Preview";
      chkPreview.TextAlign = ContentAlignment.MiddleCenter;
      chkPreview.TextImageRelation = TextImageRelation.ImageAboveText;
      toolTip1.SetToolTip(chkPreview, "Apply style to message box");
      chkPreview.UseVisualStyleBackColor = true;
      chkPreview.CheckedChanged += chkPreview_CheckedChanged;
      // 
      // btnAppendSize
      // 
      btnAppendSize.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      btnAppendSize.Location = new Point(387, 123);
      btnAppendSize.Margin = new Padding(0);
      btnAppendSize.Name = "btnAppendSize";
      btnAppendSize.Size = new Size(46, 23);
      btnAppendSize.TabIndex = 81;
      btnAppendSize.Text = "Set";
      toolTip1.SetToolTip(btnAppendSize, "Append to (or update in) parameters line");
      btnAppendSize.UseVisualStyleBackColor = true;
      btnAppendSize.Click += btnAppendSize_Click;
      // 
      // numSize
      // 
      numSize.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      numSize.Location = new Point(337, 123);
      numSize.Margin = new Padding(3, 2, 3, 2);
      numSize.Maximum = new decimal(new int[] { 999, 0, 0, 0 });
      numSize.Name = "numSize";
      numSize.Size = new Size(46, 23);
      numSize.TabIndex = 80;
      numSize.TextAlign = HorizontalAlignment.Center;
      toolTip1.SetToolTip(numSize, "Font size");
      numSize.Value = new decimal(new int[] { 30, 0, 0, 0 });
      numSize.TextChanged += txtSize_TextChanged;
      numSize.KeyDown += txtSize_KeyDown;
      // 
      // lblSize
      // 
      lblSize.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      lblSize.AutoSize = true;
      lblSize.Location = new Point(305, 126);
      lblSize.Name = "lblSize";
      lblSize.Size = new Size(27, 15);
      lblSize.TabIndex = 98;
      lblSize.Text = "Size";
      toolTip1.SetToolTip(lblSize, "Only used if no size parameter supplied\r\nDouble-click to clear the size value from the parameters line\r\n");
      lblSize.DoubleClick += lblSize_DoubleClick;
      // 
      // lblStyle
      // 
      lblStyle.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      lblStyle.AutoSize = true;
      lblStyle.Location = new Point(446, 97);
      lblStyle.Name = "lblStyle";
      lblStyle.Size = new Size(32, 15);
      lblStyle.TabIndex = 75;
      lblStyle.Text = "Style";
      toolTip1.SetToolTip(lblStyle, "Double-click to remove this style\r\nRight-click to fill frm elements from current parameters");
      lblStyle.DoubleClick += lblStyle_DoubleClick;
      lblStyle.MouseClick += lblStyle_MouseClick;
      // 
      // numBPM
      // 
      numBPM.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      numBPM.Location = new Point(387, 152);
      numBPM.Margin = new Padding(3, 2, 3, 2);
      numBPM.Maximum = new decimal(new int[] { 999, 0, 0, 0 });
      numBPM.Name = "numBPM";
      numBPM.Size = new Size(46, 23);
      numBPM.TabIndex = 85;
      numBPM.TextAlign = HorizontalAlignment.Center;
      toolTip1.SetToolTip(numBPM, "BPM");
      numBPM.Value = new decimal(new int[] { 120, 0, 0, 0 });
      numBPM.TextChanged += txtBPM_TextChanged;
      // 
      // lblBPM
      // 
      lblBPM.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      lblBPM.AutoSize = true;
      lblBPM.Location = new Point(349, 154);
      lblBPM.Name = "lblBPM";
      lblBPM.Size = new Size(32, 15);
      lblBPM.TabIndex = 97;
      lblBPM.Text = "BPM";
      toolTip1.SetToolTip(lblBPM, "Click to reset timer");
      lblBPM.Click += lblBPM_Click;
      // 
      // chkFileRandom
      // 
      chkFileRandom.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      chkFileRandom.Appearance = Appearance.Button;
      chkFileRandom.FlatStyle = FlatStyle.System;
      chkFileRandom.Location = new Point(526, 152);
      chkFileRandom.Margin = new Padding(3, 2, 3, 2);
      chkFileRandom.Name = "chkFileRandom";
      chkFileRandom.Size = new Size(49, 23);
      chkFileRandom.TabIndex = 87;
      chkFileRandom.Text = "Rand";
      chkFileRandom.TextAlign = ContentAlignment.MiddleCenter;
      chkFileRandom.TextImageRelation = TextImageRelation.ImageAboveText;
      toolTip1.SetToolTip(chkFileRandom, "Play random line");
      chkFileRandom.UseVisualStyleBackColor = true;
      // 
      // pnlColorMessage
      // 
      pnlColorMessage.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      pnlColorMessage.BorderStyle = BorderStyle.FixedSingle;
      pnlColorMessage.Location = new Point(484, 123);
      pnlColorMessage.Name = "pnlColorMessage";
      pnlColorMessage.Size = new Size(38, 23);
      pnlColorMessage.TabIndex = 82;
      toolTip1.SetToolTip(pnlColorMessage, "Only used if no r, g, b parameters supplied");
      pnlColorMessage.Click += pnlColorMessage_Click;
      // 
      // btnAppendColor
      // 
      btnAppendColor.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      btnAppendColor.Location = new Point(526, 123);
      btnAppendColor.Margin = new Padding(0);
      btnAppendColor.Name = "btnAppendColor";
      btnAppendColor.Size = new Size(49, 23);
      btnAppendColor.TabIndex = 83;
      btnAppendColor.Text = "Set";
      toolTip1.SetToolTip(btnAppendColor, "Append to (or update in) parameters line");
      btnAppendColor.UseVisualStyleBackColor = true;
      btnAppendColor.Click += btnAppendColor_Click;
      // 
      // lblColor
      // 
      lblColor.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      lblColor.AutoSize = true;
      lblColor.Location = new Point(441, 127);
      lblColor.Name = "lblColor";
      lblColor.Size = new Size(36, 15);
      lblColor.TabIndex = 96;
      lblColor.Text = "Color";
      toolTip1.SetToolTip(lblColor, "Only used if no r, g, b parameters supplied\r\nDouble-click to clear the color information from the parameters line\r\n");
      lblColor.DoubleClick += lblColor_DoubleClick;
      // 
      // btnFontAppend
      // 
      btnFontAppend.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      btnFontAppend.Location = new Point(237, 123);
      btnFontAppend.Margin = new Padding(0);
      btnFontAppend.Name = "btnFontAppend";
      btnFontAppend.Size = new Size(49, 23);
      btnFontAppend.TabIndex = 79;
      btnFontAppend.Text = "Set";
      toolTip1.SetToolTip(btnFontAppend, "Append to (or update in) parameters line");
      btnFontAppend.UseVisualStyleBackColor = true;
      btnFontAppend.Click += btnFontAppend_Click;
      // 
      // lblFont
      // 
      lblFont.AutoSize = true;
      lblFont.Location = new Point(41, 125);
      lblFont.Name = "lblFont";
      lblFont.Size = new Size(31, 15);
      lblFont.TabIndex = 95;
      lblFont.Text = "Font";
      toolTip1.SetToolTip(lblFont, "Only used if no font parameter supplied\r\nDouble-click to clear the font value from the parameters line\r\n");
      lblFont.DoubleClick += lblFont_DoubleClick;
      // 
      // cboFonts
      // 
      cboFonts.Anchor = AnchorStyles.Top | AnchorStyles.Left | AnchorStyles.Right;
      cboFonts.AutoCompleteMode = AutoCompleteMode.SuggestAppend;
      cboFonts.AutoCompleteSource = AutoCompleteSource.ListItems;
      cboFonts.DropDownStyle = ComboBoxStyle.DropDownList;
      cboFonts.FormattingEnabled = true;
      cboFonts.Location = new Point(78, 123);
      cboFonts.Name = "cboFonts";
      cboFonts.Size = new Size(153, 23);
      cboFonts.TabIndex = 78;
      toolTip1.SetToolTip(cboFonts, "Only used if no font parameter supplied");
      cboFonts.SelectedIndexChanged += cboFonts_SelectedIndexChanged;
      cboFonts.KeyDown += cboFonts_KeyDown;
      // 
      // numBeats
      // 
      numBeats.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      numBeats.Location = new Point(484, 152);
      numBeats.Margin = new Padding(3, 2, 3, 2);
      numBeats.Maximum = new decimal(new int[] { 99, 0, 0, 0 });
      numBeats.Name = "numBeats";
      numBeats.Size = new Size(38, 23);
      numBeats.TabIndex = 86;
      numBeats.TextAlign = HorizontalAlignment.Center;
      toolTip1.SetToolTip(numBeats, "Wait time between lines in beats");
      numBeats.Value = new decimal(new int[] { 8, 0, 0, 0 });
      // 
      // label7
      // 
      label7.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      label7.AutoSize = true;
      label7.Location = new Point(443, 154);
      label7.Name = "label7";
      label7.Size = new Size(35, 15);
      label7.TabIndex = 94;
      label7.Text = "Beats";
      toolTip1.SetToolTip(label7, "Wait time between lines in beats");
      // 
      // lblFromFile
      // 
      lblFromFile.AutoSize = true;
      lblFromFile.Location = new Point(16, 154);
      lblFromFile.Name = "lblFromFile";
      lblFromFile.Size = new Size(56, 15);
      lblFromFile.TabIndex = 93;
      lblFromFile.Text = "From File";
      toolTip1.SetToolTip(lblFromFile, "Double-click to reload file\r\nRight-click to load custom file");
      lblFromFile.DoubleClick += lblFromFile_DoubleClick;
      lblFromFile.MouseClick += lblFromFile_MouseClick;
      // 
      // chkAutoplay
      // 
      chkAutoplay.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      chkAutoplay.Appearance = Appearance.Button;
      chkAutoplay.FlatStyle = FlatStyle.System;
      chkAutoplay.Location = new Point(581, 123);
      chkAutoplay.Name = "chkAutoplay";
      chkAutoplay.Size = new Size(83, 52);
      chkAutoplay.TabIndex = 88;
      chkAutoplay.Text = "Autoplay";
      chkAutoplay.TextAlign = ContentAlignment.MiddleCenter;
      chkAutoplay.TextImageRelation = TextImageRelation.ImageAboveText;
      toolTip1.SetToolTip(chkAutoplay, "F12");
      chkAutoplay.UseVisualStyleBackColor = true;
      chkAutoplay.CheckedChanged += chkAutoplay_CheckedChanged;
      // 
      // txtAutoplay
      // 
      txtAutoplay.Anchor = AnchorStyles.Top | AnchorStyles.Left | AnchorStyles.Right;
      txtAutoplay.Location = new Point(78, 152);
      txtAutoplay.Name = "txtAutoplay";
      txtAutoplay.ReadOnly = true;
      txtAutoplay.Size = new Size(251, 23);
      txtAutoplay.TabIndex = 84;
      toolTip1.SetToolTip(txtAutoplay, "Next line from file script-default.txt");
      // 
      // btnSaveParam
      // 
      btnSaveParam.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      btnSaveParam.FlatStyle = FlatStyle.System;
      btnSaveParam.Location = new Point(581, 94);
      btnSaveParam.Name = "btnSaveParam";
      btnSaveParam.Size = new Size(83, 23);
      btnSaveParam.TabIndex = 77;
      btnSaveParam.Text = "Save";
      toolTip1.SetToolTip(btnSaveParam, "Save current parameters as style");
      btnSaveParam.UseVisualStyleBackColor = true;
      btnSaveParam.Click += btnSaveParam_Click;
      // 
      // lblParameters
      // 
      lblParameters.AutoSize = true;
      lblParameters.Location = new Point(6, 97);
      lblParameters.Name = "lblParameters";
      lblParameters.Size = new Size(66, 15);
      lblParameters.TabIndex = 91;
      lblParameters.Text = "Parameters";
      toolTip1.SetToolTip(lblParameters, "Double-click to clear all saved styles");
      lblParameters.DoubleClick += lblParameters_DoubleClick;
      // 
      // cboParameters
      // 
      cboParameters.Anchor = AnchorStyles.Top | AnchorStyles.Left | AnchorStyles.Right;
      cboParameters.DropDownStyle = ComboBoxStyle.DropDownList;
      cboParameters.Location = new Point(78, 94);
      cboParameters.Name = "cboParameters";
      cboParameters.Size = new Size(355, 23);
      cboParameters.TabIndex = 74;
      toolTip1.SetToolTip(cboParameters, resources.GetString("cboParameters.ToolTip"));
      cboParameters.SelectedIndexChanged += cboParameters_SelectedIndexChanged;
      cboParameters.TextChanged += cboParameters_TextChanged;
      cboParameters.KeyDown += cboParameters_KeyDown;
      // 
      // lblWindow
      // 
      lblWindow.AutoSize = true;
      lblWindow.Location = new Point(23, 67);
      lblWindow.Name = "lblWindow";
      lblWindow.Size = new Size(51, 15);
      lblWindow.TabIndex = 89;
      lblWindow.Text = "Window";
      toolTip1.SetToolTip(lblWindow, "Double-click to start Visualizer if no window found\r\nand restore default position and size\r\n");
      lblWindow.DoubleClick += lblWindow_DoubleClick;
      // 
      // txtWindowTitle
      // 
      txtWindowTitle.Anchor = AnchorStyles.Top | AnchorStyles.Left | AnchorStyles.Right;
      txtWindowTitle.Location = new Point(78, 66);
      txtWindowTitle.Name = "txtWindowTitle";
      txtWindowTitle.Size = new Size(251, 23);
      txtWindowTitle.TabIndex = 72;
      txtWindowTitle.Text = "Milkwave Visualizer";
      toolTip1.SetToolTip(txtWindowTitle, "Partitial window title of Milkwave Visualizer window");
      // 
      // btnSend
      // 
      btnSend.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      btnSend.Location = new Point(581, 6);
      btnSend.Name = "btnSend";
      btnSend.Size = new Size(83, 55);
      btnSend.TabIndex = 71;
      btnSend.Text = "Send";
      toolTip1.SetToolTip(btnSend, "Send to Visualizer (F2)");
      btnSend.UseVisualStyleBackColor = true;
      btnSend.Click += btnSend_Click;
      // 
      // txtMessage
      // 
      txtMessage.Anchor = AnchorStyles.Top | AnchorStyles.Left | AnchorStyles.Right;
      txtMessage.Location = new Point(78, 6);
      txtMessage.Multiline = true;
      txtMessage.Name = "txtMessage";
      txtMessage.Size = new Size(497, 54);
      txtMessage.TabIndex = 70;
      txtMessage.Text = "Hi from Milkwave Remote!";
      toolTip1.SetToolTip(txtMessage, "F1: Select all text\r\nEnter: Send to Visualizer\r\nShift+Enter: line break (or use // in message text)");
      txtMessage.KeyDown += txtMessage_KeyDown;
      // 
      // btnSpace
      // 
      tableLayoutPanel1.SetColumnSpan(btnSpace, 2);
      btnSpace.Dock = DockStyle.Fill;
      btnSpace.Location = new Point(7, 6);
      btnSpace.Margin = new Padding(3, 2, 3, 2);
      btnSpace.Name = "btnSpace";
      btnSpace.Size = new Size(158, 41);
      btnSpace.TabIndex = 0;
      btnSpace.Text = "Next Preset\r\n(Space)";
      toolTip1.SetToolTip(btnSpace, "F3");
      btnSpace.UseVisualStyleBackColor = true;
      btnSpace.Click += btnSpace_Click;
      // 
      // btnBackspace
      // 
      tableLayoutPanel1.SetColumnSpan(btnBackspace, 2);
      btnBackspace.Dock = DockStyle.Fill;
      btnBackspace.Location = new Point(171, 6);
      btnBackspace.Margin = new Padding(3, 2, 3, 2);
      btnBackspace.Name = "btnBackspace";
      btnBackspace.Size = new Size(158, 41);
      btnBackspace.TabIndex = 1;
      btnBackspace.Text = "Previous Preset\r\n(Backspace)";
      toolTip1.SetToolTip(btnBackspace, "F4");
      btnBackspace.UseVisualStyleBackColor = true;
      btnBackspace.Click += btnBackspace_Click;
      // 
      // lblPreset
      // 
      lblPreset.AutoSize = true;
      lblPreset.Location = new Point(35, 9);
      lblPreset.Name = "lblPreset";
      lblPreset.Size = new Size(39, 15);
      lblPreset.TabIndex = 98;
      lblPreset.Text = "Preset";
      toolTip1.SetToolTip(lblPreset, "Click to copy full path to clipboard\r\nDouble-click to clear all items");
      lblPreset.Click += lblPreset_Click;
      lblPreset.DoubleClick += lblPreset_DoubleClick;
      // 
      // btnPresetLoadFile
      // 
      btnPresetLoadFile.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      btnPresetLoadFile.FlatStyle = FlatStyle.System;
      btnPresetLoadFile.Location = new Point(423, 6);
      btnPresetLoadFile.Name = "btnPresetLoadFile";
      btnPresetLoadFile.Size = new Size(75, 23);
      btnPresetLoadFile.TabIndex = 99;
      btnPresetLoadFile.Text = "File";
      toolTip1.SetToolTip(btnPresetLoadFile, "Load a single preset");
      btnPresetLoadFile.UseVisualStyleBackColor = true;
      btnPresetLoadFile.Click += btnPresetOpen_Click;
      // 
      // btnPresetSend
      // 
      btnPresetSend.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      btnPresetSend.FlatStyle = FlatStyle.System;
      btnPresetSend.Location = new Point(581, 6);
      btnPresetSend.Name = "btnPresetSend";
      btnPresetSend.Size = new Size(83, 23);
      btnPresetSend.TabIndex = 100;
      btnPresetSend.Text = "Send";
      toolTip1.SetToolTip(btnPresetSend, "Send to Visualizer (F6)\r\nCtrl+Click: Send next to Visualizer (F7)");
      btnPresetSend.UseVisualStyleBackColor = true;
      btnPresetSend.Click += btnPresetSend_Click;
      // 
      // btnPresetLoadDirectory
      // 
      btnPresetLoadDirectory.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      btnPresetLoadDirectory.FlatStyle = FlatStyle.System;
      btnPresetLoadDirectory.Location = new Point(504, 6);
      btnPresetLoadDirectory.Name = "btnPresetLoadDirectory";
      btnPresetLoadDirectory.Size = new Size(71, 23);
      btnPresetLoadDirectory.TabIndex = 101;
      btnPresetLoadDirectory.Text = "Directory";
      toolTip1.SetToolTip(btnPresetLoadDirectory, "Load all presets from a directory");
      btnPresetLoadDirectory.UseVisualStyleBackColor = true;
      btnPresetLoadDirectory.Click += btnPresetLoadDirectory_Click;
      // 
      // txtVisRunning
      // 
      txtVisRunning.Anchor = AnchorStyles.Top | AnchorStyles.Left | AnchorStyles.Right;
      txtVisRunning.Location = new Point(78, 35);
      txtVisRunning.Name = "txtVisRunning";
      txtVisRunning.ReadOnly = true;
      txtVisRunning.Size = new Size(497, 23);
      txtVisRunning.TabIndex = 96;
      toolTip1.SetToolTip(txtVisRunning, "Currently running Visualizer preset");
      // 
      // cboPresets
      // 
      cboPresets.Anchor = AnchorStyles.Top | AnchorStyles.Left | AnchorStyles.Right;
      cboPresets.AutoCompleteMode = AutoCompleteMode.SuggestAppend;
      cboPresets.AutoCompleteSource = AutoCompleteSource.ListItems;
      cboPresets.DropDownStyle = ComboBoxStyle.DropDownList;
      cboPresets.FormattingEnabled = true;
      cboPresets.Location = new Point(78, 6);
      cboPresets.Name = "cboPresets";
      cboPresets.Size = new Size(339, 23);
      cboPresets.TabIndex = 97;
      toolTip1.SetToolTip(cboPresets, "Alt+Mousewheel: Send to Visualizer");
      cboPresets.SelectedIndexChanged += cboPresets_SelectedIndexChanged;
      cboPresets.KeyDown += cboPresets_KeyDown;
      // 
      // txtDirFilter
      // 
      txtDirFilter.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      txtDirFilter.Location = new Point(581, 35);
      txtDirFilter.Name = "txtDirFilter";
      txtDirFilter.Size = new Size(83, 23);
      txtDirFilter.TabIndex = 102;
      toolTip1.SetToolTip(txtDirFilter, "When using Directory loading, load only files containing the text entered here");
      // 
      // numAmpLeft
      // 
      numAmpLeft.DecimalPlaces = 1;
      numAmpLeft.Increment = new decimal(new int[] { 1, 0, 0, 65536 });
      numAmpLeft.Location = new Point(78, 63);
      numAmpLeft.Maximum = new decimal(new int[] { 999, 0, 0, 65536 });
      numAmpLeft.Name = "numAmpLeft";
      numAmpLeft.Size = new Size(60, 23);
      numAmpLeft.TabIndex = 103;
      numAmpLeft.TextAlign = HorizontalAlignment.Center;
      toolTip1.SetToolTip(numAmpLeft, "Amplification factor for left channel");
      numAmpLeft.Value = new decimal(new int[] { 10, 0, 0, 65536 });
      numAmpLeft.ValueChanged += numAmpLeft_ValueChanged;
      // 
      // lblAmp
      // 
      lblAmp.AutoSize = true;
      lblAmp.Location = new Point(18, 66);
      lblAmp.Name = "lblAmp";
      lblAmp.Size = new Size(56, 15);
      lblAmp.TabIndex = 104;
      lblAmp.Text = "Amp Left";
      toolTip1.SetToolTip(lblAmp, "Double-Click to reset");
      lblAmp.Click += lblAmpLeft_Click;
      // 
      // lblAmpRight
      // 
      lblAmpRight.AutoSize = true;
      lblAmpRight.Location = new Point(146, 65);
      lblAmpRight.Name = "lblAmpRight";
      lblAmpRight.Size = new Size(35, 15);
      lblAmpRight.TabIndex = 105;
      lblAmpRight.Text = "Right";
      toolTip1.SetToolTip(lblAmpRight, "Double-Click to reset");
      lblAmpRight.Click += lblAmpRight_Click;
      // 
      // numAmpRight
      // 
      numAmpRight.DecimalPlaces = 1;
      numAmpRight.Increment = new decimal(new int[] { 1, 0, 0, 65536 });
      numAmpRight.Location = new Point(185, 63);
      numAmpRight.Margin = new Padding(3, 2, 3, 2);
      numAmpRight.Maximum = new decimal(new int[] { 999, 0, 0, 65536 });
      numAmpRight.Name = "numAmpRight";
      numAmpRight.Size = new Size(60, 23);
      numAmpRight.TabIndex = 106;
      numAmpRight.TextAlign = HorizontalAlignment.Center;
      toolTip1.SetToolTip(numAmpRight, "Amplification factor for right channel");
      numAmpRight.Value = new decimal(new int[] { 10, 0, 0, 65536 });
      numAmpRight.ValueChanged += numAmpRight_ValueChanged;
      // 
      // numWavemode
      // 
      numWavemode.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      numWavemode.Location = new Point(408, 63);
      numWavemode.Margin = new Padding(3, 2, 3, 2);
      numWavemode.Maximum = new decimal(new int[] { 15, 0, 0, 0 });
      numWavemode.Name = "numWavemode";
      numWavemode.Size = new Size(38, 23);
      numWavemode.TabIndex = 109;
      numWavemode.TextAlign = HorizontalAlignment.Center;
      toolTip1.SetToolTip(numWavemode, "Alt+Mousewheel: Send to Visualizer");
      numWavemode.Value = new decimal(new int[] { 7, 0, 0, 0 });
      numWavemode.ValueChanged += numWavemode_ValueChanged;
      // 
      // btnSendWave
      // 
      btnSendWave.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      btnSendWave.FlatStyle = FlatStyle.System;
      btnSendWave.Location = new Point(581, 63);
      btnSendWave.Name = "btnSendWave";
      btnSendWave.Size = new Size(83, 23);
      btnSendWave.TabIndex = 110;
      btnSendWave.Text = "Send";
      toolTip1.SetToolTip(btnSendWave, "Send to Visualizer\r\n(only alters wave already defined in preset)");
      btnSendWave.UseVisualStyleBackColor = true;
      btnSendWave.Click += btnSendWave_Click;
      // 
      // lblWaveColor
      // 
      lblWaveColor.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      lblWaveColor.AutoSize = true;
      lblWaveColor.Location = new Point(451, 67);
      lblWaveColor.Name = "lblWaveColor";
      lblWaveColor.Size = new Size(36, 15);
      lblWaveColor.TabIndex = 112;
      lblWaveColor.Text = "Color";
      toolTip1.SetToolTip(lblWaveColor, "Wave color\r\nDouble-click: Copy R,G,B to clipboard\r\nAlt+Double-click: Copy R,G,B as float values (0..1)");
      lblWaveColor.DoubleClick += lblWaveColor_DoubleClick;
      // 
      // lblWavemode
      // 
      lblWavemode.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      lblWavemode.AutoSize = true;
      lblWavemode.Location = new Point(333, 66);
      lblWavemode.Name = "lblWavemode";
      lblWavemode.Size = new Size(70, 15);
      lblWavemode.TabIndex = 108;
      lblWavemode.Text = "Wave Mode";
      toolTip1.SetToolTip(lblWavemode, "Wave Mode (0-15)");
      // 
      // pnlColorWave
      // 
      pnlColorWave.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      pnlColorWave.BorderStyle = BorderStyle.FixedSingle;
      pnlColorWave.Location = new Point(490, 63);
      pnlColorWave.Name = "pnlColorWave";
      pnlColorWave.Size = new Size(37, 23);
      pnlColorWave.TabIndex = 111;
      toolTip1.SetToolTip(pnlColorWave, "Wave color");
      pnlColorWave.Click += pnlColorWave_Click;
      // 
      // lblCurrentPreset
      // 
      lblCurrentPreset.AutoSize = true;
      lblCurrentPreset.Location = new Point(22, 37);
      lblCurrentPreset.Name = "lblCurrentPreset";
      lblCurrentPreset.Size = new Size(52, 15);
      lblCurrentPreset.TabIndex = 95;
      lblCurrentPreset.Text = "Running";
      toolTip1.SetToolTip(lblCurrentPreset, "Double-click to copy full path");
      lblCurrentPreset.DoubleClick += lblCurrentPreset_DoubleClick;
      // 
      // chkAmpLinked
      // 
      chkAmpLinked.Appearance = Appearance.Button;
      chkAmpLinked.Checked = true;
      chkAmpLinked.CheckState = CheckState.Checked;
      chkAmpLinked.FlatStyle = FlatStyle.System;
      chkAmpLinked.Location = new Point(250, 62);
      chkAmpLinked.Margin = new Padding(3, 2, 3, 2);
      chkAmpLinked.Name = "chkAmpLinked";
      chkAmpLinked.Size = new Size(51, 23);
      chkAmpLinked.TabIndex = 113;
      chkAmpLinked.Text = "Link";
      chkAmpLinked.TextAlign = ContentAlignment.MiddleCenter;
      chkAmpLinked.TextImageRelation = TextImageRelation.ImageAboveText;
      toolTip1.SetToolTip(chkAmpLinked, "Link amp for both channels");
      chkAmpLinked.UseVisualStyleBackColor = true;
      // 
      // chkWrap
      // 
      chkWrap.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      chkWrap.Appearance = Appearance.Button;
      chkWrap.Checked = true;
      chkWrap.CheckState = CheckState.Checked;
      chkWrap.FlatStyle = FlatStyle.System;
      chkWrap.Location = new Point(526, 65);
      chkWrap.Margin = new Padding(3, 2, 3, 2);
      chkWrap.Name = "chkWrap";
      chkWrap.Size = new Size(48, 23);
      chkWrap.TabIndex = 99;
      chkWrap.Text = "Wrap";
      chkWrap.TextAlign = ContentAlignment.MiddleCenter;
      chkWrap.TextImageRelation = TextImageRelation.ImageAboveText;
      toolTip1.SetToolTip(chkWrap, "If the line is longer than this value, try to wrap it in the middle");
      chkWrap.UseVisualStyleBackColor = true;
      // 
      // numWrap
      // 
      numWrap.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      numWrap.Location = new Point(484, 66);
      numWrap.Margin = new Padding(3, 2, 3, 2);
      numWrap.Maximum = new decimal(new int[] { 99, 0, 0, 0 });
      numWrap.Minimum = new decimal(new int[] { 10, 0, 0, 0 });
      numWrap.Name = "numWrap";
      numWrap.Size = new Size(38, 23);
      numWrap.TabIndex = 100;
      numWrap.TextAlign = HorizontalAlignment.Center;
      toolTip1.SetToolTip(numWrap, "If the line is longer than this value, try to wrap it in the middle");
      numWrap.Value = new decimal(new int[] { 30, 0, 0, 0 });
      // 
      // btnSetAudioDevice
      // 
      btnSetAudioDevice.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      btnSetAudioDevice.Location = new Point(581, 92);
      btnSetAudioDevice.Name = "btnSetAudioDevice";
      btnSetAudioDevice.Size = new Size(83, 23);
      btnSetAudioDevice.TabIndex = 116;
      btnSetAudioDevice.Text = "Set";
      toolTip1.SetToolTip(btnSetAudioDevice, "Set this device as Visualizer audio source");
      btnSetAudioDevice.UseVisualStyleBackColor = true;
      btnSetAudioDevice.Click += btnSetAudioDevice_Click;
      // 
      // cboAudioDevice
      // 
      cboAudioDevice.Anchor = AnchorStyles.Top | AnchorStyles.Left | AnchorStyles.Right;
      cboAudioDevice.DropDownStyle = ComboBoxStyle.DropDownList;
      cboAudioDevice.FormattingEnabled = true;
      cboAudioDevice.Location = new Point(78, 92);
      cboAudioDevice.Name = "cboAudioDevice";
      cboAudioDevice.Size = new Size(496, 23);
      cboAudioDevice.TabIndex = 114;
      toolTip1.SetToolTip(cboAudioDevice, "Alt+Mousewheel: Set in Visualizer");
      cboAudioDevice.SelectedIndexChanged += cboAudioDevice_SelectedIndexChanged;
      // 
      // numAlpha
      // 
      numAlpha.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      numAlpha.DecimalPlaces = 1;
      numAlpha.Increment = new decimal(new int[] { 1, 0, 0, 65536 });
      numAlpha.Location = new Point(532, 63);
      numAlpha.Margin = new Padding(3, 2, 3, 2);
      numAlpha.Maximum = new decimal(new int[] { 1, 0, 0, 0 });
      numAlpha.Name = "numAlpha";
      numAlpha.Size = new Size(43, 23);
      numAlpha.TabIndex = 117;
      numAlpha.TextAlign = HorizontalAlignment.Center;
      toolTip1.SetToolTip(numAlpha, "Alpha (opacity)");
      numAlpha.Value = new decimal(new int[] { 10, 0, 0, 65536 });
      // 
      // colorDialogMessage
      // 
      colorDialogMessage.AnyColor = true;
      colorDialogMessage.Color = Color.White;
      colorDialogMessage.FullOpen = true;
      colorDialogMessage.SolidColorOnly = true;
      // 
      // splitContainer1
      // 
      splitContainer1.BorderStyle = BorderStyle.FixedSingle;
      splitContainer1.Dock = DockStyle.Fill;
      splitContainer1.Location = new Point(0, 0);
      splitContainer1.Margin = new Padding(0);
      splitContainer1.Name = "splitContainer1";
      splitContainer1.Orientation = Orientation.Horizontal;
      // 
      // splitContainer1.Panel1
      // 
      splitContainer1.Panel1.Controls.Add(numWrap);
      splitContainer1.Panel1.Controls.Add(chkWrap);
      splitContainer1.Panel1.Controls.Add(chkPreview);
      splitContainer1.Panel1.Controls.Add(btnAppendSize);
      splitContainer1.Panel1.Controls.Add(numSize);
      splitContainer1.Panel1.Controls.Add(lblSize);
      splitContainer1.Panel1.Controls.Add(lblStyle);
      splitContainer1.Panel1.Controls.Add(txtStyle);
      splitContainer1.Panel1.Controls.Add(numBPM);
      splitContainer1.Panel1.Controls.Add(lblBPM);
      splitContainer1.Panel1.Controls.Add(chkFileRandom);
      splitContainer1.Panel1.Controls.Add(pnlColorMessage);
      splitContainer1.Panel1.Controls.Add(btnAppendColor);
      splitContainer1.Panel1.Controls.Add(lblColor);
      splitContainer1.Panel1.Controls.Add(btnFontAppend);
      splitContainer1.Panel1.Controls.Add(lblFont);
      splitContainer1.Panel1.Controls.Add(cboFonts);
      splitContainer1.Panel1.Controls.Add(numBeats);
      splitContainer1.Panel1.Controls.Add(label7);
      splitContainer1.Panel1.Controls.Add(lblFromFile);
      splitContainer1.Panel1.Controls.Add(chkAutoplay);
      splitContainer1.Panel1.Controls.Add(txtAutoplay);
      splitContainer1.Panel1.Controls.Add(btnSaveParam);
      splitContainer1.Panel1.Controls.Add(lblParameters);
      splitContainer1.Panel1.Controls.Add(cboParameters);
      splitContainer1.Panel1.Controls.Add(label2);
      splitContainer1.Panel1.Controls.Add(lblWindow);
      splitContainer1.Panel1.Controls.Add(txtWindowTitle);
      splitContainer1.Panel1.Controls.Add(btnSend);
      splitContainer1.Panel1.Controls.Add(txtMessage);
      splitContainer1.Panel1.RightToLeft = RightToLeft.No;
      splitContainer1.Panel1.Paint += splitContainer1_Panel1_Paint;
      splitContainer1.Panel1MinSize = 0;
      // 
      // splitContainer1.Panel2
      // 
      splitContainer1.Panel2.Controls.Add(splitContainer2);
      splitContainer1.Panel2.RightToLeft = RightToLeft.No;
      splitContainer1.Panel2MinSize = 0;
      splitContainer1.Size = new Size(671, 535);
      splitContainer1.SplitterDistance = 189;
      splitContainer1.SplitterWidth = 8;
      splitContainer1.TabIndex = 28;
      splitContainer1.SplitterMoved += splitContainer1_SplitterMoved;
      splitContainer1.SizeChanged += splitContainer1_SizeChanged;
      // 
      // txtStyle
      // 
      txtStyle.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      txtStyle.Location = new Point(484, 94);
      txtStyle.Margin = new Padding(3, 2, 3, 2);
      txtStyle.Name = "txtStyle";
      txtStyle.Size = new Size(91, 23);
      txtStyle.TabIndex = 76;
      txtStyle.Text = "Style A";
      txtStyle.KeyDown += txtStyle_KeyDown;
      // 
      // label2
      // 
      label2.AutoSize = true;
      label2.Location = new Point(19, 9);
      label2.Name = "label2";
      label2.Size = new Size(53, 15);
      label2.TabIndex = 90;
      label2.Text = "Message";
      // 
      // splitContainer2
      // 
      splitContainer2.BorderStyle = BorderStyle.FixedSingle;
      splitContainer2.Dock = DockStyle.Fill;
      splitContainer2.Location = new Point(0, 0);
      splitContainer2.Margin = new Padding(0);
      splitContainer2.Name = "splitContainer2";
      splitContainer2.Orientation = Orientation.Horizontal;
      // 
      // splitContainer2.Panel1
      // 
      splitContainer2.Panel1.Controls.Add(numAlpha);
      splitContainer2.Panel1.Controls.Add(btnSetAudioDevice);
      splitContainer2.Panel1.Controls.Add(lblAudioDevice);
      splitContainer2.Panel1.Controls.Add(cboAudioDevice);
      splitContainer2.Panel1.Controls.Add(chkAmpLinked);
      splitContainer2.Panel1.Controls.Add(pnlColorWave);
      splitContainer2.Panel1.Controls.Add(lblWaveColor);
      splitContainer2.Panel1.Controls.Add(btnSendWave);
      splitContainer2.Panel1.Controls.Add(numWavemode);
      splitContainer2.Panel1.Controls.Add(lblWavemode);
      splitContainer2.Panel1.Controls.Add(numAmpRight);
      splitContainer2.Panel1.Controls.Add(lblAmpRight);
      splitContainer2.Panel1.Controls.Add(numAmpLeft);
      splitContainer2.Panel1.Controls.Add(lblAmp);
      splitContainer2.Panel1.Controls.Add(txtDirFilter);
      splitContainer2.Panel1.Controls.Add(btnPresetLoadDirectory);
      splitContainer2.Panel1.Controls.Add(btnPresetSend);
      splitContainer2.Panel1.Controls.Add(btnPresetLoadFile);
      splitContainer2.Panel1.Controls.Add(lblPreset);
      splitContainer2.Panel1.Controls.Add(cboPresets);
      splitContainer2.Panel1.Controls.Add(txtVisRunning);
      splitContainer2.Panel1.Controls.Add(lblCurrentPreset);
      splitContainer2.Panel1.RightToLeft = RightToLeft.No;
      splitContainer2.Panel1.Paint += splitContainer2_Panel1_Paint;
      splitContainer2.Panel1MinSize = 44;
      // 
      // splitContainer2.Panel2
      // 
      splitContainer2.Panel2.Controls.Add(tableLayoutPanel1);
      splitContainer2.Panel2.RightToLeft = RightToLeft.No;
      splitContainer2.Panel2.Paint += splitContainer2_Panel2_Paint;
      splitContainer2.Panel2MinSize = 0;
      splitContainer2.Size = new Size(671, 338);
      splitContainer2.SplitterDistance = 138;
      splitContainer2.SplitterWidth = 8;
      splitContainer2.TabIndex = 0;
      // 
      // lblAudioDevice
      // 
      lblAudioDevice.AutoSize = true;
      lblAudioDevice.Location = new Point(30, 95);
      lblAudioDevice.Name = "lblAudioDevice";
      lblAudioDevice.Size = new Size(42, 15);
      lblAudioDevice.TabIndex = 115;
      lblAudioDevice.Text = "Device";
      // 
      // tableLayoutPanel1
      // 
      tableLayoutPanel1.ColumnCount = 8;
      tableLayoutPanel1.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 12.5F));
      tableLayoutPanel1.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 12.5F));
      tableLayoutPanel1.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 12.5F));
      tableLayoutPanel1.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 12.5F));
      tableLayoutPanel1.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 12.5F));
      tableLayoutPanel1.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 12.5F));
      tableLayoutPanel1.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 12.5F));
      tableLayoutPanel1.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 12.5F));
      tableLayoutPanel1.Controls.Add(btn77, 7, 3);
      tableLayoutPanel1.Controls.Add(btn66, 6, 3);
      tableLayoutPanel1.Controls.Add(btn55, 5, 3);
      tableLayoutPanel1.Controls.Add(btn44, 4, 3);
      tableLayoutPanel1.Controls.Add(btn33, 3, 3);
      tableLayoutPanel1.Controls.Add(btn22, 2, 3);
      tableLayoutPanel1.Controls.Add(btnK, 2, 2);
      tableLayoutPanel1.Controls.Add(btnF2, 0, 1);
      tableLayoutPanel1.Controls.Add(btnN, 4, 0);
      tableLayoutPanel1.Controls.Add(btnAltEnter, 0, 2);
      tableLayoutPanel1.Controls.Add(btnY, 4, 2);
      tableLayoutPanel1.Controls.Add(btn11, 1, 3);
      tableLayoutPanel1.Controls.Add(btnTilde, 6, 0);
      tableLayoutPanel1.Controls.Add(btn00, 0, 3);
      tableLayoutPanel1.Controls.Add(btnSpace, 0, 0);
      tableLayoutPanel1.Controls.Add(btnF7, 6, 1);
      tableLayoutPanel1.Controls.Add(btnF4, 4, 1);
      tableLayoutPanel1.Controls.Add(btnF3, 2, 1);
      tableLayoutPanel1.Controls.Add(btnBackspace, 2, 0);
      tableLayoutPanel1.Controls.Add(btnDelete, 6, 2);
      tableLayoutPanel1.Dock = DockStyle.Fill;
      tableLayoutPanel1.Location = new Point(0, 0);
      tableLayoutPanel1.Margin = new Padding(3, 2, 3, 2);
      tableLayoutPanel1.Name = "tableLayoutPanel1";
      tableLayoutPanel1.Padding = new Padding(4);
      tableLayoutPanel1.RowCount = 4;
      tableLayoutPanel1.RowStyles.Add(new RowStyle(SizeType.Percent, 25F));
      tableLayoutPanel1.RowStyles.Add(new RowStyle(SizeType.Percent, 25F));
      tableLayoutPanel1.RowStyles.Add(new RowStyle(SizeType.Percent, 25F));
      tableLayoutPanel1.RowStyles.Add(new RowStyle(SizeType.Percent, 25F));
      tableLayoutPanel1.RowStyles.Add(new RowStyle(SizeType.Absolute, 15F));
      tableLayoutPanel1.Size = new Size(669, 190);
      tableLayoutPanel1.TabIndex = 34;
      tableLayoutPanel1.Paint += tableLayoutPanel1_Paint;
      // 
      // btn77
      // 
      btn77.Anchor = AnchorStyles.Top | AnchorStyles.Bottom | AnchorStyles.Left | AnchorStyles.Right;
      btn77.Location = new Point(581, 141);
      btn77.Margin = new Padding(3, 2, 3, 2);
      btn77.Name = "btn77";
      btn77.Size = new Size(81, 43);
      btn77.TabIndex = 19;
      btn77.Text = "77";
      btn77.UseVisualStyleBackColor = true;
      btn77.Click += btn77_Click;
      // 
      // btn66
      // 
      btn66.Anchor = AnchorStyles.Top | AnchorStyles.Bottom | AnchorStyles.Left | AnchorStyles.Right;
      btn66.Location = new Point(499, 141);
      btn66.Margin = new Padding(3, 2, 3, 2);
      btn66.Name = "btn66";
      btn66.Size = new Size(76, 43);
      btn66.TabIndex = 18;
      btn66.Text = "66";
      btn66.UseVisualStyleBackColor = true;
      btn66.Click += btn66_Click;
      // 
      // btn55
      // 
      btn55.Anchor = AnchorStyles.Top | AnchorStyles.Bottom | AnchorStyles.Left | AnchorStyles.Right;
      btn55.Location = new Point(417, 141);
      btn55.Margin = new Padding(3, 2, 3, 2);
      btn55.Name = "btn55";
      btn55.Size = new Size(76, 43);
      btn55.TabIndex = 17;
      btn55.Text = "55";
      btn55.UseVisualStyleBackColor = true;
      btn55.Click += btn55_Click;
      // 
      // btn44
      // 
      btn44.Anchor = AnchorStyles.Top | AnchorStyles.Bottom | AnchorStyles.Left | AnchorStyles.Right;
      btn44.Location = new Point(335, 141);
      btn44.Margin = new Padding(3, 2, 3, 2);
      btn44.Name = "btn44";
      btn44.Size = new Size(76, 43);
      btn44.TabIndex = 16;
      btn44.Text = "44";
      btn44.UseVisualStyleBackColor = true;
      btn44.Click += btn44_Click;
      // 
      // btn33
      // 
      btn33.Anchor = AnchorStyles.Top | AnchorStyles.Bottom | AnchorStyles.Left | AnchorStyles.Right;
      btn33.Location = new Point(253, 141);
      btn33.Margin = new Padding(3, 2, 3, 2);
      btn33.Name = "btn33";
      btn33.Size = new Size(76, 43);
      btn33.TabIndex = 15;
      btn33.Text = "33";
      btn33.UseVisualStyleBackColor = true;
      btn33.Click += btn33_Click;
      // 
      // btn22
      // 
      btn22.Anchor = AnchorStyles.Top | AnchorStyles.Bottom | AnchorStyles.Left | AnchorStyles.Right;
      btn22.Location = new Point(171, 141);
      btn22.Margin = new Padding(3, 2, 3, 2);
      btn22.Name = "btn22";
      btn22.Size = new Size(76, 43);
      btn22.TabIndex = 14;
      btn22.Text = "22";
      btn22.UseVisualStyleBackColor = true;
      btn22.Click += btn22_Click;
      // 
      // btnK
      // 
      tableLayoutPanel1.SetColumnSpan(btnK, 2);
      btnK.Dock = DockStyle.Fill;
      btnK.Location = new Point(171, 96);
      btnK.Margin = new Padding(3, 2, 3, 2);
      btnK.Name = "btnK";
      btnK.Size = new Size(158, 41);
      btnK.TabIndex = 9;
      btnK.Text = "Sprite Mode\r\n(K)";
      btnK.UseVisualStyleBackColor = true;
      btnK.Click += btnK_Click;
      // 
      // btnF2
      // 
      tableLayoutPanel1.SetColumnSpan(btnF2, 2);
      btnF2.Dock = DockStyle.Fill;
      btnF2.Location = new Point(7, 51);
      btnF2.Margin = new Padding(3, 2, 3, 2);
      btnF2.Name = "btnF2";
      btnF2.Size = new Size(158, 41);
      btnF2.TabIndex = 4;
      btnF2.Text = "Borderless \r\n(F2)";
      btnF2.UseVisualStyleBackColor = true;
      btnF2.Click += btnF2_Click;
      // 
      // btnN
      // 
      tableLayoutPanel1.SetColumnSpan(btnN, 2);
      btnN.Dock = DockStyle.Fill;
      btnN.Location = new Point(335, 6);
      btnN.Margin = new Padding(3, 2, 3, 2);
      btnN.Name = "btnN";
      btnN.Size = new Size(158, 41);
      btnN.TabIndex = 2;
      btnN.Text = "Music Info\r\n(N)";
      btnN.UseVisualStyleBackColor = true;
      btnN.Click += btnN_Click;
      // 
      // btnAltEnter
      // 
      tableLayoutPanel1.SetColumnSpan(btnAltEnter, 2);
      btnAltEnter.Dock = DockStyle.Fill;
      btnAltEnter.Location = new Point(7, 96);
      btnAltEnter.Margin = new Padding(3, 2, 3, 2);
      btnAltEnter.Name = "btnAltEnter";
      btnAltEnter.Size = new Size(158, 41);
      btnAltEnter.TabIndex = 8;
      btnAltEnter.Text = "Fullscreen\r\n(Alt+Enter)";
      btnAltEnter.UseVisualStyleBackColor = true;
      btnAltEnter.Click += btnAltEnter_Click;
      // 
      // btnY
      // 
      tableLayoutPanel1.SetColumnSpan(btnY, 2);
      btnY.Dock = DockStyle.Fill;
      btnY.Location = new Point(335, 96);
      btnY.Margin = new Padding(3, 2, 3, 2);
      btnY.Name = "btnY";
      btnY.Size = new Size(158, 41);
      btnY.TabIndex = 10;
      btnY.Text = "Message Mode\r\n(Y)";
      btnY.UseVisualStyleBackColor = true;
      btnY.Click += btnY_Click;
      // 
      // btn11
      // 
      btn11.Anchor = AnchorStyles.Top | AnchorStyles.Bottom | AnchorStyles.Left | AnchorStyles.Right;
      btn11.Location = new Point(89, 141);
      btn11.Margin = new Padding(3, 2, 3, 2);
      btn11.Name = "btn11";
      btn11.Size = new Size(76, 43);
      btn11.TabIndex = 13;
      btn11.Text = "11";
      btn11.UseVisualStyleBackColor = true;
      btn11.Click += btn11_Click;
      // 
      // btnTilde
      // 
      tableLayoutPanel1.SetColumnSpan(btnTilde, 2);
      btnTilde.Dock = DockStyle.Fill;
      btnTilde.Location = new Point(499, 6);
      btnTilde.Margin = new Padding(3, 2, 3, 2);
      btnTilde.Name = "btnTilde";
      btnTilde.Size = new Size(163, 41);
      btnTilde.TabIndex = 3;
      btnTilde.Text = "Preset Lock \r\n(~)";
      btnTilde.UseVisualStyleBackColor = true;
      btnTilde.Click += btnTilde_Click;
      // 
      // btn00
      // 
      btn00.Anchor = AnchorStyles.Top | AnchorStyles.Bottom | AnchorStyles.Left | AnchorStyles.Right;
      btn00.Location = new Point(7, 141);
      btn00.Margin = new Padding(3, 2, 3, 2);
      btn00.Name = "btn00";
      btn00.Size = new Size(76, 43);
      btn00.TabIndex = 12;
      btn00.Text = "00";
      btn00.UseVisualStyleBackColor = true;
      btn00.Click += btn00_Click;
      // 
      // btnF7
      // 
      tableLayoutPanel1.SetColumnSpan(btnF7, 2);
      btnF7.Dock = DockStyle.Fill;
      btnF7.Location = new Point(499, 51);
      btnF7.Margin = new Padding(3, 2, 3, 2);
      btnF7.Name = "btnF7";
      btnF7.Size = new Size(163, 41);
      btnF7.TabIndex = 7;
      btnF7.Text = "Always On Top\r\n(F7)";
      btnF7.UseVisualStyleBackColor = true;
      btnF7.Click += btnF7_Click;
      // 
      // btnF4
      // 
      tableLayoutPanel1.SetColumnSpan(btnF4, 2);
      btnF4.Dock = DockStyle.Fill;
      btnF4.Location = new Point(335, 51);
      btnF4.Margin = new Padding(3, 2, 3, 2);
      btnF4.Name = "btnF4";
      btnF4.Size = new Size(158, 41);
      btnF4.TabIndex = 6;
      btnF4.Text = "Preset Info\r\n(F4)";
      btnF4.UseVisualStyleBackColor = true;
      btnF4.Click += btnF4_Click;
      // 
      // btnF3
      // 
      tableLayoutPanel1.SetColumnSpan(btnF3, 2);
      btnF3.Dock = DockStyle.Fill;
      btnF3.Location = new Point(171, 51);
      btnF3.Margin = new Padding(3, 2, 3, 2);
      btnF3.Name = "btnF3";
      btnF3.Size = new Size(158, 41);
      btnF3.TabIndex = 5;
      btnF3.Text = "Change FPS\r\n(F3)";
      btnF3.UseVisualStyleBackColor = true;
      btnF3.Click += btnF3_Click;
      // 
      // btnDelete
      // 
      tableLayoutPanel1.SetColumnSpan(btnDelete, 2);
      btnDelete.Dock = DockStyle.Fill;
      btnDelete.Location = new Point(499, 96);
      btnDelete.Margin = new Padding(3, 2, 3, 2);
      btnDelete.Name = "btnDelete";
      btnDelete.Size = new Size(163, 41);
      btnDelete.TabIndex = 11;
      btnDelete.Text = "Clear Latest Sprite\r\n(Delete)";
      btnDelete.UseVisualStyleBackColor = true;
      btnDelete.Click += btnDelete_Click;
      // 
      // colorDialogWave
      // 
      colorDialogWave.AnyColor = true;
      colorDialogWave.Color = Color.White;
      colorDialogWave.FullOpen = true;
      colorDialogWave.SolidColorOnly = true;
      // 
      // tabControl1
      // 
      tabControl1.Alignment = TabAlignment.Right;
      tabControl1.Controls.Add(tabRemote);
      tabControl1.Dock = DockStyle.Fill;
      tabControl1.Location = new Point(0, 0);
      tabControl1.Multiline = true;
      tabControl1.Name = "tabControl1";
      tabControl1.Padding = new Point(0, 0);
      tabControl1.SelectedIndex = 0;
      tabControl1.Size = new Size(702, 543);
      tabControl1.TabIndex = 114;
      // 
      // tabRemote
      // 
      tabRemote.Controls.Add(splitContainer1);
      tabRemote.Location = new Point(4, 4);
      tabRemote.Margin = new Padding(0);
      tabRemote.Name = "tabRemote";
      tabRemote.Size = new Size(671, 535);
      tabRemote.TabIndex = 0;
      tabRemote.Text = "Remote";
      tabRemote.UseVisualStyleBackColor = true;
      // 
      // MilkwaveRemoteForm
      // 
      AutoScaleDimensions = new SizeF(96F, 96F);
      AutoScaleMode = AutoScaleMode.Dpi;
      ClientSize = new Size(702, 569);
      Controls.Add(tabControl1);
      Controls.Add(statusStrip1);
      Icon = (Icon)resources.GetObject("$this.Icon");
      KeyPreview = true;
      Margin = new Padding(3, 2, 3, 2);
      MinimumSize = new Size(264, 84);
      Name = "MilkwaveRemoteForm";
      Text = "Milkwave Remote";
      FormClosing += MainForm_FormClosing;
      Load += MilkwaveRemoteForm_Load;
      Shown += MainForm_Shown;
      KeyDown += MainForm_KeyDown;
      statusStrip1.ResumeLayout(false);
      statusStrip1.PerformLayout();
      ((System.ComponentModel.ISupportInitialize)numSize).EndInit();
      ((System.ComponentModel.ISupportInitialize)numBPM).EndInit();
      ((System.ComponentModel.ISupportInitialize)numBeats).EndInit();
      ((System.ComponentModel.ISupportInitialize)numAmpLeft).EndInit();
      ((System.ComponentModel.ISupportInitialize)numAmpRight).EndInit();
      ((System.ComponentModel.ISupportInitialize)numWavemode).EndInit();
      ((System.ComponentModel.ISupportInitialize)numWrap).EndInit();
      ((System.ComponentModel.ISupportInitialize)numAlpha).EndInit();
      splitContainer1.Panel1.ResumeLayout(false);
      splitContainer1.Panel1.PerformLayout();
      splitContainer1.Panel2.ResumeLayout(false);
      ((System.ComponentModel.ISupportInitialize)splitContainer1).EndInit();
      splitContainer1.ResumeLayout(false);
      splitContainer2.Panel1.ResumeLayout(false);
      splitContainer2.Panel1.PerformLayout();
      splitContainer2.Panel2.ResumeLayout(false);
      ((System.ComponentModel.ISupportInitialize)splitContainer2).EndInit();
      splitContainer2.ResumeLayout(false);
      tableLayoutPanel1.ResumeLayout(false);
      tabControl1.ResumeLayout(false);
      tabRemote.ResumeLayout(false);
      ResumeLayout(false);
      PerformLayout();
    }

    private void TxtBPM_TextChanged(object sender, EventArgs e) {
      throw new NotImplementedException();
    }

    #endregion
    private StatusStrip statusStrip1;
    private ToolStripStatusLabel statusBar;
    private ToolTip toolTip1;
    private ColorDialog colorDialogMessage;
    private ToolStripStatusLabel statusHelp;
    private ToolStripStatusLabel statusSupporters;
    private SplitContainer splitContainer1;
    private SplitContainer splitContainer2;
    private CheckBox chkPreview;
    private Button btnAppendSize;
    private NumericUpDown numSize;
    private Label lblSize;
    private Label lblStyle;
    private TextBox txtStyle;
    private NumericUpDown numBPM;
    private Label lblBPM;
    private CheckBox chkFileRandom;
    private Panel pnlColorMessage;
    private Button btnAppendColor;
    private Label lblColor;
    private Button btnFontAppend;
    private Label lblFont;
    private ComboBox cboFonts;
    private NumericUpDown numBeats;
    private Label label7;
    private Label lblFromFile;
    private CheckBox chkAutoplay;
    private TextBox txtAutoplay;
    private Button btnSaveParam;
    private Label lblParameters;
    private ComboBox cboParameters;
    private Label label2;
    private Label lblWindow;
    private TextBox txtWindowTitle;
    private Button btnSend;
    private TextBox txtMessage;
    private TableLayoutPanel tableLayoutPanel1;
    private Button btn77;
    private Button btn66;
    private Button btn55;
    private Button btn44;
    private Button btn33;
    private Button btn22;
    private Button btnK;
    private Button btnF2;
    private Button btnN;
    private Button btnAltEnter;
    private Button btnY;
    private Button btn11;
    private Button btnTilde;
    private Button btn00;
    private Button btnSpace;
    private Button btnF7;
    private Button btnF4;
    private Button btnF3;
    private Button btnBackspace;
    private Button btnDelete;
    private Label lblCurrentPreset;
    private TextBox txtVisRunning;
    private ToolStripDropDownButton toolStripDropDownButton;
    private ToolStripMenuItem toolStripMenuItemHelp;
    private ToolStripMenuItem toolStripMenuItemSupporters;
    private ToolStripMenuItem toolStripMenuItemHomepage;
    private ToolStripSeparator toolStripSeparator1;
    private ToolStripMenuItem toolStripMenuItemDarkMode;
    private ToolStripMenuItem toolStripMenuItemButtonPanel;
    private ToolStripMenuItem toolStripMenuItemPresetPanel;
    private ToolStripSeparator toolStripSeparator2;
    private ToolStripMenuItem toolStripMenuItemMessagePanel;
    private Label lblPreset;
    private ComboBox cboPresets;
    private Button btnPresetLoadFile;
    private Button btnPresetSend;
    private Button btnPresetLoadDirectory;
    private TextBox txtDirFilter;
    private NumericUpDown numAmpLeft;
    private Label lblAmp;
    private NumericUpDown numAmpRight;
    private Label lblAmpRight;
    private Button btnSendWave;
    private NumericUpDown numWavemode;
    private Label lblWavemode;
    private Panel pnlColorWave;
    private Label lblWaveColor;
    private ColorDialog colorDialogWave;
    private CheckBox chkAmpLinked;
    private TabControl tabControl1;
    private TabPage tabRemote;
    private NumericUpDown numWrap;
    private CheckBox chkWrap;
    private Button btnSetAudioDevice;
    private Label lblAudioDevice;
    private ComboBox cboAudioDevice;
    private NumericUpDown numAlpha;
  }
}
