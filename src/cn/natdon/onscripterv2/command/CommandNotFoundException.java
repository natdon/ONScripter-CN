package cn.natdon.onscripterv2.command;

public class CommandNotFoundException extends RuntimeException {

	private static final long serialVersionUID = 4191722791965346804L;

	public CommandNotFoundException(int id) {
		super("Command " + id + " not found(check your registration).");
	}
	
}
