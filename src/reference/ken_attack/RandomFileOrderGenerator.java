import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.util.Random;

/**
 * This program reads all of the filenames in a directory and then writes 
 * them in a random order to an output text file. 
 */

/**
 * Version: $Id$
 * 
 * $Log$
 * 
 * @author kensmithjr
 */
public class RandomFileOrderGenerator {

	public RandomFileOrderGenerator() throws Exception {

		//
		// Parameters
		//
		//final String inputDir = "converted_done_up_to_4020_only";
		final String inputDir = "converted_done";
		final String outputFile = "trace_order.txt";
		//
		// End Parameters
		//

		// Output file init
		BufferedWriter out = new BufferedWriter(new FileWriter(new File(
				outputFile)));

		// Trace files
		File traceDir = new File(inputDir);
		String traces[] = traceDir.list();

		// Which traces files have been used
		boolean traceFilesUsed[] = new boolean[traces.length];
		for (int i = 0; i < traceFilesUsed.length; i++)
			traceFilesUsed[i] = false;

		int numTracesLeft = traces.length;
		int currentTrace;

		Random rand = new Random();

		// until all traces have been considered
		while (numTracesLeft > 0) {

			// next random trace number (of the ones left)
			currentTrace = rand.nextInt(numTracesLeft);
			currentTrace++;

			// count through until you hit that trace
			for (int i = 0; i < traceFilesUsed.length && currentTrace > 0; i++) {

				if (traceFilesUsed[i] == false)
					currentTrace--;

				if (currentTrace == 0) {
					traceFilesUsed[i] = true;
					numTracesLeft--;
					out.write("" + i); // XXX
					if (numTracesLeft != 0)
						out.write("\n");
				}
			}
		}

		out.close();
		System.out.println("RandomFileOrderGenerator has finished");
	}
}
