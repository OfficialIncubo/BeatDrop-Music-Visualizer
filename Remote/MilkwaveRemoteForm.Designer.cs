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
      statusHelp = new ToolStripStatusLabel();
      statusSupporters = new ToolStripStatusLabel();
      statusGitHub = new ToolStripStatusLabel();
      toolTip1 = new ToolTip(components);
      pnlColor = new Panel();
      bnAppendColor = new Button();
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
      txtWindowTitle = new TextBox();
      chkFileRandom = new CheckBox();
      btnSend = new Button();
      btnSpace = new Button();
      btnBackspace = new Button();
      txtMessage = new TextBox();
      txtBPM = new NumericUpDown();
      label3 = new Label();
      lblPreset = new Label();
      txtSize = new NumericUpDown();
      lblSize = new Label();
      btnAppendSize = new Button();
      chkPreview = new CheckBox();
      lblWindow = new Label();
      txtPreset = new TextBox();
      colorDialog1 = new ColorDialog();
      btnF4 = new Button();
      S = new GroupBox();
      label5 = new Label();
      label2 = new Label();
      groupBox2 = new GroupBox();
      tableLayoutPanel1 = new TableLayoutPanel();
      btn77 = new Button();
      btn66 = new Button();
      btn55 = new Button();
      btn44 = new Button();
      btn33 = new Button();
      btnm22 = new Button();
      btnK = new Button();
      btnF2 = new Button();
      btnN = new Button();
      btnAltEnter = new Button();
      btnY = new Button();
      btn11 = new Button();
      btnTilde = new Button();
      btn00 = new Button();
      btnF7 = new Button();
      btnF3 = new Button();
      btnShiftK = new Button();
      statusStrip1.SuspendLayout();
      ((System.ComponentModel.ISupportInitialize)txtBeats).BeginInit();
      ((System.ComponentModel.ISupportInitialize)txtBPM).BeginInit();
      ((System.ComponentModel.ISupportInitialize)txtSize).BeginInit();
      S.SuspendLayout();
      groupBox2.SuspendLayout();
      tableLayoutPanel1.SuspendLayout();
      SuspendLayout();
      // 
      // statusStrip1
      // 
      statusStrip1.ImageScalingSize = new Size(20, 20);
      statusStrip1.Items.AddRange(new ToolStripItem[] { statusBar, statusHelp, statusSupporters, statusGitHub });
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
      statusBar.Size = new Size(618, 20);
      statusBar.Spring = true;
      statusBar.TextAlign = ContentAlignment.TopLeft;
      // 
      // statusHelp
      // 
      statusHelp.IsLink = true;
      statusHelp.Name = "statusHelp";
      statusHelp.Padding = new Padding(2, 0, 2, 0);
      statusHelp.Size = new Size(45, 20);
      statusHelp.Text = "Help";
      statusHelp.TextAlign = ContentAlignment.TopCenter;
      statusHelp.VisitedLinkColor = Color.Blue;
      statusHelp.Click += statusHelp_Click;
      // 
      // statusSupporters
      // 
      statusSupporters.IsLink = true;
      statusSupporters.Name = "statusSupporters";
      statusSupporters.Padding = new Padding(2, 0, 2, 0);
      statusSupporters.Size = new Size(85, 20);
      statusSupporters.Text = "Supporters";
      statusSupporters.TextAlign = ContentAlignment.TopCenter;
      statusSupporters.VisitedLinkColor = Color.Blue;
      statusSupporters.Click += statusSupporters_Click;
      // 
      // statusGitHub
      // 
      statusGitHub.IsLink = true;
      statusGitHub.Name = "statusGitHub";
      statusGitHub.Padding = new Padding(2, 0, 2, 0);
      statusGitHub.Size = new Size(60, 20);
      statusGitHub.Text = "GitHub";
      statusGitHub.TextAlign = ContentAlignment.TopCenter;
      statusGitHub.VisitedLinkColor = Color.Blue;
      statusGitHub.Click += statusGitHub_Click;
      // 
      // pnlColor
      // 
      pnlColor.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      pnlColor.BorderStyle = BorderStyle.FixedSingle;
      pnlColor.Location = new Point(629, 179);
      pnlColor.Name = "pnlColor";
      pnlColor.Size = new Size(44, 26);
      pnlColor.TabIndex = 11;
      toolTip1.SetToolTip(pnlColor, "Only used if no r, g, b parameters supplied");
      pnlColor.Click += pnlColor_Click;
      // 
      // bnAppendColor
      // 
      bnAppendColor.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      bnAppendColor.Location = new Point(678, 177);
      bnAppendColor.Margin = new Padding(0);
      bnAppendColor.Name = "bnAppendColor";
      bnAppendColor.Size = new Size(56, 28);
      bnAppendColor.TabIndex = 12;
      bnAppendColor.Text = "Set";
      toolTip1.SetToolTip(bnAppendColor, "Append to (or update in) parameters line");
      bnAppendColor.UseVisualStyleBackColor = true;
      bnAppendColor.Click += btnAppendColor_Click;
      // 
      // lblColor
      // 
      lblColor.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      lblColor.AutoSize = true;
      lblColor.Location = new Point(580, 181);
      lblColor.Name = "lblColor";
      lblColor.Size = new Size(45, 20);
      lblColor.TabIndex = 62;
      lblColor.Text = "Color";
      toolTip1.SetToolTip(lblColor, "Only used if no r, g, b parameters supplied\r\nDouble-click to clear the color information from the parameters line\r\n");
      lblColor.DoubleClick += lblColor_DoubleClick;
      // 
      // btnFontAppend
      // 
      btnFontAppend.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      btnFontAppend.Location = new Point(358, 177);
      btnFontAppend.Margin = new Padding(0);
      btnFontAppend.Name = "btnFontAppend";
      btnFontAppend.Size = new Size(56, 28);
      btnFontAppend.TabIndex = 8;
      btnFontAppend.Text = "Set";
      toolTip1.SetToolTip(btnFontAppend, "Append to (or update in) parameters line");
      btnFontAppend.UseVisualStyleBackColor = true;
      btnFontAppend.Click += btnFontAppend_Click;
      // 
      // lblFont
      // 
      lblFont.AutoSize = true;
      lblFont.Location = new Point(64, 181);
      lblFont.Name = "lblFont";
      lblFont.Size = new Size(38, 20);
      lblFont.TabIndex = 60;
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
      cboFonts.Location = new Point(108, 178);
      cboFonts.Name = "cboFonts";
      cboFonts.Size = new Size(247, 28);
      cboFonts.TabIndex = 7;
      toolTip1.SetToolTip(cboFonts, "Only used if no font parameter supplied");
      cboFonts.SelectedIndexChanged += cboFonts_SelectedIndexChanged;
      cboFonts.KeyDown += cboFonts_KeyDown;
      // 
      // txtBeats
      // 
      txtBeats.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      txtBeats.Location = new Point(628, 212);
      txtBeats.Maximum = new decimal(new int[] { 99, 0, 0, 0 });
      txtBeats.Name = "txtBeats";
      txtBeats.Size = new Size(44, 27);
      txtBeats.TabIndex = 15;
      txtBeats.TextAlign = HorizontalAlignment.Center;
      toolTip1.SetToolTip(txtBeats, "Wait time between lines in beats");
      txtBeats.Value = new decimal(new int[] { 8, 0, 0, 0 });
      // 
      // label7
      // 
      label7.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      label7.AutoSize = true;
      label7.Location = new Point(580, 215);
      label7.Name = "label7";
      label7.Size = new Size(45, 20);
      label7.TabIndex = 57;
      label7.Text = "Beats";
      toolTip1.SetToolTip(label7, "Wait time between lines in beats");
      // 
      // lblFromFile
      // 
      lblFromFile.AutoSize = true;
      lblFromFile.Location = new Point(33, 214);
      lblFromFile.Name = "lblFromFile";
      lblFromFile.Size = new Size(70, 20);
      lblFromFile.TabIndex = 56;
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
      chkAutoplay.Location = new Point(745, 178);
      chkAutoplay.Name = "chkAutoplay";
      chkAutoplay.Size = new Size(88, 61);
      chkAutoplay.TabIndex = 17;
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
      txtAutoplay.Location = new Point(108, 212);
      txtAutoplay.Name = "txtAutoplay";
      txtAutoplay.ReadOnly = true;
      txtAutoplay.Size = new Size(360, 27);
      txtAutoplay.TabIndex = 13;
      toolTip1.SetToolTip(txtAutoplay, "Next line from file script-default.txt");
      // 
      // btnSaveParam
      // 
      btnSaveParam.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      btnSaveParam.FlatStyle = FlatStyle.System;
      btnSaveParam.Location = new Point(745, 142);
      btnSaveParam.Margin = new Padding(0);
      btnSaveParam.Name = "btnSaveParam";
      btnSaveParam.Size = new Size(88, 29);
      btnSaveParam.TabIndex = 6;
      btnSaveParam.Text = "Save";
      toolTip1.SetToolTip(btnSaveParam, "Save current parameters as preset");
      btnSaveParam.UseVisualStyleBackColor = true;
      btnSaveParam.Click += btnSaveParam_Click;
      // 
      // lblParameters
      // 
      lblParameters.AutoSize = true;
      lblParameters.Location = new Point(20, 146);
      lblParameters.Name = "lblParameters";
      lblParameters.Size = new Size(82, 20);
      lblParameters.TabIndex = 51;
      lblParameters.Text = "Parameters";
      toolTip1.SetToolTip(lblParameters, "Double-click to clear all saved presets");
      lblParameters.DoubleClick += lblParameters_DoubleClick;
      // 
      // cboParameters
      // 
      cboParameters.Anchor = AnchorStyles.Top | AnchorStyles.Left | AnchorStyles.Right;
      cboParameters.DropDownStyle = ComboBoxStyle.DropDownList;
      cboParameters.Location = new Point(108, 143);
      cboParameters.Name = "cboParameters";
      cboParameters.Size = new Size(464, 28);
      cboParameters.TabIndex = 4;
      toolTip1.SetToolTip(cboParameters, resources.GetString("cboParameters.ToolTip"));
      cboParameters.SelectedIndexChanged += cboParameters_SelectedIndexChanged;
      cboParameters.TextChanged += cboParameters_TextChanged;
      cboParameters.KeyDown += cboParameters_KeyDown;
      // 
      // txtWindowTitle
      // 
      txtWindowTitle.Anchor = AnchorStyles.Top | AnchorStyles.Left | AnchorStyles.Right;
      txtWindowTitle.Location = new Point(108, 110);
      txtWindowTitle.Name = "txtWindowTitle";
      txtWindowTitle.Size = new Size(342, 27);
      txtWindowTitle.TabIndex = 2;
      txtWindowTitle.Text = "Milkwave Visualizer";
      toolTip1.SetToolTip(txtWindowTitle, "Partitial window title of Milkwave Visualizer window");
      // 
      // chkFileRandom
      // 
      chkFileRandom.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      chkFileRandom.Appearance = Appearance.Button;
      chkFileRandom.FlatStyle = FlatStyle.System;
      chkFileRandom.Location = new Point(678, 210);
      chkFileRandom.Name = "chkFileRandom";
      chkFileRandom.Size = new Size(56, 29);
      chkFileRandom.TabIndex = 16;
      chkFileRandom.Text = "Rand";
      chkFileRandom.TextAlign = ContentAlignment.MiddleCenter;
      chkFileRandom.TextImageRelation = TextImageRelation.ImageAboveText;
      toolTip1.SetToolTip(chkFileRandom, "Play random line");
      chkFileRandom.UseVisualStyleBackColor = true;
      // 
      // btnSend
      // 
      btnSend.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      btnSend.FlatStyle = FlatStyle.System;
      btnSend.Location = new Point(745, 26);
      btnSend.Margin = new Padding(0);
      btnSend.Name = "btnSend";
      btnSend.Size = new Size(88, 78);
      btnSend.TabIndex = 1;
      btnSend.Text = "Send";
      toolTip1.SetToolTip(btnSend, "F2");
      btnSend.UseVisualStyleBackColor = true;
      btnSend.Click += btnSend_Click;
      // 
      // btnSpace
      // 
      tableLayoutPanel1.SetColumnSpan(btnSpace, 2);
      btnSpace.Dock = DockStyle.Fill;
      btnSpace.Location = new Point(8, 8);
      btnSpace.Name = "btnSpace";
      btnSpace.Size = new Size(200, 62);
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
      btnBackspace.Location = new Point(214, 8);
      btnBackspace.Name = "btnBackspace";
      btnBackspace.Size = new Size(200, 62);
      btnBackspace.TabIndex = 1;
      btnBackspace.Text = "Previous Preset\r\n(Backspace)";
      toolTip1.SetToolTip(btnBackspace, "F4");
      btnBackspace.UseVisualStyleBackColor = true;
      btnBackspace.Click += btnBackspace_Click;
      // 
      // txtMessage
      // 
      txtMessage.Anchor = AnchorStyles.Top | AnchorStyles.Left | AnchorStyles.Right;
      txtMessage.Location = new Point(108, 27);
      txtMessage.Multiline = true;
      txtMessage.Name = "txtMessage";
      txtMessage.Size = new Size(626, 77);
      txtMessage.TabIndex = 0;
      txtMessage.Text = "Hi from Milkwave Remote!";
      toolTip1.SetToolTip(txtMessage, "F1");
      txtMessage.KeyDown += txtMessage_KeyDown;
      // 
      // txtBPM
      // 
      txtBPM.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      txtBPM.Location = new Point(519, 212);
      txtBPM.Maximum = new decimal(new int[] { 999, 0, 0, 0 });
      txtBPM.Name = "txtBPM";
      txtBPM.Size = new Size(53, 27);
      txtBPM.TabIndex = 14;
      txtBPM.TextAlign = HorizontalAlignment.Center;
      toolTip1.SetToolTip(txtBPM, "BPM");
      txtBPM.Value = new decimal(new int[] { 120, 0, 0, 0 });
      txtBPM.TextChanged += txtBPM_TextChanged;
      // 
      // label3
      // 
      label3.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      label3.AutoSize = true;
      label3.Location = new Point(474, 215);
      label3.Name = "label3";
      label3.Size = new Size(39, 20);
      label3.TabIndex = 65;
      label3.Text = "BPM";
      toolTip1.SetToolTip(label3, "Click to reset timer");
      label3.Click += lblBPM;
      // 
      // lblPreset
      // 
      lblPreset.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      lblPreset.AutoSize = true;
      lblPreset.Location = new Point(577, 146);
      lblPreset.Name = "lblPreset";
      lblPreset.Size = new Size(49, 20);
      lblPreset.TabIndex = 5;
      lblPreset.Text = "Preset";
      toolTip1.SetToolTip(lblPreset, "Double-click to remove this preset\r\nRight-click to fill form elements from current parameters");
      lblPreset.DoubleClick += lblPreset_DoubleClick;
      lblPreset.MouseClick += lblPreset_MouseClick;
      // 
      // txtSize
      // 
      txtSize.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      txtSize.Location = new Point(460, 178);
      txtSize.Maximum = new decimal(new int[] { 999, 0, 0, 0 });
      txtSize.Name = "txtSize";
      txtSize.Size = new Size(53, 27);
      txtSize.TabIndex = 9;
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
      lblSize.Location = new Point(421, 181);
      lblSize.Name = "lblSize";
      lblSize.Size = new Size(36, 20);
      lblSize.TabIndex = 69;
      lblSize.Text = "Size";
      toolTip1.SetToolTip(lblSize, "Only used if no size parameter supplied\r\nDouble-click to clear the size value from the parameters line\r\n");
      lblSize.DoubleClick += lblSize_DoubleClick;
      // 
      // btnAppendSize
      // 
      btnAppendSize.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      btnAppendSize.Location = new Point(519, 177);
      btnAppendSize.Margin = new Padding(0);
      btnAppendSize.Name = "btnAppendSize";
      btnAppendSize.Size = new Size(53, 28);
      btnAppendSize.TabIndex = 10;
      btnAppendSize.Text = "Set";
      toolTip1.SetToolTip(btnAppendSize, "Append to (or update in) parameters line");
      btnAppendSize.UseVisualStyleBackColor = true;
      btnAppendSize.Click += btnAppendSize_Click;
      // 
      // chkPreview
      // 
      chkPreview.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      chkPreview.Appearance = Appearance.Button;
      chkPreview.Checked = true;
      chkPreview.CheckState = CheckState.Checked;
      chkPreview.FlatStyle = FlatStyle.System;
      chkPreview.Location = new Point(629, 111);
      chkPreview.Name = "chkPreview";
      chkPreview.Size = new Size(105, 27);
      chkPreview.TabIndex = 3;
      chkPreview.Text = "Preview";
      chkPreview.TextAlign = ContentAlignment.MiddleCenter;
      chkPreview.TextImageRelation = TextImageRelation.ImageAboveText;
      toolTip1.SetToolTip(chkPreview, "Play random line");
      chkPreview.UseVisualStyleBackColor = true;
      chkPreview.CheckedChanged += chkPreview_CheckedChanged;
      // 
      // lblWindow
      // 
      lblWindow.AutoSize = true;
      lblWindow.Location = new Point(38, 113);
      lblWindow.Name = "lblWindow";
      lblWindow.Size = new Size(64, 20);
      lblWindow.TabIndex = 48;
      lblWindow.Text = "Window";
      toolTip1.SetToolTip(lblWindow, "Double-click to restore saved Visualizer window position and size\r\nHold Ctrl while closing this window to clear all locally saved settings");
      lblWindow.DoubleClick += lblWindow_DoubleClick;
      // 
      // txtPreset
      // 
      txtPreset.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      txtPreset.Location = new Point(629, 143);
      txtPreset.Name = "txtPreset";
      txtPreset.Size = new Size(105, 27);
      txtPreset.TabIndex = 5;
      txtPreset.Text = "Preset 1";
      txtPreset.KeyDown += txtPreset_KeyDown;
      // 
      // colorDialog1
      // 
      colorDialog1.AnyColor = true;
      colorDialog1.Color = Color.White;
      colorDialog1.FullOpen = true;
      colorDialog1.SolidColorOnly = true;
      // 
      // btnF4
      // 
      tableLayoutPanel1.SetColumnSpan(btnF4, 2);
      btnF4.Dock = DockStyle.Fill;
      btnF4.Location = new Point(420, 76);
      btnF4.Name = "btnF4";
      btnF4.Size = new Size(200, 62);
      btnF4.TabIndex = 6;
      btnF4.Text = "Preset Info\r\n(F4)";
      btnF4.UseVisualStyleBackColor = true;
      btnF4.Click += btnF4_Click;
      // 
      // S
      // 
      S.Anchor = AnchorStyles.Top | AnchorStyles.Left | AnchorStyles.Right;
      S.Controls.Add(chkPreview);
      S.Controls.Add(btnAppendSize);
      S.Controls.Add(txtSize);
      S.Controls.Add(lblSize);
      S.Controls.Add(lblPreset);
      S.Controls.Add(txtPreset);
      S.Controls.Add(txtBPM);
      S.Controls.Add(label3);
      S.Controls.Add(chkFileRandom);
      S.Controls.Add(pnlColor);
      S.Controls.Add(bnAppendColor);
      S.Controls.Add(lblColor);
      S.Controls.Add(btnFontAppend);
      S.Controls.Add(lblFont);
      S.Controls.Add(cboFonts);
      S.Controls.Add(txtBeats);
      S.Controls.Add(label7);
      S.Controls.Add(lblFromFile);
      S.Controls.Add(chkAutoplay);
      S.Controls.Add(txtAutoplay);
      S.Controls.Add(btnSaveParam);
      S.Controls.Add(label5);
      S.Controls.Add(lblParameters);
      S.Controls.Add(cboParameters);
      S.Controls.Add(label2);
      S.Controls.Add(lblWindow);
      S.Controls.Add(txtWindowTitle);
      S.Controls.Add(btnSend);
      S.Controls.Add(txtMessage);
      S.Location = new Point(12, 12);
      S.Name = "S";
      S.Size = new Size(844, 252);
      S.TabIndex = 26;
      S.TabStop = false;
      S.Text = "Send Messages";
      // 
      // label5
      // 
      label5.Anchor = AnchorStyles.Top | AnchorStyles.Right;
      label5.AutoSize = true;
      label5.Location = new Point(456, 113);
      label5.Name = "label5";
      label5.Size = new Size(116, 20);
      label5.TabIndex = 52;
      label5.Text = "(partitial match)";
      // 
      // label2
      // 
      label2.AutoSize = true;
      label2.Location = new Point(35, 30);
      label2.Name = "label2";
      label2.Size = new Size(67, 20);
      label2.TabIndex = 49;
      label2.Text = "Message";
      // 
      // groupBox2
      // 
      groupBox2.Anchor = AnchorStyles.Top | AnchorStyles.Bottom | AnchorStyles.Left | AnchorStyles.Right;
      groupBox2.Controls.Add(tableLayoutPanel1);
      groupBox2.Location = new Point(12, 270);
      groupBox2.Name = "groupBox2";
      groupBox2.Size = new Size(844, 310);
      groupBox2.TabIndex = 27;
      groupBox2.TabStop = false;
      groupBox2.Text = "Send Keys";
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
      tableLayoutPanel1.Controls.Add(btnm22, 2, 3);
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
      tableLayoutPanel1.Controls.Add(btnShiftK, 6, 2);
      tableLayoutPanel1.Dock = DockStyle.Fill;
      tableLayoutPanel1.Location = new Point(3, 23);
      tableLayoutPanel1.Name = "tableLayoutPanel1";
      tableLayoutPanel1.Padding = new Padding(5);
      tableLayoutPanel1.RowCount = 4;
      tableLayoutPanel1.RowStyles.Add(new RowStyle(SizeType.Percent, 25F));
      tableLayoutPanel1.RowStyles.Add(new RowStyle(SizeType.Percent, 25F));
      tableLayoutPanel1.RowStyles.Add(new RowStyle(SizeType.Percent, 25F));
      tableLayoutPanel1.RowStyles.Add(new RowStyle(SizeType.Percent, 25F));
      tableLayoutPanel1.RowStyles.Add(new RowStyle(SizeType.Absolute, 20F));
      tableLayoutPanel1.Size = new Size(838, 284);
      tableLayoutPanel1.TabIndex = 33;
      // 
      // btn77
      // 
      btn77.Anchor = AnchorStyles.Top | AnchorStyles.Bottom | AnchorStyles.Left | AnchorStyles.Right;
      btn77.Location = new Point(729, 212);
      btn77.Name = "btn77";
      btn77.Size = new Size(101, 64);
      btn77.TabIndex = 19;
      btn77.Text = "77";
      btn77.UseVisualStyleBackColor = true;
      btn77.Click += btn77_Click;
      // 
      // btn66
      // 
      btn66.Anchor = AnchorStyles.Top | AnchorStyles.Bottom | AnchorStyles.Left | AnchorStyles.Right;
      btn66.Location = new Point(626, 212);
      btn66.Name = "btn66";
      btn66.Size = new Size(97, 64);
      btn66.TabIndex = 18;
      btn66.Text = "66";
      btn66.UseVisualStyleBackColor = true;
      btn66.Click += btn66_Click;
      // 
      // btn55
      // 
      btn55.Anchor = AnchorStyles.Top | AnchorStyles.Bottom | AnchorStyles.Left | AnchorStyles.Right;
      btn55.Location = new Point(523, 212);
      btn55.Name = "btn55";
      btn55.Size = new Size(97, 64);
      btn55.TabIndex = 17;
      btn55.Text = "55";
      btn55.UseVisualStyleBackColor = true;
      btn55.Click += btn55_Click;
      // 
      // btn44
      // 
      btn44.Anchor = AnchorStyles.Top | AnchorStyles.Bottom | AnchorStyles.Left | AnchorStyles.Right;
      btn44.Location = new Point(420, 212);
      btn44.Name = "btn44";
      btn44.Size = new Size(97, 64);
      btn44.TabIndex = 16;
      btn44.Text = "44";
      btn44.UseVisualStyleBackColor = true;
      btn44.Click += btn44_Click;
      // 
      // btn33
      // 
      btn33.Anchor = AnchorStyles.Top | AnchorStyles.Bottom | AnchorStyles.Left | AnchorStyles.Right;
      btn33.Location = new Point(317, 212);
      btn33.Name = "btn33";
      btn33.Size = new Size(97, 64);
      btn33.TabIndex = 15;
      btn33.Text = "33";
      btn33.UseVisualStyleBackColor = true;
      btn33.Click += btn33_Click;
      // 
      // btnm22
      // 
      btnm22.Anchor = AnchorStyles.Top | AnchorStyles.Bottom | AnchorStyles.Left | AnchorStyles.Right;
      btnm22.Location = new Point(214, 212);
      btnm22.Name = "btnm22";
      btnm22.Size = new Size(97, 64);
      btnm22.TabIndex = 14;
      btnm22.Text = "22";
      btnm22.UseVisualStyleBackColor = true;
      btnm22.Click += btnm22_Click;
      // 
      // btnK
      // 
      tableLayoutPanel1.SetColumnSpan(btnK, 2);
      btnK.Dock = DockStyle.Fill;
      btnK.Location = new Point(214, 144);
      btnK.Name = "btnK";
      btnK.Size = new Size(200, 62);
      btnK.TabIndex = 9;
      btnK.Text = "Sprite Mode\r\n(K)";
      btnK.UseVisualStyleBackColor = true;
      btnK.Click += btnK_Click;
      // 
      // btnF2
      // 
      tableLayoutPanel1.SetColumnSpan(btnF2, 2);
      btnF2.Dock = DockStyle.Fill;
      btnF2.Location = new Point(8, 76);
      btnF2.Name = "btnF2";
      btnF2.Size = new Size(200, 62);
      btnF2.TabIndex = 4;
      btnF2.Text = "Borderless \r\n(F2)";
      btnF2.UseVisualStyleBackColor = true;
      btnF2.Click += btnF2_Click;
      // 
      // btnN
      // 
      tableLayoutPanel1.SetColumnSpan(btnN, 2);
      btnN.Dock = DockStyle.Fill;
      btnN.Location = new Point(420, 8);
      btnN.Name = "btnN";
      btnN.Size = new Size(200, 62);
      btnN.TabIndex = 2;
      btnN.Text = "Music Info\r\n(N)";
      btnN.UseVisualStyleBackColor = true;
      btnN.Click += btnN_Click;
      // 
      // btnAltEnter
      // 
      tableLayoutPanel1.SetColumnSpan(btnAltEnter, 2);
      btnAltEnter.Dock = DockStyle.Fill;
      btnAltEnter.Location = new Point(8, 144);
      btnAltEnter.Name = "btnAltEnter";
      btnAltEnter.Size = new Size(200, 62);
      btnAltEnter.TabIndex = 8;
      btnAltEnter.Text = "Fullscreen\r\n(Alt+Enter)";
      btnAltEnter.UseVisualStyleBackColor = true;
      btnAltEnter.Click += btnAltEnter_Click;
      // 
      // btnY
      // 
      tableLayoutPanel1.SetColumnSpan(btnY, 2);
      btnY.Dock = DockStyle.Fill;
      btnY.Location = new Point(420, 144);
      btnY.Name = "btnY";
      btnY.Size = new Size(200, 62);
      btnY.TabIndex = 10;
      btnY.Text = "Message Mode\r\n(Y)";
      btnY.UseVisualStyleBackColor = true;
      btnY.Click += btnY_Click;
      // 
      // btn11
      // 
      btn11.Anchor = AnchorStyles.Top | AnchorStyles.Bottom | AnchorStyles.Left | AnchorStyles.Right;
      btn11.Location = new Point(111, 212);
      btn11.Name = "btn11";
      btn11.Size = new Size(97, 64);
      btn11.TabIndex = 13;
      btn11.Text = "11";
      btn11.UseVisualStyleBackColor = true;
      btn11.Click += btn11_Click;
      // 
      // btnTilde
      // 
      tableLayoutPanel1.SetColumnSpan(btnTilde, 2);
      btnTilde.Dock = DockStyle.Fill;
      btnTilde.Location = new Point(626, 8);
      btnTilde.Name = "btnTilde";
      btnTilde.Size = new Size(204, 62);
      btnTilde.TabIndex = 3;
      btnTilde.Text = "Preset Lock \r\n(~)";
      btnTilde.UseVisualStyleBackColor = true;
      btnTilde.Click += btnTilde_Click;
      // 
      // btn00
      // 
      btn00.Anchor = AnchorStyles.Top | AnchorStyles.Bottom | AnchorStyles.Left | AnchorStyles.Right;
      btn00.Location = new Point(8, 212);
      btn00.Name = "btn00";
      btn00.Size = new Size(97, 64);
      btn00.TabIndex = 12;
      btn00.Text = "00";
      btn00.UseVisualStyleBackColor = true;
      btn00.Click += btn00_Click;
      // 
      // btnF7
      // 
      tableLayoutPanel1.SetColumnSpan(btnF7, 2);
      btnF7.Dock = DockStyle.Fill;
      btnF7.Location = new Point(626, 76);
      btnF7.Name = "btnF7";
      btnF7.Size = new Size(204, 62);
      btnF7.TabIndex = 7;
      btnF7.Text = "Always On Top\r\n(F7)";
      btnF7.UseVisualStyleBackColor = true;
      btnF7.Click += btnF7_Click;
      // 
      // btnF3
      // 
      tableLayoutPanel1.SetColumnSpan(btnF3, 2);
      btnF3.Dock = DockStyle.Fill;
      btnF3.Location = new Point(214, 76);
      btnF3.Name = "btnF3";
      btnF3.Size = new Size(200, 62);
      btnF3.TabIndex = 5;
      btnF3.Text = "Change FPS\r\n(F3)";
      btnF3.UseVisualStyleBackColor = true;
      btnF3.Click += btnF3_Click;
      // 
      // btnShiftK
      // 
      tableLayoutPanel1.SetColumnSpan(btnShiftK, 2);
      btnShiftK.Dock = DockStyle.Fill;
      btnShiftK.Location = new Point(626, 144);
      btnShiftK.Name = "btnShiftK";
      btnShiftK.Size = new Size(204, 62);
      btnShiftK.TabIndex = 11;
      btnShiftK.Text = "Clear Latest Sprite\r\n(Delete)";
      btnShiftK.UseVisualStyleBackColor = true;
      btnShiftK.Click += btnDelete_Click;
      // 
      // MilkwaveRemoteForm
      // 
      AutoScaleDimensions = new SizeF(8F, 20F);
      AutoScaleMode = AutoScaleMode.Font;
      ClientSize = new Size(869, 605);
      Controls.Add(statusStrip1);
      Controls.Add(groupBox2);
      Controls.Add(S);
      Icon = (Icon)resources.GetObject("$this.Icon");
      KeyPreview = true;
      MinimumSize = new Size(640, 370);
      Name = "MilkwaveRemoteForm";
      Text = "Milkwave Remote";
      FormClosing += MainForm_FormClosing;
      Shown += MainForm_Shown;
      KeyDown += MainForm_KeyDown;
      statusStrip1.ResumeLayout(false);
      statusStrip1.PerformLayout();
      ((System.ComponentModel.ISupportInitialize)txtBeats).EndInit();
      ((System.ComponentModel.ISupportInitialize)txtBPM).EndInit();
      ((System.ComponentModel.ISupportInitialize)txtSize).EndInit();
      S.ResumeLayout(false);
      S.PerformLayout();
      groupBox2.ResumeLayout(false);
      tableLayoutPanel1.ResumeLayout(false);
      ResumeLayout(false);
      PerformLayout();
    }

    #endregion
    private StatusStrip statusStrip1;
    private ToolStripStatusLabel statusBar;
    private ToolTip toolTip1;
    private ColorDialog colorDialog1;
    private Button btnF4;
    private GroupBox S;
    private Panel pnlColor;
    private Button bnAppendColor;
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
    private GroupBox groupBox2;
    private Button btnShiftK;
    private Button btnF3;
    private Button btnF7;
    private Button btnSpace;
    private Button btnBackspace;
    private TableLayoutPanel tableLayoutPanel1;
    private Button btnTilde;
    private Button btn11;
    private Button btn00;
    private Button btnY;
    private Button btnAltEnter;
    private Button btnN;
    private Button btnF2;
    private Button btnK;
    private Button btn77;
    private Button btn66;
    private Button btn55;
    private Button btn44;
    private Button btn33;
    private Button btnm22;
    private CheckBox chkFileRandom;
    private NumericUpDown txtBPM;
    private Label label3;
    private Label lblPreset;
    private TextBox txtPreset;
    private Button btnAppendSize;
    private NumericUpDown txtSize;
    private Label lblSize;
    private CheckBox chkPreview;
    private ToolStripStatusLabel statusHelp;
    private ToolStripStatusLabel statusSupporters;
    private ToolStripStatusLabel statusGitHub;
  }
}
