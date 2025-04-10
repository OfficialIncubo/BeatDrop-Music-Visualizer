namespace MilkwaveRemote.Data {
  public class Preset {
    public string DisplayName { get; set; } = "";
    public string FullPath { get; set; } = "";
    public override string ToString() {
      return DisplayName;
    }
  }
}
