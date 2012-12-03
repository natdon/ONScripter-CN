package cn.natdon.onscripterv2.anim;

public class NoTargetFoundException extends RuntimeException {

	/**
	 * 
	 */
	private static final long serialVersionUID = -7603430502231150124L;

	public NoTargetFoundException() {
		super("Target View should be set to AnimationAutomata before transfer started");
	}
}
