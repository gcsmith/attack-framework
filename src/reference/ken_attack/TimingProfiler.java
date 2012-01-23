import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;

/**
 * This program reads through a serialized waveform directory and 
 * records every time interval which has associated power information.
 * It then writes these times out in order to a text file.  
 */

/**
 * Version:
 *   $Id$
 *
 *   $Log$
 *
 * @author kensmithjr
 */
public class TimingProfiler {

    
    public TimingProfiler() throws Exception {
	
        //
        // Params
        //
//        String inputDir = "converted_done_up_to_4020_only";
//        String outputFile = "converted_done_up_to_4020_only_times.txt";
        String inputDir = "converted_done";
        String outputFile = "converted_done.txt";
        int MAX_POSSIBLE_TIME = 402001;
        //
        //
        //
        
        String files[] = (new File(inputDir)).list();
        Trace trace;
        
        int times[] = new int[MAX_POSSIBLE_TIME];
        
        // Init
        for(int i = 0; i < times.length; i++)
            times[i] = 0;
        
        for(int i = 0; i < files.length; i++){
            
            System.out.println(files[i]);
            trace = Simulation_Attack.readTrace(inputDir + "/" + files[i]);
            
            for(int j = 0; j < trace.getSize(); j++){
            times[trace.getTime(j)]++;
            }
        }
        
        // Results
        BufferedWriter out = new BufferedWriter(new FileWriter(new File(outputFile)));
        for(int i = 0; i < times.length; i++){
            
            if(times[i] != 0)
            out.write(i + "\n");
        }
        out.close();
        }
}
