package cn.natdon.onscripterv2.Button;

import android.content.Context;
import android.graphics.Canvas;
import android.util.AttributeSet;
import android.widget.Button;
import cn.natdon.onscripterv2.R;

public class RunButton extends Button{
	public RunButton(Context context) {
		super(context);
		}

		public RunButton(Context context, AttributeSet attrs){
		super(context, attrs);
		}

		@Override
		protected void onDraw(Canvas canvas) {
		//sets the button image based on whether the button in its pressed state
		setBackgroundDrawable(getResources().getDrawable(isPressed()?R.drawable.run_1 : R.drawable.run_0));
		super.onDraw(canvas);
		}
}
