package cn.natdon.onscripterv2.anim;

import java.util.ArrayList;

import android.view.animation.AccelerateInterpolator;
import android.view.animation.AlphaAnimation;
import android.view.animation.Animation;
import android.view.animation.AnimationSet;
import android.view.animation.DecelerateInterpolator;
import android.view.animation.Interpolator;
import android.view.animation.OvershootInterpolator;
import android.view.animation.ScaleAnimation;
import android.view.animation.TranslateAnimation;

public class AnimationBuilder {

	public static AnimationBuilder create() {
		return new AnimationBuilder();
	}

	ArrayList<Animation> animations = new ArrayList<Animation>();
	
	private int val_type = Animation.ABSOLUTE;

	// Utility Functions

	private Animation current() {
		Animation anim = anim(-1);
		if (anim == null)
			throw new NoAnimationException();
		return anim;
	}

	private Animation previous() {
		return anim(-2);
	}

	private Animation anim(int number) {
		if (animations.size() == 0)
			return null;
		if (number < 0)
			number += animations.size();
		if (number >= animations.size() || number < 0)
			return null;
		return animations.get(number);
	}

	private AnimationBuilder() {

	}

	// Animation Creation

	public AnimationBuilder alpha(float from, float to) {
		anim(new AlphaAnimation(from, to));
		return this;
	}
	
	public AnimationBuilder alpha(float alpha) {
		Animation anim = new AlphaAnimation(alpha, alpha);
		anim.setDuration(1);
		anim(anim);
		return this;
	}
	
	public AnimationBuilder valtype(int type) {
		val_type = type;
		return this;
	}

	public AnimationBuilder scale(float fromX, float toX, float fromY,
			float toY, int pivotXType, float pivotX, int pivotYType,
			float pivotY) {
		anim(new ScaleAnimation(fromX, toX, fromY, toY, pivotXType, pivotX,
				pivotYType, pivotY));
		return this;
	}

	public AnimationBuilder scale(float fromX, float toX, float fromY, float toY) {
		anim(new ScaleAnimation(fromX, toX, fromY, toY));
		return this;
	}

	public AnimationBuilder scale(float fromX, float toX, float fromY,
			float toY, float pivotX, float pivotY) {
		anim(new ScaleAnimation(fromX, toX, fromY, toY, val_type, pivotX, val_type, pivotY));
		return this;
	}
	
	public AnimationBuilder translate(
			float fromXDelta, float toXDelta, float fromYDelta, float toYDelta) {
		anim(new TranslateAnimation(val_type, fromXDelta, val_type, toXDelta, 
				val_type, fromYDelta, val_type, toYDelta));
		return this;
	}
	
	public AnimationBuilder translate(
			int fromXDeltaType, float fromXDelta, 
			int toXDeltaType, float toXDelta, 
			int fromYDeltaType, float fromYDelta, 
			int toYDeltaType, float toYDelta) {
		anim(new TranslateAnimation(fromXDeltaType, fromXDelta, toXDeltaType, toXDelta, 
				fromYDeltaType, fromYDelta, toYDeltaType, toYDelta));
		return this;
	}

	public AnimationBuilder anim(Animation anim) {
		animations.add(anim);
		return this;
	}

	// Timing Tweak

	/**
	 * Set the start time of the animation
	 * 
	 * @param timeMillis
	 * @return
	 */
	public AnimationBuilder startAt(long timeMillis) {
		current().setStartTime(timeMillis);
		return this;
	}

	/**
	 * Wait before the animation is started since its startTime
	 * 
	 * @param timeMillis
	 * @return
	 */
	public AnimationBuilder pending(long timeMillis) {
		current().setStartOffset(timeMillis);
		return this;
	}

	/**
	 * Animate to what time? (relative to the parent)
	 * 
	 * @param timeMillis
	 * @return
	 */
	public AnimationBuilder to(long timeMillis) {
		current().setDuration(
				timeMillis - current().getStartTime()
						- current().getStartOffset());
		return this;
	}

	/**
	 * Animate for how long?
	 * 
	 * @param timeMillis
	 * @return
	 */
	public AnimationBuilder animateFor(long timeMillis) {
		current().setDuration(timeMillis);
		return this;
	}

	/**
	 * Alias for animateFor
	 */
	public AnimationBuilder duration(long timeMillis) {
		return animateFor(timeMillis);
	}

	// synchronize the StartOffset offend the behaviour rule of serial()

	/**
	 * Parallel with previous Animation
	 * 
	 * @return
	 */
	public AnimationBuilder parallel() {
		if (previous() != null) {
			current().setStartTime(previous().getStartTime());
			current().setStartOffset(previous().getStartOffset());
		} else {
			current().setStartTime(0);
			current().setStartOffset(0);
		}
		return this;
	}

	/**
	 * Parallel with Animation for N before
	 * 
	 * @return
	 */
	public AnimationBuilder parallel(int n) {
		if (anim(-n - 1) != null) {
			current().setStartTime(anim(-n - 1).getStartTime());
			current().setStartOffset(anim(-n - 1).getStartOffset());
		} else {
			throw new NoAnimationException();
		}
		return this;
	}

	/**
	 * Serial to previous Animation
	 * 
	 * @return
	 */
	public AnimationBuilder serial() {
		long pOffset = 0, pDuration = 0;
		if (previous() != null) {
			pOffset = previous().getStartTime() + previous().getStartOffset();
			pDuration = previous().getDuration();
		}
		current().setStartTime(pOffset + pDuration);
		return this;
	}

	/**
	 * Serial with Animation for N before
	 * 
	 * @param n
	 * @return
	 */
	public AnimationBuilder serial(int n) {
		long pOffset = 0, pDuration = 0;
		if (anim(-n - 1) != null) {
			pOffset = anim(-n - 1).getStartTime()
					+ anim(-n - 1).getStartOffset();
			pDuration = anim(-n - 1).getDuration();
		}
		current().setStartTime(pOffset + pDuration);
		return this;
	}

	// Interpolator

	public AnimationBuilder accelerated() {
		return interpolator(new AccelerateInterpolator());
	}

	public AnimationBuilder decelerated() {
		return interpolator(new DecelerateInterpolator());
	}

	public AnimationBuilder overshoot() {
		return interpolator(new OvershootInterpolator());
	}

	public AnimationBuilder accelerated(float factor) {
		return interpolator(new AccelerateInterpolator(factor));
	}

	public AnimationBuilder decelerated(float factor) {
		return interpolator(new DecelerateInterpolator(factor));
	}

	public AnimationBuilder overshoot(float factor) {
		return interpolator(new OvershootInterpolator(factor));
	}

	private Interpolator global_interpolator = null;

	public AnimationBuilder interpolator(Interpolator i) {
		try {
			current().setInterpolator(i);
		} catch (NoAnimationException e) {
			global_interpolator = i;
		}
		return this;
	}

	// FillOptions

	public FillOptions Fill = new FillOptions();

	public class FillOptions {

		private boolean global_fillafter = true;
		private boolean global_fillbefore = true;
		private boolean global_fillenabled = true;

		public FillOptions after(boolean enabled) {
			try {
				current().setFillAfter(enabled);
			} catch (NoAnimationException e) {
				global_fillafter = enabled;
			}
			return this;
		}

		public FillOptions before(boolean enabled) {
			try {
				current().setFillBefore(enabled);
			} catch (NoAnimationException e) {
				global_fillbefore = enabled;
			}
			return this;
		}

		public FillOptions enabled(boolean enabled) {
			try {
				current().setFillEnabled(enabled);
			} catch (NoAnimationException e) {
				global_fillenabled = enabled;
			}
			return this;
		}

		public AnimationBuilder upward() {
			return AnimationBuilder.this;
		}

	}

	/**
	 * Build for output of the animation
	 * 
	 * @return
	 */
	public Animation build() {
		if (animations.size() <= 1) {
			Animation anim = current();
			anim.setFillEnabled(Fill.global_fillenabled);
			anim.setFillBefore(Fill.global_fillbefore);
			anim.setFillAfter(Fill.global_fillafter);
			if (global_interpolator != null)
				anim.setInterpolator(global_interpolator);
			animations.clear();
			return anim;
		} else {
			AnimationSet set = new AnimationSet(global_interpolator != null);
			set.setFillEnabled(Fill.global_fillenabled);
			set.setFillBefore(Fill.global_fillbefore);
			set.setFillAfter(Fill.global_fillafter);
			if (global_interpolator != null)
				set.setInterpolator(global_interpolator);
			for (Animation anim : animations) {
				set.addAnimation(anim);
			}
			animations.clear();
			return set;
		}
	}

	public Animation build(Animation.AnimationListener listener) {
		Animation anim = build();
		anim.setAnimationListener(listener);
		return anim;
	}

}
