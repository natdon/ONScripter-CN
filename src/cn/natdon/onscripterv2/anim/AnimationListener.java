package cn.natdon.onscripterv2.anim;

import android.view.animation.Animation;

public abstract class AnimationListener implements
		android.view.animation.Animation.AnimationListener {
	
	public void onAnimationEnd(Animation animation) {
		After(animation);
	}
	
	public void onAnimationStart(Animation animation) {
		Before(animation);
	}
	
	public void onAnimationRepeat(Animation animation) {
		
	}
	
	protected void Before(Animation animation) {
		
	}
	
	protected void After(Animation animation) {
		
	}

}
