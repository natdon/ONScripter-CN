/*
 2012/7 Created by AKIZUKI Katane
 */

package cn.natdon.onscripterv2;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Path;
import android.graphics.drawable.Drawable;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.View;
import android.widget.AbsoluteLayout;
import android.widget.AbsoluteLayout.LayoutParams;
import android.widget.Button;
import android.widget.HorizontalScrollView;
import android.widget.LinearLayout;
import android.widget.ScrollView;
import cn.natdon.onscripterv2.Button.OkCancelButton;


class TouchMode implements DifferentTouchInput.OnInputEventListener
{
	public static TouchMode getTouchMode(String name, MainView mainView)
	{
		if(name.equals("Touch")){
			return new TouchTouchMode(mainView);
		}
		if(name.equals("TrackPad")){
			return new TrackPadTouchMode(mainView);
		}
		if(name.equals("GamePad")){
			return new GamePadTouchMode(mainView);
		}
		return new InvalidTouchMode(mainView);
	}
	
	private LinearLayout mLayoutLeft = null;
	private Button[] mButtonLeftArray = null;
	private ScrollView SVLeft = null;
	private LinearLayout SlayoutLeft = null;
	
	private LinearLayout mLayoutRight = null;
	private Button[] mButtonRightArray = null;
	private ScrollView SVRight = null;
	private LinearLayout SlayoutRight = null;
	
	private LinearLayout mLayoutTop = null;
	private Button[] mButtonTopArray = null;
	private HorizontalScrollView SVTop = null;
	private LinearLayout SlayoutTop = null;
	
	private LinearLayout mLayoutBottom = null;
	private Button[] mButtonBottomArray = null;
	private HorizontalScrollView SVBottom = null;
	private LinearLayout SlayoutBottom = null;

	
	private int mXMargin = 0;
	private int mYMargin = 0;
	
	private int mScreenX = 0;
	private int mScreenY = 0;
	private int mScreenWidth  = 0;
	private int mScreenHeight = 0;

	private MainView mMainView = null;
	
	private ONSVariable onsVariable;

	
	private class OnButtonTouchListener implements View.OnTouchListener
	{
		//private MainView mMainView;
		private int mKey;
		
		public OnButtonTouchListener(MainView mainView, int key)
		{
			//mMainView = mainView;
			mKey = key;
		}
		
		public boolean onTouch(View v, MotionEvent event){
			v.onTouchEvent(event);
			switch(event.getAction()){
				case MotionEvent.ACTION_DOWN:
					mMainView.nativeKey( mKey, 1 );
					break;
				case MotionEvent.ACTION_UP:
					mMainView.nativeKey( mKey, 0 );
					
					Integer sdlKey = (Integer)Globals.SDLKeyAdditionalKeyMap.get(new Integer(mKey));
					if(sdlKey != null){
						
						if(Globals.SDLKeyFunctionNameMap.get(sdlKey).equals(mMainView.getResources().getString(R.string.button_get_default_screenshot)))
							mMainView.ShowToast("截图成功,保存在"+Globals.CurrentDirectoryPath);
					}
					
					break;
			}
			return true;
		}
	}
	
	@SuppressWarnings("deprecation")
	private Button newButtonToLayout(LinearLayout layout, int key)
	{
		Drawable d = Drawable.createFromPath(Globals.CurrentDirectoryPathForLauncher+"/btn.png");

		Button button = new Button(mMainView.getActivity());
		if(d != null)
		button.setBackgroundDrawable(d);
		button.setHeight(onsVariable.dh/4);
		button.setWidth(onsVariable.dw/4);
		button.setOnTouchListener(new OnButtonTouchListener(mMainView, key));
		layout.addView(button, new LinearLayout.LayoutParams( LinearLayout.LayoutParams.MATCH_PARENT, LinearLayout.LayoutParams.MATCH_PARENT, 1));
		return button;
	}
	
	public TouchMode(MainView mainView)
	{
		mMainView = mainView;

		SVLeft = new ScrollView(mMainView.getActivity());
		SlayoutLeft = new LinearLayout(mMainView.getActivity());
		mLayoutLeft = new LinearLayout(mMainView.getActivity());
		mLayoutLeft.setOrientation(LinearLayout.VERTICAL);
		SlayoutLeft.setOrientation(LinearLayout.VERTICAL);
		mButtonLeftArray = new Button[Globals.BUTTON_LEFT_MAX];
		for(int i = 0; i < mButtonLeftArray.length; i ++){
			mButtonLeftArray[i] = newButtonToLayout(mLayoutLeft, Globals.BUTTON_LEFT_KEY_ARRAY[i]);
			if(Locals.gWindowScreen)
				mButtonLeftArray[i].setVisibility(View.GONE);
		}

		SVRight = new ScrollView(mMainView.getActivity());
		SlayoutRight = new LinearLayout(mMainView.getActivity());
		mLayoutRight = new LinearLayout(mMainView.getActivity());
		mLayoutRight.setOrientation(LinearLayout.VERTICAL);
		SlayoutRight.setOrientation(LinearLayout.VERTICAL);
		mButtonRightArray = new Button[Globals.BUTTON_RIGHT_MAX];
		for(int i = 0; i < mButtonRightArray.length; i ++){
			mButtonRightArray[i] = newButtonToLayout(mLayoutRight, Globals.BUTTON_RIGHT_KEY_ARRAY[i]);
			if(Locals.gWindowScreen)
				mButtonRightArray[i].setVisibility(View.GONE);
		}
		
		SVTop = new HorizontalScrollView(mMainView.getActivity());
		SlayoutTop = new LinearLayout(mMainView.getActivity());
		mLayoutTop = new LinearLayout(mMainView.getActivity());
		mLayoutTop.setOrientation(LinearLayout.HORIZONTAL);
		SlayoutTop.setOrientation(LinearLayout.HORIZONTAL);
		mButtonTopArray = new Button[Globals.BUTTON_BOTTOM_MAX];
		for(int i = 0; i < mButtonTopArray.length; i ++){
			mButtonTopArray[i] = newButtonToLayout(mLayoutTop, Globals.BUTTON_TOP_KEY_ARRAY[i]);
			if(Locals.gWindowScreen)
				mButtonTopArray[i].setVisibility(View.GONE);
		}
		
		SVBottom = new HorizontalScrollView(mMainView.getActivity());
		SlayoutBottom = new LinearLayout(mMainView.getActivity());
		mLayoutBottom = new LinearLayout(mMainView.getActivity());
		mLayoutBottom.setOrientation(LinearLayout.HORIZONTAL);
		SlayoutBottom.setOrientation(LinearLayout.HORIZONTAL);
		mButtonBottomArray = new Button[Globals.BUTTON_BOTTOM_MAX];
		for(int i = 0; i < mButtonBottomArray.length; i ++){
			mButtonBottomArray[i] = newButtonToLayout(mLayoutBottom, Globals.BUTTON_BOTTOM_KEY_ARRAY[i]);
			if(Locals.gWindowScreen)
				mButtonBottomArray[i].setVisibility(View.GONE);
		}
	}
	
	protected MainView getMainView()
	{
		return mMainView;
	}
	
	protected void setXMargin(int margin)
	{
		mXMargin = margin;
	}
	
	protected void setYMargin(int margin)
	{
		mYMargin = margin;
	}
	
	protected int getScreenX()
	{
		return mScreenX;
	}
	
	protected int getScreenY()
	{
		return mScreenY;
	}
	
	protected int getScreenWidth()
	{
		return mScreenWidth;
	}
	
	protected int getScreenHeight()
	{
		return mScreenHeight;
	}
	
	void setup()
	{
		mMainView.setMouseCursorRGB(0, 0, 0, 255, 255, 255);

		mMainView.addView(SVLeft);
		mMainView.addView(SVRight);
		mMainView.addView(SVTop);
		mMainView.addView(SVBottom);
	}
	
	void cleanup()
	{
		SlayoutRight.removeView(mLayoutRight);
		SVRight.removeView(SlayoutRight);
		SlayoutLeft.removeView(mLayoutLeft);
		SVLeft.removeView(SlayoutLeft);
		SlayoutTop.removeView(mLayoutTop);
		SVTop.removeView(SlayoutTop);
		SlayoutBottom.removeView(mLayoutBottom);
		SVBottom.removeView(SlayoutBottom);

		mMainView.removeView(SVLeft);
		mMainView.removeView(SVRight);
		mMainView.removeView(SVTop);
		mMainView.removeView(SVBottom);
		
		mMainView.setMouseCursorRGB(0, 0, 0, 255, 255, 255);
	}
	
	void update()
	{
		int i;
		
		int xnum = (Globals.BUTTON_USE && Locals.ButtonLeftEnabled ? 1 : 0) + (Globals.BUTTON_USE && Locals.ButtonRightEnabled ? 1 : 0) + mXMargin + Locals.VideoXMargin;
		int wsum = mMainView.getDisplayWidth() - mMainView.getGLViewWidth();
		int w1 = (xnum > 0 ? (wsum / xnum) : 0);
		int lw = (Globals.BUTTON_USE && Locals.ButtonLeftEnabled ? w1 : 0);
		int rw = (Globals.BUTTON_USE && Locals.ButtonRightEnabled ? w1 : 0);
		
		int ynum = (Globals.BUTTON_USE && Locals.ButtonTopEnabled ? 1 : 0) + (Globals.BUTTON_USE && Locals.ButtonBottomEnabled ? 1 : 0) + mYMargin + Locals.VideoYMargin;
		int hsum = mMainView.getDisplayHeight() - mMainView.getGLViewHeight();
		int h1 = (ynum > 0 ? (hsum / ynum) : 0);
		int th = (Globals.BUTTON_USE && Locals.ButtonTopEnabled ? h1 : 0);
		int bh = (Globals.BUTTON_USE && Locals.ButtonBottomEnabled ? h1 : 0);
		
		mScreenX = lw;
		mScreenY = th;
		mScreenWidth  = mMainView.getDisplayWidth()  - (lw + rw);
		mScreenHeight = mMainView.getDisplayHeight() - (th + bh);
		
		int lx = 0;
		int ly = 0;
		int lh = mMainView.getDisplayHeight();
		if(lw > 0 && lh > 0){
			for(i = 0; i < Globals.ButtonLeftNum && i < mButtonLeftArray.length; i ++){
				mButtonLeftArray[i].setVisibility(View.VISIBLE);
				
				String name = null;
				Integer sdlKey = (Integer)Globals.SDLKeyAdditionalKeyMap.get(new Integer(Globals.BUTTON_LEFT_KEY_ARRAY[i]));
				if(sdlKey != null){
					name = Globals.SDLKeyFunctionNameMap.get(sdlKey);
				}
				mButtonLeftArray[i].setText(name);
			}
			for(; i < mButtonRightArray.length; i ++){
				mButtonLeftArray[i].setVisibility(View.GONE);
			}
			mLayoutLeft.setVisibility(View.VISIBLE);

			SVLeft.setLayoutParams(new AbsoluteLayout.LayoutParams(lw, LayoutParams.FILL_PARENT, lx, ly));
			
			SlayoutLeft.removeView(mLayoutLeft);
			SVLeft.removeView(SlayoutLeft);
			SlayoutLeft.addView(mLayoutLeft);
			SVLeft.addView(SlayoutLeft);
		} else {
			mLayoutLeft.setVisibility(View.GONE);
		}
		
		int rx = mMainView.getDisplayWidth() - rw;
		int ry = 0;
		int rh = mMainView.getDisplayHeight();
		if(rw > 0 && rh > 0){
			for(i = 0; i < Globals.ButtonRightNum && i < mButtonRightArray.length; i ++){
				mButtonRightArray[i].setVisibility(View.VISIBLE);
				String name = null;
				Integer sdlKey = (Integer)Globals.SDLKeyAdditionalKeyMap.get(new Integer(Globals.BUTTON_RIGHT_KEY_ARRAY[i]));
				if(sdlKey != null){
					name = Globals.SDLKeyFunctionNameMap.get(sdlKey);
				}
				mButtonRightArray[i].setText(name);
			}
			for(; i < mButtonRightArray.length; i ++){
				mButtonRightArray[i].setVisibility(View.GONE);
			}

			
			
			mLayoutRight.setVisibility(View.VISIBLE);
			//mLayoutRight.setLayoutParams(new AbsoluteLayout.LayoutParams(rw, rh/4*mButtonRightArray.length, rx, ry));
			
			SVRight.setLayoutParams(new AbsoluteLayout.LayoutParams(rw, LayoutParams.FILL_PARENT, rx, ry));
			
			SlayoutRight.removeView(mLayoutRight);
			SVRight.removeView(SlayoutRight);
			SlayoutRight.addView(mLayoutRight);
			SVRight.addView(SlayoutRight);
			
		} else {
			mLayoutRight.setVisibility(View.GONE);
		}
		
		int tx = mScreenX;
		int ty = 0;
		int tw = mScreenWidth;
		if(tw > 0 && th > 0){
			for(i = 0; i < Globals.ButtonTopNum && i < mButtonTopArray.length; i ++){
				mButtonTopArray[i].setVisibility(View.VISIBLE);
				
				String name = null;
				Integer sdlKey = (Integer)Globals.SDLKeyAdditionalKeyMap.get(new Integer(Globals.BUTTON_TOP_KEY_ARRAY[i]));
				if(sdlKey != null){
					name = Globals.SDLKeyFunctionNameMap.get(sdlKey);
				}
				mButtonTopArray[i].setText(name);
			}
			for(; i < mButtonTopArray.length; i ++){
				mButtonTopArray[i].setVisibility(View.GONE);
			}
			mLayoutTop.setVisibility(View.VISIBLE);
			
			SVTop.setLayoutParams(new AbsoluteLayout.LayoutParams(LayoutParams.FILL_PARENT, th, tx, ty));
			
			SlayoutTop.removeView(mLayoutTop);
			SVTop.removeView(SlayoutTop);
			SlayoutTop.addView(mLayoutTop);
			SVTop.addView(SlayoutTop);

		} else {
			mLayoutTop.setVisibility(View.GONE);
		}
		
		int bx = mScreenX;
		int by = mMainView.getDisplayHeight() - bh;
		int bw = mScreenWidth;
		if(bw > 0 && bh > 0){
			for(i = 0; i < Globals.ButtonBottomNum && i < mButtonBottomArray.length; i ++){
				mButtonBottomArray[i].setVisibility(View.VISIBLE);
				
				String name = null;
				Integer sdlKey = (Integer)Globals.SDLKeyAdditionalKeyMap.get(new Integer(Globals.BUTTON_BOTTOM_KEY_ARRAY[i]));
				if(sdlKey != null){
					name = Globals.SDLKeyFunctionNameMap.get(sdlKey);
				}
				mButtonBottomArray[i].setText(name);
			}
			for(; i < mButtonBottomArray.length; i ++){
				mButtonBottomArray[i].setVisibility(View.GONE);
			}
			mLayoutBottom.setVisibility(View.VISIBLE);

			SVBottom.setLayoutParams(new AbsoluteLayout.LayoutParams(LayoutParams.FILL_PARENT, bh, bx, by));
			
			SlayoutBottom.removeView(mLayoutBottom);
			SVBottom.removeView(SlayoutBottom);
			SlayoutBottom.addView(mLayoutBottom);
			SVBottom.addView(SlayoutBottom);
		} else {
			mLayoutBottom.setVisibility(View.GONE);
		}
		
		//
		
		int x;
		int y;
		
		if(Locals.VideoXPosition == 0){
			x = mScreenX + ((mScreenWidth - getMainView().getGLViewWidth())  / 2);
		} else if(Locals.VideoXPosition < 0){
			x = mScreenX;
		} else {
			x = mScreenX + (mScreenWidth - getMainView().getGLViewWidth());
		}
		if(Locals.VideoYPosition == 0){
			y = mScreenY + ((mScreenHeight - getMainView().getGLViewHeight())  / 2);
		} else if(Locals.VideoYPosition < 0){
			y = mScreenY;
		} else {
			y = mScreenY + (mScreenHeight - getMainView().getGLViewHeight());
		}
		
		getMainView().setGLViewPos(x, y);
	}
	
	public void onKeyEvent(int keyCode, int pressed)
	{
	}
	
	public void onMotionEvent(int x, int y, int action, int pointerId, int pressure, int radius, int touchCount)
	{
	}
	
	public void onMouseButtonEvent(int buttonId, int pressed)
	{
	}
}

class InvalidTouchMode extends TouchMode
{
	private boolean mIsMouseShowed = true;
	
	public InvalidTouchMode(MainView mainView)
	{
		super(mainView);
	}
	
	@Override
	void setup()
	{
		super.setup();
		
		mIsMouseShowed = getMainView().isMouseCursorShowed();
		getMainView().showMouseCursor(false);
	}
	
	@Override
	void cleanup()
	{
		if(!getMainView().isMouseCursorShowed()){
			getMainView().showMouseCursor(mIsMouseShowed);
		}
		
		super.cleanup();
	}
	
	@Override
	public void onKeyEvent(int keyCode, int pressed)
	{
		getMainView().nativeKey( keyCode, pressed );
	}
}

class TouchTouchMode extends TouchMode
{
	private int mFirstPointerId = -1;
	private boolean mRightClickMode = false;

	public TouchTouchMode(MainView mainView)
	{
		super(mainView);
	}
	
	@Override
	public void onKeyEvent(int keyCode, int pressed)
	{
		if(keyCode == KeyEvent.KEYCODE_BACK){
			if(pressed == 0){
				if(!mRightClickMode){
					mRightClickMode = true;
					getMainView().setMouseCursorRGB(255, 255, 255, 0, 0, 0);
				} else {
					mRightClickMode = false;
					getMainView().setMouseCursorRGB(0, 0, 0, 255, 255, 255);
				}
			}
		} else {
			getMainView().nativeKey( keyCode, pressed );
		}
	}
	
	@Override
	public void onMotionEvent(int x, int y, int action, int pointerId, int pressure, int radius, int touchCount)
	{
		if(mFirstPointerId < 0 && touchCount == 1 && action == Mouse.SDL_FINGER_DOWN){
			mFirstPointerId = pointerId;
		}
		
		if(pointerId == mFirstPointerId){
			int wx = x - getMainView().getGLViewX();
			int wy = y - getMainView().getGLViewY();
			
			getMainView().setMousePoint(wx, wy);
			getMainView().nativeMotionEvent( wx, wy );
			
			//Log.i("onMotionEvent", "action:" + action + " x=" + wx + " y=" + wy);
			
			switch(action){
				case Mouse.SDL_FINGER_DOWN:
					onMouseButtonEvent(MotionEvent.BUTTON_PRIMARY, 1);
					break;
				case Mouse.SDL_FINGER_UP:
					onMouseButtonEvent(MotionEvent.BUTTON_PRIMARY, 0);
					mFirstPointerId = -1;
					break;
			}
		}
	}

	@Override
	public void onMouseButtonEvent(int buttonId, int pressed)
	{
		getMainView().nativeMouseButtonsPressed( (mRightClickMode ? MotionEvent.BUTTON_SECONDARY : buttonId), pressed );
		if(pressed == 0){
			mRightClickMode = false;
			getMainView().setMouseCursorRGB(0, 0, 0, 255, 255, 255);
		}
	}
}

class TrackPadTouchMode extends TouchMode implements View.OnTouchListener
{
	private int mFirstPointerId = -1;
	private int mFirstPointX = 0;
	private int mFirstPointY = 0;
	private int mFirstMousePointX = 0;
	private int mFirstMousePointY = 0;
	
	private int mSecondPointerId = -1;
	
	private boolean mDirectTouchMode = false;
	private boolean mDirectTouchDown = false;

	private Button mRightClickButton = null;
	private Button mLeftClickButton = null;
	
	public TrackPadTouchMode(MainView mainView)
	{
		super(mainView);
		
		setXMargin(1);
		
		mRightClickButton = new Button(mainView.getActivity());
		mRightClickButton.setText("R");
		mRightClickButton.setOnTouchListener(this);
		
		mLeftClickButton = new Button(mainView.getActivity());
		mLeftClickButton.setText("L");
		mLeftClickButton.setOnTouchListener(this);
	}
	
	public boolean onTouch(View v, MotionEvent event) {
		if(v == mLeftClickButton || v == mRightClickButton){
			v.onTouchEvent(event);
			
			int buttonId = (v == mLeftClickButton ? MotionEvent.BUTTON_PRIMARY : MotionEvent.BUTTON_SECONDARY);
			switch(event.getAction()){
				case MotionEvent.ACTION_DOWN:
					getMainView().nativeMouseButtonsPressed( buttonId, 1 );
					break;
				case MotionEvent.ACTION_UP:
					getMainView().nativeMouseButtonsPressed( buttonId, 0 );
					break;
				default:
					break;
			}
		}
		return true;
	}
	
	@Override
	void setup()
	{
		super.setup();
		
		getMainView().setMouseCursorRGB(120, 120, 120, 255, 255, 255);
		
		getMainView().addView(mLeftClickButton);
		getMainView().addView(mRightClickButton);
	}
	
	@Override
	void cleanup()
	{
		getMainView().removeView(mLeftClickButton);
		getMainView().removeView(mRightClickButton);
		
		super.cleanup();
	}
	
	@Override
	void update()
	{
		super.update();
		
		//
		
		int x;
		int y;
		
		x = getScreenX();
		if(Locals.VideoYPosition == 0){
			y = getScreenY() + ((getScreenHeight() - getMainView().getGLViewHeight())  / 2);
		} else if(Locals.VideoYPosition < 0){
			y = getScreenY();
		} else {
			y = getScreenY() + (getScreenHeight() - getMainView().getGLViewHeight());
		}
		
		getMainView().setGLViewPos(x, y);
		
		//
		
		int rx, ry, rw, rh;
		int lx, ly, lw, lh;
		
		if(getMainView().getGLViewWidth() + (getMainView().getGLViewWidth() / 18) < getScreenWidth()){
			rx = getMainView().getGLViewX() + getMainView().getGLViewWidth();
			ry = getScreenY();
			rw = getScreenX() + getScreenWidth() - rx;
			rh = (getScreenHeight() / 8) * 1;
		} else {
			rw = getMainView().getGLViewWidth() / 18;
			rh = (getScreenHeight() / 8) * 1;
			rx = getScreenX() + getScreenWidth() - rw;
			ry = getScreenY();
		}
		
		lx = rx;
		ly = ry + rh;
		lw = rw;
		lh = (getScreenHeight() / 8) * 3;
		
		mRightClickButton.setLayoutParams(new AbsoluteLayout.LayoutParams(rw, rh, rx, ry));
		mLeftClickButton.setLayoutParams(new AbsoluteLayout.LayoutParams(lw, lh, lx, ly));
	}

	@Override
	public void onKeyEvent(int keyCode, int pressed)
	{
		if(keyCode == KeyEvent.KEYCODE_BACK){
			if(pressed == 0){
				if(!mDirectTouchMode){
					mDirectTouchMode = true;
					getMainView().setMouseCursorRGB(0, 0, 0, 255, 255, 255);
				} else {
					mDirectTouchMode = false;
					getMainView().setMouseCursorRGB(120, 120, 120, 255, 255, 255);
				}
			}
		} else {
			getMainView().nativeKey( keyCode, pressed );
		}
	}
	
	@Override
	public void onMotionEvent(int x, int y, int action, int pointerId, int pressure, int radius, int touchCount)
	{
		if(action == Mouse.SDL_FINGER_DOWN){
			if(mFirstPointerId < 0){
				mFirstPointerId = pointerId;
				mFirstPointX = x;
				mFirstPointY = y;
				mFirstMousePointX = getMainView().getMousePointX();
				mFirstMousePointY = getMainView().getMousePointY();
				if(mDirectTouchMode && x >= getMainView().getGLViewX() && x < getMainView().getGLViewX() + getMainView().getGLViewWidth() && y >= getMainView().getGLViewY() && y < getMainView().getGLViewY() + getMainView().getGLViewHeight()){
					mDirectTouchDown = true;
				}
			} else if(mSecondPointerId < 0){
				mSecondPointerId = pointerId;
				onMouseButtonEvent( MotionEvent.BUTTON_PRIMARY, 1 );
			}
		}
		
		if(pointerId == mFirstPointerId){
			if(mDirectTouchDown){
				int wx = x - getMainView().getGLViewX();
				int wy = y - getMainView().getGLViewY();
				
				getMainView().setMousePoint(wx, wy);
				getMainView().nativeMotionEvent( wx, wy );
				
				switch(action){
					case Mouse.SDL_FINGER_DOWN:
						onMouseButtonEvent(MotionEvent.BUTTON_PRIMARY, 1);
						break;
					case Mouse.SDL_FINGER_UP:
						onMouseButtonEvent(MotionEvent.BUTTON_PRIMARY, 0);
						break;
				}
			} else {
				int wx = mFirstMousePointX + (x - mFirstPointX);
				int wy = mFirstMousePointY + (y - mFirstPointY);
				if(wx < 0){
					wx = 0;
				} else if(wx >= getMainView().getGLViewWidth()){
					wx = getMainView().getGLViewWidth() - 1;
				}
				if(wy < 0){
					wy = 0;
				} else if(wy >= getMainView().getGLViewHeight()){
					wy = getMainView().getGLViewHeight() - 1;
				}
			
				getMainView().setMousePoint( wx, wy );
				getMainView().nativeMotionEvent( wx, wy );
			}
		}
		
		if(action == Mouse.SDL_FINGER_UP){
			if(pointerId == mFirstPointerId){
				mFirstPointerId = -1;
				if(mDirectTouchDown){
					mDirectTouchDown = false;
				}
			} else if(pointerId == mSecondPointerId){
				mSecondPointerId = -1;
				onMouseButtonEvent( MotionEvent.BUTTON_PRIMARY, 0 );
			}
		}
	}
	
	@Override
	public void onMouseButtonEvent(int buttonId, int pressed)
	{
		int action = (pressed == 0 ? MotionEvent.ACTION_UP : MotionEvent.ACTION_DOWN);
		MotionEvent event = MotionEvent.obtain( 0L, 0L, action, 0.0f, 0.0f, 0 );
		
		switch(buttonId){
			case MotionEvent.BUTTON_PRIMARY:
				onTouch(mLeftClickButton, event);
				break;
			case MotionEvent.BUTTON_SECONDARY:
				onTouch(mRightClickButton, event);
				break;
			default:
				getMainView().nativeMouseButtonsPressed( buttonId, pressed );
				break;
		}
	}
}

class GamePadTouchMode extends TouchMode
{
	class ArrowButton extends View
	{
		public final static int BUTTON_NONE       = 0;
		public final static int BUTTON_UP         = 1 << Globals.GAMEPAD_BUTTON_ARROW_UP_INDEX;
		public final static int BUTTON_RIGHT      = 1 << Globals.GAMEPAD_BUTTON_ARROW_RIGHT_INDEX;
		public final static int BUTTON_DOWN       = 1 << Globals.GAMEPAD_BUTTON_ARROW_DOWN_INDEX;
		public final static int BUTTON_LEFT       = 1 << Globals.GAMEPAD_BUTTON_ARROW_LEFT_INDEX;
		
		private Path mPath;
		private Path mTouchPath;
		private Paint mPaint;
		
		private int mButtonState = 0;
		
		public ArrowButton(Context c)
		{
			super(c);
			
			mPath = new Path();
			mTouchPath = new Path();
			
			mPaint = new Paint();
			setBackgroundColor(Color.TRANSPARENT);
		}
		
		@Override
		protected void onSizeChanged (int w, int h, int oldw, int oldh)
		{
			super.onSizeChanged(w, h, oldw, oldh);
			
			int x = w / 2;
			int y = h / 2;
			int r = w / 2 - 2;
			
			mPath.reset();
			
			mPath.addCircle( x, y, r, Path.Direction.CW );
			
			mPath.moveTo(x-(r/6), y-(r/2));
			mPath.lineTo(x, y-(r*3/4));
			mPath.lineTo(x+(r/6), y-(r/2));
			//mPath.close();
			
			mPath.moveTo(x-(r/6), y+(r/2));
			mPath.lineTo(x, y+(r*3/4));
			mPath.lineTo(x+(r/6), y+(r/2));
			//mPath.close();
			
			mPath.moveTo(x-(r/2), y-(r/6));
			mPath.lineTo(x-(r*3/4), y);
			mPath.lineTo(x-(r/2), y+(r/6));
			//mPath.close();
			
			mPath.moveTo(x+(r/2), y-(r/6));
			mPath.lineTo(x+(r*3/4), y);
			mPath.lineTo(x+(r/2), y+(r/6));
			//mPath.close();
			
			setButtonState(mButtonState);
		}
		
		@Override
		protected void onDraw(Canvas canvas) {
			super.onDraw(canvas);
			
			int a = Globals.GamePadOpacity * 255 / 100;
			
			mPaint.setStyle(Paint.Style.FILL);
			mPaint.setARGB(a, 243, 152, 0);
			canvas.drawPath(mTouchPath, mPaint);
		
			mPaint.setStyle(Paint.Style.STROKE);
			mPaint.setStrokeWidth(4);
			mPaint.setARGB(a, 0, 0, 0);
			canvas.drawPath(mPath, mPaint);
	
			mPaint.setStyle(Paint.Style.STROKE);
			mPaint.setStrokeWidth(2);
			mPaint.setARGB(a, 255, 255, 255);
			canvas.drawPath(mPath, mPaint);
		}
		
		public int getButtonState()
		{
			return mButtonState;
		}
		
		public void setButtonState(int buttonState)
		{
			mButtonState = buttonState;
			
			int x = getWidth() / 2;
			int y = getHeight() / 2;
			int r = getWidth() / 2 - 2;
			
			mTouchPath.reset();
			
			if((buttonState & BUTTON_UP) != 0){
				mTouchPath.addCircle( x, y-(r*5/8), (r*3/8), Path.Direction.CW );
			}
			if((buttonState & BUTTON_DOWN) != 0){
				mTouchPath.addCircle( x, y+(r*5/8), (r*3/8), Path.Direction.CW );
			}
			if((buttonState & BUTTON_LEFT) != 0){
				mTouchPath.addCircle( x-(r*5/8), y, (r*3/8), Path.Direction.CW );
			}
			if((buttonState & BUTTON_RIGHT) != 0){
				mTouchPath.addCircle( x+(r*5/8), y, (r*3/8), Path.Direction.CW );
			}
			
			//Log.i("setButtonState", "buttonState:" + buttonState);
			
			invalidate();
		}

	}
	
	class ActionButton extends View
	{
		public final static int BUTTON_NONE  = 0;
		public final static int BUTTON_UP    = 1 << Globals.GAMEPAD_BUTTON_ACTION_UP_INDEX;
		public final static int BUTTON_RIGHT = 1 << Globals.GAMEPAD_BUTTON_ACTION_RIGHT_INDEX;
		public final static int BUTTON_DOWN  = 1 << Globals.GAMEPAD_BUTTON_ACTION_DOWN_INDEX;
		public final static int BUTTON_LEFT  = 1 << Globals.GAMEPAD_BUTTON_ACTION_LEFT_INDEX;
		
		private Path mPath;
		private Path mTouchPath;
		private Paint mPaint;
		
		private int mButtonState = 0;
		
		public ActionButton(Context c)
		{
			super(c);
			
			mPath = new Path();
			mTouchPath = new Path();
			
			mPaint = new Paint();
			setBackgroundColor(Color.TRANSPARENT);
		}
		
		@Override
		protected void onSizeChanged (int w, int h, int oldw, int oldh)
		{
			super.onSizeChanged(w, h, oldw, oldh);
			
			int x = w / 2;
			int y = h / 2;
			int r = w / 2 - 2;
			
			mPath.reset();
			
			mPath.addCircle( x, y, r, Path.Direction.CW );
			
			mPath.addCircle( x, y-(r*5/8), (r*3/8), Path.Direction.CW );
			mPath.addCircle( x, y+(r*5/8), (r*3/8), Path.Direction.CW );
			mPath.addCircle( x-(r*5/8), y, (r*3/8), Path.Direction.CW );
			mPath.addCircle( x+(r*5/8), y, (r*3/8), Path.Direction.CW );
			
			setButtonState(mButtonState);
		}
		
		@Override
		protected void onDraw(Canvas canvas) {
			super.onDraw(canvas);
			
			int a = Globals.GamePadOpacity * 255 / 100;
			
			mPaint.setStyle(Paint.Style.FILL);
			mPaint.setARGB(a, 243, 152, 0);
			//mPaint.setARGB(80, 100, 100, 100);
			canvas.drawPath(mTouchPath, mPaint);
			
			mPaint.setStyle(Paint.Style.STROKE);
			mPaint.setStrokeWidth(4);
			mPaint.setARGB(a, 0, 0, 0);
			canvas.drawPath(mPath, mPaint);
			
			mPaint.setStyle(Paint.Style.STROKE);
			mPaint.setStrokeWidth(2);
			mPaint.setARGB(a, 255, 255, 255);
			canvas.drawPath(mPath, mPaint);
		}
		
		public int getButtonState()
		{
			return mButtonState;
		}
		
		public void setButtonState(int buttonState)
		{
			mButtonState = buttonState;
			
			int x = getWidth() / 2;
			int y = getHeight() / 2;
			int r = getWidth() / 2 - 2;
			
			mTouchPath.reset();
			
			if((buttonState & BUTTON_UP) != 0){
				mTouchPath.addCircle( x, y-(r*5/8), (r*3/8), Path.Direction.CW );
			}
			if((buttonState & BUTTON_DOWN) != 0){
				mTouchPath.addCircle( x, y+(r*5/8), (r*3/8), Path.Direction.CW );
			}
			if((buttonState & BUTTON_LEFT) != 0){
				mTouchPath.addCircle( x-(r*5/8), y, (r*3/8), Path.Direction.CW );
			}
			if((buttonState & BUTTON_RIGHT) != 0){
				mTouchPath.addCircle( x+(r*5/8), y, (r*3/8), Path.Direction.CW );
			}
			
			//Log.i("setButtonState", "buttonState:" + buttonState);
			
			invalidate();
		}
	}
	
	private boolean mIsMouseShowed = true;
	
	private ArrowButton mArrowButton = null;
	private int mArrowButtonX = 0;
	private int mArrowButtonY = 0;
	private int mArrowButtonWidth  = 0;
	private int mArrowButtonHeight = 0;
	private int mArrowButtonState = 0;
	private int mArrowButtonAsAxisPointerId = -1;
	private int[] mArrowButtonAsButtonPointerIdAry = new int[Globals.GAMEPAD_BUTTON_ARROW_NUM];
	
	private ActionButton mActionButton = null;
	private int mActionButtonX = 0;
	private int mActionButtonY = 0;
	private int mActionButtonWidth  = 0;
	private int mActionButtonHeight = 0;
	private int mActionButtonState = 0;
	private int mActionButtonAsAxisPointerId = -1;
	private int[] mActionButtonAsButtonPointerIdAry = new int[Globals.GAMEPAD_BUTTON_ACTION_NUM];
	
	public GamePadTouchMode(MainView mainView)
	{
		super(mainView);
		
		mArrowButton  = new ArrowButton(mainView.getActivity());
		mActionButton = new ActionButton(mainView.getActivity());
		
		for(int i = 0; i < mArrowButtonAsButtonPointerIdAry.length; i ++){
			mArrowButtonAsButtonPointerIdAry[i] = -1;
		}
		for(int i = 0; i < mActionButtonAsButtonPointerIdAry.length; i ++){
			mActionButtonAsButtonPointerIdAry[i] = -1;
		}
	}
	
	@Override
	void setup()
	{
		super.setup();
		
		mIsMouseShowed = getMainView().isMouseCursorShowed();
		getMainView().showMouseCursor(false);
		
		getMainView().addView(mArrowButton);
		getMainView().addView(mActionButton);
	}
	
	@Override
	void cleanup()
	{
		getMainView().removeView(mArrowButton);
		getMainView().removeView(mActionButton);
		
		if(!getMainView().isMouseCursorShowed()){
			getMainView().showMouseCursor(mIsMouseShowed);
		}
		
		super.cleanup();
	}
	
	@Override
	void update()
	{
		super.update();
		
		//
		
		int buttonSize = (getScreenWidth() / 2) * Globals.GamePadSize / 100;
		
		mArrowButtonWidth  = mArrowButtonHeight  = buttonSize;
		mActionButtonWidth = mActionButtonHeight = buttonSize;
		
		mArrowButtonX = getScreenX();
		mArrowButtonY = getScreenY() + ((getScreenHeight() - mArrowButtonHeight) * Globals.GamePadPosition / 100);
		
		mArrowButton.setLayoutParams(new AbsoluteLayout.LayoutParams(mArrowButtonWidth, mArrowButtonHeight, mArrowButtonX, mArrowButtonY));
		
		mActionButtonX = getScreenX() + getScreenWidth() - mActionButtonWidth;
		mActionButtonY = getScreenY() + ((getScreenHeight() - mActionButtonHeight) * Globals.GamePadPosition / 100);
		
		mActionButton.setLayoutParams(new AbsoluteLayout.LayoutParams(mActionButtonWidth, mActionButtonHeight, mActionButtonX, mActionButtonY));
		
		mArrowButton.invalidate();
		mActionButton.invalidate();
		
		//Log.i("TouchMode", "Screen: x=" + getScreenX() + " y=" + getScreenY());
		//Log.i("TouchMode", "ArrowButton: x=" + mArrowButtonX + " y=" + mArrowButtonY);
	}
	
	@Override
	public void onKeyEvent(int keyCode, int pressed)
	{
		getMainView().nativeKey( keyCode, pressed );
	}
	
	@Override
	public void onMotionEvent(int x, int y, int action, int pointerId, int pressure, int radius, int touchCount)
	{		
		//Log.i("onMotionEvent", "action:" + action + " x=" + x + " y=" + y);
		
		int arrowButtonState  = mArrowButtonState;
		int actionButtonState = mActionButtonState;
		
		if(action == Mouse.SDL_FINGER_DOWN || action == Mouse.SDL_FINGER_MOVE){
			if(x >= mArrowButtonX && x < mArrowButtonX + mArrowButtonWidth /*&& y >= mArrowButtonY && y < mArrowButtonY + mArrowButtonHeight*/){
				int ax = x - mArrowButtonX;
				int ay = y - mArrowButtonY;
				if(Globals.GamePadArrowButtonAsAxis){
					if(action == Mouse.SDL_FINGER_DOWN || mArrowButtonAsAxisPointerId == pointerId){
						arrowButtonState = 0;
						if(ax < mArrowButtonWidth * 1 / 3){
							arrowButtonState |= (1 << Globals.GAMEPAD_BUTTON_ARROW_LEFT_INDEX);
						} else if(ax >= mArrowButtonWidth * 2 / 3){
							arrowButtonState |= (1 << Globals.GAMEPAD_BUTTON_ARROW_RIGHT_INDEX);
						}
						if(ay < mArrowButtonHeight * 1 / 3){
							arrowButtonState |= (1 << Globals.GAMEPAD_BUTTON_ARROW_UP_INDEX);
						} else if(ay >= mArrowButtonHeight * 2 / 3){
							arrowButtonState |= (1 << Globals.GAMEPAD_BUTTON_ARROW_DOWN_INDEX);
						}
						mArrowButtonAsAxisPointerId = pointerId;
					}
				} else if(action == Mouse.SDL_FINGER_DOWN){
					int index;
					if(ax + ay < mArrowButtonWidth){
						if(ax <= ay){
							index = Globals.GAMEPAD_BUTTON_ARROW_LEFT_INDEX;
						} else {
							index = Globals.GAMEPAD_BUTTON_ARROW_UP_INDEX;
						}
					} else {
						if(ax >= ay){
							index = Globals.GAMEPAD_BUTTON_ARROW_RIGHT_INDEX;
						} else {
							index = Globals.GAMEPAD_BUTTON_ARROW_DOWN_INDEX;
						}
					}
					if(mArrowButtonAsButtonPointerIdAry[index] < 0){
						mArrowButtonAsButtonPointerIdAry[index] = pointerId;
						arrowButtonState |= (1 << index);
					}
				}
			} else if(x >= mActionButtonX && x < mActionButtonX + mActionButtonWidth && y >= mActionButtonY && y < mActionButtonY + mActionButtonHeight){
				int ax = x - mActionButtonX;
				int ay = y - mActionButtonY;
				if(Globals.GamePadActionButtonAsAxis){
					if(action == Mouse.SDL_FINGER_DOWN || mActionButtonAsAxisPointerId == pointerId){
						actionButtonState = 0;
						if(ax < mActionButtonWidth * 1 / 3){
							actionButtonState |= (1 << Globals.GAMEPAD_BUTTON_ACTION_LEFT_INDEX);
						} else if(ax >= mActionButtonWidth * 2 / 3){
							actionButtonState |= (1 << Globals.GAMEPAD_BUTTON_ACTION_RIGHT_INDEX);
						}
						if(ay < mActionButtonHeight * 1 / 3){
							actionButtonState |= (1 << Globals.GAMEPAD_BUTTON_ACTION_UP_INDEX);
						} else if(ay >= mActionButtonHeight * 2 / 3){
							actionButtonState |= (1 << Globals.GAMEPAD_BUTTON_ACTION_DOWN_INDEX);
						}
						mActionButtonAsAxisPointerId = pointerId;
					}
				} else if(action == Mouse.SDL_FINGER_DOWN){
					int index;
					if((ax + ay) < mActionButtonWidth){
						if(ax <= ay){
							index = Globals.GAMEPAD_BUTTON_ACTION_LEFT_INDEX;
						} else {
							index = Globals.GAMEPAD_BUTTON_ACTION_UP_INDEX;
						}
					} else {
						if(ax >= ay){
							index = Globals.GAMEPAD_BUTTON_ACTION_RIGHT_INDEX;
						} else {
							index = Globals.GAMEPAD_BUTTON_ACTION_DOWN_INDEX;
						}
					}
					if(mActionButtonAsButtonPointerIdAry[index] < 0){
						mActionButtonAsButtonPointerIdAry[index] = pointerId;
						actionButtonState |= (1 << index);
					}
				}
			}
		} else if(action == Mouse.SDL_FINGER_UP){
			if(pointerId == mArrowButtonAsAxisPointerId){
				arrowButtonState = 0;
				mArrowButtonAsAxisPointerId = -1;
			}
			for(int i = 0; i < mArrowButtonAsButtonPointerIdAry.length; i ++){
				if(pointerId == mArrowButtonAsButtonPointerIdAry[i]){
					arrowButtonState &= ~(1 << i);
					mArrowButtonAsButtonPointerIdAry[i] = -1;
				}
			}
			if(pointerId == mActionButtonAsAxisPointerId){
				actionButtonState = 0;
				mActionButtonAsAxisPointerId = -1;
			}
			for(int i = 0; i < mActionButtonAsButtonPointerIdAry.length; i ++){
				if(pointerId == mActionButtonAsButtonPointerIdAry[i]){
					actionButtonState &= ~(1 << i);
					mActionButtonAsButtonPointerIdAry[i] = -1;
				}
			}
		}
		
		if(arrowButtonState != mArrowButtonState){
			mArrowButton.setButtonState(arrowButtonState);
			
			int diff  = arrowButtonState ^ mArrowButtonState;
			int state = arrowButtonState;
			
			for(int i = 0; diff != 0 ; diff>>= 1, state>>= 1, i ++){
				if( (diff & 1) != 0 ){
					getMainView().nativeKey( Globals.GAMEPAD_BUTTON_ARROW_KEY_ARRAY[i], ((state & 1) != 0 ? 1 : 0) );
				}
			}
			
			mArrowButtonState = arrowButtonState;
			
			//Log.i("onMotionEvent", "arrowButtonState" + mArrowButtonState);
		}
		if(actionButtonState != mActionButtonState){
			mActionButton.setButtonState(actionButtonState);
			
			int diff  = actionButtonState ^ mActionButtonState;
			int state = actionButtonState;
			
			for(int i = 0; diff != 0 ; diff>>= 1, state>>= 1, i ++){
				if( (diff & 1) != 0 ){
					getMainView().nativeKey( Globals.GAMEPAD_BUTTON_ACTION_KEY_ARRAY[i], ((state & 1) != 0 ? 1 : 0) );
				}
			}
			
			mActionButtonState = actionButtonState;
		}
	}
	
	@Override
	public void onMouseButtonEvent(int buttonId, int pressed)
	{
	}
}
