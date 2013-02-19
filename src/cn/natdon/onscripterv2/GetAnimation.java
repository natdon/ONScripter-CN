package cn.natdon.onscripterv2;

import cn.natdon.onscripterv2.anim.AnimationBuilder;
import cn.natdon.onscripterv2.anim.AnimationListener;

import android.view.animation.Animation;

public class GetAnimation {

	public static class For {
		
		public static class MainInterface {

			public static Animation ToShowCover(AnimationListener listener) {
				Animation anim = 
						AnimationBuilder.create()
						// Change Fill Options
						.Fill.after(true).upward()
						// Set the valtype of the value to be inturrpted
						.valtype(Animation.RELATIVE_TO_SELF)
						// Add a Scale Animation
						.scale(0.5f, 1.0f, 0.5f, 1.0f, 0.5f, 0.5f).overshoot()
						.animateFor(300)
						// Add an Alpha Animation
						.alpha(0, 1).animateFor(300)
						// Build Animation
						.build(listener);
				return anim;
			}


			public static Animation ToShowVideoPlayerFrame(
					AnimationListener listener) {
				Animation anim = AnimationBuilder.create()
						.alpha(0, 1).animateFor(200).accelerated(1.5f)
						.build(listener);
				return anim;
			}

			public static Animation ToHideVideoPlayerFrame(
					AnimationListener listener) {
				Animation anim = AnimationBuilder.create()
						.alpha(1, 0).animateFor(200).accelerated(1.5f)
						.build(listener);
				return anim;
			}

		}
	}
}
