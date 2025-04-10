namespace MilkwaveRemote.Data {
  public class Style {
    public required string Name { get; set; }
    public required string Value { get; set; }
    public override string ToString() {
      return Name + ": " + Value;
    }
  }
}
