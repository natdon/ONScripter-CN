package cn.natdon.onscripterv2;

import java.text.SimpleDateFormat;
import java.util.Date;

import com.umeng.analytics.MobclickAgent;

import cn.natdon.onscripterv2.Button.OkCancelButton;
import cn.natdon.onscripterv2.VideoPlayer.activity.PlayerActivity;
import android.R.integer;
import android.annotation.SuppressLint;
import android.app.Activity;
import android.app.ActivityManager;
import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.graphics.Color;
import android.graphics.Rect;
import android.graphics.drawable.BitmapDrawable;
import android.net.Uri;
import android.os.Bundle;
import android.os.PowerManager;
import android.os.Vibrator;
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
import android.view.WindowManager;
import android.view.View.OnClickListener;
import android.view.View.OnLongClickListener;
import android.view.View.OnTouchListener;
import android.view.ViewGroup.LayoutParams;
import android.widget.Button;
import android.widget.EditText;
import android.widget.HorizontalScrollView;
import android.widget.ImageButton;
import android.widget.LinearLayout;
import android.widget.PopupWindow;
import android.widget.ScrollView;
import android.widget.SeekBar;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.SeekBar.OnSeekBarChangeListener;

public class ONSView extends Activity{
	
	public static PopupWindow light_popupWindow,time_popupWindow,button_popupWindow;	
	
	private native int nativeFontSize(int font_x,int font_y,int font_px,int font_py);
	private native int nativeFontColor(boolean usecolor,int fcolor1,int fcolor2,int fcolor3);
	private native int nativeInitJavaCallbacks();
	private native int nativeVibrator();
	private native int nativeHW(boolean useHW);
	private native int nativeInputBox();
	
	public static WindowManager wdwm;
	public static WindowManager wm;
	public static WindowManager.LayoutParams params = new WindowManager.LayoutParams();
	public static WindowManager.LayoutParams wdparams = new WindowManager.LayoutParams();
	
	private WindowManager.LayoutParams lp;
	
	private Vibrator vt;
	
	private PowerManager.WakeLock wakeLock = null;
	
	
	boolean _isPaused = false;
	
	public static MainView mView = null;
	
	//public ONSVariable ONSVariable;
	
	private LinearLayout layout1 = null;
	private LinearLayout Btnlayout;
	private LinearLayout Fulllayout = null;
	
	public static LinearLayout wdlayout = null;
	
	private HorizontalScrollView HSV = null;
	private ScrollView FullSV = null;
	private ImageButton popbtn;
	
	
	private SimpleDateFormat df;
	private String oldtime, nowtime, battery,myTime;
	private float light=0;
	private float x, y;
	private float startX, startY;
	
	
	private static final int TOUCH_SLOP = 50;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		
		MobclickAgent.onError(this);
		
		this.registerReceiver(mBatteryInfoReceiver, new IntentFilter(
				Intent.ACTION_BATTERY_CHANGED));
		df = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");   
        	oldtime = df.format(new Date());
        	
        if(ONSVariable.set_keepON){
        	getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        }
        	
        	if(!Locals.gWindowScreen)
			{
				runApp();
			}
			else{
				runWD();
			}
	}
	
	
	@SuppressLint("NewApi")
	public void playVideo(char[] filename) {
		if (Locals.gDisableVideo == false) {
			SharedPreferences sp = getSharedPreferences("pref", MODE_PRIVATE);
			ONSVariable.mPlayer = sp.getBoolean("playerchoose", true);
			try {
				String filename2 = "file:/"+ "/"+Globals.CurrentDirectoryPath + "/"
						+ new String(filename);
				filename2 = filename2.replace('\\', '/');
				Log.v("ONS", "playVideo: " + filename2);
				if (Locals.gOtherPL) {
					Uri uri = Uri.parse(filename2);
					Intent i = new Intent(Intent.ACTION_VIEW);
					i.setDataAndType(uri, "video/*");
					startActivityForResult(i, -1);
				} else {

					if(ONSVariable.mPlayer)
					{
						Intent in = new Intent();
						in.putExtra("one", filename2);
						in.setClass(ONSView.this, VitamioPlayer.class);
						ONSView.this.startActivity(in);
					}
					else {
						Intent in = new Intent();
						in.putExtra("one", filename2);
						in.setClass(ONSView.this, PlayerActivity.class);
						ONSView.this.startActivity(in);
					}
					
				}
				overridePendingTransition(android.R.anim.fade_in,
						android.R.anim.fade_out);
			} catch (Exception e) {
				Log.e("ONS", "playVideo error:  " + e.getClass().getName());
			}
		}
	}
	
	public void Vibrator(int milliseconds)
	{
		vt=(Vibrator)getApplication().getSystemService(Service.VIBRATOR_SERVICE);
		vt.vibrate(milliseconds);
	}
	
	public void InputBox(char[] title,final char[] text) {

		Intent in = new Intent();
		in.putExtra("title", new String(title));
		in.putExtra("text", new String(text));
		in.setClass(ONSView.this, InputBox.class);
		ONSView.this.startActivity(in);

	}
	
	
	public void runApp()
	{
		nativeHW(ONSVariable.set_checkHW);
		nativeInitJavaCallbacks();
		nativeVibrator();
		nativeInputBox();
		
		if(Locals.gFontSize)
		nativeFontSize(ONSVariable.myfontsize,ONSVariable.myfontsize,ONSVariable.myfontpx,ONSVariable.myfontpx);
		if(Locals.gFontColor)
		nativeFontColor(Locals.gFontColor,ONSVariable.myfont_color1,ONSVariable.myfont_color2,ONSVariable.myfont_color3);

		if(Locals.Logout)
		debug.Logcat_out(Globals.CurrentDirectoryPath);
		
		ONSVariable.isRunning = true;
		
		PowerManager pm = (PowerManager) getSystemService(Context.POWER_SERVICE);
		wakeLock = pm.newWakeLock(PowerManager.SCREEN_DIM_WAKE_LOCK, "ONScripter");
		wakeLock.acquire();

		
		
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
		
		//if(extra == null)
		//FreeMemory();
		System.gc();
	}


	private void runWD() {
		Rect frame2 = new Rect();
		getWindow().getDecorView().getWindowVisibleDisplayFrame(frame2);
		nativeInitJavaCallbacks();
		nativeVibrator();
		nativeHW(ONSVariable.set_checkHW);

		if(Locals.gFontSize)
			nativeFontSize(ONSVariable.myfontsize,ONSVariable.myfontsize,ONSVariable.myfontpx,ONSVariable.myfontpx);
			if(Locals.gFontColor)
			nativeFontColor(Locals.gFontColor,ONSVariable.myfont_color1,ONSVariable.myfont_color2,ONSVariable.myfont_color3);

		if(Locals.Logout)
		debug.Logcat_out(Globals.CurrentDirectoryPath);

		ONSVariable.isRunning = true;
		lp  = getWindow().getAttributes();  
		
		if(mView == null){
			mView = new MainView(this);
			//setContentView(mView);
		}
		mView.setFocusableInTouchMode(true);
		mView.setFocusable(true);
		mView.requestFocus();

		lp  = getWindow().getAttributes();
		
		//FreeMemory();
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
						
						ONSVariable.isMoved = true;
						
					}
					if (ONSVariable.isMoved == true) {
						if(Locals.HideClick){
						 ONSView.wdupdatePosition(x,y,startX,startY);
						
						 ONSVariable.isMoved = false;
						}
					}
					break;
				case MotionEvent.ACTION_UP:
					if(Locals.HideClick){
					ONSView.wdupdatePosition(x,y,startX,startY);
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
		

		wdlayout.addView(mView, new LinearLayout.LayoutParams(ONSVariable.wdw,
				ONSVariable.wdh));
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
		// View current position		
		wdparams.x = (int)( a - c);  
     	wdparams.y = (int) (b - d);  
		wdwm.updateViewLayout(wdlayout, wdparams);
	}
	
	public static void HideWindow()
	{
		Locals.ScreenHide = true;
		mView.ScreenHide();
		mView.setVisibility(View.GONE);
		if (mView != null)
			mView.onPause();
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
					light_popupWindow.setBackgroundDrawable(getResources().getDrawable(R.drawable.config_upper));
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
						ONSVariable.isMoved = true;
					}
					if (ONSVariable.isMoved == true) {
						updatePosition();
						
						ONSVariable.isMoved = false;
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
		btnup.setTextSize(ONSVariable.textsize);
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
		btndown.setTextSize(ONSVariable.textsize);
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
		btnleft.setTextSize(ONSVariable.textsize);
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
		btnright.setTextSize(ONSVariable.textsize);
		btnright.setTextColor(Color.WHITE);
		btnright.setText(getResources().getString(R.string.Key_RightClick));
		btnright.setOnClickListener(new OnClickListener() {
			public void onClick(View v) {
				mView.nativeKey(KeyEvent.KEYCODE_BACK, 1);
				mView.nativeKey(KeyEvent.KEYCODE_BACK, 0);
			}
		});

		Button btnleft2 = new Button(this); //left
		btnleft2.setBackgroundColor(Color.argb(10, 10, 10, 10));
		btnleft2.setTextSize(ONSVariable.textsize);
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
		btnright2.setTextSize(ONSVariable.textsize);
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
		btnvolup.setTextSize(ONSVariable.textsize);
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
		btnvoldown.setTextSize(ONSVariable.textsize);
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
		btngetfshot.setTextSize(ONSVariable.textsize);
		btngetfshot.setTextColor(Color.WHITE);
		btngetfshot.setOnClickListener(new OnClickListener() {
			public void onClick(View v) {
				openOptionsMenu();
			}
		});

		Button btngetshot = new Button(this);
		btngetshot.setText(getResources().getString(R.string.button_get_default_screenshot));
		btngetshot.setBackgroundColor(Color.argb(10, 10, 10, 10));
		btngetshot.setTextSize(ONSVariable.textsize);
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
		params.x = (int) (x - 30);
		params.y = (int) (y - 60);
		wm.updateViewLayout(popbtn, params);
	}
	
	
	public void GetTime()
	{
		try  
		   {   
				Date   curDate   =   new   Date(System.currentTimeMillis());   
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
					time_popupWindow.setAnimationStyle(R.style.Animation_ConfigPanelAnimation);
					time_popupWindow.setTouchable(true);
					time_popupWindow.setOutsideTouchable(true);
					time_popupWindow.setBackgroundDrawable(getResources().getDrawable(R.drawable.config_upper));
					time_popupWindow.showAtLocation(mView, Gravity.CENTER, 0, 0); 
					
				}

		
		
		final TextView timeText=new TextView(this);
		timeText.setTextSize(21);
		timeText.setText(myTime);
		timeText.setGravity(Gravity.CENTER_VERTICAL);
		timeText.setBackgroundColor(Color.argb(0, 0, 0, 0));
		timeText.setTextColor(Color.BLACK);
		time_layout.addView(timeText);

		final OkCancelButton timeBtn=new OkCancelButton(this);
		timeBtn.setTextSize(21);
		timeBtn.setTextColor(Color.BLACK);
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
	
	//
	public void showTaskbarNotification()
	{
		showTaskbarNotification("MiNE正在后台运行", "MiNE "+getApplicationVersion(), "MiNE正在后台运行, 点击恢复。");
	}

	// Stolen from SDL port by Mamaich
	public void showTaskbarNotification(String text0, String text1, String text2)
	{
		NotificationManager NotificationManager = (NotificationManager) getSystemService(Context.NOTIFICATION_SERVICE);
		Intent intent = new Intent(this, ONSView.class);
		PendingIntent pendingIntent = PendingIntent.getActivity(this, 0, intent, Intent.FLAG_ACTIVITY_CLEAR_TOP|Intent.FLAG_ACTIVITY_NEW_TASK);
		Notification n = new Notification(R.drawable.icon, text0, System.currentTimeMillis());
		n.flags |=Notification.FLAG_ONGOING_EVENT|Notification.FLAG_NO_CLEAR;
		n.setLatestEventInfo(this, text1, text2, pendingIntent);
		NotificationManager.notify(NOTIFY_ID, n);
	}

	public void hideTaskbarNotification()
	{
		NotificationManager NotificationManager = (NotificationManager) getSystemService(Context.NOTIFICATION_SERVICE);
		NotificationManager.cancel(NOTIFY_ID);
	}

	public String getApplicationVersion()
	{
		try {
			PackageInfo packageInfo = getPackageManager().getPackageInfo(getPackageName(), 0);
			return packageInfo.versionName;
		} catch (PackageManager.NameNotFoundException e) {
			System.out.println("libSDL: Cannot get the version of our own package: " + e);
		}
		return null;
	}

	static int NOTIFY_ID = 12367098; // Random ID
	
	///////////////////////////////////////////////////////////////////////////////////////////////////////
	
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
		
		if(!Locals.gWindowScreen && ONSVariable.isRunning)
			showTaskbarNotification();
		
		if (wakeLock != null)
			wakeLock.release();
		
		super.onPause();
	}
	
	@Override
	protected void onResume() {
		if (wakeLock != null && !wakeLock.isHeld())
			wakeLock.acquire();
		super.onResume();
		MobclickAgent.onResume(this);

		if( mView != null )
		{
			mView.onResume();
		}
		_isPaused = false;
		
		hideTaskbarNotification();
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
		super.onDestroy();
		hideTaskbarNotification();
		//System.exit(0);
	}

}
