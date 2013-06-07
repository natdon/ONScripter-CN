/*
Simple DirectMedia Layer
Java source code (C) 2009-2011 Sergii Pylypenko
  
This software is provided 'as-is', without any express or implied
warranty.  In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:
  
1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required. 
2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/
/*
 2012/7 Modified by AKIZUKI Katane
 */

package cn.natdon.onscripterv2;

import io.vov.vitamio.MediaPlayer;
import io.vov.vitamio.MediaPlayer.OnCompletionListener;
import io.vov.vitamio.MediaPlayer.OnErrorListener;
import io.vov.vitamio.Vitamio;

import java.io.File;
import java.io.FileFilter;
import java.math.BigDecimal;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Comparator;

import org.renpy.android.PythonActivity;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.app.AlertDialog;
import android.app.ProgressDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnKeyListener;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.content.pm.ActivityInfo;
import android.content.pm.PackageManager.NameNotFoundException;
import android.content.res.Configuration;
import android.graphics.Bitmap;
import android.graphics.Color;
import android.graphics.drawable.Drawable;
import android.media.AudioManager;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.PowerManager;
import android.telephony.TelephonyManager;
import android.text.InputType;
import android.util.Log;
import android.view.Display;
import android.view.Gravity;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.View.OnTouchListener;
import android.view.ViewGroup.LayoutParams;
import android.view.Window;
import android.view.WindowManager;
import android.view.animation.Animation;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.PopupWindow;
import android.widget.ScrollView;
import android.widget.SeekBar;
import android.widget.SeekBar.OnSeekBarChangeListener;
import android.widget.TextView;
import android.widget.Toast;
import cn.natdon.onscripterv2.Button.OkCancelButton;
import cn.natdon.onscripterv2.Button.RunButton;
import cn.natdon.onscripterv2.Button.SaoButton;
import cn.natdon.onscripterv2.Button.ShortcutButton;
import cn.natdon.onscripterv2.Button.SwapButton;
import cn.natdon.onscripterv2.Class.Shortcut;
import cn.natdon.onscripterv2.Class.UnZip;
import cn.natdon.onscripterv2.Downloader.DataDownloader;
import cn.natdon.onscripterv2.anim.AnimationAutomata;
import cn.natdon.onscripterv2.anim.AnimationBuilder;
import cn.natdon.onscripterv2.anim.AutomataAction;
import cn.natdon.onscripterv2.anim.StateRunner;
import cn.natdon.onscripterv2.command.Command;
import cn.natdon.onscripterv2.command.CommandHandler;
import cn.natdon.onscripterv2.decoder.BackgroundDecoder;
import cn.natdon.onscripterv2.decoder.CoverDecoder;
import cn.natdon.onscripterv2.widget.AudioPlayer;
import cn.natdon.onscripterv2.widget.MediaController;
import cn.natdon.onscripterv2.widget.VideoView;
import cn.natdon.onscripterv2.widget.VideoViewContainer;

import com.footmark.utils.cache.FileCache;
import com.footmark.utils.image.ImageManager;
import com.footmark.utils.image.ImageSetter;
import com.umeng.analytics.MobclickAgent;
import com.umeng.update.UmengUpdateAgent;


@SuppressLint("NewApi")
public class ONScripter extends Activity implements OnItemClickListener,Button.OnClickListener, DialogInterface.OnClickListener,OnTouchListener, Runnable{

public static TextView about, Repair, Language, ONSSetting, SetOrientation, DirSetting, VerChange ,TextSize, OnlineVideo, ColorText;
public static PopupWindow m_popupWindow,wd_popupWindow,ver_popupWindow,size_popupWindow,font_popupWindow,config_popupWindow,setting_popupWindow,video_popupWindow;	
	
	private Drawable bg2 = null;

	public String myname, Gamenames;
	public static String extra;
	public static String mysetting;
	
	private DataDownloader downloader = null;
	private AlertDialog.Builder alertDialogBuilder = null;
	private AlertDialog ErroralertDialog = null;
	private ProgressDialog progDialog = null;
	private PowerManager.WakeLock wakeLock = null;
	
	private int num_file = 0;
	private byte[] buf = null;
	
	private float x;
	private float y;
	private float startX;
	

	private boolean isMoved = false;
	private boolean isSelect = false;


	private ColorPickerDialog Colordialog;


	public ONScripter mActivity;
	
	/******************************************************************************************************************/
	
	{
		// Set the priority, trick useful for some CPU
		Thread.currentThread().setPriority(Thread.MAX_PRIORITY);
	}

	private static final String DIRECTORY = "ONS";
	
	// Controls Initialization Block {{{
	private ListView games;
	private ImageView cover, background;
	private TextView gametitle;
	private VideoViewContainer videoframe;
	private VideoView preview;
	private ImageView btn_settings, btn_about;
	
	private int mPlaybackErrCounter = 0;
	private static final int PLAYBACK_ERR_TOLERANCE = 3;
	private static final int PLAYBACK_ERR_IGNORED_TOLERANCE = 5;
	
	private static final int TOUCH_SLOP = 100;

	private <T> T $(int id) {
		return U.$(findViewById(id));
	}
	
	private void findViews() {
		games = $(R.id.games);
		cover = $(R.id.cover);
		background = $(R.id.background);
		gametitle = $(R.id.gametitle);
		videoframe = $(R.id.videoframe);
		preview = videoframe.getVideoView();
		btn_settings = $(R.id.btn_settings);
		btn_about = $(R.id.btn_about);
	}

	// }}}
	
	// ImageManager Block {{{
	private ImageManager imgMgr;
	
	private void initImageManager() {
		destroyImageManager();
		if(Environment.MEDIA_MOUNTED.equals(
				Environment.getExternalStorageState())){
			imgMgr = new ImageManager(new FileCache(
					new File(
							Environment.getExternalStorageDirectory(),
							DIRECTORY + "/.cover")));
		}else{
			imgMgr = new ImageManager(new FileCache(
					new File(
							getCacheDir(),
							"cover")));
		}
	}

	private void destroyImageManager() {
		if(imgMgr != null)
			imgMgr.shutdown();
	}

	// }}}
	
	// VideoPlayer Block {{{
	private static volatile boolean isVideoInitialized = false;

	private AudioPlayer mAudioPlayer = null;
	
	private void configureVideoPlayer() {
		preview.setVideoQuality(MediaPlayer.VIDEOQUALITY_HIGH);
		preview.setOnCompletionListener(new OnCompletionListener() {

			public void onCompletion(MediaPlayer player) {
				mPlaybackErrCounter = 0;
				Command.invoke(LOOP_VIDEO_PREVIEW).args(preview).send();
			}

		});
		preview.setOnErrorListener(new OnErrorListener() {

			public boolean onError(MediaPlayer player, int framework_err, int impl_err) {
				mStatePreview.gotoState(STATE_COVER_VISIBLE);
				mPlaybackErrCounter++;
				if(mPlaybackErrCounter < PLAYBACK_ERR_TOLERANCE) {
					Toast.makeText(getApplicationContext(), 
							R.string.error_play_video, Toast.LENGTH_LONG).show();
				} else {
					if(mPlaybackErrCounter < PLAYBACK_ERR_IGNORED_TOLERANCE) {
						Toast.makeText(getApplicationContext(), 
								R.string.error_repeated_playfail, Toast.LENGTH_LONG).show();
					}
				}
				return true;
			}

		});
		preview.setMediaController(new MediaController(this));

		mAudioPlayer = new AudioPlayer(this);
		mAudioPlayer.setMediaController(new MediaController(this), preview.getRootView());
		
		mAudioPlayer.setOnCompletionListener(new OnCompletionListener() {

			public void onCompletion(MediaPlayer player) {
				mPlaybackErrCounter = 0;
				Command.invoke(LOOP_AUDIO_PLAY).args(mAudioPlayer).send();
			}

		});
		
		mAudioPlayer.setOnErrorListener(new OnErrorListener() {

			public boolean onError(MediaPlayer player, int framework_err, int impl_err) {
				mPlaybackErrCounter++;
				if(mPlaybackErrCounter < PLAYBACK_ERR_TOLERANCE) {
					Toast.makeText(getApplicationContext(), 
							R.string.error_play_audio, Toast.LENGTH_LONG).show();
				} else {
					if(mPlaybackErrCounter < PLAYBACK_ERR_IGNORED_TOLERANCE) {
						Toast.makeText(getApplicationContext(), 
								R.string.error_repeated_playfail, Toast.LENGTH_LONG).show();
					}
				}
				return true;
			}

		});
		
		cover.setOnClickListener(new OnClickListener() {

			long lastClick = 0;
			
			public void onClick(View v) {
				if(mAudioPlayer.isInPlaybackState()) {
					mAudioPlayer.toggleMediaControlsVisiblity();
				} else {
					long time = System.currentTimeMillis();
					if(time - lastClick < 500) {
						Command.invoke(ACTION_AFTER_DISPLAY_COVER)
						.args(ONScripter.this).only().send();
						lastClick = 0;
					}else{
						lastClick = time;
					}
				}
			}
			
		});
		
		// Initialize the Vitamio codecs
		if(!Vitamio.isInitialized(this)) {
			new AsyncTask<Object, Object, Boolean>() {

				protected void onPreExecute() {
					isVideoInitialized = false;
				}

				protected Boolean doInBackground(Object... params) {
					Thread.currentThread().setPriority(Thread.MIN_PRIORITY);
					boolean inited = Vitamio.initialize(ONScripter.this);
					Thread.currentThread().setPriority(Thread.NORM_PRIORITY);
					return inited;
				}

				protected void onPostExecute(Boolean inited) {
					if (inited) {
						isVideoInitialized = true;
						
						// Play video if exists
						Command.invoke(ACTION_AFTER_DISPLAY_COVER).args(ONScripter.this).only().sendDelayed(3000);
					}
				}

			}.execute();
		}else{
			isVideoInitialized = true;
		}
	}
	
	// }}}

	// Game Entries Block {{{
	private GameAdapter items;
	

	// }}}
	
	// Background Image Animation & Action Block {{{
	private StateRunner mStateBackground = new StateRunner(STATE_BKG_VISIBLE);;

	private static final int STATE_BKG_HIDDEN = 3000;
	private static final int STATE_BKG_VISIBLE = 3001;
	
	private void setupBackgroundAutomata() {
		AnimationAutomata.refer(mStateBackground).target(background)
		
		.edit(STATE_BKG_VISIBLE, STATE_BKG_HIDDEN)
		.setAnimation(AnimationBuilder.create()
				.alpha(1, 0).animateFor(1000).accelerated(1.5f)
				.build())
		.addAction(new AutomataAction() {
			public void onAnimationEnd(Animation animation) {
				Command.invoke(TRY_DISPLAY_BKG)
				.args(ONScripter.this).send();
			}
		})
		
		.edit(STATE_BKG_HIDDEN, STATE_BKG_VISIBLE)
		.setAnimation(AnimationBuilder.create()
				.alpha(0, 1).animateFor(1000).decelerated(1.5f)
				.build())
		;
	}

	public void tryDisplayBackground() {
		if(background.getTag() instanceof Bitmap && !mStateBackground.isAnyAnimatingAutomata()) {
			background.setImageBitmap((Bitmap) background.getTag());
			background.setBackgroundDrawable(null);
			background.setTag(null);
			mStateBackground.gotoState(STATE_BKG_HIDDEN, STATE_BKG_VISIBLE);
		}
		if(background.getTag() instanceof Drawable) {
			background.setImageDrawable((Drawable) background.getTag());
			background.setBackgroundDrawable(null);
			background.setTag(null);
			mStateBackground.gotoState(STATE_BKG_HIDDEN, STATE_BKG_VISIBLE);
		}
	}

	private void updateBackground(String url) {
		Object o = background.getTag();
		if(o instanceof ImageSetter) {
			((ImageSetter) o).cancel();
		}
		mStateBackground.gotoState(STATE_BKG_HIDDEN);
		imgMgr.requestImageAsync(url, new ImageSetter(background) {

			protected void act() {
				background.setTag(image().bmp());
				Command.invoke(TRY_DISPLAY_BKG)
				.args(ONScripter.this).send();
			}

		}, new BackgroundDecoder());
	}
	// }}}
		
	// Cover/VideoPlayer/AudioPlayer Animation & Action Block {{{
	private StateRunner mStatePreview = new StateRunner(STATE_COVER_VISIBLE);

	private static final int STATE_COVER_HIDDEN = 2000;
	private static final int STATE_COVER_VISIBLE = 2001;
	private static final int STATE_AUDIO_PLAY = 2002;
	private static final int STATE_VIDEO_PLAY = 2003;
	
	private void setupCoverPreviewAutomata() {
		AnimationAutomata.refer(mStatePreview).target(cover)
		
		.edit(STATE_COVER_VISIBLE, STATE_COVER_HIDDEN)
		.setAnimation(AnimationBuilder.create()
				// Set the valtype of the value to be inturrpted
				.valtype(Animation.RELATIVE_TO_SELF)
				// Add a Scale Animation
				.scale(1.0f, 0.6f, 1.0f, 0.6f, 0.5f, 0.5f).animateFor(100)
				// Add an Alpha Animation
				.alpha(1, 0).animateFor(100)
				// Build Animation
				.build())
		.addAction(new AutomataAction() {
			public void Before(Animation animation) {
				getAutomata().target().setVisibility(View.VISIBLE);
			}
			public void After(Animation animation) {
				getAutomata().target().setVisibility(View.GONE);
				Command.invoke(TRY_DISPLAY_COVER).only()
				.args(ONScripter.this).send();
			}
		})
		
		.edit(STATE_COVER_HIDDEN, STATE_COVER_VISIBLE)
		.setAnimation(AnimationBuilder.create()
				// Set the valtype of the value to be inturrpted
				.valtype(Animation.RELATIVE_TO_SELF)
				// Add a Scale Animation
				.scale(0.5f, 1.0f, 0.5f, 1.0f, 0.5f, 0.5f).overshoot()
				.animateFor(300)
				// Add an Alpha Animation
				.alpha(0, 1).animateFor(300)
				.build())
		.addAction(new AutomataAction() {
			public void Before(Animation animation) {
				getAutomata().target().setVisibility(View.VISIBLE);
			}
		})
		
		.edit(STATE_COVER_VISIBLE, STATE_AUDIO_PLAY)
		.addAction(new AutomataAction() {
			public void onStateChanged(int from, int to) {
				startAudioPlay();
			}
			private void startAudioPlay() {
				Game item = items.getSelectedItem();
				if(item.audio != null && isVideoInitialized) {
					mAudioPlayer.setAudioURI(null);
					mAudioPlayer.setAudioPath(item.audio);
				}
			}
		})
		
		.edit(STATE_AUDIO_PLAY, STATE_COVER_VISIBLE)
		.addAction(new AutomataAction() {
			public void onStateChanged(int from, int to) {
				releaseAudioPlay();
			}
			private void releaseAudioPlay() {
				mAudioPlayer.stopPlayback();
				mAudioPlayer.setAudioURI(null);
				mAudioPlayer.setMediaControlsVisibility(false);
			}
		})
		
		.edit(STATE_AUDIO_PLAY, STATE_COVER_HIDDEN)
		.setAnimation(STATE_COVER_VISIBLE, STATE_COVER_HIDDEN)
		.addAction(STATE_COVER_VISIBLE, STATE_COVER_HIDDEN)
		.addAction(STATE_AUDIO_PLAY, STATE_COVER_VISIBLE)
		
		.edit(STATE_COVER_VISIBLE, STATE_VIDEO_PLAY)
		// Code Control Video Play/Stop cannot dependent on the animation of cover
		.setAnimation(AnimationBuilder.create()
				.alpha(1, 0).animateFor(300)
				.build())
		.addAction(new AutomataAction() {
			public void Before(Animation animation) {
				getAutomata().target().setVisibility(View.VISIBLE);
			}
			public void After(Animation animation) {
				getAutomata().target().setVisibility(View.GONE);
			}
		})
		
		.edit(STATE_VIDEO_PLAY, STATE_COVER_VISIBLE)
		.setAnimation(AnimationBuilder.create()
				.alpha(0, 1).animateFor(300)
				.build())
		.setAction(STATE_COVER_HIDDEN, STATE_COVER_VISIBLE)
				
		.edit(STATE_VIDEO_PLAY, STATE_COVER_HIDDEN)
		.setAction(STATE_COVER_VISIBLE, STATE_COVER_HIDDEN)
		
		;
		
		AnimationAutomata.refer(mStatePreview).target(videoframe)
		
		.edit(STATE_COVER_VISIBLE, STATE_VIDEO_PLAY)
		.setAnimation(AnimationBuilder.create()
				.alpha(0, 1).animateFor(300)
				.build())
		.addAction(new AutomataAction() {
			public void Before(Animation animation) {
				getAutomata().target().setVisibility(View.VISIBLE);
			}
			public void After(Animation animation) {
				startVideoPlay();
			}
			private void startVideoPlay() {
				Game item = items.getSelectedItem();
				if(item.video != null && isVideoInitialized) {
					videoframe.setVisibility(View.VISIBLE);
					Command.revoke(RELEASE_VIDEO_PREVIEW);
					preview.setVideoURI(null);
					preview.setVideoPath(item.video);
				}
			}
		})
		
		.edit(STATE_COVER_HIDDEN, STATE_VIDEO_PLAY)
		.setAnimation(STATE_COVER_VISIBLE, STATE_VIDEO_PLAY)
		.setAction(STATE_COVER_VISIBLE, STATE_VIDEO_PLAY)
		
		.edit(STATE_VIDEO_PLAY, STATE_COVER_VISIBLE)
		.setAnimation(AnimationBuilder.create()
				.alpha(1, 0).animateFor(300)
				.build())
		.addAction(new AutomataAction() {
			public void onStateChanged(int from, int to) {
				releaseVideoPlay();
			}
			public void After(Animation animation) {
				getAutomata().target().setVisibility(View.GONE);
			}
			public void releaseVideoPlay() {
				// Clear Video Player
				if(preview.isInPlaybackState()){
					preview.stopPlayback();
				}
				preview.setVisibility(View.GONE);
				Command.invoke(RELEASE_VIDEO_PREVIEW)
				.args(preview).sendDelayed(2000);
			}
		})
		
		.edit(STATE_VIDEO_PLAY, STATE_COVER_HIDDEN)
		.setAnimation(STATE_VIDEO_PLAY, STATE_COVER_VISIBLE)
		.setAction(STATE_VIDEO_PLAY, STATE_COVER_VISIBLE)
		;
		
	}
	
	public void tryDisplayCover() {
		if(cover.getTag() instanceof Bitmap && !mStatePreview.isAnyAnimatingAutomata(cover)) {
			cover.setImageBitmap((Bitmap) cover.getTag());
			cover.setBackgroundDrawable(null);
			cover.setTag(null);
			mStatePreview.gotoState(STATE_COVER_HIDDEN, STATE_COVER_VISIBLE);
		}
		if(cover.getTag() instanceof Drawable) {
			cover.setImageDrawable((Drawable) cover.getTag());
			cover.setBackgroundDrawable(null);
			cover.setTag(null);
			mStatePreview.gotoState(STATE_COVER_HIDDEN, STATE_COVER_VISIBLE);
		}
	}
	
	public void checkActionAfterDisplayCover() {
		if(items == null) return;
		Game item = items.getSelectedItem();
		if(item == null) return;
		if(isVideoInitialized) {
			if(item.video != null) {
				mStatePreview.gotoState(STATE_VIDEO_PLAY);
			}else if(item.audio != null) {
				mStatePreview.gotoState(STATE_AUDIO_PLAY);
			}
		}
	}

	private void updateCover(final String url, final boolean coverToBkg) {
		Object o = cover.getTag();
		if(o instanceof ImageSetter) {
			((ImageSetter) o).cancel();
		}
		
		imgMgr.requestImageAsync(url,
				new ImageSetter(cover) {

			protected void act() {
				cover.setTag(image().bmp());
				Command.invoke(TRY_DISPLAY_COVER).args(ONScripter.this).only().send();
				if(coverToBkg) {
					String background = CoverDecoder.getThumbernailCache(url);
					// Exception for Web Images
					if(background == null)
						background = CoverDecoder.getThumbernailCache(image().file().getAbsolutePath());
					if(background != null) {
						updateBackground(background);
					}
				}
			}

		}, new CoverDecoder(cover.getWidth(), cover.getHeight()));

		mStatePreview.gotoState(STATE_COVER_HIDDEN);
	}
	
	// }}}
		
		private void loadGameItem(Game item) {
			
			gametitle.setText(item.title);
			myname = gametitle.getText().toString().trim();  

			if(item.background != null) {
				updateBackground(item.background);
			}

			if(item.cover != null) {
				updateCover(item.cover, item.background == null);
			}else{
				cover.setTag(getResources().getDrawable(R.drawable.dbkg_und));
				mStatePreview.gotoState(STATE_COVER_HIDDEN);
				if(item.background == null) {
					background.setTag(getResources().getDrawable(R.drawable.dbkg_und_blur));
					mStateBackground.gotoState(STATE_BKG_HIDDEN);
				}
			}
			
			// Perform Action After Display Cover in a Time-out way
			Command.invoke(ACTION_AFTER_DISPLAY_COVER).args(ONScripter.this).only().sendDelayed(4000);
			
		}
		
		private void setupUIAutomata() {
			setupBackgroundAutomata();
			setupCoverPreviewAutomata();
		}
	
	

	
	/******************************************************************************************************************/


	
	
	
	
	@SuppressWarnings("deprecation")
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		
		requestWindowFeature(Window.FEATURE_NO_TITLE);
		getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
				WindowManager.LayoutParams.FLAG_FULLSCREEN);
		
		UmengUpdateAgent.setUpdateOnlyWifi(false);
		UmengUpdateAgent.update(this);
		
				
		//setRequestedOrientation( ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE ); //for Test

		getWindow().setFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON, WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
		
		setVolumeControlStream(AudioManager.STREAM_MUSIC);
		
		instance = this;
		
		cn.natdon.onscripterv2.Settings.LoadGlobals(this);

		Display disp = ((WindowManager) this
				.getSystemService(Context.WINDOW_SERVICE)).getDefaultDisplay();
		ONSVariable.dw = disp.getWidth();
		ONSVariable.dh = disp.getHeight();

		checkCurrentDirectory(false);

//


		SharedPreferences sp = getSharedPreferences("pref", MODE_PRIVATE);
		ONSVariable.textsize = sp.getInt("textsize", 15);
		
		/*if(Globals.APP_LAUNCHER_USE){
			final Intent intent = getIntent();
			extra = intent.getStringExtra("path");
			mysetting = intent.getStringExtra("mysetting");
			if (extra != null) {
				FSetting(mysetting);
				Globals.CurrentDirectoryPath = extra;
				runApp();
			} else{
			runAppLauncher();
			}
		} else {
			runAppLaunchConfig();
		}*/
		
			final Intent intent = getIntent();
			extra = intent.getStringExtra("path");
			mysetting = intent.getStringExtra("mysetting");
			if (extra != null) {
				FSetting(mysetting);
				Globals.CurrentDirectoryPath = extra;
				runApp();
				this.finish();
			}

			SharedPreferences sp2 = getSharedPreferences("myver", MODE_PRIVATE);
			int about = sp2.getInt("about", 13);
			if (about == 13) {
				About();
				Editor e = getSharedPreferences("myver", MODE_PRIVATE).edit();
				e.putInt("about", 14);
				e.commit();
			}
			
			if(Build.VERSION.SDK_INT < 9) {
				setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
			}
			
			TelephonyManager telephony = (TelephonyManager) this.getSystemService(Context.TELEPHONY_SERVICE);
	        int type = telephony.getPhoneType();
			
			BigDecimal b = new BigDecimal((float)(ONSVariable.dw)/(float)(ONSVariable.dh));  
			double size = b.setScale(2,   BigDecimal.ROUND_HALF_UP).doubleValue();  
			if(((float)(ONSVariable.dw)/(float)(ONSVariable.dh)) == 1.5 ||  size == 1.33)
			{
				setContentView(R.layout.activity_ft);
			}else if(type == TelephonyManager.PHONE_TYPE_NONE){
				setContentView(R.layout.activity_pad);
			}
			else{
				setContentView(R.layout.activity_main);
			}
			findViews();

			// Pass parameters to CoverDecoder to get better performance
			CoverDecoder.init(getApplicationContext(), cover.getWidth(), cover.getHeight());

			initImageManager();

			configureVideoPlayer();
			
			setupUIAutomata();
			
			// Initializing data and binding to ListView
			items = new GameAdapter(this, R.layout.gamelist_item, new ArrayList<Game>());
			games.setAdapter(items);
			games.setOnItemClickListener(this);
			games.setOnTouchListener(this);
			
			Command.invoke(
					new Runnable() { public void run() {loadCurrentDirectory();}}
			).sendDelayed(500);
			
			
			
			btn_settings.setOnClickListener(this);
			btn_about.setOnClickListener(this);
			items.setOnConfigClickListener(this);
			items.setOnPlayClickListener(this);
			
			
	}

	public boolean checkCurrentDirectory(boolean quitting)
	{
		String curDirPath = Globals.CurrentDirectoryPathForLauncher;
		if(curDirPath != null && !curDirPath.equals("")){
			return true;
		}
		
		StringBuffer buf = new StringBuffer();
		for(String s: Globals.CurrentDirectoryPathArray){
			if(buf.length()>0) {
				buf.append("\n");
			}
			buf.append(s);
		}
		curDirPath = buf.toString();

		AlertDialog.Builder alertDialogBuilder = new AlertDialog.Builder(this);
		alertDialogBuilder.setTitle(getString(R.string.Error));
		alertDialogBuilder.setMessage(getString(R.string.Launch_CouldNotOpenDirectory) + "\n" + curDirPath);
		if(quitting){
			alertDialogBuilder.setPositiveButton(getString(R.string.Quit), new DialogInterface.OnClickListener(){
				public void onClick(DialogInterface dialog, int whichButton) {
					finish();
				}
			});
		} else {
			alertDialogBuilder.setPositiveButton(getString(R.string.Launch_SetDirectory), new DialogInterface.OnClickListener(){
					public void onClick(DialogInterface dialog, int whichButton) {	
						chooseDir();
					}
				});
		}
		alertDialogBuilder.setCancelable(false);
		AlertDialog alertDialog = alertDialogBuilder.create();
		alertDialog.show();
		
		return false;
	}
	
	//
	
	
		
		
		
		
		private File [] mDirFileArray;

		public String iconPath = null;


		public void FreeMemory(){
			bg2 = null;
			if(games != null){
			games.setBackgroundDrawable(null);
			games.setAdapter(null);
			games = null;
			}
		}


			
		
		public void loadCurrentDirectory()
		{
			if(Globals.CurrentDirectoryPathForLauncher == null || Globals.CurrentDirectoryPathForLauncher.equals("")){
				//mCurDirText.setText("");
			} else {
				//mCurDirText.setText(Globals.CurrentDirectoryPathForLauncher);
				

				try {
					
					items.clear();
					items.notifyDataSetChanged();
					
					File searchDirFile = new File(Globals.CurrentDirectoryPathForLauncher);
				
					mDirFileArray = searchDirFile.listFiles(new FileFilter() {
						public boolean accept(File file) {
							return (!file.isHidden() && file.isDirectory() && file.canRead() && (!Globals.CURRENT_DIRECTORY_NEED_WRITABLE || file.canWrite()));
						}
					});
					
					Arrays.sort(mDirFileArray, new Comparator<File>(){
						public int compare(File src, File target){
							return src.getName().compareTo(target.getName());
						}
					});
					
					
					new Thread() {
						
						public void run() {
							Looper.prepare();
							for(File file: mDirFileArray) {
								if(!file.isHidden() && file.isDirectory()) {
									Game g = Game.scanGameDir(file);
									if(g != null) {
										// Add Game to Game List
										Command.invoke(ADD_ITEM_TO_LISTADAPTER)
										.args(items, g).send();
										
									}
								}
							}
						}
						
					}.start();
					
				} catch(Exception e){
					AlertDialog.Builder alertDialogBuilder = new AlertDialog.Builder(ONScripter.this);
					alertDialogBuilder.setTitle(getString(R.string.Error));
					alertDialogBuilder.setMessage(getString(R.string.Launch_CouldNotOpenDirectory) + "\n" + Globals.CurrentDirectoryPathForLauncher);
					alertDialogBuilder.setPositiveButton(getString(R.string.OK), null);
					AlertDialog alertDialog = alertDialogBuilder.create();
					alertDialog.show();
					
					ArrayAdapter<String> arrayAdapter = new ArrayAdapter<String>(ONScripter.this, android.R.layout.simple_list_item_1, new String[0]);
					games.setAdapter(arrayAdapter);
				}
			}
		}

		public void chooseDir()
		{
			String[] items = new String[Globals.CurrentDirectoryValidPathArray.length + 1];
			for(int i = 0; i < Globals.CurrentDirectoryValidPathArray.length; i ++){
				items[i] = Globals.CurrentDirectoryValidPathArray[i];
			}
			items[items.length - 1] = getString(R.string.Other) + "...";
			
			AlertDialog.Builder alertDialogBuilder = new AlertDialog.Builder(this);
			alertDialogBuilder.setInverseBackgroundForced(true);
			alertDialogBuilder.setTitle(getString(R.string.Launch_ChooseDirectory));
			alertDialogBuilder.setItems(items, this);
			alertDialogBuilder.setNegativeButton(getString(R.string.Cancel), null);
			alertDialogBuilder.setCancelable(true);
			AlertDialog alertDialog = alertDialogBuilder.create();
			alertDialog.show();
		}
		
	/*	public void onClick(View v)
		{
			chooseDir();
		}*/
		
		private AlertDialog mDirBrowserDialog = null;
		private File[] mDirBrowserDirFileArray = null;
		private String mDirBrowserCurDirPath = null;
		
		public void onClick(DialogInterface dialog, int which)
		{
			if(dialog == mDirBrowserDialog){
				mDirBrowserCurDirPath   = mDirBrowserDirFileArray[which].getAbsolutePath();
				mDirBrowserDialog       = null;
				mDirBrowserDirFileArray = null;
			} else {
				if(which < Globals.CurrentDirectoryValidPathArray.length){
					Globals.CurrentDirectoryPathForLauncher = Globals.CurrentDirectoryValidPathArray[which];
					cn.natdon.onscripterv2.Settings.SaveGlobals(this);
					loadCurrentDirectory();
					return;
				} else {
					if(Globals.CurrentDirectoryPathForLauncher == null || Globals.CurrentDirectoryPathForLauncher.equals("")){
							mDirBrowserCurDirPath = "/";
					} else {
						mDirBrowserCurDirPath = Globals.CurrentDirectoryPathForLauncher;
					}
				}
			}
			
			try {
				File searchDirFile = new File(mDirBrowserCurDirPath);
				
				mDirBrowserDirFileArray = searchDirFile.listFiles(new FileFilter() {
					public boolean accept(File file) {
						return (file.isDirectory() && file.canRead());
					}
				});
				
				Arrays.sort(mDirBrowserDirFileArray, new Comparator<File>(){
					public int compare(File src, File target){
						return src.getName().compareTo(target.getName());
					}
				});
				
				File parentFile = searchDirFile.getParentFile();
				if(parentFile != null){
					if(parentFile.canRead()){
						File[] newDirFileArray = new File[mDirBrowserDirFileArray.length + 1];
						newDirFileArray[0] = parentFile;
						for(int i=0; i < mDirBrowserDirFileArray.length; i ++){
							newDirFileArray[i+1] = mDirBrowserDirFileArray[i];
						}
						mDirBrowserDirFileArray = newDirFileArray;
					} else {
						parentFile = null;
					}
				}
				String[] dirPathArray = new String[mDirBrowserDirFileArray.length];
				for(int i = 0; i < mDirBrowserDirFileArray.length; i ++){
					dirPathArray[i] = mDirBrowserDirFileArray[i].getName();
				}
				if(parentFile != null && dirPathArray.length > 0){
					dirPathArray[0] = "..";
				}
				
				AlertDialog.Builder alertDialogBuilder = new AlertDialog.Builder(this);
				alertDialogBuilder.setTitle(mDirBrowserCurDirPath);
				alertDialogBuilder.setItems(dirPathArray, this);
				alertDialogBuilder.setPositiveButton(getString(R.string.Launch_SetDirectory), new DialogInterface.OnClickListener(){
					public void onClick(DialogInterface dialog, int whichButton) {	
						Globals.CurrentDirectoryPathForLauncher = mDirBrowserCurDirPath;
						cn.natdon.onscripterv2.Settings.SaveGlobals(ONScripter.this);
						loadCurrentDirectory();
					}
				});
				alertDialogBuilder.setNegativeButton(getString(R.string.Cancel), null);
				alertDialogBuilder.setCancelable(true);
				mDirBrowserDialog = alertDialogBuilder.create();
				mDirBrowserDialog.show();
			} catch(Exception e){
				AlertDialog.Builder alertDialogBuilder = new AlertDialog.Builder(this);
				alertDialogBuilder.setTitle(getString(R.string.Error));
				alertDialogBuilder.setMessage(getString(R.string.Launch_CouldNotOpenDirectory) + "\n" + mDirBrowserCurDirPath);
				alertDialogBuilder.setPositiveButton(getString(R.string.OK), null);
				AlertDialog alertDialog = alertDialogBuilder.create();
				alertDialog.show();
			}
		}

		
		
		
	/*	public void onItemClick(AdapterView<?> parent, View v, int position, long id)
		{
			Globals.CurrentDirectoryPath = mDirFileArray[position-1].getAbsolutePath();
			mDirFileArray = null;
			TextView textView = (TextView) v.findViewById(R.id.ItemTitle);
			myname = textView.getText().toString().trim();
			runAppLaunchConfig();
		}*/
	
	public void runApp()
	{
		if(mStatePreview.currentState() == STATE_VIDEO_PLAY || 
    			mStatePreview.currentState() == STATE_AUDIO_PLAY) {
    		mStatePreview.gotoState(STATE_COVER_VISIBLE);
    	}
		
		if (extra == null) {
			Game item = items.getSelectedItem();
		
			if(item.gameapk != null )
			{
				Intent onsRunner=new Intent();
				onsRunner.setClass(ONScripter.this, PythonActivity.class);
				onsRunner.putExtra("renpypath", item.gameapk);
				onsRunner.putExtra("renpysave", Globals.CurrentDirectoryPath);
				ONScripter.this.startActivity(onsRunner);
			}
			else {
				Runner();
			}
		}
		else {
			Runner();
		}
		
		
		//overridePendingTransition(R.anim.gameconfig_enter, R.anim.gameconfig_exit);
	}
	
	public void Runner()
	{
		if(!checkCurrentDirectory(true)){
			return;
		}
		
		cn.natdon.onscripterv2.Settings.LoadLocals(ONScripter.this);
		
		if (extra == null)
		{
			Game item = items.getSelectedItem();
			if(item.seentxt ==null && item.xsystemgr ==null)
			{
				if(!checkAppNeedFiles()){
					return;
				}
			}
			else {
				Locals.AppCommandOptions ="";
				Globals.ButtonLeftNum = 6;
				Globals.ButtonRightNum = 6;
			}
		}

		Intent onsRunner=new Intent();
		onsRunner.setClass(ONScripter.this, ONSView.class);
		ONScripter.this.startActivity(onsRunner);
	}
	
	public void runAppLauncher()
	{
		checkCurrentDirectory(false);
		//view = new AppLauncherView(this);
		//setContentView(view);
	}
	
	//
	
	private class AppLaunchConfigView extends LinearLayout
	{
		ONScripter mActivity;

		ScrollView mConfView;
		LinearLayout mConfLayout;

		TextView mExecuteModuleText;
		TextView mVideoDepthText;
		TextView mScreenRatioText;
		TextView mScreenOrientationText;
		
		TextView[] mEnvironmentTextArray;
		Button[]   mEnvironmentButtonArray;
		
		ShortcutButton mRunButton2;
		RunButton mRunButton;
		Button mRunButton3;

		
		public AppLaunchConfigView(ONScripter activity)
		{
			super(activity);
			mActivity = activity;
			
			setOrientation(LinearLayout.VERTICAL);
			{
				mConfView = new ScrollView(mActivity);
				{
					mConfLayout = new LinearLayout(mActivity);
					//mConfLayout.setBackgroundColor(0xf5f5f5f5);
					mConfLayout.setBackgroundResource(R.drawable.config_upper);
					mConfLayout.setOrientation(LinearLayout.VERTICAL);
					{
						//Execute Module
						LinearLayout moduleLayout = new LinearLayout(mActivity);
						{
							LinearLayout txtLayout = new LinearLayout(mActivity);
							txtLayout.setOrientation(LinearLayout.VERTICAL);
							{
								TextView txt1 = new TextView(mActivity);
								txt1.setTextSize(18.0f);
								txt1.setTextColor(Color.BLACK);
								txt1.setText(getString(R.string.Conf_ExecuteModule));
								txtLayout.addView(txt1, new LinearLayout.LayoutParams( LinearLayout.LayoutParams.FILL_PARENT, LinearLayout.LayoutParams.WRAP_CONTENT));
								
								mExecuteModuleText = new TextView(mActivity);
								mExecuteModuleText.setPadding(5, 0, 0, 0);
								mExecuteModuleText.setText(Locals.AppModuleName);
								mExecuteModuleText.setTextColor(Color.BLACK);
								txtLayout.addView(mExecuteModuleText, new LinearLayout.LayoutParams( LinearLayout.LayoutParams.FILL_PARENT, LinearLayout.LayoutParams.WRAP_CONTENT));
							}
							moduleLayout.addView(txtLayout, new LinearLayout.LayoutParams( LinearLayout.LayoutParams.WRAP_CONTENT, LinearLayout.LayoutParams.WRAP_CONTENT, 1));
							
							if(Globals.APP_MODULE_NAME_ARRAY.length >= 2){
								SaoButton btn = new SaoButton(mActivity);
								//btn.setText(getString(R.string.Conf_Change));
								btn.setOnClickListener(new OnClickListener(){
									public void onClick(View v){
										AlertDialog.Builder alertDialogBuilder = new AlertDialog.Builder(mActivity);
										alertDialogBuilder.setTitle(getString(R.string.Conf_ExecuteModule));
										alertDialogBuilder.setItems(Globals.APP_MODULE_NAME_ARRAY, new DialogInterface.OnClickListener(){
											public void onClick(DialogInterface dialog, int which)
											{
												Locals.AppModuleName = Globals.APP_MODULE_NAME_ARRAY[which];
												mExecuteModuleText.setText(Locals.AppModuleName);
												cn.natdon.onscripterv2.Settings.SaveLocals(mActivity);
											}
										});
										alertDialogBuilder.setNegativeButton(getString(R.string.Cancel), null);
										alertDialogBuilder.setCancelable(true);
										AlertDialog alertDialog = alertDialogBuilder.create();
										alertDialog.show();
									}
								});
								moduleLayout.addView(btn, new LinearLayout.LayoutParams(LinearLayout.LayoutParams.WRAP_CONTENT, LinearLayout.LayoutParams.WRAP_CONTENT));
							}
						}
						mConfLayout.addView(moduleLayout, new LinearLayout.LayoutParams( LinearLayout.LayoutParams.FILL_PARENT, LinearLayout.LayoutParams.WRAP_CONTENT));

						//Video Depth
						LinearLayout videoDepthLayout = new LinearLayout(mActivity);
						{
							LinearLayout txtLayout = new LinearLayout(mActivity);
							txtLayout.setOrientation(LinearLayout.VERTICAL);
							{
								TextView txt1 = new TextView(mActivity);
								txt1.setTextSize(18.0f);
								txt1.setText(getString(R.string.Conf_VideoDepth));
								txt1.setTextColor(Color.BLACK);
								txtLayout.addView(txt1, new LinearLayout.LayoutParams( LinearLayout.LayoutParams.FILL_PARENT, LinearLayout.LayoutParams.WRAP_CONTENT));
								
								mVideoDepthText = new TextView(mActivity);
								mVideoDepthText.setPadding(5, 0, 0, 0);
								mVideoDepthText.setText("" + Locals.VideoDepthBpp + "bpp");
								mVideoDepthText.setTextColor(Color.BLACK);
								txtLayout.addView(mVideoDepthText, new LinearLayout.LayoutParams( LinearLayout.LayoutParams.FILL_PARENT, LinearLayout.LayoutParams.WRAP_CONTENT));
							}
							videoDepthLayout.addView(txtLayout, new LinearLayout.LayoutParams( LinearLayout.LayoutParams.WRAP_CONTENT, LinearLayout.LayoutParams.WRAP_CONTENT, 1));
							
							if(Globals.VIDEO_DEPTH_BPP_ITEMS.length >= 2){
								SaoButton btn = new SaoButton(mActivity);
								//btn.setText(getString(R.string.Conf_Change));
								btn.setOnClickListener(new OnClickListener(){
									public void onClick(View v){
										String[] bppItems = new String[Globals.VIDEO_DEPTH_BPP_ITEMS.length];
										for(int i = 0; i < bppItems.length; i ++){
											bppItems[i] = "" + Globals.VIDEO_DEPTH_BPP_ITEMS[i] + "bpp";
										}
										
										AlertDialog.Builder alertDialogBuilder = new AlertDialog.Builder(mActivity);
										alertDialogBuilder.setTitle(getString(R.string.Conf_VideoDepth));
										alertDialogBuilder.setItems(bppItems, new DialogInterface.OnClickListener(){
											public void onClick(DialogInterface dialog, int which)
											{
												Locals.VideoDepthBpp = Globals.VIDEO_DEPTH_BPP_ITEMS[which];
												mVideoDepthText.setText("" + Locals.VideoDepthBpp + "bpp");
												cn.natdon.onscripterv2.Settings.SaveLocals(mActivity);
											}
										});
										alertDialogBuilder.setNegativeButton(getString(R.string.Cancel), null);
										alertDialogBuilder.setCancelable(true);
										AlertDialog alertDialog = alertDialogBuilder.create();
										alertDialog.show();
									}
								});
								videoDepthLayout.addView(btn, new LinearLayout.LayoutParams(LinearLayout.LayoutParams.WRAP_CONTENT, LinearLayout.LayoutParams.WRAP_CONTENT));
							}
						}
						mConfLayout.addView(videoDepthLayout, new LinearLayout.LayoutParams( LinearLayout.LayoutParams.FILL_PARENT, LinearLayout.LayoutParams.WRAP_CONTENT));
						
						//Screen Ratio
						LinearLayout screenRatioLayout = new LinearLayout(mActivity);
						{
							LinearLayout txtLayout = new LinearLayout(mActivity);
							txtLayout.setOrientation(LinearLayout.VERTICAL);
							{
								TextView txt1 = new TextView(mActivity);
								txt1.setTextSize(18.0f);
								txt1.setText(getString(R.string.Conf_ScreenRatio));
								txt1.setTextColor(Color.BLACK);
								txtLayout.addView(txt1, new LinearLayout.LayoutParams( LinearLayout.LayoutParams.FILL_PARENT, LinearLayout.LayoutParams.WRAP_CONTENT));
								
								mScreenRatioText = new TextView(mActivity);
								mScreenRatioText.setPadding(5, 0, 0, 0);
								if(Locals.VideoXRatio > 0 && Locals.VideoYRatio > 0){
									mScreenRatioText.setText("" + Locals.VideoXRatio + ":" + Locals.VideoYRatio);
								} else {
									mScreenRatioText.setText(getString(R.string.Full));
								}
								mScreenRatioText.setTextColor(Color.BLACK);
								txtLayout.addView(mScreenRatioText, new LinearLayout.LayoutParams( LinearLayout.LayoutParams.FILL_PARENT, LinearLayout.LayoutParams.WRAP_CONTENT));
							}
							screenRatioLayout.addView(txtLayout, new LinearLayout.LayoutParams( LinearLayout.LayoutParams.WRAP_CONTENT, LinearLayout.LayoutParams.WRAP_CONTENT, 1));
							
							SwapButton btn1 = new SwapButton(mActivity);
							//btn1.setText(getString(R.string.Conf_Swap));
							btn1.setOnClickListener(new OnClickListener(){
								public void onClick(View v){
									int tmp = Locals.VideoXRatio;
									Locals.VideoXRatio = Locals.VideoYRatio;
									Locals.VideoYRatio = tmp;
									if(Locals.VideoXRatio > 0 && Locals.VideoYRatio > 0){
										mScreenRatioText.setText("" + Locals.VideoXRatio + ":" + Locals.VideoYRatio);
									} else {
										mScreenRatioText.setText(getString(R.string.Full));
									}
									cn.natdon.onscripterv2.Settings.SaveLocals(mActivity);
								}
							});
							screenRatioLayout.addView(btn1, new LinearLayout.LayoutParams(LinearLayout.LayoutParams.WRAP_CONTENT, LinearLayout.LayoutParams.WRAP_CONTENT));
							
							if(Globals.VIDEO_RATIO_ITEMS.length >= 2){
								SaoButton btn = new SaoButton(mActivity);
								//btn.setText(getString(R.string.Conf_Change));
								btn.setOnClickListener(new OnClickListener(){
									public void onClick(View v){
										String[] ratioItems = new String[Globals.VIDEO_RATIO_ITEMS.length];
										for(int i = 0; i < ratioItems.length; i ++){
											int w = Globals.VIDEO_RATIO_ITEMS[i][0];
											int h = Globals.VIDEO_RATIO_ITEMS[i][1];
											if(w > 0 && h > 0){
												ratioItems[i] = "" + w + ":" + h;
											} else {
												ratioItems[i] = getString(R.string.Full);
											}
										}
										
										AlertDialog.Builder alertDialogBuilder = new AlertDialog.Builder(mActivity);
										alertDialogBuilder.setTitle(getString(R.string.Conf_ScreenRatio));
										alertDialogBuilder.setItems(ratioItems, new DialogInterface.OnClickListener(){
											public void onClick(DialogInterface dialog, int which)
											{
												Locals.VideoXRatio = Globals.VIDEO_RATIO_ITEMS[which][0];
												Locals.VideoYRatio = Globals.VIDEO_RATIO_ITEMS[which][1];
												if(Locals.VideoXRatio > 0 && Locals.VideoYRatio > 0){
													mScreenRatioText.setText("" + Locals.VideoXRatio + ":" + Locals.VideoYRatio);
												} else {
													mScreenRatioText.setText(getString(R.string.Full));
												}
												cn.natdon.onscripterv2.Settings.SaveLocals(mActivity);
											}
										});
										alertDialogBuilder.setNegativeButton(getString(R.string.Cancel), null);
										alertDialogBuilder.setCancelable(true);
										AlertDialog alertDialog = alertDialogBuilder.create();
										alertDialog.show();
									}
								});
								screenRatioLayout.addView(btn, new LinearLayout.LayoutParams(LinearLayout.LayoutParams.WRAP_CONTENT, LinearLayout.LayoutParams.WRAP_CONTENT));
							}
						}
						mConfLayout.addView(screenRatioLayout, new LinearLayout.LayoutParams( LinearLayout.LayoutParams.FILL_PARENT, LinearLayout.LayoutParams.WRAP_CONTENT));

						//Screen Orientation
						LinearLayout screenOrientationLayout = new LinearLayout(mActivity);
						{
							LinearLayout txtLayout = new LinearLayout(mActivity);
							txtLayout.setOrientation(LinearLayout.VERTICAL);
							{
								TextView txt1 = new TextView(mActivity);
								txt1.setTextSize(18.0f);
								txt1.setText(getString(R.string.Conf_ScreenOrientation));
								txt1.setTextColor(Color.BLACK);
								txtLayout.addView(txt1, new LinearLayout.LayoutParams( LinearLayout.LayoutParams.FILL_PARENT, LinearLayout.LayoutParams.WRAP_CONTENT));
								
								mScreenOrientationText = new TextView(mActivity);
								mScreenOrientationText.setPadding(5, 0, 0, 0);
								mScreenOrientationText.setTextColor(Color.BLACK);
								switch(Locals.ScreenOrientation){
									case ActivityInfo.SCREEN_ORIENTATION_PORTRAIT:
										mScreenOrientationText.setText(getString(R.string.Portrait));
										break;
									case ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE:
										mScreenOrientationText.setText(getString(R.string.Landscape));
										break;
									case ActivityInfo.SCREEN_ORIENTATION_REVERSE_PORTRAIT:
										mScreenOrientationText.setText(getString(R.string.ReversePortrait));
										break;
									case ActivityInfo.SCREEN_ORIENTATION_REVERSE_LANDSCAPE:
										mScreenOrientationText.setText(getString(R.string.ReverseLandscape));
										break;
									default:
										mScreenOrientationText.setText(getString(R.string.Unknown));
										break;
								}
								txtLayout.addView(mScreenOrientationText, new LinearLayout.LayoutParams( LinearLayout.LayoutParams.FILL_PARENT, LinearLayout.LayoutParams.WRAP_CONTENT));
							}
							screenOrientationLayout.addView(txtLayout, new LinearLayout.LayoutParams( LinearLayout.LayoutParams.WRAP_CONTENT, LinearLayout.LayoutParams.WRAP_CONTENT, 1));
							
							SaoButton btn = new SaoButton(mActivity);
							//btn.setText(getString(R.string.Conf_Change));
							btn.setOnClickListener(new OnClickListener(){
								public void onClick(View v){
									String[] screenOrientationItems;
									if(android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.GINGERBREAD){
										screenOrientationItems = new String[]{getString(R.string.Portrait), getString(R.string.Landscape), getString(R.string.ReversePortrait), getString(R.string.ReverseLandscape)};
									} else {
										screenOrientationItems = new String[]{getString(R.string.Portrait), getString(R.string.Landscape)};
									}
									
									AlertDialog.Builder alertDialogBuilder = new AlertDialog.Builder(mActivity);
									alertDialogBuilder.setTitle(getString(R.string.Conf_ScreenOrientation));
									alertDialogBuilder.setItems(screenOrientationItems, new DialogInterface.OnClickListener(){
										public void onClick(DialogInterface dialog, int which)
										{
											switch(which){
												case 0:
													Locals.ScreenOrientation = ActivityInfo.SCREEN_ORIENTATION_PORTRAIT;
													mScreenOrientationText.setText(getString(R.string.Portrait));
													break;
												case 1:
													Locals.ScreenOrientation = ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE;
													mScreenOrientationText.setText(getString(R.string.Landscape));
													break;
												case 2:
													Locals.ScreenOrientation = ActivityInfo.SCREEN_ORIENTATION_REVERSE_PORTRAIT;
													mScreenOrientationText.setText(getString(R.string.ReversePortrait));
													break;
												case 3:
													Locals.ScreenOrientation = ActivityInfo.SCREEN_ORIENTATION_REVERSE_LANDSCAPE;
													mScreenOrientationText.setText(getString(R.string.ReverseLandscape));
													break;
											}
											cn.natdon.onscripterv2.Settings.SaveLocals(mActivity);
										}
									});
									alertDialogBuilder.setNegativeButton(getString(R.string.Cancel), null);
									alertDialogBuilder.setCancelable(true);
									AlertDialog alertDialog = alertDialogBuilder.create();
									alertDialog.show();
								}
							});
							screenOrientationLayout.addView(btn, new LinearLayout.LayoutParams(LinearLayout.LayoutParams.WRAP_CONTENT, LinearLayout.LayoutParams.WRAP_CONTENT));
						}
						mConfLayout.addView(screenOrientationLayout, new LinearLayout.LayoutParams( LinearLayout.LayoutParams.FILL_PARENT, LinearLayout.LayoutParams.WRAP_CONTENT));
												

						//Smooth video
						LinearLayout videoSmoothLayout = new LinearLayout(mActivity);
						{
							CheckBox chk = new CheckBox(mActivity);
							chk.setChecked(Locals.VideoSmooth);
							chk.setOnClickListener(new OnClickListener(){
								public void onClick(View v){
									CheckBox c = (CheckBox)v;
									Locals.VideoSmooth = c.isChecked();
									cn.natdon.onscripterv2.Settings.SaveLocals(mActivity);
								}
							});
							videoSmoothLayout.addView(chk, new LinearLayout.LayoutParams(LinearLayout.LayoutParams.WRAP_CONTENT, LinearLayout.LayoutParams.WRAP_CONTENT));
							
							LinearLayout txtLayout = new LinearLayout(mActivity);
							txtLayout.setOrientation(LinearLayout.VERTICAL);
							{
								TextView txt1 = new TextView(mActivity);
								txt1.setTextSize(18.0f);
								txt1.setText(getString(R.string.Conf_SmoothVideo));
								txt1.setTextColor(Color.BLACK);
								txtLayout.addView(txt1, new LinearLayout.LayoutParams( LinearLayout.LayoutParams.FILL_PARENT, LinearLayout.LayoutParams.WRAP_CONTENT));
								
								TextView txt2 = new TextView(mActivity);
								txt2.setPadding(5, 0, 0, 0);
								txt2.setText(getString(R.string.Conf_Linear));
								txt2.setTextColor(Color.BLACK);
								txtLayout.addView(txt2, new LinearLayout.LayoutParams( LinearLayout.LayoutParams.FILL_PARENT, LinearLayout.LayoutParams.WRAP_CONTENT));
							}
							videoSmoothLayout.addView(txtLayout, new LinearLayout.LayoutParams( LinearLayout.LayoutParams.WRAP_CONTENT, LinearLayout.LayoutParams.WRAP_CONTENT, 1));
						}
						mConfLayout.addView(videoSmoothLayout, new LinearLayout.LayoutParams( LinearLayout.LayoutParams.FILL_PARENT, LinearLayout.LayoutParams.WRAP_CONTENT));

						checkHW = new CheckBox(mActivity);
						checkHW.setText("硬件加�?);
						checkHW.setBackgroundColor(Color.argb(0, 0, 0, 0));
						checkHW.setTextColor(Color.BLACK);
						mConfLayout.addView(checkHW, new LinearLayout.LayoutParams( LinearLayout.LayoutParams.FILL_PARENT, LinearLayout.LayoutParams.WRAP_CONTENT));


						//Full Video
						LinearLayout FullvideoLayout = new LinearLayout(mActivity);
						{
							FullScreen = new CheckBox(mActivity);
							FullScreen.setBackgroundColor(Color.argb(0, 0, 0, 0));
							FullScreen.setTextColor(Color.BLACK);
							FullvideoLayout.addView(FullScreen, new LinearLayout.LayoutParams(LinearLayout.LayoutParams.WRAP_CONTENT, LinearLayout.LayoutParams.WRAP_CONTENT));
							
							LinearLayout txtLayout = new LinearLayout(mActivity);
							txtLayout.setOrientation(LinearLayout.VERTICAL);
							{
								TextView txt1 = new TextView(mActivity);
								txt1.setTextSize(18.0f);
								txt1.setText("比例全屏");
								txt1.setTextColor(Color.BLACK);
								txtLayout.addView(txt1, new LinearLayout.LayoutParams( LinearLayout.LayoutParams.FILL_PARENT, LinearLayout.LayoutParams.WRAP_CONTENT));
								
								TextView txt2 = new TextView(mActivity);
								txt2.setPadding(5, 0, 0, 0);
								txt2.setText("原比例全�?需拖动,非拉�?);
								txt2.setTextColor(Color.BLACK);
								txtLayout.addView(txt2, new LinearLayout.LayoutParams( LinearLayout.LayoutParams.FILL_PARENT, LinearLayout.LayoutParams.WRAP_CONTENT));
							}
							FullvideoLayout.addView(txtLayout, new LinearLayout.LayoutParams( LinearLayout.LayoutParams.WRAP_CONTENT, LinearLayout.LayoutParams.WRAP_CONTENT, 1));
						}
						mConfLayout.addView(FullvideoLayout, new LinearLayout.LayoutParams( LinearLayout.LayoutParams.FILL_PARENT, LinearLayout.LayoutParams.WRAP_CONTENT));

						//OtherSetting
						LinearLayout OtherSettingLayout = new LinearLayout(mActivity);
						OtherSettingLayout.setOrientation(LinearLayout.VERTICAL);
						{
							

							
			
							checkWD = new CheckBox(mActivity);
							checkWD.setText("窗口�?);
							checkWD.setBackgroundColor(Color.argb(0, 0, 0, 0));
							checkWD.setTextColor(Color.BLACK);
							checkWD.setOnCheckedChangeListener(new OnCheckedChangeListener() {
			
								public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
									if(isChecked)
									WDDialog();
				
								}
							});
							OtherSettingLayout.addView(checkWD);

							checkSP = new CheckBox(mActivity);
							checkSP.setText("屏蔽视频");
							checkSP.setBackgroundColor(Color.argb(0, 0, 0, 0));
							checkSP.setTextColor(Color.BLACK);
							OtherSettingLayout.addView(checkSP);

							OtherPL = new CheckBox(mActivity);
							OtherPL.setText("外部播放�?);
							OtherPL.setBackgroundColor(Color.argb(0, 0, 0, 0));
							OtherPL.setTextColor(Color.BLACK);
							OtherSettingLayout.addView(OtherPL);

							FontSize = new CheckBox(mActivity);
							FontSize.setText("字体大小");
							FontSize.setBackgroundColor(Color.argb(0, 0, 0, 0));
							FontSize.setTextColor(Color.BLACK);
							FontSize.setOnCheckedChangeListener(new OnCheckedChangeListener() {
			
								public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
									if(isChecked && ONSVariable.DialogOpen)
									FontDialog();
				
								}
							});
							OtherSettingLayout.addView(FontSize);

							FontColor = new CheckBox(mActivity);
							FontColor.setText("字体颜色");
							FontColor.setBackgroundColor(Color.argb(0, 0, 0, 0));
							FontColor.setTextColor(ONSVariable.cbColor);
							FontColor.setOnCheckedChangeListener(new OnCheckedChangeListener() {
			
								public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
									if(isChecked && ONSVariable.DialogOpen)
									ColorDialog();
				
								}
							});
							OtherSettingLayout.addView(FontColor);

							keepON = new CheckBox(mActivity);
							keepON.setText("保持长亮 ");
							keepON.setBackgroundColor(Color.argb(0, 0, 0, 0));
							keepON.setTextColor(Color.BLACK);
							OtherSettingLayout.addView(keepON);

							checkLog = new CheckBox(mActivity);
							checkLog.setText("调试输出 ");
							checkLog.setBackgroundColor(Color.argb(0, 0, 0, 0));
							checkLog.setTextColor(Color.BLACK);
							OtherSettingLayout.addView(checkLog);
							
							SharedPreferences checkset = getSharedPreferences(myname, MODE_PRIVATE);
							boolean ck = checkset.contains("checkSP");
							if (!ck) {
								Editor e = getSharedPreferences(myname, MODE_PRIVATE).edit();

								ONSVariable.set_FullScreen = false;
								e.putBoolean("FullScreen", ONSVariable.set_FullScreen);

								ONSVariable.set_checkSP = false;
								e.putBoolean("checkSP", ONSVariable.set_checkSP);

								ONSVariable.set_OtherPL = false;
								e.putBoolean("OtherPL", ONSVariable.set_OtherPL);

								ONSVariable.set_FontSize = false;
								e.putBoolean("FontSize", ONSVariable.set_FontSize);

								ONSVariable.set_FontColor = false;
								e.putBoolean("FontColor", ONSVariable.set_FontColor);


								ONSVariable.set_keepON = false;
								e.putBoolean("keepON", ONSVariable.set_keepON);
								
								ONSVariable.set_putLog = false;
								e.putBoolean("checkLog", ONSVariable.set_putLog);
								
								ONSVariable.set_checkHW = true;
								e.putBoolean("checkHW", ONSVariable.set_checkHW);

								e.commit();
							}
							ReadSetting(myname);
							
						}
						


						mConfLayout.addView(OtherSettingLayout, new LinearLayout.LayoutParams( LinearLayout.LayoutParams.FILL_PARENT, LinearLayout.LayoutParams.WRAP_CONTENT));
						
						//Command Options
						for(int i = 0; i < Globals.APP_COMMAND_OPTIONS_ITEMS.length; i ++){
							LinearLayout cmdOptLayout = new LinearLayout(mActivity);
							{
								final int index = i;
								
								CheckBox chk = new CheckBox(mActivity);
								chk.setChecked(Locals.AppCommandOptions.indexOf(Globals.APP_COMMAND_OPTIONS_ITEMS[index][1]) >= 0);
								chk.setOnClickListener(new OnClickListener(){
									public void onClick(View v){
										CheckBox c = (CheckBox)v;
										if(!c.isChecked()){
											int start = Locals.AppCommandOptions.indexOf(Globals.APP_COMMAND_OPTIONS_ITEMS[index][1]);
											if(start == 0){
												Locals.AppCommandOptions = Locals.AppCommandOptions.replace(Globals.APP_COMMAND_OPTIONS_ITEMS[index][1], "");
											} else if(start >= 0){
												Locals.AppCommandOptions = Locals.AppCommandOptions.replace(" " + Globals.APP_COMMAND_OPTIONS_ITEMS[index][1], "");
											}
										} else {
											if(Locals.AppCommandOptions.equals("")){
												Locals.AppCommandOptions = Globals.APP_COMMAND_OPTIONS_ITEMS[index][1];
											} else {
												Locals.AppCommandOptions += " " + Globals.APP_COMMAND_OPTIONS_ITEMS[index][1];
											}
										}
										cn.natdon.onscripterv2.Settings.SaveLocals(mActivity);
									}
								});
								cmdOptLayout.addView(chk, new LinearLayout.LayoutParams(LinearLayout.LayoutParams.WRAP_CONTENT, LinearLayout.LayoutParams.WRAP_CONTENT));
								
								LinearLayout txtLayout = new LinearLayout(mActivity);
								txtLayout.setOrientation(LinearLayout.VERTICAL);
								{
									TextView txt1 = new TextView(mActivity);
									txt1.setTextSize(18.0f);
									txt1.setText(Globals.APP_COMMAND_OPTIONS_ITEMS[index][0]);
									txt1.setTextColor(Color.BLACK);
									txtLayout.addView(txt1, new LinearLayout.LayoutParams( LinearLayout.LayoutParams.FILL_PARENT, LinearLayout.LayoutParams.WRAP_CONTENT));
									
									TextView txt2 = new TextView(mActivity);
									txt2.setPadding(5, 0, 0, 0);
									txt2.setText(Globals.APP_COMMAND_OPTIONS_ITEMS[index][1]);
									txt2.setTextColor(Color.BLACK);
									txtLayout.addView(txt2, new LinearLayout.LayoutParams( LinearLayout.LayoutParams.FILL_PARENT, LinearLayout.LayoutParams.WRAP_CONTENT));
								}
								cmdOptLayout.addView(txtLayout, new LinearLayout.LayoutParams( LinearLayout.LayoutParams.WRAP_CONTENT, LinearLayout.LayoutParams.WRAP_CONTENT, 1));
							}
							mConfLayout.addView(cmdOptLayout, new LinearLayout.LayoutParams( LinearLayout.LayoutParams.FILL_PARENT, LinearLayout.LayoutParams.WRAP_CONTENT));
						}
						
						
					}
					mConfView.addView(mConfLayout);
				}
				addView(mConfView, new LinearLayout.LayoutParams( LinearLayout.LayoutParams.WRAP_CONTENT, 0, 1) );
				
				//View divider = new View(mActivity);
				//divider.setBackgroundColor(Color.GRAY);
				//addView(divider, new LinearLayout.LayoutParams(LinearLayout.LayoutParams.WRAP_CONTENT, 2) );
				
				LinearLayout runLayout = new LinearLayout(mActivity);
				runLayout.setOrientation(LinearLayout.HORIZONTAL);
				runLayout.setBackgroundColor(getResources().getColor(R.color.sao_transparent_white));
				runLayout.setGravity(Gravity.CENTER_HORIZONTAL);
				//runLayout.setBackgroundDrawable(getResources().getDrawable(R.drawable.btn_light_nm));
				
				mRunButton = new RunButton(mActivity);
				//mRunButton.setText(getString(R.string.Conf_Run));
				mRunButton.setTextSize(24.0f);
				mRunButton.setGravity(Gravity.CENTER_HORIZONTAL);
				mRunButton.setOnClickListener(new OnClickListener(){
					public void onClick(View v){
						WriteSetting(myname);
						ReadSetting(myname);
						
							runApp();

						
					}
				});
				runLayout.addView(mRunButton, new LinearLayout.LayoutParams( LinearLayout.LayoutParams.WRAP_CONTENT, LinearLayout.LayoutParams.WRAP_CONTENT) );
				
				mRunButton3 = new Button(mActivity);
				mRunButton3.setText(" ");
				mRunButton3.setTextSize(24.0f);
				mRunButton3.setBackgroundColor(Color.argb(0, 0, 0, 0));
				mRunButton3.setGravity(Gravity.CENTER_HORIZONTAL);
				runLayout.addView(mRunButton3, new LinearLayout.LayoutParams( LinearLayout.LayoutParams.WRAP_CONTENT, LinearLayout.LayoutParams.WRAP_CONTENT) );

				mRunButton2 = new ShortcutButton(mActivity);
				//mRunButton2.setText("创建快捷方式");
				mRunButton2.setGravity(Gravity.CENTER_HORIZONTAL);
				mRunButton2.setTextSize(24.0f);
				mRunButton2.setOnClickListener(new OnClickListener(){
					public void onClick(View v){
						Shortcut.addShortcut(myname, Globals.CurrentDirectoryPath ,ONScripter.this);
					}
				});
				runLayout.addView(mRunButton2, new LinearLayout.LayoutParams( LinearLayout.LayoutParams.WRAP_CONTENT, LinearLayout.LayoutParams.WRAP_CONTENT) );

				
				
				addView(runLayout, new LinearLayout.LayoutParams( LinearLayout.LayoutParams.FILL_PARENT, LinearLayout.LayoutParams.WRAP_CONTENT) );
			}
		}
	}

	public void WriteSetting(String Gamename) {
		Editor e = getSharedPreferences(Gamename, MODE_PRIVATE).edit();
		
		if (checkSP.isChecked()) {
			ONSVariable.set_checkSP = true;
			e.putBoolean("checkSP", ONSVariable.set_checkSP);
		} else {
			ONSVariable.set_checkSP = false;
			e.putBoolean("checkSP", ONSVariable.set_checkSP);
		}
		if (FullScreen.isChecked()) {
			ONSVariable.set_FullScreen = true;
			e.putBoolean("FullScreen", ONSVariable.set_FullScreen);
		} else {
			ONSVariable.set_FullScreen = false;
			e.putBoolean("FullScreen", ONSVariable.set_FullScreen);
		}
		if (OtherPL.isChecked()) {
			ONSVariable.set_OtherPL = true;
			e.putBoolean("OtherPL", ONSVariable.set_OtherPL);
		} else {
			ONSVariable.set_OtherPL = false;
			e.putBoolean("OtherPL", ONSVariable.set_OtherPL);
		}
		
		if (FontSize.isChecked()) {
			ONSVariable.set_FontSize = true;
			e.putBoolean("FontSize", ONSVariable.set_FontSize);
		} else {
			ONSVariable.set_FontSize = false;
			e.putBoolean("FontSize", ONSVariable.set_FontSize);
		}

		if (FontColor.isChecked()) {
			ONSVariable.set_FontColor = true;
			e.putBoolean("FontColor", ONSVariable.set_FontColor);
		} else {
			ONSVariable.set_FontColor = false;
			e.putBoolean("FontColor", ONSVariable.set_FontColor);
		}
		
		if (keepON.isChecked()) {
			ONSVariable.set_keepON = true;
			e.putBoolean("keepON", ONSVariable.set_keepON);
		} else {
			ONSVariable.set_keepON = false;
			e.putBoolean("keepON", ONSVariable.set_keepON);
		}
		
		if (checkLog.isChecked()) {
			ONSVariable.set_putLog = true;
			e.putBoolean("checkLog", ONSVariable.set_putLog);
		} else {
			ONSVariable.set_putLog = false;
			e.putBoolean("checkLog", ONSVariable.set_putLog);
		}
		
		if (checkHW.isChecked()) {
			ONSVariable.set_checkHW = true;
			e.putBoolean("checkHW", ONSVariable.set_checkHW);
		} else {
			ONSVariable.set_checkHW = false;
			e.putBoolean("checkHW", ONSVariable.set_checkHW);
		}
		e.commit();
	}

	public void ReadSetting(String setting) {
		SharedPreferences spset = getSharedPreferences(setting, MODE_PRIVATE);// 读取设置
		
		ONSVariable.set_checkSP = spset.getBoolean("checkSP", ONSVariable.set_checkSP);
		if (ONSVariable.set_checkSP) {
			checkSP.setChecked(true);
		}
		ONSVariable.set_FullScreen = spset.getBoolean("FullScreen", ONSVariable.set_FullScreen);
		if (ONSVariable.set_FullScreen)
			FullScreen.setChecked(true);
		ONSVariable.set_OtherPL = spset.getBoolean("OtherPL", ONSVariable.set_OtherPL);
		if (ONSVariable.set_OtherPL) {
			OtherPL.setChecked(true);
		}
		ONSVariable.set_FullScreen = spset.getBoolean("FullScreen", ONSVariable.set_FullScreen);
		if (ONSVariable.set_FullScreen) {
			FullScreen.setChecked(true);
		}
		ONSVariable.set_FontSize = spset.getBoolean("FontSize", ONSVariable.set_FontSize);
		if (ONSVariable.set_FontSize) {
			FontSize.setChecked(true);
			ONSVariable.myfontsize = spset.getInt("getfontsize", 0);
			ONSVariable.myfontpx = spset.getInt("getfontpx", 0);
		}
		ONSVariable.set_FontColor = spset.getBoolean("FontColor", ONSVariable.set_FontColor);
		if (ONSVariable.set_FontColor) {
			FontColor.setChecked(true);
			ONSVariable.myfont_color1 = spset.getInt("fontr", 255);
			ONSVariable.myfont_color2 = spset.getInt("fontg", 255);
			ONSVariable.myfont_color3 = spset.getInt("fontb", 255);
			
		}
		ONSVariable.set_keepON = spset.getBoolean("keepON", ONSVariable.set_keepON);
		if (ONSVariable.set_keepON) {
			keepON.setChecked(true);
		}
		
		ONSVariable.set_putLog = spset.getBoolean("checkLog", ONSVariable.set_putLog);
		if (ONSVariable.set_putLog) {
			checkLog.setChecked(true);
		}
		
		ONSVariable.set_checkHW = spset.getBoolean("checkHW", ONSVariable.set_checkHW);
		if (ONSVariable.set_checkHW) {
			checkHW.setChecked(true);
		}

		ONSVariable.cbColor = spset.getInt("cbcolor", Color.BLACK);
		FontColor.setTextColor(ONSVariable.cbColor);
		
		//ONSVariable.mPlayer = spset.getBoolean("playerchoose", true);

		Locals.gDisableVideo = checkSP.isChecked();
		Locals.gOtherPL = OtherPL.isChecked();
		Locals.gWindowScreen = checkWD.isChecked();
		Locals.gFullScreen = FullScreen.isChecked();
		if (keepON.isChecked()) {
			getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
		}
		Locals.Logout = checkLog.isChecked();
		Locals.gFontSize = FontSize.isChecked();
		Locals.gFontColor = FontColor.isChecked();
	}

	public void FSetting(String setting) {
		SharedPreferences spset = getSharedPreferences(setting, MODE_PRIVATE);// 读取设置
	
		ONSVariable.set_FullScreen = spset.getBoolean("FullScreen", ONSVariable.set_FullScreen);
		if (ONSVariable.set_FullScreen)
			Locals.gFullScreen = true;
		ONSVariable.set_ScaleFullScreen = spset.getBoolean("ScaleFullScreen", ONSVariable.set_ScaleFullScreen);

		ONSVariable.set_checkSP = spset.getBoolean("checkSP", ONSVariable.set_checkSP);
		if (ONSVariable.set_checkSP)
			Locals.gDisableVideo = true;
		ONSVariable.set_OtherPL = spset.getBoolean("OtherPL", ONSVariable.set_OtherPL);
		if (ONSVariable.set_OtherPL)
			Locals.gOtherPL = true;
		ONSVariable.set_FontSize = spset.getBoolean("FontSize", ONSVariable.set_FontSize);
		if (ONSVariable.set_FontSize){
			Locals.gFontSize = true;
			ONSVariable.myfontsize = spset.getInt("getfontsize", 0);
			ONSVariable.myfontpx = spset.getInt("getfontpx", 0);
		}
		ONSVariable.set_FontColor= spset.getBoolean("FontColor", ONSVariable.set_FontColor);
		if (ONSVariable.set_FontColor){
			Locals.gFontColor = true;
			ONSVariable.myfont_color1 = spset.getInt("fontr", 255);
			ONSVariable.myfont_color2 = spset.getInt("fontg", 255);
			ONSVariable.myfont_color3 = spset.getInt("fontb", 255);
		}
		
		ONSVariable.mPlayer = spset.getBoolean("playerchoose", true);
		
		ONSVariable.set_keepON = spset.getBoolean("keepON", ONSVariable.set_keepON);
		if (ONSVariable.set_keepON) {
			Locals.gKeepON = true;
		}
		ONSVariable.set_putLog = spset.getBoolean("checkLog", ONSVariable.set_putLog);
		if (ONSVariable.set_putLog)
			Locals.Logout = true;
		
		ONSVariable.set_checkHW = spset.getBoolean("checkHW", ONSVariable.set_checkHW);
	}
	
	public void runAppLaunchConfig()
	{
		if(!checkCurrentDirectory(true)){
			return;
		}
		
		cn.natdon.onscripterv2.Settings.LoadLocals(this);
		
		if(!Locals.AppLaunchConfigUse){
			//runApp();
			return;
		}
		
		AppLaunchConfigView view = new AppLaunchConfigView(this);
		//setContentView(view);
		gameConfig(view);
		ONSVariable.DialogOpen = true;
	}

	
	//
	
	private boolean checkAppNeedFiles()
	{
		String missingFileNames = "";
		int missingCount = 0;
		
		for(String fileName : Globals.APP_NEED_FILENAME_ARRAY){
			String[] itemNameArray = fileName.split("\\|");
			boolean flag = false;
			for(String itemName : itemNameArray){
				File file = new File(Globals.CurrentDirectoryPath + "/" + itemName.trim());
				if(file.exists() && file.canRead()){
					flag = true;
					break;
				}
			}
			if(!flag){
				missingCount ++;
				missingFileNames += "[" + missingCount + "]" + fileName.replace("|"," or ") + "\n";
				File ttfFile = new File(Globals.CurrentDirectoryPath + "/" + "default.ttf");
				if(!ttfFile.exists() && !ttfFile.canRead())
				{
					missingFileNames = "";
					Locals.AppCommandOptions += " " + "--font /system/fonts/DroidSansFallback.ttf";
					flag = true;
					break;
				}
			}
		}
		
		if(!missingFileNames.equals("")){
			AlertDialog.Builder alertDialogBuilder = new AlertDialog.Builder(this);
			alertDialogBuilder.setTitle(getString(R.string.Error));
			alertDialogBuilder.setMessage(getString(R.string.Need_Following) + " " + missingCount + " " + getString(R.string.Need_FilesAreMissing) + "\n" + missingFileNames);
			alertDialogBuilder.setPositiveButton(getString(R.string.Quit), new DialogInterface.OnClickListener(){
				public void onClick(DialogInterface dialog, int whichButton) {
					finish();
				}
			});
			alertDialogBuilder.setCancelable(false);
			AlertDialog alertDialog = alertDialogBuilder.create();
			alertDialog.show();
			
			return false;
		}
		
		return true;
	}
	
	


	public void WDDialog() {

		final LinearLayout wd_layout = new LinearLayout(this);
		wd_layout.setOrientation(LinearLayout.VERTICAL);		
		
		if (wd_popupWindow != null && wd_popupWindow.isShowing()) {
					wd_popupWindow.dismiss();
				} else {
					
					wd_popupWindow = new PopupWindow(wd_layout, LayoutParams.WRAP_CONTENT, LayoutParams.WRAP_CONTENT,true);
					wd_popupWindow.setAnimationStyle(R.style.Animation_ConfigPanelAnimation);
					wd_popupWindow.setTouchable(true);
					wd_popupWindow.setOutsideTouchable(true);
					wd_popupWindow.setBackgroundDrawable(getResources().getDrawable(R.drawable.config_upper));
					wd_popupWindow.showAtLocation(cover, Gravity.CENTER, 0, 0); 
					
				}

		SharedPreferences sp = getSharedPreferences("pref", MODE_PRIVATE);
		ONSVariable.wdw = sp.getInt("wdw", 640);
		ONSVariable.wdh = sp.getInt("wdh", 480);
		
		final TextView wdText=new TextView(this);
		wdText.setTextSize(21);
		wdText.setText(" 输入窗口大小:Width,Height ");
		wdText.setGravity(Gravity.CENTER_VERTICAL);
		wdText.setBackgroundColor(Color.argb(0, 0, 0, 0));
		wdText.setTextColor(Color.BLACK);
		wd_layout.addView(wdText);

		final EditText Width =new EditText(this);
		Width.setText(String.valueOf(ONSVariable.wdw));
		Width.setHint("只能输入数字");
		Width.setInputType(InputType.TYPE_CLASS_NUMBER);
		Width.setSelection(Width.length());
		Width.setTextColor(Color.BLACK);
		Width.setGravity(Gravity.CENTER_VERTICAL);
		Width.setBackgroundColor(Color.argb(0, 0, 0, 0));
		wd_layout.addView(Width);

		final EditText Height =new EditText(this);
		Height.setText(String.valueOf(ONSVariable.wdh));
		Height.setHint("只能输入数字");
		Height.setInputType(InputType.TYPE_CLASS_NUMBER);
		Height.setSelection(Height.length());
		Height.setTextColor(Color.BLACK);
		Height.setGravity(Gravity.CENTER_VERTICAL);
		Height.setBackgroundColor(Color.argb(0, 0, 0, 0));
		wd_layout.addView(Height);
		
		final OkCancelButton wh=new OkCancelButton(this);
		wh.setBackgroundColor(Color.argb(0, 0, 0, 0));
		wh.setTextSize(21);
		wh.setText("确定");
		wh.setTextColor(Color.BLACK);
		wh.setOnClickListener(new OnClickListener() {
			public void onClick(View v) {
				
				ONSVariable.wdw=Integer.parseInt(Width.getText().toString());
				ONSVariable.wdh=Integer.parseInt(Height.getText().toString());
				
				Editor e = getSharedPreferences("pref", MODE_PRIVATE).edit();
		
				e.putInt("wdw", ONSVariable.wdw);
				e.putInt("wdh", ONSVariable.wdh);
				e.commit();
				wd_popupWindow.dismiss();
			}
		});
		wd_layout.addView(wh);

	}


	public void FontDialog() {

		final LinearLayout font_layout = new LinearLayout(this);
		font_layout.setOrientation(LinearLayout.VERTICAL);	

		final LinearLayout font_layout2 = new LinearLayout(this);
		font_layout2.setOrientation(LinearLayout.HORIZONTAL);	
		
		if (font_popupWindow != null && font_popupWindow.isShowing()) {
					font_popupWindow.dismiss();
				} else {
					
					font_popupWindow = new PopupWindow(font_layout, LayoutParams.WRAP_CONTENT, LayoutParams.WRAP_CONTENT,true);
					font_popupWindow.setAnimationStyle(R.style.Animation_ConfigPanelAnimation);
					font_popupWindow.setTouchable(true);
					font_popupWindow.setOutsideTouchable(true);
					font_popupWindow.setBackgroundDrawable(getResources().getDrawable(R.drawable.config_upper));
					font_popupWindow.showAtLocation(cover, Gravity.CENTER, 0, 0); 
					
				}

		final TextView Text_font=new TextView(this);
		Text_font.setTextSize(21);
		Text_font.setText("选择字体大小");
		Text_font.setGravity(Gravity.CENTER_HORIZONTAL);
		Text_font.setBackgroundColor(Color.argb(0, 0, 0, 0));
		Text_font.setTextColor(Color.BLACK);
		font_layout.addView(Text_font);

		final TextView Text_font2=new TextView(this);
		Text_font2.setTextSize(ONSVariable.myfontsize);
		Text_font2.setText("测试文字");
		Text_font2.setGravity(Gravity.CENTER_HORIZONTAL);
		Text_font2.setBackgroundColor(Color.argb(0, 0, 0, 0));
		Text_font2.setTextColor(Color.BLACK);
		font_layout.addView(Text_font2);

		final SeekBar TextSeekBar=new SeekBar(this);
		TextSeekBar.setMax(100);
		TextSeekBar.setProgress(ONSVariable.myfontsize);
		TextSeekBar.setSecondaryProgress(0);
		TextSeekBar.setBackgroundColor(Color.argb(0, 0, 0, 0));
		TextSeekBar.setOnSeekBarChangeListener(new OnSeekBarChangeListener() {

			public void onStopTrackingTouch(SeekBar seekBar) {
				// TODO Auto-generated method stub

			}

			public void onStartTrackingTouch(SeekBar seekBar) {
				// TODO Auto-generated method stub

			}

			public void onProgressChanged(SeekBar seekBar, int progress,
					boolean fromUser) {
				Text_font2.setTextSize(TextSeekBar.getProgress());
			}
		}); 
		font_layout.addView(TextSeekBar);
		
		final OkCancelButton font_btn=new OkCancelButton(this);
		font_btn.setBackgroundColor(Color.argb(0, 0, 0, 0));
		font_btn.setTextSize(21);
		font_btn.setText("     确定      ");
		font_btn.setGravity(Gravity.CENTER_VERTICAL);
		font_btn.setTextColor(Color.BLACK);
		font_btn.setOnClickListener(new OnClickListener() {
			public void onClick(View v) {

				ONSVariable.myfontsize=TextSeekBar.getProgress()+15;
				ONSVariable.myfontpx = ONSVariable.myfontsize+2;

				Editor e = getSharedPreferences(myname, MODE_PRIVATE).edit();
		
				e.putInt("getfontsize", ONSVariable.myfontsize);
				e.putInt("getfontpx", ONSVariable.myfontpx);
				e.commit();
				font_popupWindow.dismiss();
			}
		});
		font_layout2.addView(font_btn);

		final OkCancelButton font_btn2=new OkCancelButton(this);
		font_btn2.setBackgroundColor(Color.argb(0, 0, 0, 0));
		font_btn2.setTextSize(21);
		font_btn2.setText("      默认    ");
		font_btn2.setGravity(Gravity.CENTER_VERTICAL);
		font_btn2.setTextColor(Color.BLACK);
		font_btn2.setOnClickListener(new OnClickListener() {
			public void onClick(View v) {

				ONSVariable.myfontsize=0;
				ONSVariable.myfontpx=0;

				Editor e = getSharedPreferences(myname, MODE_PRIVATE).edit();
		
				e.putInt("getfontsize", ONSVariable.myfontsize);
				e.putInt("getfontpx", ONSVariable.myfontpx);

				e.commit();
				font_popupWindow.dismiss();
			}
		});
		font_layout2.addView(font_btn2);

		font_layout.addView(font_layout2);

	}


	

	public void TextDialog() {

		final LinearLayout size_layout = new LinearLayout(this);
		size_layout.setOrientation(LinearLayout.VERTICAL);		
		
		if (size_popupWindow != null && size_popupWindow.isShowing()) {
					size_popupWindow.dismiss();
				} else {
					
					size_popupWindow = new PopupWindow(size_layout, LayoutParams.WRAP_CONTENT, LayoutParams.WRAP_CONTENT,true);
					size_popupWindow.setAnimationStyle(R.style.Animation_ConfigPanelAnimation);
					size_popupWindow.setTouchable(true);
					size_popupWindow.setOutsideTouchable(true);
					size_popupWindow.setBackgroundDrawable(getResources().getDrawable(R.drawable.config_upper));
					size_popupWindow.showAtLocation(cover, Gravity.CENTER, 0, 0); 
					
				}


		final TextView Text_size=new TextView(this);
		Text_size.setTextSize(21);
		Text_size.setText(" 输入虚拟按键字体大小 ");
		Text_size.setGravity(Gravity.CENTER_VERTICAL);
		Text_size.setBackgroundColor(Color.argb(0, 0, 0, 0));
		Text_size.setTextColor(Color.BLACK);
		size_layout.addView(Text_size);

		final EditText editSize =new EditText(this);
		editSize.setText(String.valueOf(ONSVariable.textsize));
		editSize.setHint("只能输入数字");
		editSize.setInputType(InputType.TYPE_CLASS_NUMBER);
		editSize.setSelection(editSize.length());
		editSize.setTextColor(Color.BLACK);
		editSize.setGravity(Gravity.CENTER_VERTICAL);
		editSize.setBackgroundColor(Color.argb(0, 0, 0, 0));
		size_layout.addView(editSize);
		
		final OkCancelButton size_btn=new OkCancelButton(this);
		size_btn.setBackgroundColor(Color.argb(0, 0, 0, 0));
		size_btn.setTextSize(21);
		size_btn.setText("确定");
		size_btn.setTextColor(Color.BLACK);
		size_btn.setOnClickListener(new OnClickListener() {
			public void onClick(View v) {
				
				ONSVariable.textsize=Integer.parseInt(editSize.getText().toString());
				
				Editor e = getSharedPreferences("pref", MODE_PRIVATE).edit();
		
				e.putInt("textsize", ONSVariable.textsize);
				e.commit();
				size_popupWindow.dismiss();
			}
		});
		size_layout.addView(size_btn);

	}
	
	public void VideoDialog() {

		final LinearLayout video_layout = new LinearLayout(this);
		video_layout.setOrientation(LinearLayout.VERTICAL);	
		
		SharedPreferences sp = getSharedPreferences("pref", MODE_PRIVATE);
		ONSVariable.mPlayer = sp.getBoolean("playerchoose", true);
		
		if (video_popupWindow != null && video_popupWindow.isShowing()) {
			video_popupWindow.dismiss();
				} else {
					
					video_popupWindow = new PopupWindow(video_layout, LayoutParams.WRAP_CONTENT, LayoutParams.WRAP_CONTENT,true);
					video_popupWindow.setAnimationStyle(R.style.Animation_ConfigPanelAnimation);
					video_popupWindow.setTouchable(true);
					video_popupWindow.setOutsideTouchable(true);
					video_popupWindow.setBackgroundDrawable(getResources().getDrawable(R.drawable.config_upper));
					video_popupWindow.showAtLocation(cover, Gravity.CENTER, 0, 0); 
					
				}
		
		final TextView Text_size=new TextView(this);
		Text_size.setTextSize(21);
		Text_size.setText("        选择视频解码�?     ");
		Text_size.setGravity(Gravity.CENTER_VERTICAL);
		Text_size.setBackgroundColor(Color.argb(0, 0, 0, 0));
		Text_size.setTextColor(Color.BLACK);
		video_layout.addView(Text_size);


		SFDecode = new CheckBox(this);
		SFDecode.setText(" 软解");
		SFDecode.setTextSize(21);
		SFDecode.setBackgroundColor(Color.argb(0, 0, 0, 0));
		SFDecode.setTextColor(Color.BLACK);
		SFDecode.setOnCheckedChangeListener(new OnCheckedChangeListener() {

			public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
				if(isChecked)
				{
					ONSVariable.mPlayer = true;
					HWDecode.setChecked(false);
				}
				else {
					ONSVariable.mPlayer = false;
					HWDecode.setChecked(true);
				}
				Log.d("ss", String.valueOf(ONSVariable.mPlayer));
			}
		});
		video_layout.addView(SFDecode);

		HWDecode = new CheckBox(this);
		HWDecode.setText(" 硬解");
		HWDecode.setTextSize(21);
		//HWDecode.setGravity(Gravity.CENTER_VERTICAL);
		HWDecode.setBackgroundColor(Color.argb(0, 0, 0, 0));
		HWDecode.setTextColor(Color.BLACK);
		HWDecode.setOnCheckedChangeListener(new OnCheckedChangeListener() {

			public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
				if(isChecked)
				{
					ONSVariable.mPlayer = false;
					SFDecode.setChecked(false);
				}
				else {
					ONSVariable.mPlayer = true;
					SFDecode.setChecked(true);
				}

			}
		});
		video_layout.addView(HWDecode);
		
		if(ONSVariable.mPlayer)
			SFDecode.setChecked(true);
		else {
			HWDecode.setChecked(true);
		}
		
		final OkCancelButton ok_btn=new OkCancelButton(this);
		ok_btn.setBackgroundColor(Color.argb(0, 0, 0, 0));
		ok_btn.setTextSize(21);
		ok_btn.setText("确定");
		ok_btn.setTextColor(Color.BLACK);
		ok_btn.setOnClickListener(new OnClickListener() {
			public void onClick(View v) {
				
				Editor e = getSharedPreferences("pref", MODE_PRIVATE).edit();
		
				e.putBoolean("playerchoose", ONSVariable.mPlayer);
				e.commit();
				video_popupWindow.dismiss();
			}
		});
		video_layout.addView(ok_btn);

	}
	
	
	public void gameConfig(View v) {	
		
		if (config_popupWindow != null && config_popupWindow.isShowing()) {
			config_popupWindow.dismiss();
				} else {
					
					config_popupWindow = new PopupWindow(v, LayoutParams.WRAP_CONTENT, LayoutParams.WRAP_CONTENT,true);
					config_popupWindow.setAnimationStyle(R.style.Animation_ConfigPanelAnimation);
					config_popupWindow.setTouchable(true);
					config_popupWindow.setOutsideTouchable(true);
					config_popupWindow.setBackgroundDrawable(getResources().getDrawable(R.drawable.transparent));
					config_popupWindow.showAtLocation(cover, Gravity.RIGHT, 0, 0); 
					//config_popupWindow.update();
					
				}




	}
	
public void cursetting() {
		


		final LinearLayout setting_layout = new LinearLayout(this);
		setting_layout.setOrientation(LinearLayout.VERTICAL);	
		setting_layout.setGravity(Gravity.CENTER);
		
		if (setting_popupWindow != null && setting_popupWindow.isShowing()) {
			setting_popupWindow.dismiss();
				} else {
					
					setting_popupWindow = new PopupWindow(setting_layout, LayoutParams.WRAP_CONTENT, LayoutParams.WRAP_CONTENT,true);
					setting_popupWindow.setAnimationStyle(R.style.Animation_ConfigPanelAnimation);
					setting_popupWindow.setTouchable(true);
					setting_popupWindow.setOutsideTouchable(true);
					setting_popupWindow.setBackgroundDrawable(getResources().getDrawable(R.drawable.config_upper));
					setting_popupWindow.showAtLocation(cover, Gravity.RIGHT, 0, 0); 
					
				}

		final OkCancelButton text_btn=new OkCancelButton(this);
		text_btn.setBackgroundColor(Color.argb(0, 0, 0, 0));
		text_btn.setTextSize(21);
		text_btn.setText("虚拟按键大小");
		text_btn.setTextColor(Color.BLACK);
		text_btn.setOnClickListener(new OnClickListener() {
			
			public void onClick(View v) {
				TextDialog();
				
			}
		});
		setting_layout.addView(text_btn);
		
		final OkCancelButton video_btn=new OkCancelButton(this);
		video_btn.setBackgroundColor(Color.argb(0, 0, 0, 0));
		video_btn.setTextSize(21);
		video_btn.setText("视频解码�?);
		video_btn.setTextColor(Color.BLACK);
		video_btn.setOnClickListener(new OnClickListener() {
			
			public void onClick(View v) {
				VideoDialog();
				
			}
		});
		setting_layout.addView(video_btn);
		
		final OkCancelButton dir_btn=new OkCancelButton(this);
		dir_btn.setBackgroundColor(Color.argb(0, 0, 0, 0));
		dir_btn.setTextSize(21);
		dir_btn.setText("游戏路径设置");
		dir_btn.setTextColor(Color.BLACK);
		dir_btn.setOnClickListener(new OnClickListener() {
			
			public void onClick(View v) {
				chooseDir();	
			}
		});
		setting_layout.addView(dir_btn);
		
		final OkCancelButton unzip_btn=new OkCancelButton(this);
		unzip_btn.setBackgroundColor(Color.argb(0, 0, 0, 0));
		unzip_btn.setTextSize(21);
		unzip_btn.setText("跑分测试");
		unzip_btn.setTextColor(Color.BLACK);
		unzip_btn.setOnClickListener(new OnClickListener() {
			
			public void onClick(View v) {
				Toast.makeText(ONScripter.this, "正在后台解压,完成后将刷新列表.",Toast.LENGTH_LONG).show();
				UnZip.StartUnZip(ONScripter.this);	
			}
		});
		setting_layout.addView(unzip_btn);
		
		final OkCancelButton down_btn=new OkCancelButton(this);
		down_btn.setBackgroundColor(Color.argb(0, 0, 0, 0));
		down_btn.setTextSize(21);
		down_btn.setText("下载示例");
		down_btn.setTextColor(Color.BLACK);
		down_btn.setOnClickListener(new OnClickListener() {
			
			public void onClick(View v) {
				runDownloader("22");
			}
		});
		setting_layout.addView(down_btn);


	}

	public void ColorDialog()
	{
		Colordialog = new ColorPickerDialog(this, R.style.TextAppearanceDialogWindowTitle,FontColor.getTextColors().getDefaultColor(), 
						"FontColor", 
						new ColorPickerDialog.OnColorChangedListener() {
					
					public void colorChanged(int color) {
						FontColor.setTextColor(color);
						ONSVariable.myfont_color1 = Colordialog.getR();
						ONSVariable.myfont_color2 = Colordialog.getG();
						ONSVariable.myfont_color3 = Colordialog.getB();

						Editor e = getSharedPreferences(myname, MODE_PRIVATE).edit();
		
						e.putInt("fontr", ONSVariable.myfont_color1);
						e.putInt("fontg", ONSVariable.myfont_color2);
						e.putInt("fontb", ONSVariable.myfont_color3);
						e.putInt("cbcolor", color);
						e.commit();
					}
				});
		Colordialog.show();
	}


	

	
	public void About() {
		
		boolean sdkv = false;

		AlertDialog.Builder builder;
		try {
			builder = new AlertDialog.Builder(this,R.style.AliDialog);
		} catch (NoSuchMethodError e) {
		    builder = new AlertDialog.Builder(this);
		    sdkv = true;
		}
		builder.setCancelable(false);
		final AlertDialog alert = builder.create(); 
		
		LayoutInflater factory = LayoutInflater.from(this);
        View view = factory.inflate(R.layout.sao_dialog, null);
        
        if(sdkv) {
        	view.setBackgroundDrawable(getResources().getDrawable(R.drawable.config_upper));
		}
        alert.show();
		alert.setContentView(view);
        
        TextView title=(TextView)view.findViewById(R.id.titleView2);
        TextView info=(TextView)view.findViewById(R.id.infoView1);
        OkCancelButton okButton=(OkCancelButton)view.findViewById(R.id.okbutton2);
        OkCancelButton urlButton=(OkCancelButton)view.findViewById(R.id.urlbutton1);
        title.setText("  关于");
        info.setText(getResources().getString(R.string.info));
        okButton.setOnClickListener(new OnClickListener() {
			
			public void onClick(View v) {
				alert.dismiss();
				
			}
		});
        urlButton.setOnClickListener(new OnClickListener() {
			
			public void onClick(View v) {
				Uri uri = Uri.parse("http://portal.bakerist.info/node/68");
				Intent web = new Intent(Intent.ACTION_VIEW, uri);
				startActivity(web);
				
			}
		});
		
	}
	
	private void runDownloader(String version) {
		String version_filename = version;
		File file = new File(Globals.CurrentDirectoryPath + "/" + version_filename);
		if (file.exists() == false){
			progDialog = new ProgressDialog(this);
			progDialog.setCanceledOnTouchOutside(false);
			progDialog.setProgressStyle(ProgressDialog.STYLE_HORIZONTAL);
			progDialog.setMessage("Downloading archives from Internet:");
			progDialog.setOnKeyListener(new OnKeyListener(){
				public boolean onKey(DialogInterface dialog, int keyCode, KeyEvent event){
						if (KeyEvent.KEYCODE_SEARCH == keyCode || KeyEvent.KEYCODE_BACK == keyCode)
								return true;
						return false;
				}
			});
			progDialog.show();

			PowerManager pm = (PowerManager) getSystemService(Context.POWER_SERVICE);
			wakeLock = pm.newWakeLock(PowerManager.SCREEN_DIM_WAKE_LOCK, "ONScripter");
			wakeLock.acquire();
			alertDialogBuilder = new AlertDialog.Builder(this);

			runDownloaderSub(version,"http://portal.bakerist.info/wp-content/uploads/2013/02/Luatest.zip");
		}
	
	}
	
	private void runDownloaderSub(String version,String download_url)
	{
		final String version_filename = version;
		String url = download_url;

		if (url.length() != 0){
			//String deviceId = Secure.getString(this.getContentResolver(), Secure.ANDROID_ID);

			String zip_dir = Globals.CurrentDirectoryPath;
			String zip_filename = url.substring(url.lastIndexOf("/")+1);

			downloader = new DataDownloader(zip_dir, zip_filename, Globals.CurrentDirectoryPath, version_filename, url, -1, handler);
		}
		
	}

	
	final Handler handler = new Handler(){
		public void handleMessage(Message msg){
			int current = msg.getData().getInt("current");
			if (current == -1){
				progDialog.dismiss();
				loadCurrentDirectory();
			}
			else if (current == -2){
				progDialog.dismiss();
				showErrorDialog(msg.getData().getString("message"));
			}
			else{
				progDialog.setMessage(msg.getData().getString("message"));
				int total = msg.getData().getInt("total");
				if (total != progDialog.getMax())
					progDialog.setMax(total);
				progDialog.setProgress(current);
			}
		}
	};

	private void showErrorDialog(String mes)
	{
		

		alertDialogBuilder.setTitle("Error");
		alertDialogBuilder.setMessage(mes);
		alertDialogBuilder.setPositiveButton("Quit", new DialogInterface.OnClickListener(){
			public void onClick(DialogInterface dialog, int whichButton) {
				//finish();
				ErroralertDialog.dismiss();
			}
		});
		ErroralertDialog = alertDialogBuilder.create();
		ErroralertDialog.show();
	}

	public void sendMessage(int current, int total, String str)
	{
		Message msg = handler.obtainMessage();
		Bundle b = new Bundle();
		b.putInt("total", total);
		b.putInt("current", current);
		b.putString("message", str);
		msg.setData(b);
		handler.sendMessage(msg);
	}

	/**********************************************************************************************************/

	public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
		U.scrollViewToCenter(view, games);

		if(items.getSelectedPosition() != position) {

			// Set Selection
			items.setSelectedPosition(position);

			loadGameItem(items.getSelectedItem());
			
			Globals.CurrentDirectoryPath = mDirFileArray[position].getAbsolutePath();
			
			isSelect = true;
			
		}
		
		Command.invoke(GameAdapter.SHOW_PANEL).args(items).sendDelayed(100);
	}
	
	
	public boolean onTouch(View v, MotionEvent event) {
		// TODO Auto-generated method stub
		x = event.getRawX();
		
		switch (event.getAction()) {

		case MotionEvent.ACTION_DOWN:
			startX = x;
			break;
		case MotionEvent.ACTION_MOVE:
			if (Math.abs(startX - x) > TOUCH_SLOP) {
				isMoved = true;
			}
			break;
		case MotionEvent.ACTION_UP:
			if (isMoved == true && isSelect) {
				runAppLaunchConfig();
				isMoved = isSelect = false;
			//return true;
			}
			break;
		}
		return false;
	}
	
	public void onClick(View v) {
		// TODO Handle Click Events Here
		//Game item = items.getItem(items.getSelectedPosition());
		switch(v.getId()) {
		case R.id.btn_settings:
			cursetting();
			break;
		case R.id.btn_about:
			About();
			break;
		case R.id.btn_config:
			Game item = items.getSelectedItem();
			if(item.gameapk != null )
			{
				Toast.makeText(ONScripter.this, "ren'py游戏暂未提供设置�?..",Toast.LENGTH_LONG).show();
			}
			else {
				if(item.xsystemgr != null )
					Toast.makeText(ONScripter.this, "这是xsystem游戏，需将版本设置为xsystem35...",Toast.LENGTH_LONG).show();
				if(item.seentxt != null )
					Toast.makeText(ONScripter.this, "这是xclannad游戏，需将版本设置为xclannad...",Toast.LENGTH_LONG).show();
				runAppLaunchConfig();
			}
			break;
		case R.id.btn_play:
			FSetting(myname);
			runApp();
			break;
		}
	}
	
	private boolean onBackKeyPressed() {
    	if(videoframe.isVideoFullscreen()) {
    		videoframe.toggleFullscreen();
    		return true;
    	}
    	if(mStatePreview.currentState() == STATE_VIDEO_PLAY || 
    			mStatePreview.currentState() == STATE_AUDIO_PLAY) {
    		mStatePreview.gotoState(STATE_COVER_VISIBLE);
    		return true;
    	}
    	return false;
	}

	private long last_backkey_pressed = 0;
	
	public boolean onKeyUp(int keyCode, KeyEvent msg) {
        if (keyCode == KeyEvent.KEYCODE_BACK) {
        	if(onBackKeyPressed()) {
        		return true;
        	}
            if (msg.getEventTime()-last_backkey_pressed<2000) {
            	MobclickAgent.onKillProcess(this);
                finish();
                System.exit(0);
            } else {
                Toast.makeText(
                		this, 
                		R.string.notify_exit, Toast.LENGTH_SHORT
                		).show();
                last_backkey_pressed=msg.getEventTime();
            }
            return true;
        }
		return super.onKeyUp(keyCode, msg);
	}
	
	/**********************************************************************************************************/
	

    

	

	@Override
	protected void onDestroy() 
	{
		super.onDestroy();
		
		destroyImageManager();
		
	}

	@Override
	public void onConfigurationChanged(Configuration newConfig)
	{
		super.onConfigurationChanged(newConfig);
		// Do nothing here
	}
	
// Async Operation Block {{{
	
	static {
		// Register Async Operation
		cn.natdon.onscripterv2.command.Command.register(ONScripter.class);
	}

	public static final int LOOP_VIDEO_PREVIEW = 13;
	
	public static final int RELEASE_VIDEO_PREVIEW = 14;

	public static final int LOOP_AUDIO_PLAY = 21;

	public static final int TRY_DISPLAY_COVER = 35;
	
	public static final int TRY_DISPLAY_BKG = 36;
	
	public static final int ACTION_AFTER_DISPLAY_COVER = 38;
	
	public static final int ADD_ITEM_TO_LISTADAPTER = 102;

	public static final int DATASET_CHANGED_LISTADAPTER = 103;
	
	@CommandHandler(id = LOOP_VIDEO_PREVIEW)
	public static void LOOP_VIDEO_PREVIEW(VideoView player) {
		player.seekTo(0);
		player.start();
	}
	
	@CommandHandler(id = RELEASE_VIDEO_PREVIEW)
	public static void RELEASE_VIDEO_PREVIEW(VideoView player) {
		player.setVideoURI(null);
	}
	
	@CommandHandler(id = LOOP_AUDIO_PLAY)
	public static void LOOP_AUDIO_PLAY(AudioPlayer player) {
		player.seekTo(0);
		player.start();
	}
	
	@CommandHandler(id = TRY_DISPLAY_COVER)
	public static void TRY_DISPLAY_COVER(ONScripter activity) {
		activity.tryDisplayCover();
	}

	@CommandHandler(id = TRY_DISPLAY_BKG)
	public static void TRY_DISPLAY_BKG(ONScripter activity) {
		activity.tryDisplayBackground();
	}

	@CommandHandler(id = ACTION_AFTER_DISPLAY_COVER)
	public static void ACTION_AFTER_DISPLAY_COVER(ONScripter activity) {
		activity.checkActionAfterDisplayCover();
	}
	
	@CommandHandler(id = ADD_ITEM_TO_LISTADAPTER)
	public static void ADD_ITEM_TO_LISTADAPTER(GameAdapter adapter, Game item) {
		adapter.add(item);
		Command.invoke(DATASET_CHANGED_LISTADAPTER).args(adapter).only().sendDelayed(200);
	}

	@CommandHandler(id = DATASET_CHANGED_LISTADAPTER)
	public static void DATASET_CHANGED_LISTADAPTER(GameAdapter adapter) {
		adapter.notifyDataSetChanged();
	}
	// }}}

	

	public static ONScripter instance = null;
	


	public static CheckBox checkWS = null;
		public static CheckBox checkDR = null;
		public static CheckBox checkSP = null;
		public static CheckBox keepON = null;
		public static CheckBox OtherPL = null;
		public static CheckBox FullScreen = null;
		public static CheckBox ScaleFullScreen = null;
		public static CheckBox checkWD = null;
		public static CheckBox checkLog = null;
		public static CheckBox Cursor = null;
		public static CheckBox FontSize = null;
		public static CheckBox FontColor = null;
		public static CheckBox SFDecode = null;
		public static CheckBox HWDecode = null;
		public static CheckBox checkHW = null;

		public void run() {
			// TODO Auto-generated method stub
			
		}


	



}

