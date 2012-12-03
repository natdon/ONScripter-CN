package cn.natdon.onscripterv2.anim;

public interface StateIO {

	/**
	 * Event triggered from before state to after state
	 * @param before
	 * @param after
	 * @param issueId
	 * Identifier for each event
	 */
	public void onStateTransferred(int before, int after, int issueId);
	
	/**
	 * Go from current state to destination state
	 * @param to
	 * @return
	 */
	public StateIO gotoState(int to);
	
	/**
	 * Go from current state to destination state conditionally
	 * @param to
	 * @return
	 */
	public StateIO gotoState(int cond, int to);
	
	/**
	 * Retrieve the current state of the machine
	 * @return
	 */
	public int currentState();
	
	/**
	 * Register listener
	 * @param sio
	 */
	public void addSublevelStateIO(StateIO sio);
	
	/**
	 * Remove Listener
	 */
	public boolean removeSublevelStateIO(StateIO sio);
	
	/**
	 * Clear All Listeners
	 * @param sio
	 */
	public void clearSublevelStateIO();
	
}
