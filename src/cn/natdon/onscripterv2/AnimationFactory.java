package cn.natdon.onscripterv2;

import android.view.animation.AccelerateInterpolator;
import android.view.animation.AlphaAnimation;
import android.view.animation.Animation;
import android.view.animation.AnimationSet;
import android.view.animation.DecelerateInterpolator;
import android.view.animation.OvershootInterpolator;
import android.view.animation.ScaleAnimation;
import android.view.animation.Animation.AnimationListener;

public class AnimationFactory {

	public static Animation coverInAnimation() {
		AnimationSet set = new AnimationSet(false);
		AlphaAnimation animAlpha = new AlphaAnimation(0, 1);
		ScaleAnimation animScale = new ScaleAnimation(
				0.5f, 1.0f, 0.5f, 1.0f,
				Animation.RELATIVE_TO_SELF, 0.5f, Animation.RELATIVE_TO_SELF, 0.5f);
		animScale.setInterpolator(new OvershootInterpolator());
		animAlpha.setDuration(300);
		animScale.setDuration(300);
		set.addAnimation(animAlpha);
		set.addAnimation(animScale);
		set.setFillAfter(true);
		return set;
	}

	public static Animation coverOutAnimation(AnimationListener listener) {
		AnimationSet set = new AnimationSet(true);
		AlphaAnimation animAlpha = new AlphaAnimation(1, 0);
		ScaleAnimation animScale = new ScaleAnimation(
				1.0f, 1.2f, 1.0f, 1.2f,
				Animation.RELATIVE_TO_SELF, 0.5f, Animation.RELATIVE_TO_SELF, 0.5f);
		animAlpha.setDuration(100);
		animScale.setDuration(100);
		set.addAnimation(animAlpha);
		set.addAnimation(animScale);
		set.setAnimationListener(listener);
		return set;
	}

	public static Animation bkgInAnimation() {
		AlphaAnimation animAlpha = new AlphaAnimation(0, 1);
		animAlpha.setDuration(1000);
		animAlpha.setInterpolator(new DecelerateInterpolator(1.5f));
		animAlpha.setFillAfter(true);
		return animAlpha;
	}

	public static Animation bkgOutAnimation(AnimationListener listener) {
		AlphaAnimation animAlpha = new AlphaAnimation(1, 0);
		animAlpha.setDuration(1000);
		animAlpha.setInterpolator(new AccelerateInterpolator(1.5f));
		animAlpha.setAnimationListener(listener);
		return animAlpha;
	}

	public static Animation videoPlayerAnimation(AnimationListener listener) {
		AlphaAnimation animAlpha = new AlphaAnimation(0, 1);
		animAlpha.setDuration(200);
		animAlpha.setInterpolator(new AccelerateInterpolator(1.5f));
		animAlpha.setAnimationListener(listener);
		return animAlpha;
	}

	public static Animation hideVideoPlayerAnimation(AnimationListener listener) {
		AlphaAnimation animAlpha = new AlphaAnimation(1, 0);
		animAlpha.setDuration(200);
		animAlpha.setInterpolator(new AccelerateInterpolator(1.5f));
		animAlpha.setAnimationListener(listener);
		return animAlpha;
	}

}
