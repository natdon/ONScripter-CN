package cn.natdon.onscripterv2;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;

import android.view.View;

/**
 * Utility functions
 * @author trinity
 *
 */
public class U {

	@SuppressWarnings("unchecked")
	public static <T> T $(View v, int id) {
		// Black Magic
		return (T) v.findViewById(id);
	}

	@SuppressWarnings("unchecked")
	public static <T> T $(Object o) {
		return (T) o;
	}
	
	public static String read(File file) {
		try {
			String str;
			StringBuffer sb = new StringBuffer();
			BufferedReader reader = new BufferedReader(new FileReader(file));
			while ((str = reader.readLine()) != null) {	
				sb.append(str);
				sb.append("\n");
			}
			reader.close();
			return sb.toString();
		} catch (Exception e) {
			e.printStackTrace();
			return null;
		}
	}

}
