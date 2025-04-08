using System.Collections.Specialized;

namespace MilkwaveRemote {
  public class Settings {
    public Settings() {
    }

    public List<Preset> Presets { get; set; } = new List<Preset>();
    public Point RemoteWindowLocation { get; set; } = new Point(50, 50);
    public Size RemoteWindowSize { get; set; } = new Size(800, 600);
    public Point VisualizerWindowLocation { get; set; } = new Point(850, 100);
    public Size VisualizerWindowSize { get; set; } = new Size(640, 360);
    public int SplitterDistance1 { get; set; } = 230;
    public int SplitterDistance2 { get; set; } = 50;
  }
}