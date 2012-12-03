package cn.natdon.onscripterv2.anim;

import android.view.animation.Animation;

/**
 * AnimationFactory with Automata context assistant function
 * @author trinity
 *
 */
public abstract class AutomataAnimationFactory implements AnimationFactory {
	
	private AnimationAutomata automata;
	
	public void setContext(AnimationAutomata automata) {
		this.automata = automata;
	}

	public Animation getAnimation(int from, int to) {
		return automata.getAnimation(from, to);
	}

}
