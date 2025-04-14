namespace MilkwaveRemote {
  internal static class Program {
    /// <summary>
    ///  The main entry point for the application.
    /// </summary>
    [STAThread]
    static void Main() {
      // To customize application configuration such as set high DPI settings or default font,
      // see https://aka.ms/applicationconfiguration.
      ApplicationConfiguration.Initialize();

      Application.ThreadException += new ThreadExceptionEventHandler(Application_ThreadException);
      Application.SetUnhandledExceptionMode(UnhandledExceptionMode.CatchException);
      AppDomain.CurrentDomain.UnhandledException += new UnhandledExceptionEventHandler(CurrentDomain_UnhandledException);
      Application.EnableVisualStyles();
      Application.Run(new MilkwaveRemoteForm());
    }

    static void Application_ThreadException(object sender, ThreadExceptionEventArgs e) {
      try {
        SaveErrorToFile(e.Exception, "Unhandled Thread Exception");
      } catch (Exception ex) {
        // Handle any errors that occur while logging
        MessageBox.Show($"Failed to write error log: {ex.Message}", "Logging Error");
      }
    }

    static void CurrentDomain_UnhandledException(object sender, UnhandledExceptionEventArgs e) {
      try {
        if (e.ExceptionObject is Exception ex) { // Use pattern matching to fix IDE0019
          SaveErrorToFile(ex, "Unhandled UI Exception");
        } else {
          MessageBox.Show("An unknown error occurred.", "Unhandled Exception");
        }
      } catch (Exception ex) {
        // Handle any errors that occur while logging  
        MessageBox.Show($"Failed to write error log: {ex.Message}", "Logging Error");
      }
    }

    private static void SaveErrorToFile(Exception e, string type) {
      // Get the directory of the executable
      string exeDirectory = AppDomain.CurrentDomain.BaseDirectory;

      // Create a timestamp for the log file
      string timestamp = DateTime.Now.ToString("yyyy-MM-dd_HH-mm-ss");

      // Construct the log file path
      string logFilePath = Path.Combine(exeDirectory, $"error-{timestamp}.log");

      // Write the exception details to the log file
      File.WriteAllText(logFilePath, e.ToString());

      // Notify the user
      MessageBox.Show($"An error occurred. Details have been saved to:\n{logFilePath}", type);
    }
  }
}