package cn.natdon.onscripterv2.anim;

import android.view.animation.Animation;

public class AutomataAction extends
		AnimationListener {
	
	private AnimationAutomata automata;
	private AnimationListener wrapped;

	public AutomataAction() {
		
	}
	
	/**
	 * This AutomataAction Act as an wrapper to AnimationListener
	 * @param wrapped
	 */
	public AutomataAction(AnimationListener wrapped) {
		this.wrapped = wrapped;
	}
	
	/**
	 * Set by AnimationAutomata at Runtime
	 * @param automata
	 */
	void setAutomata(AnimationAutomata automata) {
		this.automata = automata;
	}
	
	public AnimationAutomata getAutomata() {
		return automata;
	}
	
	public void onStateChanged(int from, int to) {
		
	}
	
	public void onAnimationEnd(Animation animation) {
		super.onAnimationEnd(animation);
		if(wrapped != null) wrapped.onAnimationEnd(animation);
	}
	
	public void onAnimationStart(Animation animation) {
		super.onAnimationStart(animation);
		if(wrapped != null) wrapped.onAnimationStart(animation);
	}
	
	public void onAnimationRepeat(Animation animation) {
		super.onAnimationRepeat(animation);
		if(wrapped != null) wrapped.onAnimationRepeat(animation);
	}
	
}
