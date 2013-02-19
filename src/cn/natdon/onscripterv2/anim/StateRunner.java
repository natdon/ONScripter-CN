package cn.natdon.onscripterv2.anim;

import java.util.ArrayList;
import java.util.concurrent.atomic.AtomicInteger;

import android.os.Handler;
import android.os.Message;

/**
 * StateRunner synchronized states between different AnimationAutomata
 * StateRunner itself can transfer its state.
 * @author trinity
 *
 */
public class StateRunner implements StateIO {
	
	private static AtomicInteger msgCounter = new AtomicInteger(0);
	private static Handler handler = new Handler() {
		
		public void handleMessage(Message msg) {
			Object o = msg.obj;
			if(!(o instanceof StateIO)) 
				return;
			StateIO io = (StateIO) o;
			io.onStateTransferred(msg.arg1, msg.arg2, msg.what);
		}
		
	};
	
	private int state;
	
	// To avoid circular notify
	private int lastIssue = 0;

	private ArrayList<StateIO> listener = new ArrayList<StateIO>();
	
	public StateRunner() {
		this(0);
	}
	
	public StateRunner(int initial) {
		state = initial;
	}
	
	public void addSublevelStateIO(StateIO sio) {
		listener.add(sio);
	}
	
	public boolean removeSublevelStateIO(StateIO sio) {
		return listener.remove(sio);
	}
	
	public void clearSublevelStateIO() {
		listener.clear();
	}

	public void onStateTransferred(int before, int after, int issueId) {
		if(lastIssue == issueId) return;
		lastIssue = issueId;
		for(StateIO io : listener) {
			io.onStateTransferred(before, after, issueId);
		}
	}

	public StateIO gotoState(int to) {
		if(state == to) return this;
		int pstate = state;
		state = to;
		handler.handleMessage(handler.obtainMessage(msgCounter.incrementAndGet(), pstate, state, this));
		return this;
	}
	
	public StateIO gotoState(int cond, int to) {
		if(state == to) return this;
		if(state != cond) return this;
		gotoState(to);
		return this;
	}

	public int currentState() {
		return state;
	}
	
	public boolean isAnyAnimatingAutomata() {
		for(StateIO io : listener) {
			if(io instanceof AnimationAutomata) {
				if(((AnimationAutomata) io).isAnimating())
					return true;
			}
		}
		return false;
	}
	
	public boolean isAnyAnimatingAutomata(Object target) {
		for(StateIO io : listener) {
			if(io instanceof AnimationAutomata) {
				if(((AnimationAutomata) io).isAnimating() 
						&& ((AnimationAutomata) io).target() == target)
					return true;
			}
		}
		return false;
	}

}
