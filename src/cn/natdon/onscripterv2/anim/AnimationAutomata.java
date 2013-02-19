package cn.natdon.onscripterv2.anim;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import android.view.View;
import android.view.animation.Animation;

/**
 * Automata builder for Animation states.
 * An automata should be attatched to a View and be passed a StateRunner.
 * Then when StateRunner transfer states, animation should be correctly triggered.
 * @author trinity
 *
 */
public class AnimationAutomata implements StateIO {

	public static AnimationAutomata refer(StateIO sio) {
		if(sio == null) sio = new StateRunner();
		return new AnimationAutomata(sio);
	}
	
	private final StateIO runner;
	
	private View target;
	
	// To avoid circular notify
	private int lastIssue = 0;
	
	private Map<Long, Animation> animations = new HashMap<Long, Animation>();
	private Map<Long, ArrayList<AutomataAction>> actions = new HashMap<Long, ArrayList<AutomataAction>>();
	
	private Animation current = null;
	
	private AnimationAutomata(StateIO sio) {
		runner = sio;
		runner.addSublevelStateIO(this);
	}
	
	public AnimationAutomata target(View v) {
		target = v;
		return this;
	}
	
	public View target() {
		return target;
	}

	private int cesFrom, cesTo;
	/**
	 * Set the current editing transfer path
	 * @param from
	 * @param to
	 * @return
	 */
	public AnimationAutomata edit(int from, int to) {
		cesFrom = from;
		cesTo = to;
		return this;
	}
	
	/**
	 * Attatch animation listener to current editing transfer path
	 * @param listener
	 * @return
	 */
	public AnimationAutomata addAction(AnimationListener listener) {
		this.addAction(cesFrom, cesTo, listener);
		return this;
	}
	
	/**
	 * Attatch animation listener for specific transfer
	 * @param from
	 * @param to
	 * @param action
	 * @return
	 */
	public AnimationAutomata addAction(int from, int to, AnimationListener listener) {
		this.addAction(from, to, new AutomataAction(listener));
		return this;
	}
	
	/**
	 * Add AutomataAction to current editing transfer path
	 * @param action
	 * @return
	 */
	public AnimationAutomata addAction(AutomataAction action) {
		this.addAction(cesFrom, cesTo, action);
		return this;
	}
	
	/**
	 * Add AutomataAction from this state to another state
	 * @param from
	 * @param to
	 * @param action
	 * @return
	 */
	public AnimationAutomata addAction(int from, int to, AutomataAction action) {
		long key = makeLong(from, to);
		ArrayList<AutomataAction> list = actions.get(key);
		if(list == null) {
			list = new ArrayList<AutomataAction>();
			actions.put(key, list);
		}
		list.add(action);
		return this;
	}
	
	/**
	 * Set the current editing action same as given path
	 * @param srcFrom
	 * @param srcTo
	 * @return
	 */
	public AnimationAutomata setAction(int srcFrom, int srcTo) {
		this.setAction(cesFrom, cesTo, srcFrom, srcTo);
		return this;
	}
	
	/**
	 * Add the action of given path to current editing path
	 * @param srcFrom
	 * @param srcTo
	 * @return
	 */
	public AnimationAutomata addAction(int srcFrom, int srcTo) {
		this.addAction(cesFrom, cesTo, srcFrom, srcTo);
		return this;
	}

	/**
	 * Assign actions of one path to another
	 * @param from
	 * @param to
	 * @param srcFrom
	 * @param srcTo
	 * @return
	 */
	public AnimationAutomata setAction(int from, int to, int srcFrom, int srcTo) {
		long key = makeLong(from, to);
		actions.put(key, actions.get(makeLong(srcFrom, srcTo)));
		return this;
	}
	
	/**
	 * Copy actions of one path to another
	 * @param from
	 * @param to
	 * @param srcFrom
	 * @param srcTo
	 * @return
	 */
	public AnimationAutomata addAction(int from, int to, int srcFrom, int srcTo) {
		long key = makeLong(from, to);
		ArrayList<AutomataAction> list = actions.get(key);
		if(list == null) {
			list = new ArrayList<AutomataAction>();
			actions.put(key, list);
		}
		for(AutomataAction action : actions.get(makeLong(srcFrom, srcTo))) {
			list.add(action);
		}
		return this;
	}
	
	
	/**
	 * Set animation of current editing transfer path
	 * @param anim
	 * @return
	 */
	public AnimationAutomata setAnimation(Animation anim) {
		this.setAnimation(cesFrom, cesTo, anim);
		return this;
	}
	
	/**
	 * Set animation of current editing transfer path generated from AnimationFactory
	 * @param animf
	 * @return
	 */
	public AnimationAutomata setAnimation(AnimationFactory animf) {
		this.setAnimation(cesFrom, cesTo, animf);
		return this;
	}
	
	/**
	 * Set the current editing path's Animation to the given once's
	 * @param srcFrom
	 * @param srcTo
	 * @return
	 */
	public AnimationAutomata setAnimation(int srcFrom, int srcTo) {
		this.setAnimation(cesFrom, cesTo, srcFrom, srcTo);
		return this;
	}
	
	/**
	 * Set Animation from this state to another state
	 * Animation Listener will be set, so yours should be passed via addAction method
	 * Animations added should not be modified
	 * @param from
	 * @param to
	 * @param anim
	 * @return
	 */
	public AnimationAutomata setAnimation(int from, int to, Animation anim) {
		long key = makeLong(from, to);
		animations.put(key, anim);
		return this;
	}
	
	/**
	 * Set Animation from this state to another state
	 * Animation is created from AnimationFactory
	 * @param from
	 * @param to
	 * @param animf
	 * @return
	 */
	public AnimationAutomata setAnimation(int from, int to, AnimationFactory animf) {
		long key = makeLong(from, to);
		if(animf instanceof AutomataAnimationFactory) {
			AutomataAnimationFactory animfa = (AutomataAnimationFactory) animf;
			animfa.setContext(this);
		}
		animations.put(key, animf.make());
		return this;
	}
	
	/**
	 * Set the Animation of another transfer to this transfer
	 * @param from
	 * @param to
	 * @param srcFrom
	 * @param srcTo
	 * @return
	 */
	public AnimationAutomata setAnimation(int from, int to, int srcFrom, int srcTo) {
		long key = makeLong(from, to);
		animations.put(key, animations.get(makeLong(srcFrom, srcTo)));
		return this;
	}
	
	public Animation getAnimation(int from, int to) {
		long key = makeLong(from, to);
		Animation anim = animations.get(key);
		return anim;
	}

	public void onStateTransferred(int before, int after, int issueId) {
		if(lastIssue == issueId) return;
		lastIssue = issueId;
		// Take actions
		long key = makeLong(before, after);
		Animation anim = animations.get(key);
		final List<AutomataAction> action = actions.get(key);
		if(anim != null) {
			if(target == null) throw new NoTargetFoundException();
			if(current != anim && current != null && !current.hasEnded()) {
				current.cancel();
				target.clearAnimation();
			}
			anim.reset();
			
			anim.setAnimationListener(new android.view.animation.Animation.AnimationListener() {
				
				public void onAnimationEnd(Animation animation) {
					m_isAnimating = false;
					if(action != null)
					for(AutomataAction a : action) {
						a.setAutomata(AnimationAutomata.this);
						a.onAnimationEnd(animation);
					}
				}
				
				public void onAnimationStart(Animation animation) {
					m_isAnimating = true;
					if(action != null)
					for(AutomataAction a : action) {
						a.setAutomata(AnimationAutomata.this);
						a.onAnimationStart(animation);
					}
				}
				
				public void onAnimationRepeat(Animation animation) {
					if(action != null)
					for(AutomataAction a : action) {
						a.setAutomata(AnimationAutomata.this);
						a.onAnimationRepeat(animation);
					}
				}
				
			});
			current = anim;
			target.startAnimation(anim);
		}
		if(action != null) {
			for(AutomataAction a : action) {
				a.setAutomata(AnimationAutomata.this);
				a.onStateChanged(before, after);
			}
			if(anim == null) {
				for(AutomataAction a : action) {
					a.setAutomata(AnimationAutomata.this);
					a.onAnimationStart(null);
					a.onAnimationEnd(null);
				}
			}
		}
	}

	// Following code behave as a wrapper to StateIO
	public StateIO gotoState(int to) {
		runner.gotoState(to);
		return this;
	}
	
	public StateIO gotoState(int cond, int to) {
		runner.gotoState(cond, to);
		return this;
	}

	public int currentState() {
		return runner.currentState();
	}

	public void addSublevelStateIO(StateIO sio) {
		runner.addSublevelStateIO(sio);
	}
	
	public boolean removeSublevelStateIO(StateIO sio) {
		return runner.removeSublevelStateIO(sio);
	}

	public void clearSublevelStateIO() {
		runner.clearSublevelStateIO();
	}
	
	// Utility Function
	public static long makeLong(int low, int high) { 
		return ((long)low & 0xFFFFFFFFl) | (((long)high << 32) & 0xFFFFFFFF00000000l); 
	}
	
	private boolean m_isAnimating = false;
	
	public boolean isAnimating() {
		return m_isAnimating;
	}
	
}
