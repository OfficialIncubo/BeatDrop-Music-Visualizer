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
      toolStripMenuItemHomepage = new ToolStripMenuItem();
      toolStripMenuItemSupporters = new ToolStripMenuItem();
      toolStripMenuItemHelp = new ToolStripMenuItem();
      toolStripSeparator1 = new ToolStripSeparator();
      toolStripMenuItemDarkMode = new ToolStripMenuItem();
      toolStripSeparator2 = new ToolStripSeparator();
      toolStripMenuItemMessagePanel = new ToolStripMenuItem();
      toolStripMenuItemPresetPanel = new ToolStripMenuItem();
      toolStripMenuItemButtonPanel = new ToolStripMenuItem();
      toolTip1 = new ToolTip(components);
      chkPreview = new CheckBox();
      btnAppendSize = new Button();
      txtSize = new NumericUpDown();
      lblSize = new Label();
      lblStyle = new Label();
      txtBPM = new NumericUpDown();
      lblBPM = new Label();
      chkFileRandom = new CheckBox();
      pnlColor = new Panel();
      btnAppendColor = new Button();
      lblColor = new Label();
      btnFontAppend = new Button();
      lblFont = new Label();
      cboFonts = new ComboBox();
      txtBeats = new NumericUpDown();
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
      txtVis = new TextBox();
      cboPresets = new ComboBox();
      lblCurrentPreset = new Label();
      colorDialog1 = new ColorDialog();
      splitContainer1 = new SplitContainer();
      txtStyle = new TextBox();
      label5 = new Label();
      label2 = new Label();
      splitContainer2 = new SplitContainer();
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
      statusStrip1.SuspendLayout();
      ((System.ComponentModel.ISupportInitialize)txtSize).BeginInit();
      ((System.ComponentModel.ISupportInitialize)txtBPM).BeginInit();
      ((System.ComponentModel.ISupportInitialize)txtBeats).BeginInit();
      ((System.ComponentModel.ISupportInitialize)splitContainer1).BeginInit();
      splitContainer1.Panel1.SuspendLayout();
      splitContainer1.Panel2.SuspendLayout();
      splitContainer1.SuspendLayout();
      ((System.ComponentModel.ISupportInitialize)splitContainer2).BeginInit();
      splitContainer2.Panel1.SuspendLayout();
      splitContainer2.Panel2.SuspendLayout();
      splitContainer2.SuspendLayout();
      tableLayoutPanel1.SuspendLayout();
      SuspendLayout();
      // 
      // statusStrip1
      // 
      statusStrip1.ImageScalingSize = new Size(20, 20);
      statusStrip1.Items.AddRange(new ToolStripItem[] { statusBar, toolStripDropDownButton });
      statusStrip1.Location = new Point(0, 579);
      statusStrip1.Name = "statusStrip1";
      statusStrip1.Size = new Size(869, 26);
      statusStrip1.TabIndex = 5;
      statusStrip1.Text = "statusStrip1";
      // 
      // statusBar
      // 
      statusBar.Margin = new Padding(7, 4, 0, 2);
      statusBar.Name = "statusBar";
      statusBar.Size = new Size(752, 20);
      statusBar.Spring = true;
      statusBar.TextAlign = ContentAlignment.TopLeft;
      // 
      // toolStripDropDownButton
      // 
      toolStripDropDownButton.DropDownItems.AddRange(new ToolStripItem[] { toolStripMenuItemHomepage, toolStripMenuItemSupporters, toolStripMenuItemHelp, toolStripSeparator1, toolStripMenuItemDarkMode, toolStripSeparator2, toolStripMenuItemMessagePanel, toolStripMenuItemPresetPanel, toolStripMenuItemButtonPanel });
      toolStripDropDownButton.Image = (Image)resources.GetObject("toolStripDropDownButton.Image");
      toolStripDropDownButton.ImageTransparentColor = Color.Magenta;
      toolStripDropDownButton.Name = "toolStripDropDownButton";
      toolStripDropDownButton.ShowDropDownArrow = false;
      toolStripDropDownButton.Size = new Size(95, 24);
      toolStripDropDownButton.Text = "Milkwave";
      // 
      // toolStripMenuItemHomepage
      // 
      toolStripMenuItemHomepage.Name = "toolStripMenuItemHomepage";
      toolStripMenuItemHomepage.Size = new Size(189, 26);
      toolStripMenuItemHomepage.Text = "Releases";
      toolStripMenuItemHomepage.Click += toolStripMenuItemReleases_Click;
      // 
      // toolStripMenuItemSupporters
      // 
      toolStripMenuItemSupporters.Name = "toolStripMenuItemSupporters";
      toolStripMenuItemSupporters.Size = new Size(189, 26);
      toolStripMenuItemSupporters.Text = "Supporters";
      toolStripMenuItemSupporters.Click += toolStripMenuItemSupporters_Click;
      // 
      // toolStripMenuItemHelp
      // 
      toolStripMenuItemHelp.Name = "toolStripMenuItemHelp";
      toolStripMenuItemHelp.Size = new Size(189, 26);
      toolStripMenuItemHelp.Text = "Help";
      toolStripMenuItemHelp.Click += toolStripMenuItemHelp_Click;
      // 
      // toolStripSeparator1
      // 
      toolStripSeparator1.Name = "toolStripSeparator1";
      toolStripSeparator1.Size = new Size(186, 6);
      // 
      // toolStripMenuItemDarkMode
      // 
      toolStripMenuItemDarkMode.Checked = true;
      toolStripMenuItemDarkMode.CheckState = CheckState.Checked;
      toolStripMenuItemDarkMode.Name = "toolStripMenuItemDarkMode";
      toolStripMenuItemDarkMode.Size = new Size(189, 26);
      toolStripMenuItemDarkMode.Text = "Dark Mode";
      toolStripMenuItemDarkMode.Click += toolStripMenuItemDarkMode_Click;
      // 
      // toolStripSeparator2
      // 
      toolStripSeparator2.Name = "toolStripSeparator2";
      toolStripSeparator2.Size = new Size(186, 6);
      // 
      // toolStripMenuItemMessagePanel
      // 
      toolStripMenuItemMessagePanel.Checked = true;
      toolStripMenuItemMessagePanel.CheckState = CheckState.Checked;
      toolStripMenuItemMessagePanel.Name = "toolStripMenuItemMessagePanel";
      toolStripMenuItemMessagePanel.Size = new Size(189, 26);
      toolStripMenuItemMessagePanel.Text = "Message Panel";
      toolStripMenuItemMessagePanel.Click += toolStripMenuItemMessagePanel_Click;
      // 
      // toolStripMenuItemPresetPanel
      // 
      toolStripMenuItemPresetPanel.Checked = true;
      toolStripMenuItemPresetPanel.CheckState = CheckState.Checked;
      toolStripMenuItemPresetPanel.Name = "toolStripMenuItemPresetPanel";
      toolStripMenuItemPresetPanel.Size = new Size(189, 26);
      toolStripMenuItemPresetPanel.Text = "Preset Panel";
      toolStripMenuItemPresetPanel.Click += toolStripMenuItemPresetPanel_Click;
      // 
      // toolStripMenuItemButtonPanel
      // 
      toolStripMenuItemButtonPanel.Checked = true;
      toolStripMenuItemButtonPanel.CheckState = CheckState.Checked;
      toolStripMenuItemButtonPanel.Name = "toolStripMenuItemButtonPanel";
      toolStripMenuItemButtonPanel.Size = new Size(189, 26);
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
      chkPreview.Location = new Point(663, 92);
      chkPreview.Name = "chkPreview";
      chkPreview.Size = new Size(105, 27);
      chkPreview.TabIndex = 73;
      chkPreview.Text = "Preview";
      chkPreview.TextAlign = ContentAlignment.MiddleCenter;
      chkPreview.TextImageRelation = TextImageRelation.ImageAboveText;
      toolTip1.SetToolTip(chkPreview, "Play random line");
      chkPreview.UseVisualStyleBackColor = true;
      chkPreview.CheckedChanged += chkPreview_CheckedChanged;
      // 
      // btnAppendSize
      // 
      btnAppendSize.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      btnAppendSize.Location = new Point(550, 159);
      btnAppendSize.Margin = new Padding(0);
      btnAppendSize.Name = "btnAppendSize";
      btnAppendSize.Size = new Size(53, 29);
      btnAppendSize.TabIndex = 81;
      btnAppendSize.Text = "Set";
      toolTip1.SetToolTip(btnAppendSize, "Append to (or update in) parameters line");
      btnAppendSize.UseVisualStyleBackColor = true;
      btnAppendSize.Click += btnAppendSize_Click;
      // 
      // txtSize
      // 
      txtSize.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      txtSize.Location = new Point(491, 160);
      txtSize.Maximum = new decimal(new int[] { 999, 0, 0, 0 });
      txtSize.Name = "txtSize";
      txtSize.Size = new Size(53, 27);
      txtSize.TabIndex = 80;
      txtSize.TextAlign = HorizontalAlignment.Center;
      toolTip1.SetToolTip(txtSize, "Font size");
      txtSize.Value = new decimal(new int[] { 30, 0, 0, 0 });
      txtSize.TextChanged += txtSize_TextChanged;
      txtSize.KeyDown += txtSize_KeyDown;
      // 
      // lblSize
      // 
      lblSize.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      lblSize.AutoSize = true;
      lblSize.Location = new Point(452, 163);
      lblSize.Name = "lblSize";
      lblSize.Size = new Size(36, 20);
      lblSize.TabIndex = 98;
      lblSize.Text = "Size";
      toolTip1.SetToolTip(lblSize, "Only used if no size parameter supplied\r\nDouble-click to clear the size value from the parameters line\r\n");
      lblSize.DoubleClick += lblSize_DoubleClick;
      // 
      // lblStyle
      // 
      lblStyle.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      lblStyle.AutoSize = true;
      lblStyle.Location = new Point(618, 128);
      lblStyle.Name = "lblStyle";
      lblStyle.Size = new Size(41, 20);
      lblStyle.TabIndex = 75;
      lblStyle.Text = "Style";
      toolTip1.SetToolTip(lblStyle, "Double-click to remove this style\r\nRight-click to fill form elements from current parameters");
      lblStyle.DoubleClick += lblStyle_DoubleClick;
      lblStyle.MouseClick += lblStyle_MouseClick;
      // 
      // txtBPM
      // 
      txtBPM.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      txtBPM.Location = new Point(550, 194);
      txtBPM.Maximum = new decimal(new int[] { 999, 0, 0, 0 });
      txtBPM.Name = "txtBPM";
      txtBPM.Size = new Size(53, 27);
      txtBPM.TabIndex = 85;
      txtBPM.TextAlign = HorizontalAlignment.Center;
      toolTip1.SetToolTip(txtBPM, "BPM");
      txtBPM.Value = new decimal(new int[] { 120, 0, 0, 0 });
      txtBPM.TextChanged += txtBPM_TextChanged;
      // 
      // lblBPM
      // 
      lblBPM.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      lblBPM.AutoSize = true;
      lblBPM.Location = new Point(505, 197);
      lblBPM.Name = "lblBPM";
      lblBPM.Size = new Size(39, 20);
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
      chkFileRandom.Location = new Point(711, 192);
      chkFileRandom.Name = "chkFileRandom";
      chkFileRandom.Size = new Size(58, 30);
      chkFileRandom.TabIndex = 87;
      chkFileRandom.Text = "Rand";
      chkFileRandom.TextAlign = ContentAlignment.MiddleCenter;
      chkFileRandom.TextImageRelation = TextImageRelation.ImageAboveText;
      toolTip1.SetToolTip(chkFileRandom, "Play random line");
      chkFileRandom.UseVisualStyleBackColor = true;
      // 
      // pnlColor
      // 
      pnlColor.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      pnlColor.BorderStyle = BorderStyle.FixedSingle;
      pnlColor.Location = new Point(663, 160);
      pnlColor.Name = "pnlColor";
      pnlColor.Size = new Size(44, 27);
      pnlColor.TabIndex = 82;
      toolTip1.SetToolTip(pnlColor, "Only used if no r, g, b parameters supplied");
      pnlColor.Click += pnlColor_Click;
      // 
      // btnAppendColor
      // 
      btnAppendColor.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      btnAppendColor.Location = new Point(712, 159);
      btnAppendColor.Margin = new Padding(0);
      btnAppendColor.Name = "btnAppendColor";
      btnAppendColor.Size = new Size(56, 29);
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
      lblColor.Location = new Point(614, 163);
      lblColor.Name = "lblColor";
      lblColor.Size = new Size(45, 20);
      lblColor.TabIndex = 96;
      lblColor.Text = "Color";
      toolTip1.SetToolTip(lblColor, "Only used if no r, g, b parameters supplied\r\nDouble-click to clear the color information from the parameters line\r\n");
      lblColor.DoubleClick += lblColor_DoubleClick;
      // 
      // btnFontAppend
      // 
      btnFontAppend.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      btnFontAppend.Location = new Point(377, 159);
      btnFontAppend.Margin = new Padding(0);
      btnFontAppend.Name = "btnFontAppend";
      btnFontAppend.Size = new Size(56, 29);
      btnFontAppend.TabIndex = 79;
      btnFontAppend.Text = "Set";
      toolTip1.SetToolTip(btnFontAppend, "Append to (or update in) parameters line");
      btnFontAppend.UseVisualStyleBackColor = true;
      btnFontAppend.Click += btnFontAppend_Click;
      // 
      // lblFont
      // 
      lblFont.AutoSize = true;
      lblFont.Location = new Point(50, 163);
      lblFont.Name = "lblFont";
      lblFont.Size = new Size(38, 20);
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
      cboFonts.FormattingEnabled = true;
      cboFonts.Location = new Point(94, 160);
      cboFonts.Name = "cboFonts";
      cboFonts.Size = new Size(278, 28);
      cboFonts.TabIndex = 78;
      toolTip1.SetToolTip(cboFonts, "Only used if no font parameter supplied");
      cboFonts.SelectedIndexChanged += cboFonts_SelectedIndexChanged;
      cboFonts.KeyDown += cboFonts_KeyDown;
      // 
      // txtBeats
      // 
      txtBeats.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      txtBeats.Location = new Point(662, 194);
      txtBeats.Maximum = new decimal(new int[] { 99, 0, 0, 0 });
      txtBeats.Name = "txtBeats";
      txtBeats.Size = new Size(44, 27);
      txtBeats.TabIndex = 86;
      txtBeats.TextAlign = HorizontalAlignment.Center;
      toolTip1.SetToolTip(txtBeats, "Wait time between lines in beats");
      txtBeats.Value = new decimal(new int[] { 8, 0, 0, 0 });
      // 
      // label7
      // 
      label7.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      label7.AutoSize = true;
      label7.Location = new Point(614, 197);
      label7.Name = "label7";
      label7.Size = new Size(45, 20);
      label7.TabIndex = 94;
      label7.Text = "Beats";
      toolTip1.SetToolTip(label7, "Wait time between lines in beats");
      // 
      // lblFromFile
      // 
      lblFromFile.AutoSize = true;
      lblFromFile.Location = new Point(19, 196);
      lblFromFile.Name = "lblFromFile";
      lblFromFile.Size = new Size(70, 20);
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
      chkAutoplay.Location = new Point(773, 157);
      chkAutoplay.Name = "chkAutoplay";
      chkAutoplay.Size = new Size(89, 65);
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
      txtAutoplay.Location = new Point(94, 194);
      txtAutoplay.Name = "txtAutoplay";
      txtAutoplay.ReadOnly = true;
      txtAutoplay.Size = new Size(391, 27);
      txtAutoplay.TabIndex = 84;
      toolTip1.SetToolTip(txtAutoplay, "Next line from file script-default.txt");
      // 
      // btnSaveParam
      // 
      btnSaveParam.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      btnSaveParam.FlatStyle = FlatStyle.System;
      btnSaveParam.Location = new Point(773, 125);
      btnSaveParam.Margin = new Padding(0);
      btnSaveParam.Name = "btnSaveParam";
      btnSaveParam.Size = new Size(88, 27);
      btnSaveParam.TabIndex = 77;
      btnSaveParam.Text = "Save";
      toolTip1.SetToolTip(btnSaveParam, "Save current parameters as style");
      btnSaveParam.UseVisualStyleBackColor = true;
      btnSaveParam.Click += btnSaveParam_Click;
      // 
      // lblParameters
      // 
      lblParameters.AutoSize = true;
      lblParameters.Location = new Point(6, 128);
      lblParameters.Name = "lblParameters";
      lblParameters.Size = new Size(82, 20);
      lblParameters.TabIndex = 91;
      lblParameters.Text = "Parameters";
      toolTip1.SetToolTip(lblParameters, "Double-click to clear all saved styles");
      lblParameters.DoubleClick += lblParameters_DoubleClick;
      // 
      // cboParameters
      // 
      cboParameters.Anchor = AnchorStyles.Top | AnchorStyles.Left | AnchorStyles.Right;
      cboParameters.DropDownStyle = ComboBoxStyle.DropDownList;
      cboParameters.Location = new Point(94, 125);
      cboParameters.Name = "cboParameters";
      cboParameters.Size = new Size(509, 28);
      cboParameters.TabIndex = 74;
      toolTip1.SetToolTip(cboParameters, resources.GetString("cboParameters.ToolTip"));
      cboParameters.SelectedIndexChanged += cboParameters_SelectedIndexChanged;
      cboParameters.TextChanged += cboParameters_TextChanged;
      cboParameters.KeyDown += cboParameters_KeyDown;
      // 
      // lblWindow
      // 
      lblWindow.AutoSize = true;
      lblWindow.Location = new Point(24, 95);
      lblWindow.Name = "lblWindow";
      lblWindow.Size = new Size(64, 20);
      lblWindow.TabIndex = 89;
      lblWindow.Text = "Window";
      toolTip1.SetToolTip(lblWindow, "Double-click to restore default Visualizer window position and size\r\nHold Ctrl while closing this window to clear all locally saved settings");
      lblWindow.DoubleClick += lblWindow_DoubleClick;
      // 
      // txtWindowTitle
      // 
      txtWindowTitle.Anchor = AnchorStyles.Top | AnchorStyles.Left | AnchorStyles.Right;
      txtWindowTitle.Location = new Point(94, 92);
      txtWindowTitle.Name = "txtWindowTitle";
      txtWindowTitle.Size = new Size(373, 27);
      txtWindowTitle.TabIndex = 72;
      txtWindowTitle.Text = "Milkwave Visualizer";
      toolTip1.SetToolTip(txtWindowTitle, "Partitial window title of Milkwave Visualizer window");
      // 
      // btnSend
      // 
      btnSend.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      btnSend.FlatStyle = FlatStyle.System;
      btnSend.Location = new Point(773, 9);
      btnSend.Margin = new Padding(0);
      btnSend.Name = "btnSend";
      btnSend.Size = new Size(88, 77);
      btnSend.TabIndex = 71;
      btnSend.Text = "Send";
      toolTip1.SetToolTip(btnSend, "Send to Visualizer (F2)");
      btnSend.UseVisualStyleBackColor = true;
      btnSend.Click += btnSend_Click;
      // 
      // txtMessage
      // 
      txtMessage.Anchor = AnchorStyles.Top | AnchorStyles.Left | AnchorStyles.Right;
      txtMessage.Location = new Point(94, 9);
      txtMessage.Multiline = true;
      txtMessage.Name = "txtMessage";
      txtMessage.Size = new Size(674, 77);
      txtMessage.TabIndex = 70;
      txtMessage.Text = "Hi from Milkwave Remote!";
      toolTip1.SetToolTip(txtMessage, "F1");
      txtMessage.KeyDown += txtMessage_KeyDown;
      // 
      // btnSpace
      // 
      tableLayoutPanel1.SetColumnSpan(btnSpace, 2);
      btnSpace.Dock = DockStyle.Fill;
      btnSpace.Location = new Point(8, 8);
      btnSpace.Name = "btnSpace";
      btnSpace.Size = new Size(208, 48);
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
      btnBackspace.Location = new Point(222, 8);
      btnBackspace.Name = "btnBackspace";
      btnBackspace.Size = new Size(208, 48);
      btnBackspace.TabIndex = 1;
      btnBackspace.Text = "Previous Preset\r\n(Backspace)";
      toolTip1.SetToolTip(btnBackspace, "F4");
      btnBackspace.UseVisualStyleBackColor = true;
      btnBackspace.Click += btnBackspace_Click;
      // 
      // lblPreset
      // 
      lblPreset.AutoSize = true;
      lblPreset.Location = new Point(39, 12);
      lblPreset.Name = "lblPreset";
      lblPreset.Size = new Size(49, 20);
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
      btnPresetLoadFile.Location = new Point(588, 8);
      btnPresetLoadFile.Margin = new Padding(0);
      btnPresetLoadFile.Name = "btnPresetLoadFile";
      btnPresetLoadFile.Size = new Size(88, 30);
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
      btnPresetSend.Location = new Point(773, 8);
      btnPresetSend.Margin = new Padding(0);
      btnPresetSend.Name = "btnPresetSend";
      btnPresetSend.Size = new Size(88, 30);
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
      btnPresetLoadDirectory.Location = new Point(680, 8);
      btnPresetLoadDirectory.Margin = new Padding(0);
      btnPresetLoadDirectory.Name = "btnPresetLoadDirectory";
      btnPresetLoadDirectory.Size = new Size(88, 30);
      btnPresetLoadDirectory.TabIndex = 101;
      btnPresetLoadDirectory.Text = "Directory";
      toolTip1.SetToolTip(btnPresetLoadDirectory, "Load all presets from a directory");
      btnPresetLoadDirectory.UseVisualStyleBackColor = true;
      btnPresetLoadDirectory.Click += btnPresetLoadDirectory_Click;
      // 
      // txtVis
      // 
      txtVis.Anchor = AnchorStyles.Top | AnchorStyles.Left | AnchorStyles.Right;
      txtVis.Location = new Point(94, 43);
      txtVis.Name = "txtVis";
      txtVis.ReadOnly = true;
      txtVis.Size = new Size(767, 27);
      txtVis.TabIndex = 96;
      toolTip1.SetToolTip(txtVis, "Currently running Visualizer preset");
      // 
      // cboPresets
      // 
      cboPresets.Anchor = AnchorStyles.Top | AnchorStyles.Left | AnchorStyles.Right;
      cboPresets.AutoCompleteMode = AutoCompleteMode.SuggestAppend;
      cboPresets.AutoCompleteSource = AutoCompleteSource.ListItems;
      cboPresets.FormattingEnabled = true;
      cboPresets.Location = new Point(94, 9);
      cboPresets.Name = "cboPresets";
      cboPresets.Size = new Size(490, 28);
      cboPresets.TabIndex = 97;
      toolTip1.SetToolTip(cboPresets, "Alt+Mousewheel: Send to Visualizer");
      cboPresets.SelectedIndexChanged += cboPresets_SelectedIndexChanged;
      cboPresets.KeyDown += cboPresets_KeyDown;
      // 
      // lblCurrentPreset
      // 
      lblCurrentPreset.AutoSize = true;
      lblCurrentPreset.Location = new Point(25, 46);
      lblCurrentPreset.Name = "lblCurrentPreset";
      lblCurrentPreset.Size = new Size(63, 20);
      lblCurrentPreset.TabIndex = 95;
      lblCurrentPreset.Text = "Running";
      // 
      // colorDialog1
      // 
      colorDialog1.AnyColor = true;
      colorDialog1.Color = Color.White;
      colorDialog1.FullOpen = true;
      colorDialog1.SolidColorOnly = true;
      // 
      // splitContainer1
      // 
      splitContainer1.Dock = DockStyle.Fill;
      splitContainer1.Location = new Point(0, 0);
      splitContainer1.Margin = new Padding(0);
      splitContainer1.Name = "splitContainer1";
      splitContainer1.Orientation = Orientation.Horizontal;
      // 
      // splitContainer1.Panel1
      // 
      splitContainer1.Panel1.Controls.Add(chkPreview);
      splitContainer1.Panel1.Controls.Add(btnAppendSize);
      splitContainer1.Panel1.Controls.Add(txtSize);
      splitContainer1.Panel1.Controls.Add(lblSize);
      splitContainer1.Panel1.Controls.Add(lblStyle);
      splitContainer1.Panel1.Controls.Add(txtStyle);
      splitContainer1.Panel1.Controls.Add(txtBPM);
      splitContainer1.Panel1.Controls.Add(lblBPM);
      splitContainer1.Panel1.Controls.Add(chkFileRandom);
      splitContainer1.Panel1.Controls.Add(pnlColor);
      splitContainer1.Panel1.Controls.Add(btnAppendColor);
      splitContainer1.Panel1.Controls.Add(lblColor);
      splitContainer1.Panel1.Controls.Add(btnFontAppend);
      splitContainer1.Panel1.Controls.Add(lblFont);
      splitContainer1.Panel1.Controls.Add(cboFonts);
      splitContainer1.Panel1.Controls.Add(txtBeats);
      splitContainer1.Panel1.Controls.Add(label7);
      splitContainer1.Panel1.Controls.Add(lblFromFile);
      splitContainer1.Panel1.Controls.Add(chkAutoplay);
      splitContainer1.Panel1.Controls.Add(txtAutoplay);
      splitContainer1.Panel1.Controls.Add(btnSaveParam);
      splitContainer1.Panel1.Controls.Add(label5);
      splitContainer1.Panel1.Controls.Add(lblParameters);
      splitContainer1.Panel1.Controls.Add(cboParameters);
      splitContainer1.Panel1.Controls.Add(label2);
      splitContainer1.Panel1.Controls.Add(lblWindow);
      splitContainer1.Panel1.Controls.Add(txtWindowTitle);
      splitContainer1.Panel1.Controls.Add(btnSend);
      splitContainer1.Panel1.Controls.Add(txtMessage);
      splitContainer1.Panel1.Paint += splitContainer1_Panel1_Paint;
      splitContainer1.Panel1MinSize = 0;
      // 
      // splitContainer1.Panel2
      // 
      splitContainer1.Panel2.Controls.Add(splitContainer2);
      splitContainer1.Panel2MinSize = 0;
      splitContainer1.Size = new Size(869, 579);
      splitContainer1.SplitterDistance = 244;
      splitContainer1.SplitterWidth = 10;
      splitContainer1.TabIndex = 28;
      splitContainer1.SplitterMoved += splitContainer1_SplitterMoved;
      splitContainer1.SizeChanged += splitContainer1_SizeChanged;
      // 
      // txtStyle
      // 
      txtStyle.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      txtStyle.Location = new Point(663, 125);
      txtStyle.Name = "txtStyle";
      txtStyle.Size = new Size(105, 27);
      txtStyle.TabIndex = 76;
      txtStyle.Text = "Style A";
      txtStyle.KeyDown += txtStyle_KeyDown;
      // 
      // label5
      // 
      label5.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      label5.AutoSize = true;
      label5.Location = new Point(473, 95);
      label5.Name = "label5";
      label5.Size = new Size(116, 20);
      label5.TabIndex = 92;
      label5.Text = "(partitial match)";
      // 
      // label2
      // 
      label2.AutoSize = true;
      label2.Location = new Point(21, 12);
      label2.Name = "label2";
      label2.Size = new Size(67, 20);
      label2.TabIndex = 90;
      label2.Text = "Message";
      // 
      // splitContainer2
      // 
      splitContainer2.Dock = DockStyle.Fill;
      splitContainer2.Location = new Point(0, 0);
      splitContainer2.Margin = new Padding(0);
      splitContainer2.Name = "splitContainer2";
      splitContainer2.Orientation = Orientation.Horizontal;
      // 
      // splitContainer2.Panel1
      // 
      splitContainer2.Panel1.Controls.Add(btnPresetLoadDirectory);
      splitContainer2.Panel1.Controls.Add(btnPresetSend);
      splitContainer2.Panel1.Controls.Add(btnPresetLoadFile);
      splitContainer2.Panel1.Controls.Add(lblPreset);
      splitContainer2.Panel1.Controls.Add(cboPresets);
      splitContainer2.Panel1.Controls.Add(txtVis);
      splitContainer2.Panel1.Controls.Add(lblCurrentPreset);
      splitContainer2.Panel1.Paint += splitContainer2_Panel1_Paint;
      splitContainer2.Panel1MinSize = 44;
      // 
      // splitContainer2.Panel2
      // 
      splitContainer2.Panel2.Controls.Add(tableLayoutPanel1);
      splitContainer2.Panel2.Paint += splitContainer2_Panel2_Paint;
      splitContainer2.Panel2MinSize = 0;
      splitContainer2.Size = new Size(869, 325);
      splitContainer2.SplitterDistance = 89;
      splitContainer2.SplitterWidth = 10;
      splitContainer2.TabIndex = 0;
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
      tableLayoutPanel1.Name = "tableLayoutPanel1";
      tableLayoutPanel1.Padding = new Padding(5);
      tableLayoutPanel1.RowCount = 4;
      tableLayoutPanel1.RowStyles.Add(new RowStyle(SizeType.Percent, 25F));
      tableLayoutPanel1.RowStyles.Add(new RowStyle(SizeType.Percent, 25F));
      tableLayoutPanel1.RowStyles.Add(new RowStyle(SizeType.Percent, 25F));
      tableLayoutPanel1.RowStyles.Add(new RowStyle(SizeType.Percent, 25F));
      tableLayoutPanel1.RowStyles.Add(new RowStyle(SizeType.Absolute, 20F));
      tableLayoutPanel1.Size = new Size(869, 226);
      tableLayoutPanel1.TabIndex = 34;
      tableLayoutPanel1.Paint += tableLayoutPanel1_Paint;
      // 
      // btn77
      // 
      btn77.Anchor = AnchorStyles.Top | AnchorStyles.Bottom | AnchorStyles.Left | AnchorStyles.Right;
      btn77.Location = new Point(757, 170);
      btn77.Name = "btn77";
      btn77.Size = new Size(104, 48);
      btn77.TabIndex = 19;
      btn77.Text = "77";
      btn77.UseVisualStyleBackColor = true;
      btn77.Click += btn77_Click;
      // 
      // btn66
      // 
      btn66.Anchor = AnchorStyles.Top | AnchorStyles.Bottom | AnchorStyles.Left | AnchorStyles.Right;
      btn66.Location = new Point(650, 170);
      btn66.Name = "btn66";
      btn66.Size = new Size(101, 48);
      btn66.TabIndex = 18;
      btn66.Text = "66";
      btn66.UseVisualStyleBackColor = true;
      btn66.Click += btn66_Click;
      // 
      // btn55
      // 
      btn55.Anchor = AnchorStyles.Top | AnchorStyles.Bottom | AnchorStyles.Left | AnchorStyles.Right;
      btn55.Location = new Point(543, 170);
      btn55.Name = "btn55";
      btn55.Size = new Size(101, 48);
      btn55.TabIndex = 17;
      btn55.Text = "55";
      btn55.UseVisualStyleBackColor = true;
      btn55.Click += btn55_Click;
      // 
      // btn44
      // 
      btn44.Anchor = AnchorStyles.Top | AnchorStyles.Bottom | AnchorStyles.Left | AnchorStyles.Right;
      btn44.Location = new Point(436, 170);
      btn44.Name = "btn44";
      btn44.Size = new Size(101, 48);
      btn44.TabIndex = 16;
      btn44.Text = "44";
      btn44.UseVisualStyleBackColor = true;
      btn44.Click += btn44_Click;
      // 
      // btn33
      // 
      btn33.Anchor = AnchorStyles.Top | AnchorStyles.Bottom | AnchorStyles.Left | AnchorStyles.Right;
      btn33.Location = new Point(329, 170);
      btn33.Name = "btn33";
      btn33.Size = new Size(101, 48);
      btn33.TabIndex = 15;
      btn33.Text = "33";
      btn33.UseVisualStyleBackColor = true;
      btn33.Click += btn33_Click;
      // 
      // btn22
      // 
      btn22.Anchor = AnchorStyles.Top | AnchorStyles.Bottom | AnchorStyles.Left | AnchorStyles.Right;
      btn22.Location = new Point(222, 170);
      btn22.Name = "btn22";
      btn22.Size = new Size(101, 48);
      btn22.TabIndex = 14;
      btn22.Text = "22";
      btn22.UseVisualStyleBackColor = true;
      btn22.Click += btn22_Click;
      // 
      // btnK
      // 
      tableLayoutPanel1.SetColumnSpan(btnK, 2);
      btnK.Dock = DockStyle.Fill;
      btnK.Location = new Point(222, 116);
      btnK.Name = "btnK";
      btnK.Size = new Size(208, 48);
      btnK.TabIndex = 9;
      btnK.Text = "Sprite Mode\r\n(K)";
      btnK.UseVisualStyleBackColor = true;
      btnK.Click += btnK_Click;
      // 
      // btnF2
      // 
      tableLayoutPanel1.SetColumnSpan(btnF2, 2);
      btnF2.Dock = DockStyle.Fill;
      btnF2.Location = new Point(8, 62);
      btnF2.Name = "btnF2";
      btnF2.Size = new Size(208, 48);
      btnF2.TabIndex = 4;
      btnF2.Text = "Borderless \r\n(F2)";
      btnF2.UseVisualStyleBackColor = true;
      btnF2.Click += btnF2_Click;
      // 
      // btnN
      // 
      tableLayoutPanel1.SetColumnSpan(btnN, 2);
      btnN.Dock = DockStyle.Fill;
      btnN.Location = new Point(436, 8);
      btnN.Name = "btnN";
      btnN.Size = new Size(208, 48);
      btnN.TabIndex = 2;
      btnN.Text = "Music Info\r\n(N)";
      btnN.UseVisualStyleBackColor = true;
      btnN.Click += btnN_Click;
      // 
      // btnAltEnter
      // 
      tableLayoutPanel1.SetColumnSpan(btnAltEnter, 2);
      btnAltEnter.Dock = DockStyle.Fill;
      btnAltEnter.Location = new Point(8, 116);
      btnAltEnter.Name = "btnAltEnter";
      btnAltEnter.Size = new Size(208, 48);
      btnAltEnter.TabIndex = 8;
      btnAltEnter.Text = "Fullscreen\r\n(Alt+Enter)";
      btnAltEnter.UseVisualStyleBackColor = true;
      btnAltEnter.Click += btnAltEnter_Click;
      // 
      // btnY
      // 
      tableLayoutPanel1.SetColumnSpan(btnY, 2);
      btnY.Dock = DockStyle.Fill;
      btnY.Location = new Point(436, 116);
      btnY.Name = "btnY";
      btnY.Size = new Size(208, 48);
      btnY.TabIndex = 10;
      btnY.Text = "Message Mode\r\n(Y)";
      btnY.UseVisualStyleBackColor = true;
      btnY.Click += btnY_Click;
      // 
      // btn11
      // 
      btn11.Anchor = AnchorStyles.Top | AnchorStyles.Bottom | AnchorStyles.Left | AnchorStyles.Right;
      btn11.Location = new Point(115, 170);
      btn11.Name = "btn11";
      btn11.Size = new Size(101, 48);
      btn11.TabIndex = 13;
      btn11.Text = "11";
      btn11.UseVisualStyleBackColor = true;
      btn11.Click += btn11_Click;
      // 
      // btnTilde
      // 
      tableLayoutPanel1.SetColumnSpan(btnTilde, 2);
      btnTilde.Dock = DockStyle.Fill;
      btnTilde.Location = new Point(650, 8);
      btnTilde.Name = "btnTilde";
      btnTilde.Size = new Size(211, 48);
      btnTilde.TabIndex = 3;
      btnTilde.Text = "Preset Lock \r\n(~)";
      btnTilde.UseVisualStyleBackColor = true;
      btnTilde.Click += btnTilde_Click;
      // 
      // btn00
      // 
      btn00.Anchor = AnchorStyles.Top | AnchorStyles.Bottom | AnchorStyles.Left | AnchorStyles.Right;
      btn00.Location = new Point(8, 170);
      btn00.Name = "btn00";
      btn00.Size = new Size(101, 48);
      btn00.TabIndex = 12;
      btn00.Text = "00";
      btn00.UseVisualStyleBackColor = true;
      btn00.Click += btn00_Click;
      // 
      // btnF7
      // 
      tableLayoutPanel1.SetColumnSpan(btnF7, 2);
      btnF7.Dock = DockStyle.Fill;
      btnF7.Location = new Point(650, 62);
      btnF7.Name = "btnF7";
      btnF7.Size = new Size(211, 48);
      btnF7.TabIndex = 7;
      btnF7.Text = "Always On Top\r\n(F7)";
      btnF7.UseVisualStyleBackColor = true;
      btnF7.Click += btnF7_Click;
      // 
      // btnF4
      // 
      tableLayoutPanel1.SetColumnSpan(btnF4, 2);
      btnF4.Dock = DockStyle.Fill;
      btnF4.Location = new Point(436, 62);
      btnF4.Name = "btnF4";
      btnF4.Size = new Size(208, 48);
      btnF4.TabIndex = 6;
      btnF4.Text = "Preset Info\r\n(F4)";
      btnF4.UseVisualStyleBackColor = true;
      btnF4.Click += btnF4_Click;
      // 
      // btnF3
      // 
      tableLayoutPanel1.SetColumnSpan(btnF3, 2);
      btnF3.Dock = DockStyle.Fill;
      btnF3.Location = new Point(222, 62);
      btnF3.Name = "btnF3";
      btnF3.Size = new Size(208, 48);
      btnF3.TabIndex = 5;
      btnF3.Text = "Change FPS\r\n(F3)";
      btnF3.UseVisualStyleBackColor = true;
      btnF3.Click += btnF3_Click;
      // 
      // btnDelete
      // 
      tableLayoutPanel1.SetColumnSpan(btnDelete, 2);
      btnDelete.Dock = DockStyle.Fill;
      btnDelete.Location = new Point(650, 116);
      btnDelete.Name = "btnDelete";
      btnDelete.Size = new Size(211, 48);
      btnDelete.TabIndex = 11;
      btnDelete.Text = "Clear Latest Sprite\r\n(Delete)";
      btnDelete.UseVisualStyleBackColor = true;
      btnDelete.Click += btnDelete_Click;
      // 
      // MilkwaveRemoteForm
      // 
      AutoScaleDimensions = new SizeF(8F, 20F);
      AutoScaleMode = AutoScaleMode.Font;
      ClientSize = new Size(869, 605);
      Controls.Add(splitContainer1);
      Controls.Add(statusStrip1);
      Icon = (Icon)resources.GetObject("$this.Icon");
      KeyPreview = true;
      MinimumSize = new Size(300, 100);
      Name = "MilkwaveRemoteForm";
      Text = "Milkwave Remote";
      FormClosing += MainForm_FormClosing;
      Load += MilkwaveRemoteForm_Load;
      Shown += MainForm_Shown;
      KeyDown += MainForm_KeyDown;
      statusStrip1.ResumeLayout(false);
      statusStrip1.PerformLayout();
      ((System.ComponentModel.ISupportInitialize)txtSize).EndInit();
      ((System.ComponentModel.ISupportInitialize)txtBPM).EndInit();
      ((System.ComponentModel.ISupportInitialize)txtBeats).EndInit();
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
    private ColorDialog colorDialog1;
    private ToolStripStatusLabel statusHelp;
    private ToolStripStatusLabel statusSupporters;
    private SplitContainer splitContainer1;
    private SplitContainer splitContainer2;
    private CheckBox chkPreview;
    private Button btnAppendSize;
    private NumericUpDown txtSize;
    private Label lblSize;
    private Label lblStyle;
    private TextBox txtStyle;
    private NumericUpDown txtBPM;
    private Label lblBPM;
    private CheckBox chkFileRandom;
    private Panel pnlColor;
    private Button btnAppendColor;
    private Label lblColor;
    private Button btnFontAppend;
    private Label lblFont;
    private ComboBox cboFonts;
    private NumericUpDown txtBeats;
    private Label label7;
    private Label lblFromFile;
    private CheckBox chkAutoplay;
    private TextBox txtAutoplay;
    private Button btnSaveParam;
    private Label label5;
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
    private TextBox txtVis;
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
  }
}
