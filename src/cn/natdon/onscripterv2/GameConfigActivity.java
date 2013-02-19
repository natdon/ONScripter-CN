package cn.natdon.onscripterv2;

import java.io.File;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.ComponentName;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.Intent.ShortcutIconResource;
import android.content.pm.ActivityInfo;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.os.Build;
import android.os.Bundle;
import android.view.KeyEvent;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;

public class GameConfigActivity extends Activity {

	private String original_config;
	private Game mGame;
	
	private ListView config_list;
	
	private <T> T $(int id) {
		return U.$(findViewById(id));
	}
	
	private void findViews() {
		config_list = $(R.id.config_list);
	}
	
	private void loadGameContent() {
		
	}

	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		if(Build.VERSION.SDK_INT < 9) {
			setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
		}
		setContentView(R.layout.game_config);
		findViews();
		
		/*Intent intent = getIntent();
		
		String path = intent.getStringExtra(Constant.EXTRA_GAME_PATH);
		
		mGame = Game.scanGameDir(new File(path));
		
		try {
			original_config = mGame.toJSON().toString();
		} catch (Exception e) {
			original_config = null;
		}*/
		
		loadGameContent();
	}
	
	/*private boolean save() {
		String config;
		try {
			config = mGame.toJSON().toString();
		} catch (Exception e) {
			config = null;
		}
		if(config == null) return false;
		if(original_config == config)
			setResult(RESULT_CANCELED);
		else 
			setResult(RESULT_OK);
		mGame.writeJSON();
		return original_config != config;
	}*/

	protected void onDestroy() {
		super.onDestroy();
	}

	public boolean onKeyUp(int keyCode, KeyEvent event) {
		return super.onKeyUp(keyCode, event);
	}

	protected void onPause() {
		super.onPause();
	}

	protected void onResume() {
		super.onResume();
	}
	

	
	
}
