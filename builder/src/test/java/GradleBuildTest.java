import static org.junit.Assert.assertEquals;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.Parameterized;
import org.junit.runners.Parameterized.Parameters;

import org.gradle.tooling.BuildLauncher;
import org.gradle.tooling.GradleConnector;
import org.gradle.tooling.ProjectConnection;

import java.io.File;
import java.io.FileFilter;
import java.util.Arrays;

@RunWith(Parameterized.class)
public class GradleBuildTest {

  @Parameters
  public static File[] data() {
      return new File("..").listFiles(new FileFilter() {
	      public boolean accept(File f) { return f.isDirectory() && Arrays.asList(f.list()).containsAll(Arrays.asList("build.gradle", "app")); }
      });
  }
  private File gradleProject;
  public GradleBuildTest(File gradleProject) {
      this.gradleProject = gradleProject;
  }

  @Test
  public void test() {
      GradleConnector connector = GradleConnector.newConnector();
      connector.forProjectDirectory(gradleProject);
      ProjectConnection connection = connector.connect();
      try {
	  // Configure the build
	  BuildLauncher launcher = connection.newBuild();
	  launcher.forTasks("build");
	  launcher.setStandardOutput(System.out);
	  launcher.setStandardError(System.err);
	  
	 // Run the build
	  launcher.run();
      } finally {
	  // Clean up
	  connection.close();
      }
  }
}
