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

import java.io.BufferedInputStream;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileFilter;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Comparator;
import java.util.Date;
import java.util.HashMap;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.app.ActivityManager;
import android.app.AlertDialog;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.Intent.ShortcutIconResource;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.content.pm.ActivityInfo;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager.NameNotFoundException;
import android.content.res.Configuration;
import android.graphics.Bitmap;
import android.graphics.Bitmap.Config;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Matrix;
import android.graphics.Paint;
import android.graphics.Rect;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.media.AudioManager;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.text.InputType;
import android.text.format.Formatter;
import android.util.Log;
import android.view.Display;
import android.view.Gravity;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.View.OnLongClickListener;
import android.view.View.OnTouchListener;
import android.view.ViewGroup.LayoutParams;
import android.view.Window;
import android.view.WindowManager;
import android.view.animation.Animation;
import android.view.animation.Animation.AnimationListener;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.EditText;
import android.widget.FrameLayout;
import android.widget.HorizontalScrollView;
import android.widget.ImageButton;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.PopupWindow;
import android.widget.RelativeLayout;
import android.widget.ScrollView;
import android.widget.SeekBar;
import android.widget.SeekBar.OnSeekBarChangeListener;
import android.widget.SimpleAdapter;
import android.widget.TextView;
import cn.natdon.onscripterv2.GifView.GifView;
import cn.natdon.onscripterv2.VideoPlayer.activity.PlayerActivity;
import cn.natdon.onscripterv2.decoder.BackgroundDecoder;
import cn.natdon.onscripterv2.decoder.CoverDecoder;
import cn.natdon.onscripterv2.widget.MediaController;
import cn.natdon.onscripterv2.widget.VideoView;

import com.footmark.utils.cache.FileCache;
import com.footmark.utils.image.ImageManager;
import com.footmark.utils.image.ImageSetter;
import com.umeng.analytics.MobclickAgent;
import com.umeng.update.UmengUpdateAgent;


@SuppressLint("NewApi")
public class ONScripter extends Activity implements OnItemClickListener,Button.OnClickListener, DialogInterface.OnClickListener{

public static TextView about, Repair, Language, ONSSetting, SetOrientation, DirSetting, VerChange ,TextSize, OnlineVideo, ColorText;
public static PopupWindow m_popupWindow,button_popupWindow,light_popupWindow,wd_popupWindow,ver_popupWindow,size_popupWindow,font_popupWindow,time_popupWindow;	
private boolean mIsLandscape = true;
	private boolean mButtonVisible = true;
	private boolean mScreenCentered = false;
	private boolean mButtonAtLeft = false;
	private boolean set_checkWS = false;
	private boolean set_checkSP = false;
	private boolean set_OtherPL = false;
	private boolean set_checkDR = false;
	private boolean set_keepON = false;
	private boolean set_FullScreen = false;
	private boolean set_ScaleFullScreen = false;
	private boolean set_Cursor = false;
	private boolean set_FontSize = false;
	private boolean set_FontColor = false;
	private boolean isMoved = false;
	private boolean isRunning = false;
	private boolean DialogOpen =false;
	private boolean mInLauncher = true;

	private int myfontsize;
	private int myfont_color1,myfont_color2,myfont_color3,cbColor;
	private float x, y;
	private float startX, startY;
	private float light=0;
	private SimpleDateFormat df;
	private String oldtime, nowtime, battery,myTime;
	
	private Drawable bg = null;
	private Drawable bg2 = null;

	public String myname, Gamenames, Videopath = "";
	public static String extra;
	public static String mysetting;
	private static final String SHORT_CUT_EXTRAS = "cn.natdon.onscripterv2";
	public static int textsize;

	private PackageInfo packageInfo = null;

	private WindowManager.LayoutParams lp;
	
	public static LinearLayout wdlayout = null;
	public static int wdw ,wdh;
	public static int dw, dh;
	private static final int TOUCH_SLOP = 50;
	public static WindowManager wm;
	public static WindowManager wdwm;
	public static WindowManager.LayoutParams wdparams = new WindowManager.LayoutParams();
	public static WindowManager.LayoutParams params = new WindowManager.LayoutParams();
	


	private ColorPickerDialog Colordialog;
	

	private LinearLayout layout1 = null;
	private LinearLayout Btnlayout;
	private LinearLayout Fulllayout = null;
	private HorizontalScrollView HSV = null;
	private ScrollView FullSV = null;
	private ImageButton popbtn;

	private native int nativeFontSize(int font_x,int font_y,int font_px,int font_py);
	private native int nativeFontColor(boolean usecolor,int fcolor1,int fcolor2,int fcolor3);
	private native int nativeInitJavaCallbacks();
	
	/******************************************************************************************************************/
	
	{
		// Set the priority, trick useful for some CPU
		Thread.currentThread().setPriority(Thread.MAX_PRIORITY);
	}

	private static volatile boolean isVideoInitialized = false;

	private ImageManager imgMgr;

	private ListView games;
	private ImageView cover, background;
	private TextView gametitle;
	private VideoView preview;
	private RelativeLayout videoframe;

	private GameAdapter items;
	
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
	}

	private void initImageManager() {
		destroyImageManager();
		if(Environment.MEDIA_MOUNTED.equals(
				Environment.getExternalStorageState())){
			imgMgr = new ImageManager(new FileCache(
					new File(
							Environment.getExternalStorageDirectory(),
							"saoui/cover")));
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

	private void configureVideoPlayer() {
		preview.setVideoQuality(MediaPlayer.VIDEOQUALITY_HIGH);
		preview.setOnCompletionListener(new OnCompletionListener() {

			public void onCompletion(MediaPlayer player) {
				Command.invoke(Command.LOOP_VIDEO_PREVIEW).of(preview).send();
			}

		});
		preview.setOnErrorListener(new OnErrorListener() {

			
			public boolean onError(MediaPlayer player, int framework_err, int impl_err) {
				releaseVideoPlay();
				return true;
			}

		});
		preview.setMediaController(new MediaController(this));

		// Initialize the Vitamio codecs
		if(!Vitamio.isInitialized(this)) {
			new AsyncTask<Object, Object, Boolean>() {
				@Override
				protected void onPreExecute() {
					isVideoInitialized = false;
				}

				@Override
				protected Boolean doInBackground(Object... params) {
					Thread.currentThread().setPriority(Thread.MIN_PRIORITY);
					boolean inited = Vitamio.initialize(ONScripter.this);
					Thread.currentThread().setPriority(Thread.NORM_PRIORITY);
					return inited;
				}

				@Override
				protected void onPostExecute(Boolean inited) {
					if (inited) {
						isVideoInitialized = true;
					}
				}

			}.execute();
		}else{
			isVideoInitialized = true;
		}
	}

	private Animation animCoverOut = AnimationFactory.coverOutAnimation(new AnimationListener() {

		public void onAnimationEnd(Animation animation) {
			animCoverOut = AnimationFactory.coverOutAnimation(this);
			displayCover();
		}

		public void onAnimationRepeat(Animation animation) {}

		public void onAnimationStart(Animation animation) {}

	});

	private Animation animBackgroundOut = AnimationFactory.bkgOutAnimation(new AnimationListener() {

		public void onAnimationEnd(Animation arg0) {
			animBackgroundOut = AnimationFactory.bkgOutAnimation(this);
			if(background.getTag() instanceof Bitmap) {
				background.setImageBitmap((Bitmap) background.getTag());
				background.setBackgroundDrawable(null);
				background.setTag(null);
				background.startAnimation(AnimationFactory.bkgInAnimation());
			}
		}

		public void onAnimationRepeat(Animation animation) {}

		public void onAnimationStart(Animation animation) {}

	});

	private Animation animHideVideo = AnimationFactory.hideVideoPlayerAnimation(new AnimationListener(){

		public void onAnimationEnd(Animation animation) {
			videoframe.setVisibility(View.GONE);
		}

		public void onAnimationRepeat(Animation animation) {}

		public void onAnimationStart(Animation animation) {}

	});

	private Animation animPlayVideo = AnimationFactory.videoPlayerAnimation(new AnimationListener(){

		public void onAnimationEnd(Animation animation) {
			startVideoPlay();
		}

		public void onAnimationRepeat(Animation animation) {}

		public void onAnimationStart(Animation animation) {
			videoframe.setVisibility(View.VISIBLE);
		}

	});
	
	/******************************************************************************************************************/


	public void playVideo(char[] filename) {
		if (Locals.gDisableVideo == false) {
			try {
				String filename2 = "file:/" + Globals.CurrentDirectoryPath + "/"
						+ new String(filename);
				filename2 = filename2.replace('\\', '/');
				Log.v("ONS", "playVideo: " + filename2);
				if (Locals.gOtherPL) {
					Uri uri = Uri.parse(filename2);
					Intent i = new Intent(Intent.ACTION_VIEW);
					i.setDataAndType(uri, "video/*");
					startActivityForResult(i, -1);
				} else {

					if( packageInfo != null){
					Intent in = new Intent();
					in.putExtra("one", filename2);
					in.putExtra("two", false);
					in.setClass(ONScripter.this, PlayerActivity.class);
					ONScripter.this.startActivity(in);
					}else{
					Intent in = new Intent();
					in.putExtra("one", filename2);
					in.setClass(ONScripter.this, MyVideoPlayer.class);
					startActivity(in);
					}
				}
				overridePendingTransition(android.R.anim.fade_in,
						android.R.anim.fade_out);
			} catch (Exception e) {
				Log.e("ONS", "playVideo error:  " + e.getClass().getName());
			}
		}
	}
	
	@SuppressWarnings("deprecation")
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		UmengUpdateAgent.setUpdateOnlyWifi(false);
		UmengUpdateAgent.update(this);
		MobclickAgent.onError(this);
		
		//setRequestedOrientation( ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE ); //for Test

		requestWindowFeature(Window.FEATURE_NO_TITLE);
		getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager.LayoutParams.FLAG_FULLSCREEN);
		getWindow().setFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON, WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
		
		setVolumeControlStream(AudioManager.STREAM_MUSIC);
		
		instance = this;
		
		Settings.LoadGlobals(this);

		Display disp = ((WindowManager) this
				.getSystemService(Context.WINDOW_SERVICE)).getDefaultDisplay();
		dw = disp.getWidth();
		dh = disp.getHeight();


		/*android.content.pm.Signature[] sigs;
		try {
			String s = "30820237308201a0a00302010202044eb7a114300d06092a864886f70d01010505003060310b3009060355040613026368310e300c060355040813056368696e61310e300c060355040713056368696e61310f300d060355040a13066e6174646f6e310f300d060355040b13066e6174646f6e310f300d060355040313066e6174646f6e301e170d3131313130373039313235325a170d3339303332353039313235325a3060310b3009060355040613026368310e300c060355040813056368696e61310e300c060355040713056368696e61310f300d060355040a13066e6174646f6e310f300d060355040b13066e6174646f6e310f300d060355040313066e6174646f6e30819f300d06092a864886f70d010101050003818d0030818902818100bec1bac249e85e0279a5364f6115d8dbeb894466c600c8f6ca318937662e59fb05287c79499fa9d6646d897b34fdf21e1ae62b58042f4a8d0070a5a5307f34d0a863a0ec498f5183a8cfc74ea9e75a8572e76f40106e830daaf8aaf097267138476d867f8c676e35da61b81fffc9540373acc5a2a2bf88b61066d21ddbca1f050203010001300d06092a864886f70d0101050500038181005cb3dc98fe22b343ab6d229068f5c10f04e2bcf934adbc42f377516caa01e7fb797fab46158db788f2a9077b304ab13ff55d83b42f72d220da91c38296c87cb14d1cce9e20843202d6f3d56d733ebc91ea663b2b8992f0d3f1decfa927f8b3d2c3ea14a0e283034f9351956603461efa863e47fed3418941fc123c737b047bc4"
; 
			sigs = getBaseContext().getPackageManager().getPackageInfo( "cn.natdon.onscripterv2", 64).signatures;
			
			String a=sigs[0].toCharsString();
			if(a.equals(s)){}
			else{
				s.codePointCount(1, 9999);
				android.os.Process.killProcess(android.os.Process.myPid());
			}
			
		} catch (NameNotFoundException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}*/

//

		try {
						packageInfo = ONScripter.this.getPackageManager().getPackageInfo("cn.natdon.onsaddlib",
								0);

					} catch (NameNotFoundException e) {
						packageInfo = null;
						e.printStackTrace();
					}
		this.registerReceiver(mBatteryInfoReceiver, new IntentFilter(
				Intent.ACTION_BATTERY_CHANGED));
		df = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");   
        	oldtime = df.format(new Date());

		SharedPreferences sp = getSharedPreferences("pref", MODE_PRIVATE);
		textsize = sp.getInt("textsize", 15);
		
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

			SharedPreferences sp2 = getSharedPreferences("myver", MODE_PRIVATE);
			int about = sp2.getInt("about", 5);
			if (about == 5) {
				About();
				Editor e = getSharedPreferences("myver", MODE_PRIVATE).edit();
				e.putInt("about", 6);
				e.commit();
			}
			
			if(Build.VERSION.SDK_INT < 9) {
				setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
			}
			setContentView(R.layout.activity_main);
			findViews();

			// Pass parameters to CoverDecoder to get better performance
			CoverDecoder.init(getApplicationContext(), cover.getWidth(), cover.getHeight());

			initImageManager();

			configureVideoPlayer();

			// Initializing data and binding to ListView
			items = new GameAdapter(this, R.layout.gamelist_item, new ArrayList<Game>());
			loadCurrentDirectory();
			games.setAdapter(items);
			games.setOnItemClickListener(this);
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
	
	
		private ONScripter mActivity;
		
		
		
		private File [] mDirFileArray;

		public String iconPath = null;


		public void FreeMemory(){
			bg = null;
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
					
					
					
					String[] dirPathArray = new String[mDirFileArray.length];
					for(int i = 0; i < mDirFileArray.length; i ++){
						dirPathArray[i] = mDirFileArray[i].getName();
						Gamenames = mDirFileArray[i].getName();
						iconPath = mDirFileArray[i].toString() + "/ICON.PNG";
						File logo = new File(iconPath);
						Videopath = mDirFileArray[i].toString() + "/PREVIEW.MP4";
						File video = new File(Videopath);
						

						if(mDirFileArray[i].isDirectory() && video.exists()){
							items.add(new Game() {{title=Gamenames; cover=iconPath; video=Videopath;}});
							
						}
						else if(mDirFileArray[i].isDirectory() && logo.exists())
							items.add(new Game() {{title=Gamenames; cover=iconPath;}});
						else {
							items.add(new Game() {{title=Gamenames; }});
						}
						
					}
					items.notifyDataSetChanged();
					
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
			
			AlertDialog.Builder alertDialogBuilder = new AlertDialog.Builder(mActivity);
			alertDialogBuilder.setTitle(getString(R.string.Launch_ChooseDirectory));
			alertDialogBuilder.setItems(items, this);
			alertDialogBuilder.setNegativeButton(getString(R.string.Cancel), null);
			alertDialogBuilder.setCancelable(true);
			AlertDialog alertDialog = alertDialogBuilder.create();
			alertDialog.show();
		}
		
		public void onClick(View v)
		{
			chooseDir();
		}
		
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
					Settings.SaveGlobals(mActivity);
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
				
				AlertDialog.Builder alertDialogBuilder = new AlertDialog.Builder(mActivity);
				alertDialogBuilder.setTitle(mDirBrowserCurDirPath);
				alertDialogBuilder.setItems(dirPathArray, this);
				alertDialogBuilder.setPositiveButton(getString(R.string.Launch_SetDirectory), new DialogInterface.OnClickListener(){
					public void onClick(DialogInterface dialog, int whichButton) {	
						Globals.CurrentDirectoryPathForLauncher = mDirBrowserCurDirPath;
						Settings.SaveGlobals(mActivity);
						loadCurrentDirectory();
					}
				});
				alertDialogBuilder.setNegativeButton(getString(R.string.Cancel), null);
				alertDialogBuilder.setCancelable(true);
				mDirBrowserDialog = alertDialogBuilder.create();
				mDirBrowserDialog.show();
			} catch(Exception e){
				AlertDialog.Builder alertDialogBuilder = new AlertDialog.Builder(mActivity);
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
		
		Button mRunButton,mRunButton2,mRunButton3;

		
		public AppLaunchConfigView(ONScripter activity)
		{
			super(activity);
			mActivity = activity;
			
			setOrientation(LinearLayout.VERTICAL);
			{
				mConfView = new ScrollView(mActivity);
				{
					mConfLayout = new LinearLayout(mActivity);
					mConfLayout.setBackgroundColor(0xf5f5f5f5);
					bg2 = Drawable.createFromPath(Globals.CurrentDirectoryPathForLauncher+"/bg2.png");
					if (bg2 != null) {
						mConfLayout.setBackgroundDrawable(bg2);
					}
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
								Button btn = new Button(mActivity);
								btn.setText(getString(R.string.Conf_Change));
								btn.setOnClickListener(new OnClickListener(){
									public void onClick(View v){
										AlertDialog.Builder alertDialogBuilder = new AlertDialog.Builder(mActivity);
										alertDialogBuilder.setTitle(getString(R.string.Conf_ExecuteModule));
										alertDialogBuilder.setItems(Globals.APP_MODULE_NAME_ARRAY, new DialogInterface.OnClickListener(){
											public void onClick(DialogInterface dialog, int which)
											{
												Locals.AppModuleName = Globals.APP_MODULE_NAME_ARRAY[which];
												mExecuteModuleText.setText(Locals.AppModuleName);
												Settings.SaveLocals(mActivity);
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
								Button btn = new Button(mActivity);
								btn.setText(getString(R.string.Conf_Change));
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
												Settings.SaveLocals(mActivity);
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
							
							Button btn1 = new Button(mActivity);
							btn1.setText(getString(R.string.Conf_Swap));
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
									Settings.SaveLocals(mActivity);
								}
							});
							screenRatioLayout.addView(btn1, new LinearLayout.LayoutParams(LinearLayout.LayoutParams.WRAP_CONTENT, LinearLayout.LayoutParams.WRAP_CONTENT));
							
							if(Globals.VIDEO_RATIO_ITEMS.length >= 2){
								Button btn = new Button(mActivity);
								btn.setText(getString(R.string.Conf_Change));
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
												Settings.SaveLocals(mActivity);
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
							
							Button btn = new Button(mActivity);
							btn.setText(getString(R.string.Conf_Change));
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
											Settings.SaveLocals(mActivity);
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
									Settings.SaveLocals(mActivity);
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
								txt2.setText("原比例全屏,需拖动,非拉伸");
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
							checkWD.setText("窗口化");
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
							OtherPL.setText("外部播放器");
							OtherPL.setBackgroundColor(Color.argb(0, 0, 0, 0));
							OtherPL.setTextColor(Color.BLACK);
							OtherSettingLayout.addView(OtherPL);

							FontSize = new CheckBox(mActivity);
							FontSize.setText("字体大小");
							FontSize.setBackgroundColor(Color.argb(0, 0, 0, 0));
							FontSize.setTextColor(Color.BLACK);
							FontSize.setOnCheckedChangeListener(new OnCheckedChangeListener() {
			
								public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
									if(isChecked && DialogOpen)
									FontDialog();
				
								}
							});
							OtherSettingLayout.addView(FontSize);

							FontColor = new CheckBox(mActivity);
							FontColor.setText("字体颜色");
							FontColor.setBackgroundColor(Color.argb(0, 0, 0, 0));
							FontColor.setTextColor(cbColor);
							FontColor.setOnCheckedChangeListener(new OnCheckedChangeListener() {
			
								public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
									if(isChecked && DialogOpen)
									ColorDialog();
				
								}
							});
							OtherSettingLayout.addView(FontColor);

							keepON = new CheckBox(mActivity);
							keepON.setText("长亮 ");
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

								set_FullScreen = false;
								e.putBoolean("FullScreen", set_FullScreen);

								set_checkSP = false;
								e.putBoolean("checkSP", set_checkSP);

								set_OtherPL = false;
								e.putBoolean("OtherPL", set_OtherPL);

								set_FontSize = false;
								e.putBoolean("FontSize", set_FontColor);

								set_FontColor = false;
								e.putBoolean("FontColor", set_FontColor);


								set_keepON = false;
								e.putBoolean("keepON", set_keepON);

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
										Settings.SaveLocals(mActivity);
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
						
						//Environment
						/*mEnvironmentTextArray = new TextView[Globals.ENVIRONMENT_ITEMS.length];
						mEnvironmentButtonArray = new Button[Globals.ENVIRONMENT_ITEMS.length];
						for(int i = 0; i < Globals.ENVIRONMENT_ITEMS.length; i ++){
							LinearLayout envLayout = new LinearLayout(mActivity);
							{
								final int index = i;
								String value = Locals.EnvironmentMap.get(Globals.ENVIRONMENT_ITEMS[index][1]);
								
								CheckBox chk = new CheckBox(mActivity);
								chk.setChecked(value != null);
								chk.setOnClickListener(new OnClickListener(){
									public void onClick(View v){
										CheckBox c = (CheckBox)v;
										if(!c.isChecked()){
											Locals.EnvironmentMap.remove(Globals.ENVIRONMENT_ITEMS[index][1]);
											mEnvironmentTextArray[index].setText(Globals.ENVIRONMENT_ITEMS[index][1]);
											mEnvironmentButtonArray[index].setVisibility(View.GONE);
										} else {
											Locals.EnvironmentMap.put(Globals.ENVIRONMENT_ITEMS[index][1], Globals.ENVIRONMENT_ITEMS[index][2]);
											mEnvironmentTextArray[index].setText(Globals.ENVIRONMENT_ITEMS[index][1] + "=" + Globals.ENVIRONMENT_ITEMS[index][2]);
											mEnvironmentButtonArray[index].setVisibility(View.VISIBLE);
										}
										Settings.SaveLocals(mActivity);
									}
								});
								envLayout.addView(chk, new LinearLayout.LayoutParams(LinearLayout.LayoutParams.WRAP_CONTENT, LinearLayout.LayoutParams.WRAP_CONTENT));
								
								LinearLayout txtLayout = new LinearLayout(mActivity);
								txtLayout.setOrientation(LinearLayout.VERTICAL);
								{
									TextView txt1 = new TextView(mActivity);
									txt1.setTextSize(18.0f);
									txt1.setText(Globals.ENVIRONMENT_ITEMS[index][0]);
									txt1.setTextColor(Color.BLACK);
									txtLayout.addView(txt1, new LinearLayout.LayoutParams( LinearLayout.LayoutParams.FILL_PARENT, LinearLayout.LayoutParams.WRAP_CONTENT));
									
									mEnvironmentTextArray[i] = new TextView(mActivity);
									mEnvironmentTextArray[i].setPadding(5, 0, 0, 0);
									mEnvironmentTextArray[i].setTextColor(Color.BLACK);
									if(value == null){
										mEnvironmentTextArray[i].setText(Globals.ENVIRONMENT_ITEMS[index][1]);
									} else {
										mEnvironmentTextArray[i].setText(Globals.ENVIRONMENT_ITEMS[index][1] + "=" + value);
									}
									txtLayout.addView(mEnvironmentTextArray[i], new LinearLayout.LayoutParams( LinearLayout.LayoutParams.FILL_PARENT, LinearLayout.LayoutParams.WRAP_CONTENT));
								}
								envLayout.addView(txtLayout, new LinearLayout.LayoutParams( LinearLayout.LayoutParams.WRAP_CONTENT, LinearLayout.LayoutParams.WRAP_CONTENT, 1));
								
								mEnvironmentButtonArray[index] = new Button(mActivity);
								mEnvironmentButtonArray[index].setText(getString(R.string.Conf_Change));
								mEnvironmentButtonArray[index].setOnClickListener(new OnClickListener(){
									public void onClick(View v){
										final EditText ed = new EditText(mActivity);
										ed.setInputType(InputType.TYPE_CLASS_TEXT);
										ed.setText(Locals.EnvironmentMap.get(Globals.ENVIRONMENT_ITEMS[index][1]));
										
										AlertDialog.Builder alertDialogBuilder = new AlertDialog.Builder(mActivity);
										alertDialogBuilder.setTitle(Globals.ENVIRONMENT_ITEMS[index][1]);
										alertDialogBuilder.setView(ed);
										alertDialogBuilder.setPositiveButton(getString(R.string.OK), new DialogInterface.OnClickListener(){
											public void onClick(DialogInterface dialog, int whichButton) {
												String newval = ed.getText().toString();
												Locals.EnvironmentMap.put(Globals.ENVIRONMENT_ITEMS[index][1], newval);
												mEnvironmentTextArray[index].setText(Globals.ENVIRONMENT_ITEMS[index][1] + "=" + newval);
												Settings.SaveLocals(mActivity);
											}
										});
										alertDialogBuilder.setNegativeButton(getString(R.string.Cancel), null);
										alertDialogBuilder.setCancelable(true);
										AlertDialog alertDialog = alertDialogBuilder.create();
										alertDialog.show();
									}
								});
								mEnvironmentButtonArray[index].setVisibility(value != null ? View.VISIBLE : View.GONE);
								envLayout.addView(mEnvironmentButtonArray[index], new LinearLayout.LayoutParams(LinearLayout.LayoutParams.WRAP_CONTENT, LinearLayout.LayoutParams.WRAP_CONTENT));
							}
							mConfLayout.addView(envLayout, new LinearLayout.LayoutParams( LinearLayout.LayoutParams.FILL_PARENT, LinearLayout.LayoutParams.WRAP_CONTENT));
						}*/
					}
					mConfView.addView(mConfLayout);
				}
				addView(mConfView, new LinearLayout.LayoutParams( LinearLayout.LayoutParams.FILL_PARENT, 0, 1) );
				
				View divider = new View(mActivity);
				divider.setBackgroundColor(Color.GRAY);
				addView(divider, new LinearLayout.LayoutParams(LinearLayout.LayoutParams.FILL_PARENT, 2) );
				
				LinearLayout runLayout = new LinearLayout(mActivity);
				runLayout.setOrientation(LinearLayout.HORIZONTAL);
				runLayout.setBackgroundColor(0xf5f5f5f5);
				//runLayout.setBackgroundDrawable(getResources().getDrawable(R.drawable.btn_light_nm));
				
				//don't ask me again
				LinearLayout askLayout = new LinearLayout(mActivity);
				{
					CheckBox chk = new CheckBox(mActivity);
					chk.setChecked(!Locals.AppLaunchConfigUse);
					chk.setOnClickListener(new OnClickListener(){
						public void onClick(View v){
							CheckBox c = (CheckBox)v;
							Locals.AppLaunchConfigUse = !c.isChecked();
							Settings.SaveLocals(mActivity);
						}
					});
					askLayout.addView(chk, new LinearLayout.LayoutParams(LinearLayout.LayoutParams.WRAP_CONTENT, LinearLayout.LayoutParams.WRAP_CONTENT));
					
					LinearLayout txtLayout = new LinearLayout(mActivity);
					txtLayout.setOrientation(LinearLayout.VERTICAL);
					{
						TextView txt1 = new TextView(mActivity);
						txt1.setTextSize(16.0f);
						txt1.setText(getString(R.string.Conf_DontAskMeAgain));
						txt1.setTextColor(Color.BLACK);
						txt1.setShadowLayer (5f, 2, 2f, 0x00000000);
						txtLayout.addView(txt1, new LinearLayout.LayoutParams( LinearLayout.LayoutParams.FILL_PARENT, LinearLayout.LayoutParams.WRAP_CONTENT));
						
						TextView txt2 = new TextView(mActivity);
						txt2.setPadding(5, 0, 0, 0);
						txt2.setText(getString(R.string.Conf_NotUseAppLaunchConfig));
						txt2.setTextColor(Color.BLACK);
						txt2.setShadowLayer (5f, 2, 2f, 0x00000000);
						txtLayout.addView(txt2, new LinearLayout.LayoutParams( LinearLayout.LayoutParams.FILL_PARENT, LinearLayout.LayoutParams.WRAP_CONTENT));
					}
					askLayout.addView(txtLayout, new LinearLayout.LayoutParams( LinearLayout.LayoutParams.WRAP_CONTENT, LinearLayout.LayoutParams.WRAP_CONTENT, 1));
				}
				runLayout.addView(askLayout, new LinearLayout.LayoutParams( LinearLayout.LayoutParams.WRAP_CONTENT, LinearLayout.LayoutParams.WRAP_CONTENT, 1));
				
				mRunButton = new Button(mActivity);
				mRunButton.setText(getString(R.string.Conf_Run));
				mRunButton.setTextSize(24.0f);
				mRunButton.setOnClickListener(new OnClickListener(){
					public void onClick(View v){
						WriteSetting(myname);
						ReadSetting(myname);
						if(!Locals.gWindowScreen)
							runApp();
						else{
							runWD();
						}
						
					}
				});
				runLayout.addView(mRunButton, new LinearLayout.LayoutParams( LinearLayout.LayoutParams.WRAP_CONTENT, LinearLayout.LayoutParams.WRAP_CONTENT) );

				mRunButton2 = new Button(mActivity);
				mRunButton2.setText("创建快捷方式");
				mRunButton2.setTextSize(24.0f);
				mRunButton2.setOnClickListener(new OnClickListener(){
					public void onClick(View v){
						addShortcut(myname, Globals.CurrentDirectoryPath );
					}
				});
				runLayout.addView(mRunButton2, new LinearLayout.LayoutParams( LinearLayout.LayoutParams.WRAP_CONTENT, LinearLayout.LayoutParams.WRAP_CONTENT) );

				mRunButton3 = new Button(mActivity);
				mRunButton3.setText("取消");
				mRunButton3.setTextSize(24.0f);
				mRunButton3.setOnClickListener(new OnClickListener(){
					public void onClick(View v){
						runAppLauncher();
						DialogOpen = false;
					}
				});
				runLayout.addView(mRunButton3, new LinearLayout.LayoutParams( LinearLayout.LayoutParams.WRAP_CONTENT, LinearLayout.LayoutParams.WRAP_CONTENT) );
				
				addView(runLayout, new LinearLayout.LayoutParams( LinearLayout.LayoutParams.FILL_PARENT, LinearLayout.LayoutParams.WRAP_CONTENT) );
			}
		}
	}

	public void WriteSetting(String Gamename) {
		Editor e = getSharedPreferences(Gamename, MODE_PRIVATE).edit();
		
		if (checkSP.isChecked()) {
			set_checkSP = true;
			e.putBoolean("checkSP", set_checkSP);
		} else {
			set_checkSP = false;
			e.putBoolean("checkSP", set_checkSP);
		}
		if (FullScreen.isChecked()) {
			set_FullScreen = true;
			e.putBoolean("FullScreen", set_FullScreen);
		} else {
			set_FullScreen = false;
			e.putBoolean("FullScreen", set_FullScreen);
		}
		if (OtherPL.isChecked()) {
			set_OtherPL = true;
			e.putBoolean("OtherPL", set_OtherPL);
		} else {
			set_OtherPL = false;
			e.putBoolean("OtherPL", set_OtherPL);
		}
		
		if (FontSize.isChecked()) {
			set_FontSize = true;
			e.putBoolean("FontSize", set_FontSize);
		} else {
			set_FontSize = false;
			e.putBoolean("FontSize", set_FontSize);
		}

		if (FontColor.isChecked()) {
			set_FontColor = true;
			e.putBoolean("FontColor", set_FontColor);
		} else {
			set_FontColor = false;
			e.putBoolean("FontColor", set_FontColor);
		}
		
		if (keepON.isChecked()) {
			set_keepON = true;
			e.putBoolean("keepON", set_keepON);
		} else {
			set_keepON = false;
			e.putBoolean("keepON", set_keepON);
		}
		e.commit();
	}

	public void ReadSetting(String setting) {
		SharedPreferences spset = getSharedPreferences(setting, MODE_PRIVATE);// 读取设置
		
		set_checkSP = spset.getBoolean("checkSP", set_checkSP);
		if (set_checkSP) {
			checkSP.setChecked(true);
		}
		set_FullScreen = spset.getBoolean("FullScreen", set_FullScreen);
		if (set_FullScreen)
			FullScreen.setChecked(true);
		set_OtherPL = spset.getBoolean("OtherPL", set_OtherPL);
		if (set_OtherPL) {
			OtherPL.setChecked(true);
		}
		set_FullScreen = spset.getBoolean("FullScreen", set_FullScreen);
		if (set_FullScreen) {
			FullScreen.setChecked(true);
		}
		set_FontSize = spset.getBoolean("FontSize", set_FontSize);
		if (set_FontSize) {
			FontSize.setChecked(true);
		}
		set_FontColor = spset.getBoolean("FontColor", set_FontColor);
		if (set_FontColor) {
			FontColor.setChecked(true);
		}
		set_keepON = spset.getBoolean("keepON", set_keepON);
		if (set_keepON) {
			keepON.setChecked(true);
		}

		myfontsize = spset.getInt("myfontsize", 0);

		myfont_color1 = spset.getInt("fontr", 255);
		myfont_color2 = spset.getInt("fontg", 255);
		myfont_color3 = spset.getInt("fontb", 255);
		cbColor = spset.getInt("cbcolor", Color.BLACK);
		FontColor.setTextColor(cbColor);

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
	
		set_FullScreen = spset.getBoolean("FullScreen", set_FullScreen);
		if (set_FullScreen)
			Locals.gFullScreen = true;
		set_ScaleFullScreen = spset.getBoolean("ScaleFullScreen", set_ScaleFullScreen);

		set_checkSP = spset.getBoolean("checkSP", set_checkSP);
		if (set_checkSP)
			Locals.gDisableVideo = true;
		set_OtherPL = spset.getBoolean("OtherPL", set_OtherPL);
		if (set_OtherPL)
			Locals.gOtherPL = true;
		set_FontSize = spset.getBoolean("FontSize", set_FontSize);
		if (set_FontSize){
			Locals.gFontSize = true;
		myfontsize = spset.getInt("myfontsize", 0);
		}
		set_FontColor= spset.getBoolean("FontColor", set_FontColor);
		if (set_FontColor){
			Locals.gFontColor = true;
		myfont_color1 = spset.getInt("fontr", 255);
		myfont_color2 = spset.getInt("fontg", 255);
		myfont_color3 = spset.getInt("fontb", 255);
		}
		set_keepON = spset.getBoolean("keepON", set_keepON);
		if (set_keepON) {
			Locals.gKeepON = true;
			if (Locals.gKeepON)
				getWindow().addFlags(
						WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
		}
	}
	
	public void runAppLaunchConfig()
	{
		if(!checkCurrentDirectory(true)){
			return;
		}
		
		Settings.LoadLocals(this);
		
		if(!Locals.AppLaunchConfigUse){
			runApp();
			return;
		}
		
		AppLaunchConfigView view = new AppLaunchConfigView(this);
		setContentView(view);
		DialogOpen = true;
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
	
	public void runApp()
	{
		nativeInitJavaCallbacks();
		if(Locals.gFontSize)
		nativeFontSize(myfontsize,myfontsize,myfontsize+2,myfontsize+2);
		if(Locals.gFontColor)
		nativeFontColor(Locals.gFontColor,myfont_color1,myfont_color2,myfont_color3);

		if(Locals.Logout)
		debug.Logcat_out(Globals.CurrentDirectoryPath);

		if(!checkCurrentDirectory(true)){
			return;
		}
		
		Settings.LoadLocals(this);
		
		if(!checkAppNeedFiles()){
			return;
		}
		
		if(mView == null){
			mView = new MainView(this);
			if(Locals.gFullScreen){
			Fulllayout =new LinearLayout(this);
			FullSV = new ScrollView(this);
			Fulllayout.addView(mView);
			FullSV.addView(Fulllayout);
			setContentView(FullSV);
			}
			
			else
			setContentView(mView);
		}
		mView.setFocusableInTouchMode(true);
		mView.setFocusable(true);
		mView.requestFocus();

		lp  = getWindow().getAttributes();
		
		if(extra == null)
		FreeMemory();
		System.gc();
	}


	private void runWD() {
		Rect frame2 = new Rect();
		getWindow().getDecorView().getWindowVisibleDisplayFrame(frame2);
		nativeInitJavaCallbacks();

		if(Locals.gFontSize)
		nativeFontSize(myfontsize,myfontsize,myfontsize+2,myfontsize+2);
		if(Locals.gFontColor)
		nativeFontColor(Locals.gFontColor,myfont_color1,myfont_color2,myfont_color3);

		if(Locals.Logout)
		debug.Logcat_out(Globals.CurrentDirectoryPath);

		isRunning = true;
		lp  = getWindow().getAttributes();  

		if(!checkCurrentDirectory(true)){
			return;
		}
		
		Settings.LoadLocals(this);
		
		if(!checkAppNeedFiles()){
			return;
		}
		
		if(mView == null){
			mView = new MainView(this);
			//setContentView(mView);
		}
		mView.setFocusableInTouchMode(true);
		mView.setFocusable(true);
		mView.requestFocus();

		lp  = getWindow().getAttributes();
		
		FreeMemory();
		System.gc();

		Intent i = new Intent(Intent.ACTION_MAIN);
		i.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
		i.addCategory(Intent.CATEGORY_HOME);
		startActivity(i);

		wdlayout = new LinearLayout(this);
		wdlayout.setOrientation(LinearLayout.VERTICAL);
		layout1 = new LinearLayout(this);
		layout1.setOrientation(LinearLayout.HORIZONTAL);
		HSV = new HorizontalScrollView(this);

	  final Button Movebtn = new Button(this);
		Movebtn.setBackgroundColor(Color.argb(100, 10, 10, 10));
		Movebtn.setTextColor(Color.WHITE);
		Movebtn.setText("移动");
		Movebtn.setOnClickListener(new OnClickListener() {
			public void onClick(View v) {
				if(Locals.ScreenMove){
					mView.ShowToast("位置已锁定");
					Locals.ScreenMove = false;
				}
				else{
					Locals.ScreenMove = true;
					mView.ShowToast("现在可移动画面，再次点击锁定位置");
				}
			}
		}); 
		layout1.addView(Movebtn);

	  final Button Movebtn2 = new Button(this);
		Movebtn2.setBackgroundColor(Color.argb(100, 10, 10, 10));
		Movebtn2.setTextColor(Color.WHITE);
		Movebtn2.setText("按键");
		Movebtn2.setOnClickListener(new OnClickListener() {
			public void onClick(View v) {
				BtnCancel();
			}
		}); 
		layout1.addView(Movebtn2);

	  	Button Movebtn3 = new Button(this);
		Movebtn3.setBackgroundColor(Color.argb(100, 10, 10, 10));
		Movebtn3.setTextColor(Color.WHITE);
		Movebtn3.setText("隐藏");
		Movebtn3.setOnClickListener(new OnClickListener() {
			public void onClick(View v) {
			  
						if(Locals.ScreenHide){
							Locals.ScreenMove = false;
							Locals.ScreenHide = false;
							mView.ScreenHide();
							mView.setVisibility(View.VISIBLE);
							Movebtn.setVisibility(View.VISIBLE);
							Movebtn2.setVisibility(View.VISIBLE);
							//if(isRunning )
							//CurCancel();
							if (mView != null){
							mView.onResume();
			
							}
						}
						else{
							Locals.ScreenMove = true;
							Locals.ScreenHide = true;
							mView.ScreenHide();
							mView.setVisibility(View.GONE);
							Movebtn.setVisibility(View.GONE);
							Movebtn2.setVisibility(View.GONE);
							//if( isRunning )
							//CurCancel();
							if (mView != null)
							mView.onPause();
						
						}
					
		    }
		}); 

		Movebtn3.setOnLongClickListener(new OnLongClickListener() {
			public boolean onLongClick(View v) {
				if(!Locals.HideClick){
				Locals.HideClick = true;
				}
				return true;
			}
		});
		
		Movebtn3.setOnTouchListener(new OnTouchListener() {

			public boolean onTouch(View v, MotionEvent event) {
				// TODO Auto-generated method stub
				x = event.getRawX();
				y = event.getRawY();
				switch (event.getAction()) {

				case MotionEvent.ACTION_DOWN:
					startX = event.getX();  
			 		startY = event.getY(); 
					break;
				case MotionEvent.ACTION_MOVE:
					if (Math.abs(startX - x) > TOUCH_SLOP
							|| Math.abs(startY - y) > TOUCH_SLOP) {
						// 移动超过阈值，则表示移动了
						isMoved = true;
						
					}
					if (isMoved == true) {
						if(Locals.HideClick){
						 ONScripter.wdupdatePosition(x,y,startX,startY);
						
						isMoved = false;
						}
					}
					break;
				case MotionEvent.ACTION_UP:
					if(Locals.HideClick){
					ONScripter.wdupdatePosition(x,y,startX,startY);
			 		startX = startY = 0; 
					
					Locals.HideClick = false;
					}
					break;
				}
				return false;
			}
		});		

		layout1.addView(Movebtn3);
		HSV.addView(layout1);
		

		wdlayout.addView(mView, new LinearLayout.LayoutParams(wdw,
				wdh));
		wdlayout.addView(HSV);

		wdwm = (WindowManager)getApplicationContext().getSystemService("window");

		wdparams.type = 2002;
		wdparams.flags =WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE
		|296;

		wdparams.width = WindowManager.LayoutParams.WRAP_CONTENT;
		wdparams.height = WindowManager.LayoutParams.WRAP_CONTENT;
		wdparams.alpha =1;
		wdparams.format=1;
		
		wdparams.gravity = Gravity.LEFT | Gravity.TOP;
		
		wdparams.x = 0;
		wdparams.y = 50;
		wdwm.addView(wdlayout, wdparams);

	}

	public static void wdupdatePosition(float a,float b,float c,float d) {
		// View的当前位�?		wdparams.x = (int)( a - c);  
     		wdparams.y = (int) (b - d);  
		wdwm.updateViewLayout(wdlayout, wdparams);
	}


	public void WDDialog() {

		final LinearLayout wd_layout = new LinearLayout(this);
		wd_layout.setOrientation(LinearLayout.VERTICAL);		
		
		if (wd_popupWindow != null && wd_popupWindow.isShowing()) {
					wd_popupWindow.dismiss();
				} else {
					
					wd_popupWindow = new PopupWindow(wd_layout, LayoutParams.WRAP_CONTENT, LayoutParams.WRAP_CONTENT,true);
					wd_popupWindow.setTouchable(true);
					wd_popupWindow.setOutsideTouchable(true);
					wd_popupWindow.setBackgroundDrawable(getResources().getDrawable(R.drawable.pub_pop_bg2));
					wd_popupWindow.showAtLocation(checkWD, Gravity.CENTER, 0, 0); 
					
				}

		SharedPreferences sp = getSharedPreferences("pref", MODE_PRIVATE);
		wdw = sp.getInt("wdw", 640);
		wdh = sp.getInt("wdh", 480);
		
		final TextView wdText=new TextView(this);
		wdText.setTextSize(21);
		wdText.setText(" 输入窗口大小:Width,Height ");
		wdText.setGravity(Gravity.CENTER_VERTICAL);
		wdText.setBackgroundColor(Color.argb(0, 0, 0, 0));
		wdText.setTextColor(Color.BLACK);
		wd_layout.addView(wdText);

		final EditText Width =new EditText(this);
		Width.setText(String.valueOf(wdw));
		Width.setHint("只能输入数字");
		Width.setInputType(InputType.TYPE_CLASS_NUMBER);
		Width.setSelection(Width.length());
		Width.setTextColor(Color.BLACK);
		Width.setGravity(Gravity.CENTER_VERTICAL);
		Width.setBackgroundColor(Color.argb(0, 0, 0, 0));
		wd_layout.addView(Width);

		final EditText Height =new EditText(this);
		Height.setText(String.valueOf(wdh));
		Height.setHint("只能输入数字");
		Height.setInputType(InputType.TYPE_CLASS_NUMBER);
		Height.setSelection(Height.length());
		Height.setTextColor(Color.BLACK);
		Height.setGravity(Gravity.CENTER_VERTICAL);
		Height.setBackgroundColor(Color.argb(0, 0, 0, 0));
		wd_layout.addView(Height);
		
		final Button wh=new Button(this);
		wh.setBackgroundColor(Color.argb(0, 0, 0, 0));
		wh.setTextSize(21);
		wh.setText("确定");
		wh.setTextColor(Color.BLACK);
		wh.setOnClickListener(new OnClickListener() {
			public void onClick(View v) {
				
				wdw=Integer.parseInt(Width.getText().toString());
				wdh=Integer.parseInt(Height.getText().toString());
				
				Editor e = getSharedPreferences("pref", MODE_PRIVATE).edit();
		
				e.putInt("wdw", wdw);
				e.putInt("wdh", wdh);
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
					font_popupWindow.setTouchable(true);
					font_popupWindow.setOutsideTouchable(true);
					font_popupWindow.setBackgroundDrawable(getResources().getDrawable(R.drawable.pub_pop_bg2));
					font_popupWindow.showAtLocation(FontSize, Gravity.CENTER, 0, 0); 
					
				}

		final TextView Text_font=new TextView(this);
		Text_font.setTextSize(21);
		Text_font.setText("           选择字体大小");
		Text_font.setGravity(Gravity.CENTER_VERTICAL);
		Text_font.setBackgroundColor(Color.argb(0, 0, 0, 0));
		Text_font.setTextColor(Color.BLACK);
		font_layout.addView(Text_font);

		final TextView Text_font2=new TextView(this);
		Text_font2.setTextSize(myfontsize);
		Text_font2.setText(" 测试文字 ");
		Text_font2.setGravity(Gravity.CENTER_VERTICAL);
		Text_font2.setBackgroundColor(Color.argb(0, 0, 0, 0));
		Text_font2.setTextColor(Color.BLACK);
		font_layout.addView(Text_font2);

		final SeekBar TextSeekBar=new SeekBar(this);
		TextSeekBar.setMax(100);
		TextSeekBar.setProgress(myfontsize);
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
		
		final Button font_btn=new Button(this);
		font_btn.setBackgroundColor(Color.argb(0, 0, 0, 0));
		font_btn.setTextSize(21);
		font_btn.setGravity(Gravity.CENTER_HORIZONTAL);
		font_btn.setText("     确定      ");
		font_btn.setTextColor(Color.BLACK);
		font_btn.setOnClickListener(new OnClickListener() {
			public void onClick(View v) {

				myfontsize=TextSeekBar.getProgress();

				Editor e = getSharedPreferences(myname, MODE_PRIVATE).edit();
		
				e.putInt("myfontsize", myfontsize);
				e.commit();
				font_popupWindow.dismiss();
			}
		});
		font_layout2.addView(font_btn);

		final Button font_btn2=new Button(this);
		font_btn2.setBackgroundColor(Color.argb(0, 0, 0, 0));
		font_btn2.setTextSize(21);
		font_btn2.setGravity(Gravity.CENTER_HORIZONTAL);
		font_btn2.setText("      默认    ");
		font_btn2.setTextColor(Color.BLACK);
		font_btn2.setOnClickListener(new OnClickListener() {
			public void onClick(View v) {

				myfontsize=0;

				Editor e = getSharedPreferences(myname, MODE_PRIVATE).edit();
		
				e.putInt("myfontsize", myfontsize);

				e.commit();
				font_popupWindow.dismiss();
			}
		});
		font_layout2.addView(font_btn2);

		font_layout.addView(font_layout2);

	}


	public void LightDialog() {

		final LinearLayout light_layout = new LinearLayout(this);
		light_layout.setOrientation(LinearLayout.VERTICAL);	

		Display disp = ((WindowManager) this
				.getSystemService(Context.WINDOW_SERVICE)).getDefaultDisplay();
		int dw = disp.getWidth();	
		
		if (light_popupWindow != null && light_popupWindow.isShowing()) {
					light_popupWindow.dismiss();
				} else {
					
					light_popupWindow = new PopupWindow(light_layout, LayoutParams.WRAP_CONTENT, LayoutParams.WRAP_CONTENT,true);
					light_popupWindow.setTouchable(true);
					light_popupWindow.setOutsideTouchable(true);
					light_popupWindow.setBackgroundDrawable(getResources().getDrawable(R.drawable.pub_pop_bg2));
					light_popupWindow.showAtLocation(mView, Gravity.CENTER, 0, 0); 
					
				}

		
		
		final TextView lightText=new TextView(this);
		lightText.setTextSize(21);
		lightText.setGravity(Gravity.CENTER_VERTICAL);
		lightText.setBackgroundColor(Color.argb(0, 0, 0, 0));
		lightText.setTextColor(Color.BLACK);
		light_layout.addView(lightText);
		
		final SeekBar lightBar=new SeekBar(this);
		lightBar.setMax(100);
		lightBar.setProgress(1);
		lightBar.setSecondaryProgress(0);
		lightBar.setProgress((int)light);
		lightBar.setBackgroundColor(0x0099ff00);
		lightBar.setLayoutParams(new LinearLayout.LayoutParams(dw/3, LinearLayout.LayoutParams.WRAP_CONTENT));
		lightBar.setOnSeekBarChangeListener(new OnSeekBarChangeListener() {

			public void onStopTrackingTouch(SeekBar seekBar) {
				// TODO Auto-generated method stub

			}

			public void onStartTrackingTouch(SeekBar seekBar) {
				// TODO Auto-generated method stub

			}

			public void onProgressChanged(SeekBar seekBar, int progress,
					boolean fromUser) {
				// TODO Auto-generated method stub
				light=lightBar.getProgress();
				if(light>1)
				{
				float a1=light/100;
				if(a1>0.05){
				lp.screenBrightness = a1;   
		        	getWindow().setAttributes(lp);
				}
				}
				
				lightText.setText(String.valueOf(light));
			}
		});
		light_layout.addView(lightBar);

	}

	public void VirtualButton() {
		Rect frame = new Rect();
		getWindow().getDecorView().getWindowVisibleDisplayFrame(frame);
		Btnlayout = new LinearLayout(this);
		Btnlayout.setOrientation(LinearLayout.VERTICAL);
		Btnlayout.setBackgroundResource(R.drawable.popupwindow);
		LinearLayout SVlayout = new LinearLayout(this);
		LinearLayout SVlayout2 = new LinearLayout(this);
		SVlayout.setBackgroundResource(R.drawable.popupwindow);
		SVlayout2.setBackgroundResource(R.drawable.popupwindow);
		HorizontalScrollView BtnHSV = new HorizontalScrollView(this);
		HorizontalScrollView BtnHSV2 = new HorizontalScrollView(this);
		BtnHSV.setBackgroundResource(R.drawable.popupwindow);
		BtnHSV2.setBackgroundResource(R.drawable.popupwindow);
		wm = (WindowManager) getApplicationContext().getSystemService("window");

	  	popbtn = new ImageButton(this);
		popbtn.setImageResource(R.drawable.icon_pop);
		popbtn.setBackgroundColor(Color.argb(0, 0, 0, 0));
		popbtn.setOnClickListener(new OnClickListener() {
			public void onClick(View v) {
				if (button_popupWindow != null && button_popupWindow.isShowing()) {
					button_popupWindow.dismiss();
				} else {
					
					button_popupWindow = new PopupWindow(Btnlayout, LayoutParams.WRAP_CONTENT, LayoutParams.WRAP_CONTENT,true);
					button_popupWindow.setTouchable(true);
					button_popupWindow.setOutsideTouchable(true);
					button_popupWindow.setBackgroundDrawable(new BitmapDrawable());
					button_popupWindow.showAsDropDown(popbtn,5,5);
					
				}

			}
		});

		popbtn.setOnTouchListener(new OnTouchListener() {

			public boolean onTouch(View v, MotionEvent event) {
				// TODO Auto-generated method stub
				x = event.getRawX();
				y = event.getRawY();
				switch (event.getAction()) {

				case MotionEvent.ACTION_DOWN:
					startX = x;
					startY = y;
					break;
				case MotionEvent.ACTION_MOVE:
					if (Math.abs(startX - x) > TOUCH_SLOP
							|| Math.abs(startY - y) > TOUCH_SLOP) {
						// 移动超过阈值，则表示移动了
						isMoved = true;
					}
					if (isMoved == true) {
						updatePosition();
						
						isMoved = false;
					}
					break;
				case MotionEvent.ACTION_UP:
					 startX = event.getX();
					 startY = event.getY();
					break;
				}
				return false;
			}
		});

		

		Button btnup = new Button(this);
		btnup.setBackgroundColor(Color.argb(10, 10, 10, 10));
		btnup.setTextSize(textsize);
		btnup.setTextColor(Color.WHITE);
		btnup.setText(getResources().getString(R.string.Key_PrevChoice));
		btnup.setOnClickListener(new OnClickListener() {
			public void onClick(View v) {
				mView.nativeKey(KeyEvent.KEYCODE_DPAD_UP, 1);
				mView.nativeKey(KeyEvent.KEYCODE_DPAD_UP, 0);
			}
		});

		Button btndown = new Button(this);
		btndown.setBackgroundColor(Color.argb(10, 10, 10, 10));
		btndown.setTextSize(textsize);
		btndown.setTextColor(Color.WHITE);
		btndown.setText(getResources().getString(R.string.Key_NextChoice));
		btndown.setOnClickListener(new OnClickListener() {
			public void onClick(View v) {
				mView.nativeKey(KeyEvent.KEYCODE_DPAD_DOWN, 1);
				mView.nativeKey(KeyEvent.KEYCODE_DPAD_DOWN, 0);
			}
		});

		Button btnleft = new Button(this);//左键
		btnleft.setBackgroundColor(Color.argb(10, 10, 10, 10));
		btnleft.setTextSize(textsize);
		btnleft.setTextColor(Color.WHITE);
		btnleft.setText(getResources().getString(R.string.Key_LeftClick));
		btnleft.setOnClickListener(new OnClickListener() {
			public void onClick(View v) {
				mView.nativeKey(KeyEvent.KEYCODE_ENTER, 1);
				mView.nativeKey(KeyEvent.KEYCODE_ENTER, 0);
			}
		});

		Button btnright = new Button(this);
		btnright.setBackgroundColor(Color.argb(10, 10, 10, 10));
		btnright.setTextSize(textsize);
		btnright.setTextColor(Color.WHITE);
		btnright.setText(getResources().getString(R.string.Key_RightClick));
		btnright.setOnClickListener(new OnClickListener() {
			public void onClick(View v) {
				mView.nativeKey(KeyEvent.KEYCODE_BACK, 1);
				mView.nativeKey(KeyEvent.KEYCODE_BACK, 0);
			}
		});

		Button btnleft2 = new Button(this); //�?		btnleft2.setBackgroundColor(Color.argb(10, 10, 10, 10));
		btnleft2.setTextSize(textsize);
		btnleft2.setTextColor(Color.WHITE);
		btnleft2.setText(getResources().getString(R.string.Key_WheelUp));
		btnleft2.setOnClickListener(new OnClickListener() {
			public void onClick(View v) {
				mView.nativeKey(KeyEvent.KEYCODE_DPAD_LEFT, 1);
				mView.nativeKey(KeyEvent.KEYCODE_DPAD_LEFT, 0);
			}
		});

		Button btnright2 = new Button(this);
		btnright2.setBackgroundColor(Color.argb(10, 10, 10, 10));
		btnright2.setTextSize(textsize);
		btnright2.setTextColor(Color.WHITE);
		btnright2.setText(getResources().getString(R.string.Key_WheelDown));
		btnright2.setOnClickListener(new OnClickListener() {
			public void onClick(View v) {
				mView.nativeKey(KeyEvent.KEYCODE_DPAD_RIGHT, 1);
				mView.nativeKey(KeyEvent.KEYCODE_DPAD_RIGHT, 0);
			}
		});
		
		Button btnvolup = new Button(this);
		btnvolup.setBackgroundColor(Color.argb(10, 10, 10, 10));
		btnvolup.setTextSize(textsize);
		btnvolup.setTextColor(Color.WHITE);
		btnvolup.setText(getResources().getString(R.string.button_vol_up));
		btnvolup.setOnClickListener(new OnClickListener() {
			public void onClick(View v) {
				mView.nativeKey(KeyEvent.KEYCODE_M, 1);
				mView.nativeKey(KeyEvent.KEYCODE_M, 0);
			}
		});
		btnvolup.setOnLongClickListener(new OnLongClickListener() {
			public boolean onLongClick(View v) {
				mView.nativeKey(KeyEvent.KEYCODE_D, 1);
				mView.nativeKey(KeyEvent.KEYCODE_D, 0);
				return true;
			}
		});


		Button btnvoldown = new Button(this);
		btnvoldown.setBackgroundColor(Color.argb(10, 10, 10, 10));
		btnvoldown.setTextSize(textsize);
		btnvoldown.setTextColor(Color.WHITE);
		btnvoldown.setText(getResources().getString(R.string.button_vol_down));
		btnvoldown.setOnClickListener(new OnClickListener() {
			public void onClick(View v) {
				mView.nativeKey(KeyEvent.KEYCODE_I, 1);
				mView.nativeKey(KeyEvent.KEYCODE_I, 0);
			}
		});
		btnvoldown.setOnLongClickListener(new OnLongClickListener() {
			public boolean onLongClick(View v) {
				mView.nativeKey(KeyEvent.KEYCODE_W, 1);
				mView.nativeKey(KeyEvent.KEYCODE_W, 0);
				return true;
			}
		});

		Button btngetfshot = new Button(this);
		btngetfshot.setText(getResources().getString(R.string.button_Menu));
		btngetfshot.setBackgroundColor(Color.argb(10, 10, 10, 10));
		btngetfshot.setTextSize(textsize);
		btngetfshot.setTextColor(Color.WHITE);
		btngetfshot.setOnClickListener(new OnClickListener() {
			public void onClick(View v) {
				openOptionsMenu();
			}
		});

		Button btngetshot = new Button(this);
		btngetshot.setText(getResources().getString(R.string.button_get_default_screenshot));
		btngetshot.setBackgroundColor(Color.argb(10, 10, 10, 10));
		btngetshot.setTextSize(textsize);
		btngetshot.setTextColor(Color.WHITE);
		btngetshot.setOnClickListener(new OnClickListener() {
			public void onClick(View v) {
				mView.nativeKey(KeyEvent.KEYCODE_U, 1);
				mView.nativeKey(KeyEvent.KEYCODE_U, 0);
				mView.ShowToast("截图成功，保存在"+Globals.CurrentDirectoryPath);
				
			}
		});


		params.type = 2003;
		params.flags = WindowManager.LayoutParams.TYPE_APPLICATION_PANEL;

		params.width = WindowManager.LayoutParams.WRAP_CONTENT;
		params.height = WindowManager.LayoutParams.WRAP_CONTENT;
		params.alpha = 80;
		params.format=1;

		params.gravity = Gravity.LEFT | Gravity.TOP;

		params.x = 0;
		params.y = 0;

		SVlayout.addView(btnup);
		SVlayout.addView(btndown);
		SVlayout.addView(btnleft);
		SVlayout.addView(btnright);
		SVlayout.addView(btngetfshot);

		SVlayout2.addView(btnleft2);
		SVlayout2.addView(btnright2);
		SVlayout2.addView(btnvolup);
		SVlayout2.addView(btnvoldown);
		SVlayout2.addView(btngetshot);

		BtnHSV.addView(SVlayout);
		BtnHSV2.addView(SVlayout2);
		Btnlayout.addView(BtnHSV);
		Btnlayout.addView(BtnHSV2);
		wm.addView(popbtn, params);
	}

	public void BtnCancel() {
		if (popbtn != null && popbtn.isShown()) {
			wm = (WindowManager) getApplicationContext().getSystemService(
					"window");
			params = new WindowManager.LayoutParams();
			wm.removeView(popbtn);
		} else{
			VirtualButton();
			mView.ShowToast("虚拟按键可任意拖动");
		}
	}

	private void updatePosition() {
		// View的当前位置?		params.x = (int) (x - 30);
		params.y = (int) (y - 60);
		wm.updateViewLayout(popbtn, params);
	}

	public void TextDialog() {

		final LinearLayout size_layout = new LinearLayout(this);
		size_layout.setOrientation(LinearLayout.VERTICAL);		
		
		if (size_popupWindow != null && size_popupWindow.isShowing()) {
					size_popupWindow.dismiss();
				} else {
					
					size_popupWindow = new PopupWindow(size_layout, LayoutParams.WRAP_CONTENT, LayoutParams.WRAP_CONTENT,true);
					size_popupWindow.setTouchable(true);
					size_popupWindow.setOutsideTouchable(true);
					size_popupWindow.setBackgroundDrawable(getResources().getDrawable(R.drawable.pub_pop_bg2));
					size_popupWindow.showAtLocation(ONSSetting, Gravity.CENTER, 0, 0); 
					
				}


		final TextView Text_size=new TextView(this);
		Text_size.setTextSize(21);
		Text_size.setText(" 输入虚拟按键字体大小 ");
		Text_size.setGravity(Gravity.CENTER_VERTICAL);
		Text_size.setBackgroundColor(Color.argb(0, 0, 0, 0));
		Text_size.setTextColor(Color.BLACK);
		size_layout.addView(Text_size);

		final EditText editSize =new EditText(this);
		editSize.setText(String.valueOf(textsize));
		editSize.setHint("只能输入数字");
		editSize.setInputType(InputType.TYPE_CLASS_NUMBER);
		editSize.setSelection(editSize.length());
		editSize.setTextColor(Color.BLACK);
		editSize.setGravity(Gravity.CENTER_VERTICAL);
		editSize.setBackgroundColor(Color.argb(0, 0, 0, 0));
		size_layout.addView(editSize);
		
		final Button size_btn=new Button(this);
		size_btn.setBackgroundColor(Color.argb(0, 0, 0, 0));
		size_btn.setTextSize(21);
		size_btn.setText("确定");
		size_btn.setTextColor(Color.BLACK);
		size_btn.setOnClickListener(new OnClickListener() {
			public void onClick(View v) {
				
				textsize=Integer.parseInt(editSize.getText().toString());
				
				Editor e = getSharedPreferences("pref", MODE_PRIVATE).edit();
		
				e.putInt("textsize", textsize);
				e.commit();
				size_popupWindow.dismiss();
			}
		});
		size_layout.addView(size_btn);

	}

	public void ColorDialog()
	{
		Colordialog = new ColorPickerDialog(this, FontColor.getTextColors().getDefaultColor(), 
						"FontColor", 
						new ColorPickerDialog.OnColorChangedListener() {
					
					public void colorChanged(int color) {
						FontColor.setTextColor(color);
						myfont_color1 = Colordialog.getR();
						myfont_color2 = Colordialog.getG();
						myfont_color3 = Colordialog.getB();

						Editor e = getSharedPreferences(myname, MODE_PRIVATE).edit();
		
						e.putInt("fontr", myfont_color1);
						e.putInt("fontg", myfont_color2);
						e.putInt("fontb", myfont_color3);
						e.putInt("cbcolor", color);
						e.commit();
					}
				});
		Colordialog.show();
	}


	public void addShortcut(String name, String path) {
		Intent shortcut = new Intent(
				"com.android.launcher.action.INSTALL_SHORTCUT");
		shortcut.putExtra(Intent.EXTRA_SHORTCUT_NAME, name);
		shortcut.putExtra("duplicate", false);
		ComponentName comp = new ComponentName(this.getPackageName(), "."
				+ this.getLocalClassName());
		shortcut.putExtra(Intent.EXTRA_SHORTCUT_INTENT, new Intent(
				Intent.ACTION_MAIN).setComponent(comp));
		Intent shortcutIntent = new Intent(Intent.ACTION_MAIN);
		shortcutIntent.setClass(this, start.class);
		shortcutIntent.putExtra(SHORT_CUT_EXTRAS, path);
		shortcutIntent.putExtra("setting", name);
		shortcut.putExtra(Intent.EXTRA_SHORTCUT_INTENT, shortcutIntent);

		String iconPath = path + "/ICON.PNG";
		Drawable d = Drawable.createFromPath(iconPath);
		if (d != null) {
			shortcut.putExtra(Intent.EXTRA_SHORTCUT_ICON,
					generatorContactCountIcon(((BitmapDrawable) (getResources()
							.getDrawable(R.drawable.icon))).getBitmap(), d));
		} else {
			ShortcutIconResource iconRes = Intent.ShortcutIconResource
					.fromContext(this, R.drawable.icon);
			shortcut.putExtra(Intent.EXTRA_SHORTCUT_ICON_RESOURCE, iconRes);
		}
		sendBroadcast(shortcut);
	}

	public Bitmap generatorContactCountIcon(Bitmap icon, Drawable d) {

		Display disp = ((WindowManager) this
				.getSystemService(Context.WINDOW_SERVICE)).getDefaultDisplay();
		int dh = disp.getHeight();
		int iconSize=32;

		// 初始化画�?		
		if (dh <= 240)
			iconSize = 48;
		else if (dh < 500 || dh >500){
			if(dh == 640)
			iconSize = 96;
			else
			iconSize = 72;

		}
		
		Bitmap contactIcon = Bitmap.createBitmap(iconSize, iconSize,
				Config.ARGB_8888);
		Canvas canvas = new Canvas(contactIcon);
		BitmapDrawable bd = (BitmapDrawable) d;
		Bitmap bm = bd.getBitmap();

		// 拷贝图片
		Paint iconPaint = new Paint();
		iconPaint.setDither(true);
		iconPaint.setFilterBitmap(true);
		Rect src = new Rect(0, 0, iconSize, iconSize);
		Rect dst = new Rect(0, 0, iconSize, iconSize);
		canvas.drawBitmap(contactIcon, src, dst, iconPaint);

		Paint countPaint = new Paint(Paint.ANTI_ALIAS_FLAG
				| Paint.FILTER_BITMAP_FLAG);


			canvas.drawBitmap(CreatMatrixBitmap(bm, iconSize, iconSize), src, dst,
					countPaint);
		
		return contactIcon;
	}

	private Bitmap CreatMatrixBitmap(Bitmap bitMap, float scr_width,
			float res_height) {

		int bitWidth = bitMap.getWidth();
		int bitHeight = bitMap.getHeight();
		float scaleWidth = scr_width / (float) bitWidth;
		float scaleHeight = res_height / (float) bitHeight;
		Matrix matrix = new Matrix();
		matrix.postScale(scaleWidth, scaleHeight);
		bitMap = Bitmap.createBitmap(bitMap, 0, 0, bitWidth, bitHeight, matrix,
				true);
		return bitMap;
	}

	public void GetTime()
	{
		try  
		   {   
		Date   curDate   =   new   Date(System.currentTimeMillis());//获取当前时间     
			    String   ctime   =   df.format(curDate);     
					
			    nowtime = df.format(new Date());
		            Date d1 = df.parse(nowtime);   
		            Date d2 = df.parse(oldtime);   
		            long diff = d1.getTime() - d2.getTime();   
		            long min = diff / (1000 * 60);   
		            long hour = 0;
		            if( min >=60 )
		            {
		            	hour = min/60;
		            	min = min - hour*60;
		            }
		            String.valueOf(min);
		            String.valueOf(hour);
		            //Toast.makeText(this, "游戏已进行+hour+"小时"+min+"分钟"+" 当前时间"+ctime+" 剩余内存"+getAvailMemory()+battery, Toast.LENGTH_LONG).show();
			   myTime=" 游戏已进行"+hour+"小时"+min+"分钟"+"\n"+" 当前时间"+ctime+"\n"+" 剩余内存"+getAvailMemory()+"\n"+battery;
		}   
		        catch (Exception e)   
		        {   
		        } 

		final LinearLayout time_layout = new LinearLayout(this);
		time_layout.setOrientation(LinearLayout.VERTICAL);	
	
		
		if (time_popupWindow != null && time_popupWindow.isShowing()) {
					time_popupWindow.dismiss();
				} else {
					
					time_popupWindow = new PopupWindow(time_layout, LayoutParams.WRAP_CONTENT, LayoutParams.WRAP_CONTENT,true);
					time_popupWindow.setTouchable(true);
					time_popupWindow.setOutsideTouchable(true);
					time_popupWindow.setBackgroundDrawable(getResources().getDrawable(R.drawable.pub_pop_bg2));
					time_popupWindow.showAtLocation(mView, Gravity.CENTER, 0, 0); 
					
				}

		
		
		final TextView timeText=new TextView(this);
		timeText.setTextSize(21);
		timeText.setText(myTime);
		timeText.setGravity(Gravity.CENTER_VERTICAL);
		timeText.setBackgroundColor(Color.argb(0, 0, 0, 0));
		timeText.setTextColor(Color.BLACK);
		time_layout.addView(timeText);

		final Button timeBtn=new Button(this);
		timeBtn.setTextSize(21);
		timeBtn.setText(getResources().getString(R.string.button_Menu));
		timeBtn.setOnClickListener(new OnClickListener() {
			public void onClick(View v) {
				openOptionsMenu();
			}
		});
		
		
		time_layout.addView(timeBtn);




	}

	public String getAvailMemory()
	{
		ActivityManager activitymanager = (ActivityManager)this.getSystemService("activity");
		android.app.ActivityManager.MemoryInfo memoryinfo = new android.app.ActivityManager.MemoryInfo();
		activitymanager.getMemoryInfo(memoryinfo);
		Context context = this;
		long l = memoryinfo.availMem;
		return Formatter.formatFileSize(context, l);
	};

	public BroadcastReceiver mBatteryInfoReceiver = new BroadcastReceiver() {
		@Override
		public void onReceive(Context context, Intent intent) {
			String action = intent.getAction();
			if (Intent.ACTION_BATTERY_CHANGED.equals(action)) {

				int level = intent.getIntExtra("level", 0);
				int scale = intent.getIntExtra("scale", 100);

				battery = " 剩余电量 "+String.valueOf(level * 100 / scale) + "%";
			}
		}
	};

	public void About() {

		AlertDialog.Builder builder = new AlertDialog.Builder(this);
		builder.setTitle("关于 updates by natdon");
		builder.setMessage(getResources().getString(R.string.info));
		builder.setPositiveButton("确定", null);
		builder.setNegativeButton("访问论坛",
				new DialogInterface.OnClickListener() {
					public void onClick(DialogInterface dialog, int whichButton) {
						Uri uri = Uri.parse("http://mobi.acgfuture.com");
						Intent web = new Intent(Intent.ACTION_VIEW, uri);
						startActivity(web);
					}
				});


		builder.create().show();
	}

	/**********************************************************************************************************/
	
	@SuppressWarnings("deprecation")
	private void displayCover() {
		Object o = cover.getTag();
		if(o instanceof Bitmap) {
			cover.setTag(null);
			cover.setImageBitmap((Bitmap) o);
			cover.setBackgroundDrawable(null);
			cover.startAnimation(AnimationFactory.coverInAnimation());
			Command.invoke(Command.MAINACTIVITY_PLAY_VIDEO).of(this).only().sendDelayed(3000);
		}
	}

	public void playVideo() {
		Game item = items.getItem(items.getSelectedPosition());
		if(item.video != null && isVideoInitialized) {
			videoframe.clearAnimation();
			videoframe.setVisibility(View.VISIBLE);
			animPlayVideo.reset();
			videoframe.startAnimation(animPlayVideo);
		}
	}

	private void startVideoPlay() {
		Game item = items.getItem(items.getSelectedPosition());
		if(item.video != null && isVideoInitialized) {
				videoframe.setVisibility(View.VISIBLE);
				preview.setVisibility(View.VISIBLE);
				Command.revoke(Command.RELEASE_VIDEO_PREVIEW, preview);
				preview.setVideoURI(null);
				preview.setVideoPath(item.video);
		}
	}

	private void releaseVideoPlay() {
		videoframe.clearAnimation();

		// Clear Video Player
		if(preview.isPlaying()){
			preview.stopPlayback();
		}
		
		preview.setVisibility(View.GONE);

		if(videoframe.getVisibility() == View.VISIBLE) {
			animHideVideo.reset();
			videoframe.startAnimation(animHideVideo);
		}
		
		Command.invoke(Command.RELEASE_VIDEO_PREVIEW).exclude(Command.MAINACTIVITY_PLAY_VIDEO)
		.of(preview).sendDelayed(2000);
	}

	private void updateCover(final String url, final boolean coverToBkg) {
		cover.setVisibility(View.INVISIBLE);
		Object o = cover.getTag();
		if(o instanceof ImageSetter) {
			((ImageSetter) o).cancel();
		}

		if(!animCoverOut.hasStarted()) {
			cover.startAnimation(animCoverOut);
		}

		imgMgr.requestImageAsync(url,
				new ImageSetter(cover) {

			protected void act() {
				cover.setTag(image().bmp());
				displayCover();
				String background = CoverDecoder.getThumbernailCache(url);
				// Exception for Web Images
				if(background == null)
					background = CoverDecoder.getThumbernailCache(image().file().getAbsolutePath());
				if(coverToBkg && background != null) {
					updateBackground(background);
				}
			}

		},
		new CoverDecoder(cover.getWidth(), cover.getHeight()));
	}

	private void updateBackground(String url) {
		background.setVisibility(View.INVISIBLE);
		Object o = background.getTag();
		if(o instanceof ImageSetter) {
			((ImageSetter) o).cancel();
		}

		if(!animBackgroundOut.hasStarted())
			background.startAnimation(animBackgroundOut);

		imgMgr.requestImageAsync(url, new ImageSetter(background) {

			protected void act() {
				if(animBackgroundOut.hasEnded()||!animBackgroundOut.hasStarted()) {
					super.act();
					background.startAnimation(AnimationFactory.bkgInAnimation());
				}else{
					background.setTag(image().bmp());
				}
			}

		},
		new BackgroundDecoder());

	}

	/**
	 * Scroll view to the center of the game list
	 * @param view
	 * child view of game list
	 */
	private void scrollViewToCenter(View view) {
		int viewY = view.getTop() + view.getHeight() / 2 - games.getHeight() / 2;
		if(viewY < 0 && games.getFirstVisiblePosition() == 0){
			games.smoothScrollToPosition(0);
		}else if(viewY > 0 && games.getLastVisiblePosition() == items.getCount() - 1){
			games.smoothScrollToPosition(items.getCount() - 1);
		}else{
			Command.invoke(Command.SCROLL_LIST_FOR_DISTANCE_IN_ANY_MILLIS)
			.of(games).only().args(viewY, 300).sendDelayed(100);
		}
	}

	public void onItemClick(AdapterView<?> parent, View view, int position, long id) {

		scrollViewToCenter(view);

		if(items.getSelectedPosition() != position) {

			releaseVideoPlay();

			// Set Selection
			items.setSelectedPosition(position);

			final Game item = items.getItem(position);

			if(item.background != null) {
				updateBackground(item.background);
			}
			if(item.cover != null) {
				updateCover(item.cover, item.background == null);
			}else{
				// If no cover but video, play video directly
				if(item.video != null) {
					playVideo();
				}else{
					// With no multimedia information
					cover.setImageResource(R.drawable.dbkg_und);
				}
				if(item.background == null) {
					background.setImageResource(R.drawable.dbkg_und_blur);
				}
			}
			

			gametitle.setText(item.title);
		}
	}
	
	/**********************************************************************************************************/
	

    @Override
	public boolean onCreateOptionsMenu(Menu menu)
	{
		return true;
	}
	
    @Override
	public boolean onPrepareOptionsMenu( Menu menu )
	{
		if(mView != null){
			return mView.onPrepareOptionsMenu(menu);
		}
		return true;
	}
	
    @Override
	public boolean onOptionsItemSelected( MenuItem item )
	{
		if(mView != null){
			return mView.onOptionsItemSelected(item);
		}
		return true;
	}

	@Override
	protected void onPause() {
		if( !Globals.APP_CAN_RESUME && mView != null ){
			mView.exitApp();
			try {
				wait(3000);
			} catch(InterruptedException e){}
		}
		
		_isPaused = true;
		if( mView != null && !Locals.gWindowScreen)
			mView.onPause();

		if (popbtn != null && popbtn.isShown()) {
			wm = (WindowManager) getApplicationContext().getSystemService(
					"window");
			params = new WindowManager.LayoutParams();
			wm.removeView(popbtn);
		}
		MobclickAgent.onPause(this);
		super.onPause();
	}

	@Override
	protected void onResume() {
		super.onResume();
		MobclickAgent.onResume(this);
		Command.invoke(Command.GENERATE_TEST_DATA).of(items).sendDelayed(300);
		if( mView != null )
		{
			mView.onResume();
		}
		_isPaused = false;
	}

	@Override
	protected void onDestroy() 
	{
		if( mView != null ){
			mView.exitApp();
			try {
				wait(3000);
			} catch(InterruptedException e){}
		}
		extra = null;
		mysetting = null;
		super.onDestroy();
		destroyImageManager();
		System.exit(0);
	}

	/*@Override
	public boolean onKeyDown(int keyCode, KeyEvent event) {
		// TODO Auto-generated method stub
		if(keyCode == KeyEvent.KEYCODE_BACK) {
			runAppLauncher();
		  }
		return true;
	}*/

	@Override
	public void onConfigurationChanged(Configuration newConfig)
	{
		super.onConfigurationChanged(newConfig);
		// Do nothing here
	}
/*
	public void showTaskbarNotification()
	{
		showTaskbarNotification("SDL application paused", "SDL application", "Application is paused, click to activate");
	}

	// Stolen from SDL port by Mamaich
	public void showTaskbarNotification(String text0, String text1, String text2)
	{
		NotificationManager NotificationManager = (NotificationManager) getSystemService(Context.NOTIFICATION_SERVICE);
		Intent intent = new Intent(this, ONScripter.class);
		PendingIntent pendingIntent = PendingIntent.getActivity(this, 0, intent, Intent.FLAG_ACTIVITY_NEW_TASK);
		Notification n = new Notification(R.drawable.icon, text0, System.currentTimeMillis());
		n.setLatestEventInfo(this, text1, text2, pendingIntent);
		NotificationManager.notify(NOTIFY_ID, n);
	}

	public void hideTaskbarNotification()
	{
		NotificationManager NotificationManager = (NotificationManager) getSystemService(Context.NOTIFICATION_SERVICE);
		NotificationManager.cancel(NOTIFY_ID);
	}

	public int getApplicationVersion()
	{
		try {
			PackageInfo packageInfo = getPackageManager().getPackageInfo(getPackageName(), 0);
			return packageInfo.versionCode;
		} catch (PackageManager.NameNotFoundException e) {
			System.out.println("libSDL: Cannot get the version of our own package: " + e);
		}
		return 0;
	}

	static int NOTIFY_ID = 12367098; // Random ID
*/
	public static ONScripter instance = null;
	public static MainView mView = null;


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

	boolean _isPaused = false;



}

