/*
 2012/7 Created by AKIZUKI Katane
 */

package cn.natdon.onscripterv2;

import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Timer;
import java.util.Date;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.content.Context;
import android.opengl.GLSurfaceView;
import android.os.Build;
import android.os.Bundle;
import android.os.PowerManager;
import android.graphics.Color;
import android.view.MotionEvent;
import android.view.KeyEvent;
import android.view.Window;
import android.view.WindowManager;
import android.view.Display;
import android.view.View;
import android.view.View.OnTouchListener;
import android.view.Menu;
import android.view.SubMenu;
import android.view.MenuItem;
import android.widget.Button;
import android.widget.AbsoluteLayout;
import android.widget.AbsoluteLayout.LayoutParams;
import android.widget.TextView;
import android.widget.Toast;
import android.media.AudioManager;
import android.util.Log;
import android.content.pm.ActivityInfo;
import java.util.TreeMap;
import java.util.Iterator;
import java.util.Set;
import android.app.AlertDialog;
//import android.app.AlertDialog.Builder;
import android.content.DialogInterface;
import android.view.InputDevice;
import android.app.ActivityManager;
import android.content.BroadcastReceiver;
import android.text.format.Formatter;
import android.widget.Toast;
import android.content.Intent;

@SuppressLint("NewApi")
public class MainView extends AbsoluteLayout
{
	public MainView(ONSView onsView)
	{
		super(onsView);
		
		mActivity = onsView;
		setBackgroundColor(Color.BLACK);
		setScreenOrientation(Locals.ScreenOrientation);
		

		//

		mAudioThread = new AudioThread(this);
		
		mGLView = new DemoGLSurfaceView(onsView);
		this.addView(mGLView);

		mMouseCursor = new MouseCursor(onsView);
		this.addView(mMouseCursor);
		showMouseCursor(Globals.MouseCursorShowed);
		
		int vw = mDisplayWidth;
		int vh = mDisplayHeight;
		if(Locals.VideoXRatio > 0 && Locals.VideoYRatio > 0){
			if(vw * Locals.VideoYRatio > vh * Locals.VideoXRatio){
				vw = vh * Locals.VideoXRatio / Locals.VideoYRatio;
			} else {
				vh = vw * Locals.VideoYRatio / Locals.VideoXRatio;
			}
		}
		
		if(Locals.gWindowScreen){
		vw=ONSVariable.wdw;
		vh=ONSVariable.wdh;
		}
		if(Locals.gFullScreen){
		vw=ONSVariable.dw;
		vh=vw/4*3;
		}
		setGLViewRect(0, 0, vw, vh);
		
		mTouchMode = TouchMode.getTouchMode(Locals.TouchMode, this);
		mTouchMode.setup();
		mTouchMode.update();
		
		mTouchInput = DifferentTouchInput.getInstance();
		mTouchInput.setOnInputEventListener(mTouchMode);
		
		nativeInitInputJavaCallbacks();
	}
	
	protected void onPause()
	{
		_isPaused = true;
		if(!Locals.gWindowScreen || Locals.ScreenHide)
		mGLView.onPause();
	}
	
	protected void onResume()
	{
		mGLView.onResume();
		DimSystemStatusBar.get().dim(this);
		DimSystemStatusBar.get().dim(mGLView);
		_isPaused = false;
	}
	
	public boolean isPaused()
	{
		return _isPaused;
	}
	
	public void exitApp()
	{
		mGLView.exitApp();
	}

	public void ScreenHide()
	{
		if(Locals.ScreenHide)
		mGLView.setVisibility(View.GONE);
		else
		mGLView.setVisibility(View.VISIBLE);
	}

	@Override
	public boolean onTouchEvent(final MotionEvent event) 
	{
		mTouchInput.process(event);
		//mGLView.limitEventRate(event);
		int pointerCount = event.getPointerCount();

		x = event.getRawX();     
		y = event.getRawY();

		if( event.getAction() == MotionEvent.ACTION_DOWN ){
			if(Locals.gWindowScreen && Locals.ScreenMove){
			 startX = event.getX();  
			 startY = event.getY(); 
			}
			
		}
		if( event.getAction() == MotionEvent.ACTION_UP ){
			if(Locals.gWindowScreen && Locals.ScreenMove){
				ONSView.wdupdatePosition(x,y,startX,startY);
			 startX = startY = 0; 
			}
			
		}
		if( event.getAction() == MotionEvent.ACTION_MOVE ){
			if(Locals.gWindowScreen && Locals.ScreenMove){
				ONSView.wdupdatePosition(x,y,startX,startY);
			}
			
		}

		//if (pointerCount == 2) {

			switch (event.getAction()) {

			case MotionEvent.ACTION_DOWN:

				break;
			case MotionEvent.ACTION_MOVE:

				break;
			case MotionEvent.ACTION_POINTER_1_UP:
				mActivity.GetTime();
				break;

			}

		//}
		if (pointerCount == 3) {
			if (event.getAction() == MotionEvent.ACTION_POINTER_3_DOWN) {
				if ((System.currentTimeMillis() - exitTime) > 2000) {
					Toast.makeText(mActivity, "再按一次退出程序", Toast.LENGTH_SHORT)
							.show();
					exitTime = System.currentTimeMillis();
				} else {

					System.exit(0);
				}
			}
			
    	        }

		return true;
	};


	
	private int mJoyStickState    = 0;
	private int mJoyStickHatState = 0;
	private long exitTime = 0;
	private float x;
	private float y;
	private float startX;
	private float startY;
	private boolean isMoved = false;
	
	@Override
	public boolean onGenericMotionEvent (final MotionEvent event)
	{
		if(android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.GINGERBREAD){
			if(AndroidGingerBreadAPI.MotionEventGetSource(event) == InputDevice.SOURCE_JOYSTICK){
				final float BORDER = 0.5f;
				int state, diff;
				
				//AXIS
				
				float x = AndroidGingerBreadAPI.MotionEventGetAxisValue( event, MotionEvent.AXIS_X );
				float y = AndroidGingerBreadAPI.MotionEventGetAxisValue( event, MotionEvent.AXIS_Y );
				
				state = 0;
				if( x < -BORDER ){
					state |= 1 << Globals.JOYSTICK_AXIS_LEFT_INDEX;
				} else if( x > BORDER ){
					state |= 1 << Globals.JOYSTICK_AXIS_RIGHT_INDEX;
				}
				if( y < -BORDER ){
					state |= 1 << Globals.JOYSTICK_AXIS_UP_INDEX;
				} else if( y > BORDER ){
					state |= 1 << Globals.JOYSTICK_AXIS_DOWN_INDEX;
				}
				
				diff = state ^ mJoyStickState;
				mJoyStickState = state;
				
				for(int i = 0; diff != 0 ; diff>>= 1, state>>= 1, i ++){
					if( (diff & 1) != 0 ){
						if(mTouchMode != null){
							mTouchMode.onKeyEvent(Globals.JOYSTICK_AXIS_KEY_ARRAY[i], ((state & 1) != 0 ? 1 : 0));
						}
					}
				}
				
				//AXIS_HAT
				
				float hatx = AndroidGingerBreadAPI.MotionEventGetAxisValue( event, MotionEvent.AXIS_HAT_X );
				float haty = AndroidGingerBreadAPI.MotionEventGetAxisValue( event, MotionEvent.AXIS_HAT_Y );
				
				state = 0;
				if( hatx < -BORDER ){
					state |= 1 << Globals.JOYSTICK_AXIS_LEFT_INDEX;
				} else if( hatx > BORDER ){
					state |= 1 << Globals.JOYSTICK_AXIS_RIGHT_INDEX;
				}
				if( haty < -BORDER ){
					state |= 1 << Globals.JOYSTICK_AXIS_UP_INDEX;
				} else if( haty > BORDER ){
					state |= 1 << Globals.JOYSTICK_AXIS_DOWN_INDEX;
				}
				
				diff = state ^ mJoyStickHatState;
				mJoyStickHatState = state;
				
				for(int i = 0; diff != 0 ; diff>>= 1, state>>= 1, i ++){
					if( (diff & 1) != 0 ){
						if(mTouchMode != null){
							mTouchMode.onKeyEvent(Globals.JOYSTICK_AXISHAT_KEY_ARRAY[i], ((state & 1) != 0 ? 1 : 0));
						}
					}
				}
				return true;
			}
		}
		
		mTouchInput.processGenericEvent(event);
		//mGLView.limitEventRate(event);
		
		return true;
	}
	
	@Override
	public boolean onKeyDown(int keyCode, final KeyEvent event) {
		//Log.i("Event", "keyDown : " + keyCode);
		
		switch(keyCode){
			case KeyEvent.KEYCODE_VOLUME_UP:
			case KeyEvent.KEYCODE_VOLUME_DOWN:
			case KeyEvent.KEYCODE_VOLUME_MUTE:
				return super.onKeyDown(keyCode, event);
			case KeyEvent.KEYCODE_MENU:
				if(!mIsKeyConfigMode){
					return super.onKeyDown(keyCode, event);
				}
			case KeyEvent.KEYCODE_BACK:
			default:
				if(mTouchMode != null){
					mTouchMode.onKeyEvent(keyCode, 1);
				}
				break;
		}
		
		return true;
	}
	
	@Override
	public boolean onKeyUp(int keyCode, final KeyEvent event) {
		//Log.i("Event", "keyUp : " + keyCode);
		
		switch(keyCode){
			case KeyEvent.KEYCODE_VOLUME_UP:
			case KeyEvent.KEYCODE_VOLUME_DOWN:
				return super.onKeyUp(keyCode, event);
			case KeyEvent.KEYCODE_MENU:
				if(!mIsKeyConfigMode){
					return super.onKeyUp(keyCode, event);
				}
				leaveKeyConfigMode();
				break;
			case KeyEvent.KEYCODE_BACK:
			default:
				if(mTouchMode != null){
					mTouchMode.onKeyEvent(keyCode, 0);
				}
				break;
		}
		
		if(keyCode == KeyEvent.KEYCODE_MENU || keyCode == KeyEvent.KEYCODE_BACK){
			DimSystemStatusBar.get().dim(this);
			DimSystemStatusBar.get().dim(mGLView);
		}
		
		return true;
	}
	
	//
	
	private static final int MENU_ITEM_ID_USER_MENU_KEY_FIRST = Menu.FIRST + 100;
	private static final int MENU_ITEM_ID_USER_MENU_KEY_LAST = MENU_ITEM_ID_USER_MENU_KEY_FIRST + Globals.MENU_KEY_NUM - 1;
	
	private static final int MENU_ITEM_ID_USER_SUBMENU_KEY_FIRST = Menu.FIRST + 200;
	private static final int MENU_ITEM_ID_USER_SUBMENU_KEY_LAST = MENU_ITEM_ID_USER_SUBMENU_KEY_FIRST + Globals.SUBMENU_KEY_NUM - 1;

	private static final int MENU_ITEM_ID_TOUCHMODE_FIRST = Menu.FIRST + 300;
	private static final int MENU_ITEM_ID_TOUCHMODE_INVALID  = MENU_ITEM_ID_TOUCHMODE_FIRST;
	private static final int MENU_ITEM_ID_TOUCHMODE_TOUCH    = MENU_ITEM_ID_TOUCHMODE_FIRST + 1;
	private static final int MENU_ITEM_ID_TOUCHMODE_TRACKPAD = MENU_ITEM_ID_TOUCHMODE_FIRST + 2;
	private static final int MENU_ITEM_ID_TOUCHMODE_GAMEPAD  = MENU_ITEM_ID_TOUCHMODE_FIRST + 3;

	private static final int MENU_ITEM_ID_SETTING_KEYCONFIG = Menu.FIRST + 400;
	
	private static final int MENU_ITEM_ID_SETTING_MOUSECURSOR_FIRST = Menu.FIRST + 500;
	private static final int MENU_ITEM_ID_SETTING_MOUSECURSOR_SHOW = MENU_ITEM_ID_SETTING_MOUSECURSOR_FIRST;
	private static final int MENU_ITEM_ID_SETTING_MOUSECURSOR_HIDE = MENU_ITEM_ID_SETTING_MOUSECURSOR_FIRST + 1;
	
	private static final int MENU_ITEM_ID_SETTING_VIDEO_XPOSITION_FIRST = Menu.FIRST + 600;
	private static final int MENU_ITEM_ID_SETTING_VIDEO_XPOSITION_LEFT = MENU_ITEM_ID_SETTING_VIDEO_XPOSITION_FIRST;
	private static final int MENU_ITEM_ID_SETTING_VIDEO_XPOSITION_CENTER = MENU_ITEM_ID_SETTING_VIDEO_XPOSITION_FIRST + 1;
	private static final int MENU_ITEM_ID_SETTING_VIDEO_XPOSITION_RIGHT = MENU_ITEM_ID_SETTING_VIDEO_XPOSITION_FIRST + 2;
	
	private static final int MENU_ITEM_ID_SETTING_VIDEO_YPOSITION_FIRST = Menu.FIRST + 605;
	private static final int MENU_ITEM_ID_SETTING_VIDEO_YPOSITION_TOP = MENU_ITEM_ID_SETTING_VIDEO_YPOSITION_FIRST;
	private static final int MENU_ITEM_ID_SETTING_VIDEO_YPOSITION_CENTER = MENU_ITEM_ID_SETTING_VIDEO_YPOSITION_FIRST + 1;
	private static final int MENU_ITEM_ID_SETTING_VIDEO_YPOSITION_BOTTOM = MENU_ITEM_ID_SETTING_VIDEO_YPOSITION_FIRST + 2;

	private static final int MENU_ITEM_ID_SETTING_VIDEO_XMARGIN_FIRST = Menu.FIRST + 620;
	private static final int MENU_ITEM_ID_SETTING_VIDEO_XMARGIN_LAST  = MENU_ITEM_ID_SETTING_VIDEO_XMARGIN_FIRST + 12;
	private static final int MENU_ITEM_ID_SETTING_VIDEO_YMARGIN_FIRST = Menu.FIRST + 640;
	private static final int MENU_ITEM_ID_SETTING_VIDEO_YMARGIN_LAST  = MENU_ITEM_ID_SETTING_VIDEO_YMARGIN_FIRST + 12;
	
	private static final int MENU_ITEM_ID_SETTING_BUTTON_LEFT_FIRST = Menu.FIRST + 700;
	private static final int MENU_ITEM_ID_SETTING_BUTTON_LEFT_ENABLED_ENABLE  = MENU_ITEM_ID_SETTING_BUTTON_LEFT_FIRST;
	private static final int MENU_ITEM_ID_SETTING_BUTTON_LEFT_ENABLED_DISABLE = MENU_ITEM_ID_SETTING_BUTTON_LEFT_FIRST + 1;
	private static final int MENU_ITEM_ID_SETTING_BUTTON_LEFT_NUM_FIRST = MENU_ITEM_ID_SETTING_BUTTON_LEFT_FIRST + 10;
	private static final int MENU_ITEM_ID_SETTING_BUTTON_LEFT_NUM_LAST  = MENU_ITEM_ID_SETTING_BUTTON_LEFT_NUM_FIRST + Globals.BUTTON_LEFT_MAX;
	
	private static final int MENU_ITEM_ID_SETTING_BUTTON_RIGHT_FIRST = Menu.FIRST + 800;
	private static final int MENU_ITEM_ID_SETTING_BUTTON_RIGHT_ENABLED_ENABLE  = MENU_ITEM_ID_SETTING_BUTTON_RIGHT_FIRST;
	private static final int MENU_ITEM_ID_SETTING_BUTTON_RIGHT_ENABLED_DISABLE = MENU_ITEM_ID_SETTING_BUTTON_RIGHT_FIRST + 1;
	private static final int MENU_ITEM_ID_SETTING_BUTTON_RIGHT_NUM_FIRST = MENU_ITEM_ID_SETTING_BUTTON_RIGHT_FIRST + 10;
	private static final int MENU_ITEM_ID_SETTING_BUTTON_RIGHT_NUM_LAST  = MENU_ITEM_ID_SETTING_BUTTON_RIGHT_NUM_FIRST + Globals.BUTTON_RIGHT_MAX;
	
	private static final int MENU_ITEM_ID_SETTING_BUTTON_TOP_FIRST = Menu.FIRST + 900;
	private static final int MENU_ITEM_ID_SETTING_BUTTON_TOP_ENABLED_ENABLE  = MENU_ITEM_ID_SETTING_BUTTON_TOP_FIRST;
	private static final int MENU_ITEM_ID_SETTING_BUTTON_TOP_ENABLED_DISABLE = MENU_ITEM_ID_SETTING_BUTTON_TOP_FIRST + 1;
	private static final int MENU_ITEM_ID_SETTING_BUTTON_TOP_NUM_FIRST = MENU_ITEM_ID_SETTING_BUTTON_TOP_FIRST + 10;
	private static final int MENU_ITEM_ID_SETTING_BUTTON_TOP_NUM_LAST  = MENU_ITEM_ID_SETTING_BUTTON_TOP_NUM_FIRST + Globals.BUTTON_TOP_MAX;
	
	private static final int MENU_ITEM_ID_SETTING_BUTTON_BOTTOM_FIRST = Menu.FIRST + 1000;
	private static final int MENU_ITEM_ID_SETTING_BUTTON_BOTTOM_ENABLED_ENABLE  = MENU_ITEM_ID_SETTING_BUTTON_BOTTOM_FIRST;
	private static final int MENU_ITEM_ID_SETTING_BUTTON_BOTTOM_ENABLED_DISABLE = MENU_ITEM_ID_SETTING_BUTTON_BOTTOM_FIRST + 1;
	private static final int MENU_ITEM_ID_SETTING_BUTTON_BOTTOM_NUM_FIRST = MENU_ITEM_ID_SETTING_BUTTON_BOTTOM_FIRST + 10;
	private static final int MENU_ITEM_ID_SETTING_BUTTON_BOTTOM_NUM_LAST  = MENU_ITEM_ID_SETTING_BUTTON_BOTTOM_NUM_FIRST + Globals.BUTTON_BOTTOM_MAX;

	private static final int MENU_ITEM_ID_SETTING_GAMEPAD_SIZE_FIRST = Menu.FIRST + 1100;
	private static final int MENU_ITEM_ID_SETTING_GAMEPAD_SIZE_LAST  = MENU_ITEM_ID_SETTING_GAMEPAD_SIZE_FIRST + 20;
	
	private static final int MENU_ITEM_ID_SETTING_GAMEPAD_OPACITY_FIRST = Menu.FIRST + 1200;
	private static final int MENU_ITEM_ID_SETTING_GAMEPAD_OPACITY_LAST  = MENU_ITEM_ID_SETTING_GAMEPAD_OPACITY_FIRST + 20;
	
	private static final int MENU_ITEM_ID_SETTING_GAMEPAD_POSITION_FIRST = Menu.FIRST + 1300;
	private static final int MENU_ITEM_ID_SETTING_GAMEPAD_POSITION_LAST  = MENU_ITEM_ID_SETTING_GAMEPAD_POSITION_FIRST + 20;
	
	private static final int MENU_ITEM_ID_SETTING_GAMEPAD_ARROW_BUTTON_FIRST     = Menu.FIRST + 1400;
	private static final int MENU_ITEM_ID_SETTING_GAMEPAD_ARROW_BUTTON_AS_AXIS   = MENU_ITEM_ID_SETTING_GAMEPAD_ARROW_BUTTON_FIRST;
	private static final int MENU_ITEM_ID_SETTING_GAMEPAD_ARROW_BUTTON_AS_BUTTON = MENU_ITEM_ID_SETTING_GAMEPAD_ARROW_BUTTON_FIRST + 1;
	
	private static final int MENU_ITEM_ID_SETTING_GAMEPAD_ACTION_BUTTON_FIRST     = Menu.FIRST + 1500;
	private static final int MENU_ITEM_ID_SETTING_GAMEPAD_ACTION_BUTTON_AS_AXIS   = MENU_ITEM_ID_SETTING_GAMEPAD_ACTION_BUTTON_FIRST;
	private static final int MENU_ITEM_ID_SETTING_GAMEPAD_ACTION_BUTTON_AS_BUTTON = MENU_ITEM_ID_SETTING_GAMEPAD_ACTION_BUTTON_FIRST + 1;
	
	private static final int MENU_ITEM_ID_SETTING_APPCONFIG_FIRST = Menu.FIRST + 9990;
	private static final int MENU_ITEM_ID_SETTING_APPCONFIG_USE = MENU_ITEM_ID_SETTING_APPCONFIG_FIRST;
	private static final int MENU_ITEM_ID_SETTING_APPCONFIG_NOTUSE = MENU_ITEM_ID_SETTING_APPCONFIG_FIRST + 1;
	
	private static final int MENU_ITEM_ID_ABOUT = Menu.FIRST + 9991;
	private static final int MENU_ITEM_ID_QUIT = Menu.FIRST + 9992;
	//private static final int MENU_ITEM_ID_AUTOSAVE = Menu.FIRST + 9993;
	private static final int MENU_ITEM_ID_BTN = Menu.FIRST + 9995;
	private static final int MENU_ITEM_ID_LIGHT = Menu.FIRST + 9996;
	
	public boolean onPrepareOptionsMenu( Menu menu )
	{
		MenuItem item;
		
		menu.clear();
		
		for(int i = 0; i < Globals.MENU_KEY_NUM; i ++){
			Integer sdlKey = (Integer)Globals.SDLKeyAdditionalKeyMap.get(new Integer(Globals.MENU_KEY_ARRAY[i]));
			if(sdlKey != null && sdlKey.intValue() != SDL_1_2_Keycodes.SDLK_UNKNOWN){
				String name = Globals.SDLKeyFunctionNameMap.get(sdlKey);
				if(name != null){
					menu.add(Menu.NONE, (MENU_ITEM_ID_USER_MENU_KEY_FIRST+i), Menu.NONE, name);
				}
			}
		}
		
		//if(Globals.SUBMENU_KEY_NUM > 0){
			SubMenu menu_userfunc = menu.addSubMenu(mActivity.getString(R.string.Menu_Function) + "...");
			for(int i = 0; i < Globals.SUBMENU_KEY_NUM; i ++){
				Integer sdlKey = (Integer)Globals.SDLKeyAdditionalKeyMap.get(new Integer(Globals.SUBMENU_KEY_ARRAY[i]));
				if(sdlKey != null && sdlKey.intValue() != SDL_1_2_Keycodes.SDLK_UNKNOWN){
					String name = Globals.SDLKeyFunctionNameMap.get(sdlKey);
					if(name != null){
						menu_userfunc.add(Menu.NONE, (MENU_ITEM_ID_USER_SUBMENU_KEY_FIRST+i), Menu.NONE, name);
					}
				}
			}
		//}
		
		SubMenu menu_touchmode = menu.addSubMenu(mActivity.getString(R.string.Menu_TouchMode) + "...");
		
		menu_touchmode.add(MENU_ITEM_ID_TOUCHMODE_FIRST, MENU_ITEM_ID_TOUCHMODE_INVALID, Menu.NONE, mActivity.getString(R.string.Invalid));
		if(Globals.MOUSE_USE){
			menu_touchmode.add(MENU_ITEM_ID_TOUCHMODE_FIRST, MENU_ITEM_ID_TOUCHMODE_TOUCH, Menu.NONE, mActivity.getString(R.string.Touch));
			menu_touchmode.add(MENU_ITEM_ID_TOUCHMODE_FIRST, MENU_ITEM_ID_TOUCHMODE_TRACKPAD, Menu.NONE, mActivity.getString(R.string.TrackPad));
		}
		menu_touchmode.add(MENU_ITEM_ID_TOUCHMODE_FIRST, MENU_ITEM_ID_TOUCHMODE_GAMEPAD, Menu.NONE, mActivity.getString(R.string.GamePad));
		
		menu_touchmode.setGroupCheckable(MENU_ITEM_ID_TOUCHMODE_FIRST, true, true);
		if(Locals.TouchMode.equals("Touch")){
			menu_touchmode.findItem(MENU_ITEM_ID_TOUCHMODE_TOUCH).setChecked(true);
		} else if(Locals.TouchMode.equals("TrackPad")){
			menu_touchmode.findItem(MENU_ITEM_ID_TOUCHMODE_TRACKPAD).setChecked(true);
		} else if(Locals.TouchMode.equals("GamePad")){
			menu_touchmode.findItem(MENU_ITEM_ID_TOUCHMODE_GAMEPAD).setChecked(true);
		} else {
			menu_touchmode.findItem(MENU_ITEM_ID_TOUCHMODE_INVALID).setChecked(true);
		}

		menu.add(Menu.NONE, (MENU_ITEM_ID_QUIT), Menu.NONE, mActivity.getString(R.string.Menu_Quit));

		if(mAutoSave)
		menu.add(Menu.NONE, Menu.FIRST + 9993, Menu.NONE, "自动存档-已开启");
		else
		menu.add(Menu.NONE, Menu.FIRST + 9994, Menu.NONE, "自动存档-已关闭");

		menu.add(Menu.NONE, (MENU_ITEM_ID_BTN), Menu.NONE, "虚拟按键");

		menu.add(Menu.NONE, (MENU_ITEM_ID_LIGHT), Menu.NONE, "亮度设置");
		
		//
		
		//menu.add(Menu.NONE, (MENU_ITEM_ID_ABOUT), Menu.NONE, "About");

		menu.add(Menu.NONE, MENU_ITEM_ID_SETTING_KEYCONFIG, Menu.NONE, mActivity.getString(R.string.Menu_KeyConfig));
		
		SubMenu menu_mousecursor = menu.addSubMenu(mActivity.getString(R.string.Menu_MouseCursor) + "...");
		menu_mousecursor.add(MENU_ITEM_ID_SETTING_MOUSECURSOR_FIRST, MENU_ITEM_ID_SETTING_MOUSECURSOR_SHOW, Menu.NONE, mActivity.getString(R.string.Show));
		menu_mousecursor.add(MENU_ITEM_ID_SETTING_MOUSECURSOR_FIRST, MENU_ITEM_ID_SETTING_MOUSECURSOR_HIDE, Menu.NONE, mActivity.getString(R.string.Hide));
		
		menu_mousecursor.setGroupCheckable(MENU_ITEM_ID_SETTING_MOUSECURSOR_FIRST, true, true);
		if(Globals.MouseCursorShowed){
			menu_mousecursor.findItem(MENU_ITEM_ID_SETTING_MOUSECURSOR_SHOW).setChecked(true);
		} else {
			menu_mousecursor.findItem(MENU_ITEM_ID_SETTING_MOUSECURSOR_HIDE).setChecked(true);
		}
		
		if(Globals.BUTTON_USE){
			SubMenu menu_btn_left = menu.addSubMenu(mActivity.getString(R.string.Menu_ButtonLeft) + "...");
			if(Locals.ButtonLeftEnabled){
				menu_btn_left.add(Menu.NONE, MENU_ITEM_ID_SETTING_BUTTON_LEFT_ENABLED_DISABLE, Menu.NONE, mActivity.getString(R.string.Disable));
				for(int i = 1; i <= Globals.BUTTON_LEFT_MAX; i ++){
					menu_btn_left.add(MENU_ITEM_ID_SETTING_BUTTON_LEFT_NUM_FIRST, (MENU_ITEM_ID_SETTING_BUTTON_LEFT_NUM_FIRST + i), Menu.NONE, "" + i + " " + mActivity.getString(R.string.unit));
				}
				menu_btn_left.setGroupCheckable(MENU_ITEM_ID_SETTING_BUTTON_LEFT_NUM_FIRST, true, true);
				menu_btn_left.findItem(MENU_ITEM_ID_SETTING_BUTTON_LEFT_NUM_FIRST + Globals.ButtonLeftNum).setChecked(true);
			} else {
				menu_btn_left.add(Menu.NONE, MENU_ITEM_ID_SETTING_BUTTON_LEFT_ENABLED_ENABLE, Menu.NONE, mActivity.getString(R.string.Enable));
			}
			
			SubMenu menu_btn_right = menu.addSubMenu(mActivity.getString(R.string.Menu_ButtonRight) + "...");
			if(Locals.ButtonRightEnabled){
				menu_btn_right.add(Menu.NONE, MENU_ITEM_ID_SETTING_BUTTON_RIGHT_ENABLED_DISABLE, Menu.NONE, mActivity.getString(R.string.Disable));
				for(int i = 1; i <= Globals.BUTTON_RIGHT_MAX; i ++){
					menu_btn_right.add(MENU_ITEM_ID_SETTING_BUTTON_RIGHT_NUM_FIRST, (MENU_ITEM_ID_SETTING_BUTTON_RIGHT_NUM_FIRST + i), Menu.NONE, "" + i + " " + mActivity.getString(R.string.unit));
				}
				menu_btn_right.setGroupCheckable(MENU_ITEM_ID_SETTING_BUTTON_RIGHT_NUM_FIRST, true, true);
				menu_btn_right.findItem(MENU_ITEM_ID_SETTING_BUTTON_RIGHT_NUM_FIRST + Globals.ButtonRightNum).setChecked(true);
			} else {
				menu_btn_right.add(Menu.NONE, MENU_ITEM_ID_SETTING_BUTTON_RIGHT_ENABLED_ENABLE, Menu.NONE, mActivity.getString(R.string.Enable));
			}
			
			SubMenu menu_btn_top = menu.addSubMenu(mActivity.getString(R.string.Menu_ButtonTop) + "...");
			if(Locals.ButtonTopEnabled){
				menu_btn_top.add(Menu.NONE, MENU_ITEM_ID_SETTING_BUTTON_TOP_ENABLED_DISABLE, Menu.NONE, mActivity.getString(R.string.Disable));
				for(int i = 1; i <= Globals.BUTTON_TOP_MAX; i ++){
					menu_btn_top.add(MENU_ITEM_ID_SETTING_BUTTON_TOP_NUM_FIRST, (MENU_ITEM_ID_SETTING_BUTTON_TOP_NUM_FIRST + i), Menu.NONE, "" + i + " " + mActivity.getString(R.string.unit));
				}
				menu_btn_top.setGroupCheckable(MENU_ITEM_ID_SETTING_BUTTON_TOP_NUM_FIRST, true, true);
				menu_btn_top.findItem(MENU_ITEM_ID_SETTING_BUTTON_TOP_NUM_FIRST + Globals.ButtonTopNum).setChecked(true);
			} else {
				menu_btn_top.add(Menu.NONE, MENU_ITEM_ID_SETTING_BUTTON_TOP_ENABLED_ENABLE, Menu.NONE, mActivity.getString(R.string.Enable));
			}
			
			SubMenu menu_btn_bottom = menu.addSubMenu(mActivity.getString(R.string.Menu_ButtonBottom) + "...");
			if(Locals.ButtonBottomEnabled){
				menu_btn_bottom.add(Menu.NONE, MENU_ITEM_ID_SETTING_BUTTON_BOTTOM_ENABLED_DISABLE, Menu.NONE, mActivity.getString(R.string.Disable));
				for(int i = 1; i <= Globals.BUTTON_BOTTOM_MAX; i ++){
					menu_btn_bottom.add(MENU_ITEM_ID_SETTING_BUTTON_BOTTOM_NUM_FIRST, (MENU_ITEM_ID_SETTING_BUTTON_BOTTOM_NUM_FIRST + i), Menu.NONE, "" + i + " " + mActivity.getString(R.string.unit));
				}
				menu_btn_bottom.setGroupCheckable(MENU_ITEM_ID_SETTING_BUTTON_BOTTOM_NUM_FIRST, true, true);
				menu_btn_bottom.findItem(MENU_ITEM_ID_SETTING_BUTTON_BOTTOM_NUM_FIRST + Globals.ButtonBottomNum).setChecked(true);
			} else {
				menu_btn_bottom.add(Menu.NONE, MENU_ITEM_ID_SETTING_BUTTON_BOTTOM_ENABLED_ENABLE, Menu.NONE, mActivity.getString(R.string.Enable));
			}
			
			SubMenu menu_video_xmargin = menu.addSubMenu(mActivity.getString(R.string.Menu_VideoXMargin) + "...");
			for(int i = 0; i <= (MENU_ITEM_ID_SETTING_VIDEO_XMARGIN_LAST - MENU_ITEM_ID_SETTING_VIDEO_XMARGIN_FIRST); i ++){
				menu_video_xmargin.add(MENU_ITEM_ID_SETTING_VIDEO_XMARGIN_FIRST, (MENU_ITEM_ID_SETTING_VIDEO_XMARGIN_FIRST + i), Menu.NONE, "" + i);
			}
			menu_video_xmargin.setGroupCheckable(MENU_ITEM_ID_SETTING_VIDEO_XMARGIN_FIRST, true, true);
			menu_video_xmargin.findItem(MENU_ITEM_ID_SETTING_VIDEO_XMARGIN_FIRST + Locals.VideoXMargin).setChecked(true);
			
			SubMenu menu_video_ymargin = menu.addSubMenu(mActivity.getString(R.string.Menu_VideoYMargin) + "...");
			for(int i = 0; i <= (MENU_ITEM_ID_SETTING_VIDEO_YMARGIN_LAST - MENU_ITEM_ID_SETTING_VIDEO_YMARGIN_FIRST); i ++){
				menu_video_ymargin.add(MENU_ITEM_ID_SETTING_VIDEO_YMARGIN_FIRST, (MENU_ITEM_ID_SETTING_VIDEO_YMARGIN_FIRST + i), Menu.NONE, "" + i);
			}
			menu_video_ymargin.setGroupCheckable(MENU_ITEM_ID_SETTING_VIDEO_YMARGIN_FIRST, true, true);
			menu_video_ymargin.findItem(MENU_ITEM_ID_SETTING_VIDEO_YMARGIN_FIRST + Locals.VideoYMargin).setChecked(true);
		}
		
		SubMenu menu_video_xpos = menu.addSubMenu(mActivity.getString(R.string.Menu_VideoXPosition) + "...");
		menu_video_xpos.add(MENU_ITEM_ID_SETTING_VIDEO_XPOSITION_FIRST, MENU_ITEM_ID_SETTING_VIDEO_XPOSITION_LEFT, Menu.NONE, mActivity.getString(R.string.Left));
		menu_video_xpos.add(MENU_ITEM_ID_SETTING_VIDEO_XPOSITION_FIRST, MENU_ITEM_ID_SETTING_VIDEO_XPOSITION_CENTER, Menu.NONE, mActivity.getString(R.string.Center));
		menu_video_xpos.add(MENU_ITEM_ID_SETTING_VIDEO_XPOSITION_FIRST, MENU_ITEM_ID_SETTING_VIDEO_XPOSITION_RIGHT, Menu.NONE, mActivity.getString(R.string.Right));
		
		menu_video_xpos.setGroupCheckable(MENU_ITEM_ID_SETTING_VIDEO_XPOSITION_FIRST, true, true);
		if(Locals.VideoXPosition < 0){
			menu_video_xpos.findItem(MENU_ITEM_ID_SETTING_VIDEO_XPOSITION_LEFT).setChecked(true);
		} else if(Locals.VideoXPosition == 0){
			menu_video_xpos.findItem(MENU_ITEM_ID_SETTING_VIDEO_XPOSITION_CENTER).setChecked(true);
		} else {
			menu_video_xpos.findItem(MENU_ITEM_ID_SETTING_VIDEO_XPOSITION_RIGHT).setChecked(true);
		}
		
		SubMenu menu_video_ypos = menu.addSubMenu(mActivity.getString(R.string.Menu_VideoYPosition) + "...");
		menu_video_ypos.add(MENU_ITEM_ID_SETTING_VIDEO_YPOSITION_FIRST, MENU_ITEM_ID_SETTING_VIDEO_YPOSITION_TOP, Menu.NONE, mActivity.getString(R.string.Top));
		menu_video_ypos.add(MENU_ITEM_ID_SETTING_VIDEO_YPOSITION_FIRST, MENU_ITEM_ID_SETTING_VIDEO_YPOSITION_CENTER, Menu.NONE, mActivity.getString(R.string.Center));
		menu_video_ypos.add(MENU_ITEM_ID_SETTING_VIDEO_YPOSITION_FIRST, MENU_ITEM_ID_SETTING_VIDEO_YPOSITION_BOTTOM, Menu.NONE, mActivity.getString(R.string.Bottom));
		
		menu_video_ypos.setGroupCheckable(MENU_ITEM_ID_SETTING_VIDEO_YPOSITION_FIRST, true, true);
		if(Locals.VideoYPosition < 0){
			menu_video_ypos.findItem(MENU_ITEM_ID_SETTING_VIDEO_YPOSITION_TOP).setChecked(true);
		} else if(Locals.VideoYPosition == 0){
			menu_video_ypos.findItem(MENU_ITEM_ID_SETTING_VIDEO_YPOSITION_CENTER).setChecked(true);
		} else {
			menu_video_ypos.findItem(MENU_ITEM_ID_SETTING_VIDEO_YPOSITION_BOTTOM).setChecked(true);
		}
		
		SubMenu menu_gamepad_position = menu.addSubMenu(mActivity.getString(R.string.Menu_GamePadPosition) + "...");
		menu_gamepad_position.add(MENU_ITEM_ID_SETTING_GAMEPAD_POSITION_FIRST, MENU_ITEM_ID_SETTING_GAMEPAD_POSITION_FIRST, Menu.NONE, "0 (" + mActivity.getString(R.string.Top) + ")");
		for(int i = 1; i < (MENU_ITEM_ID_SETTING_GAMEPAD_POSITION_LAST - MENU_ITEM_ID_SETTING_GAMEPAD_POSITION_FIRST); i ++){
			menu_gamepad_position.add(MENU_ITEM_ID_SETTING_GAMEPAD_POSITION_FIRST, (MENU_ITEM_ID_SETTING_GAMEPAD_POSITION_FIRST + i), Menu.NONE, "" + (i * 5));
		}
		menu_gamepad_position.add(MENU_ITEM_ID_SETTING_GAMEPAD_POSITION_FIRST, MENU_ITEM_ID_SETTING_GAMEPAD_POSITION_LAST, Menu.NONE, "100 (" + mActivity.getString(R.string.Bottom) + ")");
		
		menu_gamepad_position.setGroupCheckable(MENU_ITEM_ID_SETTING_GAMEPAD_POSITION_FIRST, true, true);
		menu_gamepad_position.findItem(MENU_ITEM_ID_SETTING_GAMEPAD_POSITION_FIRST + (Globals.GamePadPosition / 5)).setChecked(true);
		
		SubMenu menu_gamepad_size = menu.addSubMenu(mActivity.getString(R.string.Menu_GamePadSize) + "...");
		for(int i = 1; i <= (MENU_ITEM_ID_SETTING_GAMEPAD_SIZE_LAST - MENU_ITEM_ID_SETTING_GAMEPAD_SIZE_FIRST); i ++){
			menu_gamepad_size.add(MENU_ITEM_ID_SETTING_GAMEPAD_SIZE_FIRST, (MENU_ITEM_ID_SETTING_GAMEPAD_SIZE_FIRST + i), Menu.NONE, "" + (i * 5) + "%");
		}
		
		menu_gamepad_size.setGroupCheckable(MENU_ITEM_ID_SETTING_GAMEPAD_SIZE_FIRST, true, true);
		menu_gamepad_size.findItem(MENU_ITEM_ID_SETTING_GAMEPAD_SIZE_FIRST + (Globals.GamePadSize / 5)).setChecked(true);
		
		SubMenu menu_gamepad_opacity = menu.addSubMenu(mActivity.getString(R.string.Menu_GamePadOpacity) + "...");
		for(int i = 0; i <= (MENU_ITEM_ID_SETTING_GAMEPAD_OPACITY_LAST - MENU_ITEM_ID_SETTING_GAMEPAD_OPACITY_FIRST); i ++){
			menu_gamepad_opacity.add(MENU_ITEM_ID_SETTING_GAMEPAD_OPACITY_FIRST, (MENU_ITEM_ID_SETTING_GAMEPAD_OPACITY_FIRST + i), Menu.NONE, "" + (i * 5) + "%");
		}
		
		menu_gamepad_opacity.setGroupCheckable(MENU_ITEM_ID_SETTING_GAMEPAD_OPACITY_FIRST, true, true);
		menu_gamepad_opacity.findItem(MENU_ITEM_ID_SETTING_GAMEPAD_OPACITY_FIRST + (Globals.GamePadOpacity / 5)).setChecked(true);
		
		SubMenu menu_gamepad_arrow_button = menu.addSubMenu(mActivity.getString(R.string.Menu_GamePadArrowButton) + "...");
		menu_gamepad_arrow_button.add(MENU_ITEM_ID_SETTING_GAMEPAD_ARROW_BUTTON_FIRST, MENU_ITEM_ID_SETTING_GAMEPAD_ARROW_BUTTON_AS_AXIS, Menu.NONE, mActivity.getString(R.string.AsAxis));
		menu_gamepad_arrow_button.add(MENU_ITEM_ID_SETTING_GAMEPAD_ARROW_BUTTON_FIRST, MENU_ITEM_ID_SETTING_GAMEPAD_ARROW_BUTTON_AS_BUTTON, Menu.NONE, mActivity.getString(R.string.AsButton));
		
		menu_gamepad_arrow_button.setGroupCheckable(MENU_ITEM_ID_SETTING_GAMEPAD_ARROW_BUTTON_FIRST, true, true);
		if(Globals.GamePadArrowButtonAsAxis){
			menu_gamepad_arrow_button.findItem(MENU_ITEM_ID_SETTING_GAMEPAD_ARROW_BUTTON_AS_AXIS).setChecked(true);
		} else {
			menu_gamepad_arrow_button.findItem(MENU_ITEM_ID_SETTING_GAMEPAD_ARROW_BUTTON_AS_BUTTON).setChecked(true);
		}
		
		SubMenu menu_gamepad_action_button = menu.addSubMenu(mActivity.getString(R.string.Menu_GamePadActionButton) + "...");
		menu_gamepad_action_button.add(MENU_ITEM_ID_SETTING_GAMEPAD_ACTION_BUTTON_FIRST, MENU_ITEM_ID_SETTING_GAMEPAD_ACTION_BUTTON_AS_AXIS, Menu.NONE, mActivity.getString(R.string.AsAxis));
		menu_gamepad_action_button.add(MENU_ITEM_ID_SETTING_GAMEPAD_ACTION_BUTTON_FIRST, MENU_ITEM_ID_SETTING_GAMEPAD_ACTION_BUTTON_AS_BUTTON, Menu.NONE, mActivity.getString(R.string.AsButton));
		
		menu_gamepad_action_button.setGroupCheckable(MENU_ITEM_ID_SETTING_GAMEPAD_ACTION_BUTTON_FIRST, true, true);
		if(Globals.GamePadActionButtonAsAxis){
			menu_gamepad_action_button.findItem(MENU_ITEM_ID_SETTING_GAMEPAD_ACTION_BUTTON_AS_AXIS).setChecked(true);
		} else {
			menu_gamepad_action_button.findItem(MENU_ITEM_ID_SETTING_GAMEPAD_ACTION_BUTTON_AS_BUTTON).setChecked(true);
		}

		//SubMenu menu_appconfig = menu.addSubMenu(mActivity.getString(R.string.Menu_AppLaunchConfig) + "...");
		//menu_appconfig.add(MENU_ITEM_ID_SETTING_APPCONFIG_FIRST, MENU_ITEM_ID_SETTING_APPCONFIG_USE, Menu.NONE, mActivity.getString(R.string.Use));
		//menu_appconfig.add(MENU_ITEM_ID_SETTING_APPCONFIG_FIRST, MENU_ITEM_ID_SETTING_APPCONFIG_NOTUSE, Menu.NONE, mActivity.getString(R.string.NotUse));
		
		/*menu_appconfig.setGroupCheckable(MENU_ITEM_ID_SETTING_APPCONFIG_FIRST, true, true);
		if(Locals.AppLaunchConfigUse){
			menu_appconfig.findItem(MENU_ITEM_ID_SETTING_APPCONFIG_USE).setChecked(true);
		} else {
			menu_appconfig.findItem(MENU_ITEM_ID_SETTING_APPCONFIG_NOTUSE).setChecked(true);
		}*/

		return true;
	}
	
	public boolean onOptionsItemSelected( MenuItem item )
	{
		int d = item.getItemId();
		
		//Log.i("MainView","onOptionsItemSelected : " + d);
		
		if(d >= (MENU_ITEM_ID_USER_MENU_KEY_FIRST) && d <= (MENU_ITEM_ID_USER_MENU_KEY_LAST)){
			int index = d - MENU_ITEM_ID_USER_MENU_KEY_FIRST;
			int key = Globals.MENU_KEY_ARRAY[index];
			nativeKey( key, 1 );
			nativeKey( key, 0 );
		} else if(d >= (MENU_ITEM_ID_USER_SUBMENU_KEY_FIRST) && d <= (MENU_ITEM_ID_USER_SUBMENU_KEY_LAST)){
			int index = d - MENU_ITEM_ID_USER_SUBMENU_KEY_FIRST;
			int key = Globals.SUBMENU_KEY_ARRAY[index];
			ShowToast(String.valueOf(key));
			nativeKey( key, 1 );
			nativeKey( key, 0 );
		} else if(d == MENU_ITEM_ID_TOUCHMODE_INVALID){
			if(mTouchMode != null){
				mTouchMode.cleanup();
			}
			Locals.TouchMode = "Invalid";
			Settings.SaveLocals(mActivity);
			mTouchMode = TouchMode.getTouchMode(Locals.TouchMode, this);
			mTouchMode.setup();
			mTouchMode.update();
			mTouchInput.setOnInputEventListener(mTouchMode);
		} else if(d == MENU_ITEM_ID_TOUCHMODE_TOUCH){
			if(mTouchMode != null){
				mTouchMode.cleanup();
			}
			Locals.TouchMode = "Touch";
			Settings.SaveLocals(mActivity);
			mTouchMode = TouchMode.getTouchMode(Locals.TouchMode, this);
			mTouchMode.setup();
			mTouchMode.update();
			mTouchInput.setOnInputEventListener(mTouchMode);
		} else if(d == MENU_ITEM_ID_TOUCHMODE_TRACKPAD){
			if(mTouchMode != null){
				mTouchMode.cleanup();
			}
			Locals.TouchMode = "TrackPad";
			Settings.SaveLocals(mActivity);
			mTouchMode = TouchMode.getTouchMode(Locals.TouchMode, this);
			mTouchMode.setup();
			mTouchMode.update();
			mTouchInput.setOnInputEventListener(mTouchMode);
		} else if(d == MENU_ITEM_ID_TOUCHMODE_GAMEPAD){
			if(mTouchMode != null){
				mTouchMode.cleanup();
			}
			Locals.TouchMode = "GamePad";
			Settings.SaveLocals(mActivity);
			mTouchMode = TouchMode.getTouchMode(Locals.TouchMode, this);
			mTouchMode.setup();
			mTouchMode.update();
			mTouchInput.setOnInputEventListener(mTouchMode);
		} else if(d == MENU_ITEM_ID_SETTING_KEYCONFIG){
			enterKeyConfigMode();
		} else if(d == MENU_ITEM_ID_SETTING_MOUSECURSOR_SHOW){
			Globals.MouseCursorShowed = true;
			Settings.SaveGlobals(mActivity);
			showMouseCursor(Globals.MouseCursorShowed);
		} else if(d == MENU_ITEM_ID_SETTING_MOUSECURSOR_HIDE){
			Globals.MouseCursorShowed = false;
			Settings.SaveGlobals(mActivity);
			showMouseCursor(Globals.MouseCursorShowed);
		} else if(d == MENU_ITEM_ID_SETTING_VIDEO_XPOSITION_LEFT){
			Locals.VideoXPosition = -1;
			Settings.SaveLocals(mActivity);
			update();
		} else if(d == MENU_ITEM_ID_SETTING_VIDEO_XPOSITION_CENTER){
			Locals.VideoXPosition = 0;
			Settings.SaveLocals(mActivity);
			update();
		} else if(d == MENU_ITEM_ID_SETTING_VIDEO_XPOSITION_RIGHT){
			Locals.VideoXPosition = 1;
			Settings.SaveLocals(mActivity);
			update();
		} else if(d == MENU_ITEM_ID_SETTING_VIDEO_YPOSITION_TOP){
			Locals.VideoYPosition = -1;
			Settings.SaveLocals(mActivity);
			update();
		} else if(d == MENU_ITEM_ID_SETTING_VIDEO_YPOSITION_CENTER){
			Locals.VideoYPosition = 0;
			Settings.SaveLocals(mActivity);
			update();
		} else if(d == MENU_ITEM_ID_SETTING_VIDEO_YPOSITION_BOTTOM){
			Locals.VideoYPosition = 1;
			Settings.SaveLocals(mActivity);
			update();
		} else if(d >= MENU_ITEM_ID_SETTING_VIDEO_XMARGIN_FIRST && d <= MENU_ITEM_ID_SETTING_VIDEO_XMARGIN_LAST){
			Locals.VideoXMargin = d - MENU_ITEM_ID_SETTING_VIDEO_XMARGIN_FIRST;
			Settings.SaveLocals(mActivity);
			update();
		} else if(d >= MENU_ITEM_ID_SETTING_VIDEO_YMARGIN_FIRST && d <= MENU_ITEM_ID_SETTING_VIDEO_YMARGIN_LAST){
			Locals.VideoYMargin = d - MENU_ITEM_ID_SETTING_VIDEO_YMARGIN_FIRST;
			Settings.SaveLocals(mActivity);
			update();
		} else if(d == MENU_ITEM_ID_SETTING_BUTTON_LEFT_ENABLED_ENABLE){
			Locals.ButtonLeftEnabled = true;
			Settings.SaveGlobals(mActivity);
			Settings.SaveLocals(mActivity);
			update();
		} else if(d == MENU_ITEM_ID_SETTING_BUTTON_LEFT_ENABLED_DISABLE){
			Locals.ButtonLeftEnabled = false;
			Settings.SaveGlobals(mActivity);
			Settings.SaveLocals(mActivity);
			update();
		} else if(d >= MENU_ITEM_ID_SETTING_BUTTON_LEFT_NUM_FIRST && d <= MENU_ITEM_ID_SETTING_BUTTON_LEFT_NUM_LAST){
			Globals.ButtonLeftNum = d - MENU_ITEM_ID_SETTING_BUTTON_LEFT_NUM_FIRST;
			Settings.SaveGlobals(mActivity);
			update();
		} else if(d == MENU_ITEM_ID_SETTING_BUTTON_RIGHT_ENABLED_ENABLE){
			Locals.ButtonRightEnabled = true;
			Settings.SaveGlobals(mActivity);
			Settings.SaveLocals(mActivity);
			update();
		} else if(d == MENU_ITEM_ID_SETTING_BUTTON_RIGHT_ENABLED_DISABLE){
			Locals.ButtonRightEnabled = false;
			Settings.SaveGlobals(mActivity);
			Settings.SaveLocals(mActivity);
			update();
		} else if(d >= MENU_ITEM_ID_SETTING_BUTTON_RIGHT_NUM_FIRST && d <= MENU_ITEM_ID_SETTING_BUTTON_RIGHT_NUM_LAST){
			Globals.ButtonRightNum = d - MENU_ITEM_ID_SETTING_BUTTON_RIGHT_NUM_FIRST;
			Settings.SaveGlobals(mActivity);
			update();
		} else if(d == MENU_ITEM_ID_SETTING_BUTTON_TOP_ENABLED_ENABLE){
			Locals.ButtonTopEnabled = true;
			Settings.SaveGlobals(mActivity);
			Settings.SaveLocals(mActivity);
			update();
		} else if(d == MENU_ITEM_ID_SETTING_BUTTON_TOP_ENABLED_DISABLE){
			Locals.ButtonTopEnabled = false;
			Settings.SaveGlobals(mActivity);
			Settings.SaveLocals(mActivity);
			update();
		} else if(d >= MENU_ITEM_ID_SETTING_BUTTON_TOP_NUM_FIRST && d <= MENU_ITEM_ID_SETTING_BUTTON_TOP_NUM_LAST){
			Globals.ButtonTopNum = d - MENU_ITEM_ID_SETTING_BUTTON_TOP_NUM_FIRST;
			Settings.SaveGlobals(mActivity);
			update();
		} else if(d == MENU_ITEM_ID_SETTING_BUTTON_BOTTOM_ENABLED_ENABLE){
			Locals.ButtonBottomEnabled = true;
			Settings.SaveGlobals(mActivity);
			Settings.SaveLocals(mActivity);
			update();
		} else if(d == MENU_ITEM_ID_SETTING_BUTTON_BOTTOM_ENABLED_DISABLE){
			Locals.ButtonBottomEnabled = false;
			Settings.SaveGlobals(mActivity);
			Settings.SaveLocals(mActivity);
			update();
		} else if(d >= MENU_ITEM_ID_SETTING_BUTTON_BOTTOM_NUM_FIRST && d <= MENU_ITEM_ID_SETTING_BUTTON_BOTTOM_NUM_LAST){
			Globals.ButtonBottomNum = d - MENU_ITEM_ID_SETTING_BUTTON_BOTTOM_NUM_FIRST;
			Settings.SaveGlobals(mActivity);
			update();
		} else if(d >= MENU_ITEM_ID_SETTING_GAMEPAD_POSITION_FIRST && d <= MENU_ITEM_ID_SETTING_GAMEPAD_POSITION_LAST){
			Globals.GamePadPosition = (d - MENU_ITEM_ID_SETTING_GAMEPAD_POSITION_FIRST) * 5;
			Settings.SaveGlobals(mActivity);
			update();
		} else if(d >= MENU_ITEM_ID_SETTING_GAMEPAD_SIZE_FIRST && d <= MENU_ITEM_ID_SETTING_GAMEPAD_SIZE_LAST){
			Globals.GamePadSize = (d - MENU_ITEM_ID_SETTING_GAMEPAD_SIZE_FIRST) * 5;
			Settings.SaveGlobals(mActivity);
			update();
		} else if(d >= MENU_ITEM_ID_SETTING_GAMEPAD_OPACITY_FIRST && d <= MENU_ITEM_ID_SETTING_GAMEPAD_OPACITY_LAST){
			Globals.GamePadOpacity = (d - MENU_ITEM_ID_SETTING_GAMEPAD_OPACITY_FIRST) * 5;
			Settings.SaveGlobals(mActivity);
			update();
		} else if(d == MENU_ITEM_ID_SETTING_GAMEPAD_ARROW_BUTTON_AS_AXIS){
			Globals.GamePadArrowButtonAsAxis = true;
			Settings.SaveGlobals(mActivity);
			update();
		} else if(d == MENU_ITEM_ID_SETTING_GAMEPAD_ARROW_BUTTON_AS_BUTTON){
			Globals.GamePadArrowButtonAsAxis = false;
			Settings.SaveGlobals(mActivity);
			update();
		} else if(d == MENU_ITEM_ID_SETTING_GAMEPAD_ACTION_BUTTON_AS_AXIS){
			Globals.GamePadActionButtonAsAxis = true;
			Settings.SaveGlobals(mActivity);
			update();
		} else if(d == MENU_ITEM_ID_SETTING_GAMEPAD_ACTION_BUTTON_AS_BUTTON){
			Globals.GamePadActionButtonAsAxis = false;
			Settings.SaveGlobals(mActivity);
			update();
		} else if(d == MENU_ITEM_ID_SETTING_APPCONFIG_USE){
			Locals.AppLaunchConfigUse = true;
			Settings.SaveLocals(mActivity);
		} else if(d == MENU_ITEM_ID_SETTING_APPCONFIG_NOTUSE){
			Locals.AppLaunchConfigUse = false;
			Settings.SaveLocals(mActivity);
		} else if(d == MENU_ITEM_ID_ABOUT){
			//
		} else if(d == MENU_ITEM_ID_QUIT){
			AlertDialog.Builder alertDialogBuilder = new AlertDialog.Builder(getActivity());
			alertDialogBuilder.setTitle(mActivity.getString(R.string.Menu_AskQuit));
			alertDialogBuilder.setPositiveButton(mActivity.getString(R.string.Yes), new DialogInterface.OnClickListener(){
				public void onClick(DialogInterface dialog, int whichButton) {
					exitApp();
				}
			});
			alertDialogBuilder.setNegativeButton(mActivity.getString(R.string.No), null);
			alertDialogBuilder.setCancelable(true);
			AlertDialog alertDialog = alertDialogBuilder.create();
			alertDialog.show();
		}
		else if(d == Menu.FIRST + 9993){
			mAutoSave = false;
			nativeKey(419, 1);
			nativeKey(419, 0);
			ShowToast("自动存档已关闭");
		} 
		else if(d == Menu.FIRST + 9994){
			mAutoSave = true;
			nativeKey(419, 1);
			nativeKey(419, 0);
			ShowToast("自动存档已开启，3分钟存储一次，默认存储第一个档位。不支持自定义菜单的游戏。");
		} 
		else if(d == MENU_ITEM_ID_BTN){
			mActivity.BtnCancel();
		}
		else if(d == MENU_ITEM_ID_LIGHT){
			mActivity.LightDialog();
		}
		return true;
	}
	
	//

	public void ShowToast(String ToastText){
		Toast.makeText(mActivity, ToastText, Toast.LENGTH_SHORT).show();
	}
	
	public int getMousePointX()
	{
		return mMousePointX;
	}
	
	public int getMousePointY()
	{
		return mMousePointY;
	}
	
	public void setMousePoint(int x, int y)
	{
		mMousePointX = x;
		mMousePointY = y;
		
		updateMouseCursor();
	}
	
	public void setMousePointForNative(int x, int y) //call from native
	{
		class Callback implements Runnable
		{
			public MainView v;
			public int x;
			public int y;
			public void run()
			{
				v.setMousePoint(x, y);
			}
		}
		Callback cb = new Callback();
		cb.v = this;
		cb.x = x;
		cb.y = y;
		if(mActivity != null)
			mActivity.runOnUiThread(cb);
	}

	public boolean isMouseCursorShowed()
	{
		return (mMouseCursor.getVisibility() == View.VISIBLE);
	}
	
	public void showMouseCursor(boolean show)
	{
		if(show){
			mMouseCursor.setVisibility(View.VISIBLE);
		} else {
			mMouseCursor.setVisibility(View.GONE);
		}
	}
	
	public void setMouseCursorRGB(int fillColorR, int fillColorG, int fillColorB, int strokeColorR, int strokeColorG, int strokeColorB)
	{
		mMouseCursor.setMouseCursorRGB(fillColorR, fillColorG, fillColorB, strokeColorR, strokeColorG, strokeColorB);
	}
	
	public void updateMouseCursor()
	{
		mMouseCursor.setLayoutParams(new AbsoluteLayout.LayoutParams(MouseCursor.WIDTH, MouseCursor.HEIGHT, mGLViewX + mMousePointX, mGLViewY + mMousePointY));
		//mMouseCursor.invalidate();
	}
	
	public void update()
	{
		mTouchMode.update();
	}

	//
	
	public int getGLViewX()
	{
		return mGLViewX;
	}
	
	public int getGLViewY()
	{
		return mGLViewY;
	}
	
	public int getGLViewWidth()
	{
		return mGLViewWidth;
	}
	
	public int getGLViewHeight()
	{
		return mGLViewHeight;
	}
	
	public void setGLViewPos(int x, int y)
	{
		setGLViewRect(x, y, -1, -1);
	}
	
	public void setGLViewRect(int x, int y, int w, int h)
	{
		if(mGLView != null){
			if(w < 0){
				w = mGLViewWidth;
			}
			if(h < 0){
				h = mGLViewHeight;
			}
			
			if(mGLViewWidth > 0 && mGLViewHeight > 0){
				mMousePointX = mMousePointX * w / mGLViewWidth;
				mMousePointY = mMousePointY * h / mGLViewHeight;
			}
			
			mGLViewX = x;
			mGLViewY = y;
			mGLViewWidth  = w;
			mGLViewHeight = h;
			mGLView.setLayoutParams(new AbsoluteLayout.LayoutParams(mGLViewWidth, mGLViewHeight, mGLViewX, mGLViewY));
			
			updateMouseCursor();
			
			//Log.i("MainView", "mGLView : x,y,w,h=" + mGLViewX + "," + mGLViewY + "," + mGLViewWidth + "," + mGLViewHeight);
		}
	}
	
	public void setScreenOrientation(int orientation)
	{
		mActivity.setRequestedOrientation(orientation);
		/*
		if(orientation == ActivityInfo.SCREEN_ORIENTATION_PORTRAIT){
			mActivity.setRequestedOrientation( ActivityInfo.SCREEN_ORIENTATION_REVERSE_PORTRAIT );
		} else if(orientation == ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE){
			mActivity.setRequestedOrientation( ActivityInfo.SCREEN_ORIENTATION_REVERSE_LANDSCAPE );
		} else {
			if( android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.GINGERBREAD ){
				if( ActivityInfo.SCREEN_ORIENTATION_REVERSE_PORTRAIT ){
					mActivity.setRequestedOrientation( ActivityInfo.SCREEN_ORIENTATION_REVERSE_PORTRAIT );
				} else {
					mActivity.setRequestedOrientation( ActivityInfo.SCREEN_ORIENTATION_REVERSE_LANDSCAPE );
				}
			} else {
				mActivity.setRequestedOrientation( ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE );
			}
		}
		 */

		Display disp = ((WindowManager)mActivity.getSystemService(Context.WINDOW_SERVICE)).getDefaultDisplay();
		mDisplayWidth  = disp.getWidth();
		mDisplayHeight = disp.getHeight();
	}
	
	public int getDisplayWidth()
	{
		return mDisplayWidth;
	}
	
	public int getDisplayHeight()
	{
		return mDisplayHeight;
	}
	
	private class OnClickListenerForKeyConfigDialog implements DialogInterface.OnClickListener
	{
		private MainView mMainView;
		private int   mJavaKeyCode;
		private int[] mSdlKeyArray;
		private String[] mSdlKeyFunctionNameArray;
		
		public OnClickListenerForKeyConfigDialog(MainView mainView, int javaKeyCode, int[] sdlKeyArray, String[] sdlKeyFunctionNameArray)
		{
			mMainView = mainView;
			mJavaKeyCode = javaKeyCode;
			mSdlKeyArray = sdlKeyArray;
			mSdlKeyFunctionNameArray = sdlKeyFunctionNameArray;
		}
		
		public void onClick(DialogInterface dialog, int whichItem)
		{
			Settings.setKeymapKey(mJavaKeyCode, mSdlKeyArray[whichItem]);
			mMainView.update();
			Toast.makeText(getActivity(), "Set:" + mSdlKeyFunctionNameArray[whichItem], Toast.LENGTH_LONG).show();
		}
	}

	public int nativeKey( int keyCode, int down )
	{
		if(!mIsKeyConfigMode){
			return DemoGLSurfaceView.nativeKey( keyCode, down );
		} else if(down == 0){
			int length = Globals.SDLKeyFunctionNameMap.size();
			int[] keyArray = new int[length];
			String[] nameArray = new String[length];
			Iterator ite = Globals.SDLKeyFunctionNameMap.keySet().iterator();
			for (int i = 0; i < length && ite.hasNext(); i ++) {
				Integer key = (Integer)ite.next();
				keyArray[i] = key.intValue();
				nameArray[i] = Globals.SDLKeyFunctionNameMap.get(key);
			}
			
			AlertDialog.Builder alertDialogBuilder = new AlertDialog.Builder(getActivity());
			alertDialogBuilder.setTitle(mActivity.getString(R.string.Menu_SelectKeyFunction) + " : " + keyCode);
			alertDialogBuilder.setItems(nameArray, new OnClickListenerForKeyConfigDialog(this, keyCode, keyArray, nameArray));
			alertDialogBuilder.setCancelable(true);
			alertDialogBuilder.setNegativeButton(mActivity.getString(R.string.Cancel), null);
			AlertDialog alertDialog = alertDialogBuilder.create();
			alertDialog.show();
		}
		return 0;
	}
	
	public void nativeMotionEvent( int x, int y )
	{
		if(!mIsKeyConfigMode){
			DemoGLSurfaceView.nativeMotionEvent( x, y );
		}
	}
	
	public void nativeMouseButtonsPressed( int buttonId, int pressedState )
	{
		if(!mIsKeyConfigMode){
			DemoGLSurfaceView.nativeMouseButtonsPressed( buttonId, pressedState );
		}
	}
	
	public ONSView getActivity()
	{
		return mActivity;
	}
	
	public void enterKeyConfigMode()
	{
		if(mIsKeyConfigMode){
			return;
		}
		
		mIsKeyConfigMode = true;
		
		mKeyConfigTextView = new TextView(getActivity());
		mKeyConfigTextView.setText(mActivity.getString(R.string.Menu_KeyConfig) + " (" + mActivity.getString(R.string.Menu_EndMenuButton) + ")");
		mKeyConfigTextView.setTextColor(Color.WHITE);
		mKeyConfigTextView.setBackgroundColor(Color.BLACK);
		mKeyConfigTextView.setOnClickListener(new OnClickListener() {
			
			public void onClick(View v) {
				// TODO Auto-generated method stub
				leaveKeyConfigMode();
			}
		});
		
		addView(mKeyConfigTextView, new AbsoluteLayout.LayoutParams(mGLViewWidth, AbsoluteLayout.LayoutParams.WRAP_CONTENT, mGLViewX, mGLViewY));
		
		Toast.makeText(getActivity(), mActivity.getString(R.string.Menu_PressKeyOrButton), Toast.LENGTH_LONG).show();
	}
	
	public void leaveKeyConfigMode()
	{
		if(!mIsKeyConfigMode){
			return;
		}
		if(mKeyConfigTextView != null){
			removeView(mKeyConfigTextView);
			mKeyConfigTextView = null;
		}
		
		mIsKeyConfigMode = false;
		
		Settings.SaveGlobals(mActivity);
	}
	
	//
	private boolean _isPaused = false;
	private boolean mAutoSave = false;
	
	private AudioThread mAudioThread = null;
	private DemoGLSurfaceView mGLView = null;
	
	private int mGLViewX = 0;
	private int mGLViewY = 0;
	private int mGLViewWidth  = 0;
	private int mGLViewHeight = 0;
	
	private ONSView mActivity = null;
	private DifferentTouchInput mTouchInput = null;
	private TouchMode mTouchMode = null;
	
	private MouseCursor mMouseCursor = null;
	private int mMousePointX = 0;
	private int mMousePointY = 0;
	
	private int mDisplayWidth  = 0;
	private int mDisplayHeight = 0;
	
	private boolean mIsKeyConfigMode = false;
	private TextView mKeyConfigTextView = null;


	public native void nativeInitInputJavaCallbacks();
}

// *** HONEYCOMB / ICS FIX FOR FULLSCREEN MODE, by lmak ***
abstract class DimSystemStatusBar
{
	public static DimSystemStatusBar get()
	{
		if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.HONEYCOMB)
			return DimSystemStatusBarHoneycomb.Holder.sInstance;
		else
			return DimSystemStatusBarDummy.Holder.sInstance;
	}
	public abstract void dim(final View view);
	
	private static class DimSystemStatusBarHoneycomb extends DimSystemStatusBar
	{
		private static class Holder
		{
			private static final DimSystemStatusBarHoneycomb sInstance = new DimSystemStatusBarHoneycomb();
		}
	    public void dim(final View view)
	    {
			/*
	         if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.ICE_CREAM_SANDWICH) {
			 // ICS has the same constant redefined with a different name.
			 hiddenStatusCode = android.view.View.SYSTEM_UI_FLAG_LOW_PROFILE;
	         }
	         */
			view.setSystemUiVisibility(android.view.View.STATUS_BAR_HIDDEN);
		}
	}
	private static class DimSystemStatusBarDummy extends DimSystemStatusBar
	{
		private static class Holder
		{
			private static final DimSystemStatusBarDummy sInstance = new DimSystemStatusBarDummy();
		}
		public void dim(final View view)
		{
		}
	}
}
