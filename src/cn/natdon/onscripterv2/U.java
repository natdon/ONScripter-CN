package cn.natdon.onscripterv2;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileOutputStream;
import java.io.FileReader;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.view.View;
import android.widget.ListAdapter;
import android.widget.ListView;
import cn.natdon.onscripterv2.command.Command;
import cn.natdon.onscripterv2.command.CommandHandler;

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

	/**
	 * Scroll view to the center of the list
	 * @param view
	 * @param parent
	 */
	public static void scrollViewToCenter(View view, ListView parent) {
		ListAdapter items = parent.getAdapter();
		int viewY = view.getTop() + view.getHeight() / 2 - parent.getHeight() / 2;
		if(viewY < 0 && parent.getFirstVisiblePosition() == 0){
			parent.smoothScrollToPosition(0);
		}else if(viewY > 0 && parent.getLastVisiblePosition() == items.getCount() - 1){
			parent.smoothScrollToPosition(items.getCount() - 1);
		}else{
			Command.invoke(SCROLL_LIST_FOR_DISTANCE_IN_ANY_MILLIS)
			.args(parent, viewY, 300).only().sendDelayed(100);
		}
	}

	// Async Operation Block {{{

	static {
		// Register Async Operation
		cn.natdon.onscripterv2.command.Command.register(U.class);
	}

	public static final int SCROLL_LIST_FOR_DISTANCE_IN_ANY_MILLIS = 10;

	@CommandHandler(id = SCROLL_LIST_FOR_DISTANCE_IN_ANY_MILLIS)
	public static void SCROLL_LIST_FOR_DISTANCE_IN_ANY_MILLIS(ListView v, int distance, int duration) {
		v.smoothScrollBy(distance, duration);
	}

	// }}}


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

	public static void write(File file, String str) {
		if(file.exists())
			file.delete();
		try {
			file.createNewFile();
			FileOutputStream sout = new FileOutputStream(file);
			sout.write(str.getBytes());
			sout.close();
		} catch (Exception e) {
			e.printStackTrace();
		}
	}

	public static boolean supportVideoMedia(String name) {
		return 
				name.endsWith(".avi") || name.endsWith(".mp4") || 
				name.endsWith(".mpg") || name.endsWith(".rmvb") || 
				name.endsWith(".mpeg") || name.endsWith(".flv") || 
				name.endsWith(".rm") || name.endsWith(".f4v") || 
				name.endsWith(".hlv") || name.endsWith(".wmv") || 
				name.endsWith(".3gp") || 
				name.endsWith(".mkv");
	}

	public static boolean supportAudioMedia(String name) {
		return 
				name.endsWith(".mp3") || name.endsWith(".wma") || 
				name.endsWith(".flac") || name.endsWith(".ape") || 
				name.endsWith(".ogg") || name.endsWith(".m4a") || 
				name.endsWith(".aac");
	}



}
