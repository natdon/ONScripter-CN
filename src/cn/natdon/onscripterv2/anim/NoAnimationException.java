package cn.natdon.onscripterv2.anim;

public class NoAnimationException extends RuntimeException {

	/**
	 * 
	 */
	private static final long serialVersionUID = -7603430502231150125L;

	public NoAnimationException() {
		super("An animation should be append or created to perform actions");
	}
}
