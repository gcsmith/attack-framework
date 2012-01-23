import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;

/**
 * This program converts backwards from serialized waveforms to ASCII files. 
 */

/**
 * Version:
 *   $Id$
 *
 *   $Log$
 *
 * @author kensmithjr
 */
public class Serialized_to_ASCII {

    public Serialized_to_ASCII() throws Exception {
	
	//
	// Params
	//
	String inputDir = "converted_done";
	String outputDir = "ascii";
	//
	// End Params
	//
	
	String files[] = (new File(inputDir)).list();
	Trace trace;
	BufferedWriter out;
	
	for(int i = 0; i < 20000; i++){
	    
	    trace = Simulation_Attack.readTrace(inputDir + "/" + files[i]);
	    
	    files[i] = files[i].replace(".obj", ".txt");
	    out = new BufferedWriter(new FileWriter(new File(outputDir + "/" + files[i])));
	    
	    for(int j = 0; j < trace.getSize(); j++){
		
		out.write(String.format("%d,%e\n", trace.getTime(j), trace.getValue(j)));
	    }
	    
	    out.close();
	    
	    System.out.println(i);
	}
    }
}
