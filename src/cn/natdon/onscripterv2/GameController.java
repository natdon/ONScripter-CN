package cn.natdon.onscripterv2;

import cn.natdon.onscripterv2.widget.VideoView;

import android.app.Activity;
import android.content.pm.ActivityInfo;
import android.os.Build;
import android.os.Bundle;
import android.view.KeyEvent;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.RelativeLayout;
import android.widget.TextView;
import android.widget.Toast;

/**
 * Middle class to implement the actions on GameEngine
 * @author trinity
 *
 */
public class GameController extends Activity {

	protected ListView games;
	protected ImageView cover, background;
	protected TextView gametitle;
	protected VideoView preview;
	protected RelativeLayout videoframe;
	protected ImageView btn_settings, btn_about;

	private <T> T $(int id) {
		return U.$(findViewById(id));
	}
	
	private void findViews() {
		games = $(R.id.games);
		cover = $(R.id.cover);
		background = $(R.id.background);
		gametitle = $(R.id.gametitle);
		preview = $(R.id.surface_view);
		videoframe = $(R.id.videoframe);
		btn_settings = $(R.id.btn_settings);
		btn_about = $(R.id.btn_about);
	}

	
	protected void configForGame(Game item) {
		// TODO Code for display config page for stand-alone game
	}
	
	protected void startGame(Game item) {
		// TODO Code to run a game in GameEngine
	}
	
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		if(Build.VERSION.SDK_INT < 9) {
			setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
		}
		setContentView(R.layout.activity_main);
		findViews();
		
		// TODO Code To initialize GameEngine
	}
	
	public void onPause() {
		super.onPause();
	}
	
	public void onResume() {
		super.onResume();
		
	}

	private long last_backkey_pressed = 0;
	
	public boolean onKeyUp(int keyCode, KeyEvent msg) {
        if (keyCode == KeyEvent.KEYCODE_BACK) {
            if (msg.getEventTime()-last_backkey_pressed<2000) {
                finish();
            } else {
                Toast.makeText(
                		this, 
                		R.string.notify_exit, Toast.LENGTH_SHORT
                		).show();
                last_backkey_pressed=msg.getEventTime();
            }
            return true;
        }
		return super.onKeyDown(keyCode, msg);
	}
	
}
