import java.io.File;

/**
 * This program reads through a directory of serialized waveforms and 
 * only includes the first so many samples and writes the smaller 
 * waveforms out into another directory. 
 */

/**
 * Version: $Id$
 * 
 * $Log$
 * 
 * @author kensmithjr
 */
public class TraceShrinker {

	public TraceShrinker() throws Exception {

		//
		// Params
		//
		String inputDirectory = "converted";
		String outputDirectory = "converted_done";
		int maxTime = 402000; // not inclusive
		//
		// End Params
		//

		File traceDir = new File(inputDirectory);
		String traces[] = traceDir.list();
		Trace trace;
		Trace traceSmaller;
		int traceSmallerIndex;
		int numTimes;

		for (int i = 0; i < traces.length; i++) {

			trace = Simulation_Attack.readTrace(inputDirectory + "/"
					+ traces[i]);

			//
			// Find out how many times qualify
			//
			numTimes = 0;
			for (int j = 0; j < trace.getSize(); j++)
				if (trace.getTime(j) < maxTime)
					numTimes++;

			//
			// Create new smaller trace
			//
			traceSmaller = new Trace(numTimes);

			//
			// Fill it up
			//
			traceSmallerIndex = 0;
			for (int k = 0; k < trace.getSize(); k++)
				if (trace.getTime(k) < maxTime) {
					traceSmaller.putTime(traceSmallerIndex, trace.getTime(k));
					traceSmaller.putValue(traceSmallerIndex, trace.getValue(k));
					traceSmallerIndex++;
				}

			//
			// Write out new trace
			//
			traces[i] = outputDirectory + "/" + traces[i];
			Simulation_Attack.writeTrace(traces[i], traceSmaller);

			System.out.println("finished trace: " + i);
		}
	}
}
