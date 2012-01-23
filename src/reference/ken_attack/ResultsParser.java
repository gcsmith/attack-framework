import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.util.ArrayList;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * This program computes the Confidence Ratio for every attack result in the input directory. 
 * Then all of these ratios are writen to a file so that attacks can be directly compared. 
 */

/**
 * Version: $Id$
 * 
 * $Log$
 * 
 * @author kensmithjr
 */
public class ResultsParser {

    /*
     * This program reads in the ..._intervalMaxes.csv files and compiles the
     * resulting trace of maxes for each file into one output file.
     * 
     * ASSUMPTION: All files in inputDir have the same number of lines in each
     * ..._intervalMaxes.csv file.
     */
    public ResultsParser() throws Exception {

        //
        // Parameters
        //
        final String inputDir = "attack_results";
        final String outputFile = "compiled_results_v01.csv";
        //
        // End Parameters
        //

        // Regex
        Pattern targetBytePatt = Pattern.compile("targetByteNum_(\\d+)");
        Matcher matt;

        String allFiles[] = new File(inputDir).list();
        ArrayList<String> usedFiles = new ArrayList<String>();
        // one trace per file used
        ArrayList<ArrayList<Double>> maxTraces = new ArrayList<ArrayList<Double>>();
        ArrayList<Double> tmp;
        int targetByteNum;
        BufferedReader in;
        String line;
        String linePieces[];
        double correctMax;
        double incorrectMax;

        for (int i = 0; i < allFiles.length; i++) {

            if (!allFiles[i].endsWith("intervalMaxes.csv"))
                continue;

            // System.out.println(files[i]);

            matt = targetBytePatt.matcher(allFiles[i]);
            if (matt.find()) {

            //
            // Handle the file
            //
            // System.out.println(matt.group(1));
            targetByteNum = Integer.parseInt(matt.group(1));

            usedFiles.add(allFiles[i]);
            tmp = new ArrayList<Double>();
            in = new BufferedReader(new FileReader(new File(inputDir + "/"
                + allFiles[i])));

            line = in.readLine();

            while (line != null) {

                linePieces = line.split(",");

                correctMax = 0;
                incorrectMax = 0;

                if (linePieces.length != 257)
                throw new Exception(
                    "ERROR: ResultsParser: found != 257 pieces to maxInterval.csv line: "
                        + allFiles[i]);

                for (int j = 0; j < linePieces.length; j++) {
                if (j == targetByteNum + 1)
                    correctMax = Double.parseDouble(linePieces[j]);
                else if(j != 0) {
                    if (Double.parseDouble(linePieces[j]) > incorrectMax)
                    incorrectMax = Double
                        .parseDouble(linePieces[j]);
                }
                }

                tmp.add(correctMax / incorrectMax);

                line = in.readLine();
            }

            maxTraces.add(tmp);
            in.close();

            } else
            throw new Exception("Error: couldn't find targetByteNum in: "
                + allFiles[i]);

        } // End FOR allFiles

        //
        // Print the results
        //
        BufferedWriter out = new BufferedWriter(new FileWriter(new File(
            outputFile)));

        for (int fileNum = 0; fileNum < usedFiles.size(); fileNum++)
            out.write(usedFiles.get(fileNum) + ",");
        out.write("\n");
            
        for (int entryNum = 0; entryNum < maxTraces.get(0).size(); entryNum++) {

            for (int traceNum = 0; traceNum < maxTraces.size(); traceNum++) {

            out.write(String.format("%e,", maxTraces.get(traceNum).get(entryNum)));
            }
            out.write("\n");
        }
        out.close();
        
        System.out.println("ResultsParser finished");
    }
}
