namespace MilkwaveRemote.Data {
  public class Settings {
    public Settings() {
    }

    public static int Panel1DefaultHeight = 230;
    public static int Panel2DefaultHeight = 79;

    public static Point VisualizerDefaultWindowLocation { get; set; } = new Point(850, 50);
    public static Size VisualizerDefaultWindowSize { get; set; } = new Size(640, 360);

    public List<Style> Styles { get; set; } = new List<Style>();
    public Point RemoteWindowLocation { get; set; } = new Point(50, 50);
    public Size RemoteWindowSize { get; set; } = new Size(800, 600);
    public Point VisualizerWindowLocation { get; set; } = VisualizerDefaultWindowLocation;
    public Size VisualizerWindowSize { get; set; } = VisualizerDefaultWindowSize;
    public int SplitterDistance1 { get; set; } = Panel1DefaultHeight;
    public int SplitterDistance2 { get; set; } = Panel2DefaultHeight;
    public bool DarkMode { get; set; } = true;
    public bool ShowMessagePanel { get; set; } = true;
    public bool ShowPresetPanel { get; set; } = true;
    public bool ShowButtonPanel { get; set; } = true;
  }
}