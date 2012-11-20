package cn.natdon.onscripterv2;


import android.R.integer;
import android.graphics.Matrix;
import android.view.animation.AccelerateDecelerateInterpolator;
import android.view.animation.Animation;
import android.view.animation.Transformation;

public class TVOffAnimation extends Animation {

	private int halfWidth;

	private int halfHeight;

	@Override
	public void initialize(int width, int height, int parentWidth,
			int parentHeight) {

		super.initialize(width, height, parentWidth, parentHeight);
		setDuration(200);
		setFillAfter(true);
		
		halfWidth = width / 2;
		halfHeight = height / 2;
		setInterpolator(new AccelerateDecelerateInterpolator());
		
	}

	@Override
	protected void applyTransformation(float interpolatedTime, Transformation t) {

		final Matrix matrix = t.getMatrix();
		if (interpolatedTime < 0.8) {
			matrix.preScale(1+0.625f*interpolatedTime, 1-interpolatedTime/0.8f+0.01f,halfWidth,halfHeight);
		}else{
			matrix.preScale(7.5f*(1-interpolatedTime),0.01f,halfWidth,halfHeight);
		}
		
	}
}
