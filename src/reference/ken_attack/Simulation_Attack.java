import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.InputStreamReader;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.util.ArrayList;
import java.util.Date;
import java.util.HashMap;

/**
 * This program provides an entry point for several other programs. 
 * It also performs single/multiple bit DPA attacks on serialized waveforms. 
 */

/**
 * @author kjs4962
 * 
 */
public class Simulation_Attack {

	/**
	 * @param args
	 */
	public static void main(String[] args) {

		try {
			new Simulation_Attack();
		} catch (Exception e) {
			e.printStackTrace();
		}
	}

	private int sbox[] = new int[] { 0x63, 0x7C, 0x77, 0x7B, 0xF2, 0x6B, 0x6F,
			0xC5, 0x30, 0x01, 0x67, 0x2B, 0xFE, 0xD7, 0xAB, 0x76, 0xCA, 0x82,
			0xC9, 0x7D, 0xFA, 0x59, 0x47, 0xF0, 0xAD, 0xD4, 0xA2, 0xAF, 0x9C,
			0xA4, 0x72, 0xC0, 0xB7, 0xFD, 0x93, 0x26, 0x36, 0x3F, 0xF7, 0xCC,
			0x34, 0xA5, 0xE5, 0xF1, 0x71, 0xD8, 0x31, 0x15, 0x04, 0xC7, 0x23,
			0xC3, 0x18, 0x96, 0x05, 0x9A, 0x07, 0x12, 0x80, 0xE2, 0xEB, 0x27,
			0xB2, 0x75, 0x09, 0x83, 0x2C, 0x1A, 0x1B, 0x6E, 0x5A, 0xA0, 0x52,
			0x3B, 0xD6, 0xB3, 0x29, 0xE3, 0x2F, 0x84, 0x53, 0xD1, 0x00, 0xED,
			0x20, 0xFC, 0xB1, 0x5B, 0x6A, 0xCB, 0xBE, 0x39, 0x4A, 0x4C, 0x58,
			0xCF, 0xD0, 0xEF, 0xAA, 0xFB, 0x43, 0x4D, 0x33, 0x85, 0x45, 0xF9,
			0x02, 0x7F, 0x50, 0x3C, 0x9F, 0xA8, 0x51, 0xA3, 0x40, 0x8F, 0x92,
			0x9D, 0x38, 0xF5, 0xBC, 0xB6, 0xDA, 0x21, 0x10, 0xFF, 0xF3, 0xD2,
			0xCD, 0x0C, 0x13, 0xEC, 0x5F, 0x97, 0x44, 0x17, 0xC4, 0xA7, 0x7E,
			0x3D, 0x64, 0x5D, 0x19, 0x73, 0x60, 0x81, 0x4F, 0xDC, 0x22, 0x2A,
			0x90, 0x88, 0x46, 0xEE, 0xB8, 0x14, 0xDE, 0x5E, 0x0B, 0xDB, 0xE0,
			0x32, 0x3A, 0x0A, 0x49, 0x06, 0x24, 0x5C, 0xC2, 0xD3, 0xAC, 0x62,
			0x91, 0x95, 0xE4, 0x79, 0xE7, 0xC8, 0x37, 0x6D, 0x8D, 0xD5, 0x4E,
			0xA9, 0x6C, 0x56, 0xF4, 0xEA, 0x65, 0x7A, 0xAE, 0x08, 0xBA, 0x78,
			0x25, 0x2E, 0x1C, 0xA6, 0xB4, 0xC6, 0xE8, 0xDD, 0x74, 0x1F, 0x4B,
			0xBD, 0x8B, 0x8A, 0x70, 0x3E, 0xB5, 0x66, 0x48, 0x03, 0xF6, 0x0E,
			0x61, 0x35, 0x57, 0xB9, 0x86, 0xC1, 0x1D, 0x9E, 0xE1, 0xF8, 0x98,
			0x11, 0x69, 0xD9, 0x8E, 0x94, 0x9B, 0x1E, 0x87, 0xE9, 0xCE, 0x55,
			0x28, 0xDF, 0x8C, 0xA1, 0x89, 0x0D, 0xBF, 0xE6, 0x42, 0x68, 0x41,
			0x99, 0x2D, 0x0F, 0xB0, 0x54, 0xBB, 0x16 };

	/**
	 * Entry point for all Java source code associated with simulation attacks.
	 * 
	 * @throws Exception
	 */
	public Simulation_Attack() throws Exception {

		boolean batch = false;

		BufferedReader stdin = new BufferedReader(new InputStreamReader(
				System.in));
		int choice = 1;

		while (choice != 0) {

			batch = false;

			System.out.println("\nChoose Action:\n");
			System.out.println("  1 -> Convert");
			System.out.println("  2 -> TraceShrinker");
			System.out.println("  3 -> TimingProfiler");
			System.out.println("  4 -> RandomFileOrderGenerator");
			System.out.println("  5 -> Attack");
			System.out.println("  6 -> ResultsParser");
			System.out.println("  7 -> Run batch code");
			System.out.println("  0 -> Exit\n");

			choice = Integer.parseInt(stdin.readLine());

			switch (choice) {
			case 1:
				Convert();
				break;
			case 2:
				new TraceShrinker();
				break;
			case 3:
				new TimingProfiler();
				break;
			case 4:
				new RandomFileOrderGenerator();
				break;
			case 5:
				//Attack(9, 2, 0, 2);
				Attack(2, 1, 0, 1);
				break;
			case 6:
				new ResultsParser();
				break;
			case 7:
				batch = true;
				break;
			default:
				break;
			} // End switch(choice)

			if (batch) {

				//
				// Batch Mode
				//

				// int targetByteNum[] =
				// {2,2,2,2,2,2,2,2,9,9,9,9,9,9,9,9,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9};
				// int targetNumBits[] =
				// {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,3,3,4,4,5,5,5,6,6,6,7,7,7,7,8,8,8,8,3,3,4,4,5,5,5,6,6,6,7,7,7,7,8,8,8,8};
				// int targetBitsOffset[] =
				// {0,1,2,3,4,5,6,7,0,1,2,3,4,5,6,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
				// int targetThreshold[] =
				// {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,3,3,4,3,4,5,4,5,6,4,5,6,7,5,6,7,8,2,3,3,4,3,4,5,4,5,6,4,5,6,7,5,6,7,8};

				int targetByteNum[] = { 9, 9, 2, 9 };
				int targetNumBits[] = { 1, 8, 8, 7 };
				int targetBitsOffset[] = { 2, 0, 0, 0 };
				int targetThreshold[] = { 1, 8, 8, 5 };

				// Attack(targetByteNum, targetNumBits, targetBitsOffset,
				// targetThreshold)
				for (int i = 0; i < targetByteNum.length; i++)
					Attack(targetByteNum[i], targetNumBits[i],
							targetBitsOffset[i], targetThreshold[i]);
			}

		} // End while (choice != 0)

		System.out.println("Simulation_Attack() finished");
	}

	private void Attack(int targetByteNum, int targetNumBits,
			int targetBitsOffset, int targetThreshold) throws Exception {

		/*
		 * Prerequisites:
		 * 
		 * 1. Make sure your serialized Trace objects are in "inputDir" 2. Make
		 * sure you have an "outputDir" 3. Make sure you have an accurate Timing
		 * Profile (inputTimingProfile) 4. Make sure you have an accurate Trace
		 * Order File (inputTraceOrderFile) 5. Set numTracesToUse <= total
		 * number of traces
		 */

		//
		// Params
		//
//		final String inputDir = "converted_done_up_to_4020_only";
//		final String outputDir = "attack_results";
//		final String inputTimingProfile = "converted_done_up_to_4020_only_times.txt";
//		final String inputTraceOrderFile = "trace_file_access_order.txt";
		final String inputDir = "converted_done";
		final String outputDir = "attack_results";
		final String inputTimingProfile = "converted_done.txt";
		final String inputTraceOrderFile = "trace_order.txt";
		final int reportMaxesAfterThisManyTraces = 10;
		final int numTracesToUse = 10000; //100000;
		//
		// End Params
		//

		System.out.println("Starting up...");

		Date startDate = new Date();
		Date lastDate = new Date();

		System.out.println("Processing timing profile...");
		
		//
		// Read in the timing profile (what times have been used)
		//
		int totalTimes = 0;
		HashMap<Integer, Integer> timingTranslator = new HashMap<Integer, Integer>();
		// use to store the sample times in order to print later
		ArrayList<Integer> sampleTimes = new ArrayList<Integer>();

		BufferedReader in = new BufferedReader(new FileReader(new File(
				inputTimingProfile)));
		String line = in.readLine();
		while (line != null) {

			timingTranslator.put(Integer.parseInt(line), new Integer(
					totalTimes++));
			sampleTimes.add(new Integer(Integer.parseInt(line)));
			line = in.readLine();
		}
		in.close();

		System.out.println("Initializing differential traces...");
		
		//
		// Initialize differential traces
		//
		// diffTraces[keyGuess][target_bit][time]
		double diffTraces[][][] = new double[256][2][totalTimes];
		// diffCounts[keyGuess][group] where group == 3 means neither/both
		// groups
		int diffCounts[][] = new int[256][3];
		double maxDiffs[] = new double[256];

		for (int keyGuess = 0; keyGuess < 256; keyGuess++) {
			for (int time = 0; time < totalTimes; time++) {
				diffTraces[keyGuess][0][time] = 0;
				diffTraces[keyGuess][1][time] = 0;
			}
			diffCounts[keyGuess][0] = 0;
			diffCounts[keyGuess][1] = 0;
			diffCounts[keyGuess][2] = 0;
		}

		//
		// Get the traces
		//
		File traceDir = new File(inputDir);
		String traces[] = traceDir.list();
		java.util.Arrays.sort(traces);
		Trace trace;

		// 
		// Variables for taking the difference
		//
		double a;
		double b;
		double diff;

		//
		// Output file for interval maximum values
		//
		BufferedWriter intervalMaxesOut = new BufferedWriter(
				new FileWriter(
						new File(
								String
										.format(
												"%s/tracesUsed_%06d__targetByteNum_%d__targetNumBits_%d__targetBitsOffset_%d__targetThreshold_%d__reportMaxesAfter_%d__intervalMaxes.csv",
												outputDir, numTracesToUse,
												targetByteNum, targetNumBits,
												targetBitsOffset,
												targetThreshold,
												reportMaxesAfterThisManyTraces))));
		// Differential Output
		BufferedWriter differentialsOut = new BufferedWriter(
				new FileWriter(
						new File(
								String
										.format(
												"%s/tracesUsed_%06d__targetByteNum_%d__targetNumBits_%d__targetBitsOffset_%d__targetThreshold_%d__differentials.csv",
												outputDir, numTracesToUse,
												targetByteNum, targetNumBits,
												targetBitsOffset,
												targetThreshold))));
		BufferedWriter groupCountsOut = new BufferedWriter(
				new FileWriter(
						new File(
								String
										.format(
												"%s/tracesUsed_%06d__targetByteNum_%d__targetNumBits_%d__targetBitsOffset_%d__targetThreshold_%d__reportMaxesAfter_%d__groupCounts.csv",
												outputDir, numTracesToUse,
												targetByteNum, targetNumBits,
												targetBitsOffset,
												targetThreshold,
												reportMaxesAfterThisManyTraces))));

		//
		// Trace Order
		//
		BufferedReader traceOrderFileIn = new BufferedReader(new FileReader(
				new File(inputTraceOrderFile)));
		line = traceOrderFileIn.readLine();
		int traceNum;
		int numTracesComplete = 0;
		
		System.out.println("Processing traces...\n");

		// for (int traceNum = 0; traceNum < traces.length; traceNum++) {
		while (numTracesComplete < numTracesToUse) {

			traceNum = Integer.parseInt(line);
			line = traceOrderFileIn.readLine();

			//
			// Parse the PT from the file name
			//
			System.out.println("Processing [" + traceNum + "] " + traces[traceNum] +
                               " [" + numTracesComplete + "/" + numTracesToUse + "]");
			String bytes[] = traces[traceNum].split("_");
			bytes[15] = bytes[15].substring(0, 2);

			int pt[] = new int[16];

			for (int j = 0; j < 16; j++)
				pt[j] = Integer.parseInt(bytes[j], 16) & 0xFF;

			// Get trace
			trace = Simulation_Attack.readTrace(inputDir + "/"
					+ traces[traceNum]);

			//
			// For each keyGuess
			//
			for (int keyGuess = 0; keyGuess < 256; keyGuess++) {

				// Calculate the target byte and group
				int targetByteValue = sbox[pt[targetByteNum] ^ keyGuess];
				int group = selectionFunction(targetByteValue, targetNumBits,
						targetBitsOffset, targetThreshold);

				if (group == -1) {

					// Both/neither group
					diffCounts[keyGuess][2]++;

				} else {

					diffCounts[keyGuess][group]++;

					int time;
					double value;

					// Add all the values to the corresponding trace of the
					// differential pair
					for (int index = 0; index < trace.getSize(); index++) {

						time = trace.getTime(index);
						value = trace.getValue(index);

						diffTraces[keyGuess][group][timingTranslator.get(time)] += value;

					} // for each index

				}

			} // for each keyGuess

			numTracesComplete++;

			//
			// Report maximum value for each guess if necessary, Or print all
			// values if done
			//
			if (numTracesComplete % reportMaxesAfterThisManyTraces == 0
					|| numTracesComplete == numTracesToUse) {

				intervalMaxesOut.write(numTracesComplete + ",");

				// reset all max values
				for (int keyGuess = 0; keyGuess < 256; keyGuess++)
					maxDiffs[keyGuess] = 0;

				for (int index = 0; index < totalTimes; index++) {

					// differential
					if (numTracesComplete == numTracesToUse)
						differentialsOut.write(String.format("%d,", sampleTimes
								.get(index).intValue()));

					for (int keyGuess = 0; keyGuess < 256; keyGuess++) {

						a = diffTraces[keyGuess][0][index];
						b = diffTraces[keyGuess][1][index];

						diff = Math
								.abs((a / ((double) diffCounts[keyGuess][0]))
										- (b / ((double) diffCounts[keyGuess][1])));

						// differential
						if (numTracesComplete == numTracesToUse)
							differentialsOut.write(String.format("%e,", diff));

						if (diff > maxDiffs[keyGuess])
							maxDiffs[keyGuess] = diff;

						if (index == totalTimes - 1)
							intervalMaxesOut.write(String.format("%e,",
									maxDiffs[keyGuess]));
					}

					// differential
					if (numTracesComplete == numTracesToUse)
						differentialsOut.write("\n");

				}

				intervalMaxesOut.write("\n");

				//
				// Report Group Counts
				//
				for (int group = 0; group < 3; group++) {

					switch (group) {
					case 0:
						groupCountsOut.write(numTracesComplete + ",0,");
						break;
					case 1:
						groupCountsOut.write(",1,");
						break;
					case 2:
						groupCountsOut.write(",-1,");
						break;
					default:
						break;
					}

					for (int keyGuess = 0; keyGuess < 256; keyGuess++) {

						groupCountsOut.write(String.format("%d,",
								diffCounts[keyGuess][group]));
					}
					groupCountsOut.write("\n");
				}
			}

			// System.out.println(traces[traceNum]);
			//System.out.println("Finished trace number: " +
		    //numTracesComplete);
			
			if (0 != numTracesComplete && 0 == (numTracesComplete % 100)) {
			diff = (new Date()).getTime() - lastDate.getTime();
			lastDate = new Date();
			System.out.println("\n execution time: " + diff / 1000.0 + " seconds");
			}
			
		}

		//
		// Finish
		//
		intervalMaxesOut.close();
		differentialsOut.close();
		groupCountsOut.close();
		diff = (new Date()).getTime() - startDate.getTime();
		System.out.println("\n execution time: " + diff / 1000.0 + " seconds");
	}

	/**
	 * Determine the group to put the targetByteValue into. First it only
	 * considers the bits specified by targetNumBits and targetBitsOffset. Then
	 * it counts up the Hamming Weight of those bits. It then returns 0, 1, -1
	 * according to the following:
	 * 
	 * 0 -> Hamming Weight of target bits <= targetNumBits - targetThreshold
	 * 
	 * 1 -> Hamming Weight of target bits >= targetThreshold
	 * 
	 * -1 -> Hamming Weight fits in both or neither of the previous groups
	 * 
	 * @param targetByteValue
	 *            The value to examine
	 * @param targetNumBits
	 *            The number of sequential bits to consider in the byte (must be
	 *            between 1 and 8 inclusive)
	 * @param targetBitsOffset
	 *            The offset of the target bits within targetByteValue (must be
	 *            between 0/LSb and 7/MSb inclusive)
	 * @param targetThreshold
	 *            The Hamming Weight threshold which determines the return value
	 *            as specified above (must be between 0 and 8 inclusive)
	 * @return Integer as specified above
	 */
	private int selectionFunction(int targetByteValue, int targetNumBits,
			int targetBitsOffset, int targetThreshold) throws Exception {

		//
		// Verify inputs
		//
		if (targetNumBits < 1 || targetNumBits > 8)
			throw new Exception(
					"Error: selectionFunction(): incorrect targetNumBits value: "
							+ targetNumBits);
		if (targetBitsOffset < 0 || targetBitsOffset > 7)
			throw new Exception(
					"Error: selectionFunction(): incorrect targetBitsOffset value: "
							+ targetBitsOffset);
		if (targetThreshold < 0 || targetThreshold > 8)
			throw new Exception(
					"Error: selectionFunction(): incorrect targetThreshold value: "
							+ targetThreshold);

		// Only examine the target bits
		int targetByteStripped = 0;

		for (int i = 0; i < targetNumBits; i++)
			targetByteStripped |= targetByteValue
					& (1 << (targetBitsOffset + i));

		// Determine Hamming Weight
		int hWeight = 0;

		for (int i = 0; i < 8; i++)
			if ((targetByteStripped & (1 << i)) != 0)
				hWeight++;

		// Return int based on Hamming Weight
		if ((hWeight <= targetNumBits - targetThreshold)
				&& (hWeight >= targetThreshold))
			// If it fits into both groups, don't put it in either
			return -1;
		if (hWeight <= targetNumBits - targetThreshold)
			// below lower threshold group
			return 0;
		else if (hWeight >= targetThreshold)
			// above higher threshold group
			return 1;
		else
			// in between thresholds group
			return -1;
	}

	/**
	 * Convert instantaneous power waveform from the OUT format into serialized
	 * Trace object files. It operates on all files in a ./to_convert/ folder
	 * and puts the output into a ./converted/ folder. It goes over the input
	 * file twice, once to determine the number of entries, and again to
	 * actually read the entries and store them into the array (Trace object).
	 * 
	 * @throws Exception
	 */
	private void Convert() throws Exception {

		File traceDir = new File("to_convert");
		File traces[] = traceDir.listFiles();

		if (!traceDir.exists() || !traceDir.isDirectory()) {
			System.out.println("to_convert directory not found");
			return;
		}

		int converted = 0;
		
		Date startDate = new Date();

		for (int i = 0; i < traces.length; i++) {

			if (0 == (i % 100)) {
				double diff = (new Date()).getTime() - startDate.getTime();
				System.out.println("\n execution time: " + diff / 1000.0 + " seconds");
				startDate = new Date();
			}
			
			File trace = traces[i];

			if (trace.getName().endsWith(".out")) {

				// System.out.println(trace.getName());

				// 
				// Find number of samples in OUT file
				//
				BufferedReader in = new BufferedReader(new FileReader(trace));

				String line = in.readLine();

				int index = 0;

				while (line != null) {

					if (line.startsWith("2 ")) {

						// power
						index++;

					} else if (!line.startsWith("d")) {

						// time

					} else {

						// System.out.println(line);
					}

					line = in.readLine();
				}

				//
				// Actually pull the data
				//
				in.close();
				in = new BufferedReader(new FileReader(trace));
				line = in.readLine();

				// System.out.println(index);
				Trace traceObj = new Trace(index);
				index = 0;
				int time = 0;

				while (line != null) {

					if (line.startsWith("2 ")) {

						traceObj.putTime(index, time);
						traceObj.putValue(index, Float.parseFloat(line
								.substring(2)));
						index++;

					} else if (!line.startsWith("d")) {

						// time
						time = Integer.parseInt(line);

					} else {

						// System.out.println(line);
					}

					line = in.readLine();
				}

				in.close();

				//
				// Write to object file
				//
				String filename = trace.getName();
				filename = filename.substring(0, filename.length() - 3);
				filename = "converted/" + filename + "obj";
				Simulation_Attack.writeTrace(filename, traceObj);

				System.out.println(++converted + " files converted");
			}
		}

		System.out.println("Done. " + converted + " files converted.");
	}

	/**
	 * Writes a single serialized Trace object into the file specified.
	 * 
	 * @param filename
	 * @param trace
	 * @throws Exception
	 */
	public static void writeTrace(String filename, Trace trace)
			throws Exception {

		ObjectOutputStream oos = new ObjectOutputStream(new FileOutputStream(
				new File(filename)));
		oos.writeObject(trace);
		oos.close();
	}

	/**
	 * Reads a single serialized Trace object from the file specified.
	 * 
	 * @param filename
	 * @return
	 * @throws Exception
	 */
	public static Trace readTrace(String filename) throws Exception {

		ObjectInputStream ois = new ObjectInputStream(new FileInputStream(
				new File(filename)));
		Trace trace = (Trace) ois.readObject();
		ois.close();
		return trace;
	}
}
