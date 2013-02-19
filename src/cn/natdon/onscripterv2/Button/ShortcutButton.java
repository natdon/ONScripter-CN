package cn.natdon.onscripterv2.Button;

import android.content.Context;
import android.graphics.Canvas;
import android.util.AttributeSet;
import android.widget.Button;
import cn.natdon.onscripterv2.R;

public class ShortcutButton extends Button{
	public ShortcutButton(Context context) {
		super(context);
		}

		public ShortcutButton(Context context, AttributeSet attrs){
		super(context, attrs);
		}

		@Override
		protected void onDraw(Canvas canvas) {
		//sets the button image based on whether the button in its pressed state
		setBackgroundDrawable(getResources().getDrawable(isPressed()?R.drawable.shortcut_1 : R.drawable.shortcut_0));
		super.onDraw(canvas);
		}
}
