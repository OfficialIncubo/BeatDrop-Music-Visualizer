using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace MilkwaveRemote {
  public class Preset {
      public required string Name { get; set; }
      public required string Value { get; set; }
      public override string ToString() {
        return Name + ": " + Value;
      }    
  }
}
