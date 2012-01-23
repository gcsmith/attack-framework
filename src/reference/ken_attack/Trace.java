import java.io.Serializable;

/**
 * Encapsulates a waveform and gets serialized. 
 */

/**
 * @author kjs4962
 *
 */
public class Trace implements Serializable {

	private static final long serialVersionUID = 8721898178418692737L;
	
	private int times[];
	
	private float values[];
	
	/**
	 * 
	 * @param size
	 */
	public Trace(int size){
		
		super();
	
		times = new int[size];
		values = new float[size];
	}
	
	/**
	 * 
	 * @param index
	 * @param time
	 * @param value
	 */
	public void putTime(int index, int time){
		
		times[index] = time;
	}
	
	public void putValue(int index, float value){
		
		values[index] = value;
	}
	
	/**
	 * 
	 * @param index
	 * @return
	 */
	public int getTime(int index){
		
		return times[index];
	}
	
	/**
	 * 
	 * @param index
	 * @return
	 */
	public float getValue(int index){
		
		return values[index];
	}

	/**
	 * 
	 * @return
	 */
	public int getSize(){
		
		return times.length;
	}
}
