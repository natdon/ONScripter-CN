package cn.natdon.onscripterv2.command;

import java.lang.reflect.Method;

public class DuplicateCommandIdentifierException extends RuntimeException {

	private static final long serialVersionUID = 4191722791965346802L;

	public DuplicateCommandIdentifierException(int id, Method m1, Method m2) {
		super("Command " + id + " need by " + m2.getName() + " have been taken by " + m1.getName());
	}
	
}
