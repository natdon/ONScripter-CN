package cn.natdon.onscripterv2.Button;

import android.content.Context;
import android.graphics.Canvas;
import android.util.AttributeSet;
import android.widget.Button;
import cn.natdon.onscripterv2.R;

public class OkCancelButton extends Button{

	public OkCancelButton(Context context) {
		super(context);
		}

		public OkCancelButton(Context context, AttributeSet attrs){
		super(context, attrs);
		}

		@Override
		protected void onDraw(Canvas canvas) {
		//sets the button image based on whether the button in its pressed state
		setBackgroundDrawable(getResources().getDrawable(isPressed()?R.drawable.okcancel_1 : R.drawable.okcancel_0));
		super.onDraw(canvas);
		}
}