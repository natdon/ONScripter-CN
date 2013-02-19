package cn.natdon.onscripterv2.command;

public class CommandInvokeException extends RuntimeException {

	private static final long serialVersionUID = 4191722791965346802L;

	public CommandInvokeException(int id, Throwable reason) {
		super("Command " + id + " failed to be invoked.", reason);
	}
	
}
