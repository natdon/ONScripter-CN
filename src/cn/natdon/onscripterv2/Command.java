package cn.natdon.onscripterv2;

import cn.natdon.onscripterv2.anim.StateIO;
import cn.natdon.onscripterv2.widget.VideoView;

import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.widget.ArrayAdapter;
import android.widget.ListView;

/**
 * This class is the utility for UIHandler
 * Construction, argument assignment and some other works
 * @author trinity
 *
 */
public class Command {

	private static <T> T $(Object o) {
		return U.$(o);
	}

	// obj - Runnable
	public static final int RUN = 0;

	// obj - VideoView
	public static final int LOOP_VIDEO_PREVIEW = 13;

	// obj - VideoView
	public static final int RELEASE_VIDEO_PREVIEW = 14;

	// obj - VideoView
	public static final int UPDATE_VIDEO_SIZE = 16;

	// obj - listview, arg1 - distance, arg2 - duration
	public static final int SCROLL_LIST_FOR_DISTANCE_IN_ANY_MILLIS = 10;

	// obj - MainActivity
	public static final int MAINACTIVITY_PLAY_VIDEO = 38;
	
	// obj - ListAdapter, getData() - Game
	public static final int ADD_ITEM_TO_LISTADAPTER = 102;

	// obj - ListAdapter
	public static final int DATASET_CHANGED_LISTADAPTER = 103;
	
	// obj - StateIO, arg1 - cond, arg2 - next state
	public static final int STATE_CONTROL_COND = 209;
	
	// obj - StateIO, arg1 - next state
	public static final int STATE_CONTROL = 208;
	
	private static Handler Commander = new Handler() {

		public void handleMessage(Message msg) {
			VideoView videoview;
			ArrayAdapter<Game> adapter;
			StateIO sio;
			switch(msg.what){
			case LOOP_VIDEO_PREVIEW:
				videoview = $(msg.obj);
				videoview.seekTo(0);
				videoview.start();
				break;
			case SCROLL_LIST_FOR_DISTANCE_IN_ANY_MILLIS:
				ListView listview = $(msg.obj);
				listview.smoothScrollBy(msg.arg1, msg.arg2);
				break;
			case ADD_ITEM_TO_LISTADAPTER:
				adapter = $(msg.obj);
				adapter.add(Game.fromBundle(msg.getData()));
				// Auto Notify DataSet Change
				Command.invoke(DATASET_CHANGED_LISTADAPTER).of(adapter).only().sendDelayed(200);
				break;
			case DATASET_CHANGED_LISTADAPTER:
				adapter = $(msg.obj);
				adapter.notifyDataSetChanged();
				break;
			case MAINACTIVITY_PLAY_VIDEO:
				ONScripter mainactivity = $(msg.obj);
				mainactivity.playVideo();
				break;
			case RELEASE_VIDEO_PREVIEW:
				videoview = $(msg.obj);
				videoview.setVideoURI(null);
				break;
			case UPDATE_VIDEO_SIZE:
				videoview = $(msg.obj);
				videoview.setVideoLayout(VideoView.VIDEO_LAYOUT_PREVIOUS, 0.0f);
				break;
			case STATE_CONTROL:
				sio = $(msg.obj);
				sio.gotoState(msg.arg1);
			case STATE_CONTROL_COND:
				sio = $(msg.obj);
				sio.gotoState(msg.arg1, msg.arg2);
			default:
				if(msg.obj instanceof Runnable) {
					Runnable runnable = $(msg.obj);
					runnable.run();
				}
			}
		}

	};

	public static Command invoke(Runnable run) {
		Command cmd = new Command();
		cmd.msg.what = RUN;
		cmd.msg.obj = run;
		return cmd;
	}

	public static Command invoke(int progId) {
		Command cmd = new Command();
		cmd.msg.what = progId;
		return cmd;
	}

	public static void revoke(int progId) {
		Commander.removeMessages(progId);
	}

	public static void revoke(int progId, Object obj) {
		Commander.removeMessages(progId, obj);
	}

	private final Message msg;

	private Command() {
		msg = Message.obtain(Commander);
	}

	public Command of(Object obj) {
		msg.obj = obj;
		return this;
	}

	public Command args(int arg) {
		msg.arg1 = arg;
		return this;
	}

	public Command args(int arg1, int arg2) {
		msg.arg1 = arg1;
		msg.arg2 = arg2;
		return this;
	}

	public Command args(Bundle bundle) {
		msg.setData(bundle);
		return this;
	}
	
	public Command only() {
		if(msg.obj != null) {
			Commander.removeMessages(msg.what, msg.obj);
		}else{
			Commander.removeMessages(msg.what);
		}
		return this;
	}
	
	public Command exclude(int progId) {
		Commander.removeMessages(progId);
		return this;
	}

	public Command exclude(int progId, Object obj) {
		Commander.removeMessages(progId, obj);
		return this;
	}

	public Message getMessage() {
		return msg;
	}

	public void send() {
		Commander.sendMessage(msg);
	}

	public void sendAtTime(long timeMillis) {
		Commander.sendMessageAtTime(msg, timeMillis);
	}

	public void sendDelayed(long delay) {
		Commander.sendMessageDelayed(msg, delay);
	}

}
