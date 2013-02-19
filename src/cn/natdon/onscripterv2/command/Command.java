package cn.natdon.onscripterv2.command;

import java.lang.reflect.Method;
import java.lang.reflect.Modifier;

import android.os.Handler;
import android.os.Message;
import android.util.SparseArray;
import cn.natdon.onscripterv2.U;

/**
 * This class is the utility for UIHandler
 * Construction, argument assignment and some other works
 * @author trinity
 *
 */
public class Command {

	protected static <T> T $(Object o) {
		return U.$(o);
	}

	// obj - Runnable
	public static final int RUN = 0;
	
	private static final SparseArray<Method> exec = new SparseArray<Method>();
	
	public static void register(Class<?> clazz) {
		Method[] methods = clazz.getDeclaredMethods();
		for(Method m: methods) {
			int mod = m.getModifiers();
			if(Modifier.isStatic(mod) && Modifier.isPublic(mod)){
				CommandHandler annot = m.getAnnotation(CommandHandler.class);
				if(annot == null) continue;
				int id = annot.id();
				Method m1 = exec.get(id);
				if(m1 != null)
					throw new DuplicateCommandIdentifierException(id, m1, m);
				exec.append(id, m);
			}
		}
	}
	
	private static Handler Commander = new Handler() {

		public void handleMessage(Message msg) {
			switch(msg.what){
			case RUN:
				if(msg.obj instanceof Runnable) {
					Runnable runnable = $(msg.obj);
					runnable.run();
				}
				break;
			default:
				Method m = exec.get(msg.what);
				if(m == null) 
					throw new CommandNotFoundException(msg.what);
				Object[] args = $(msg.obj);
				try {
					m.invoke(null, args);
				} catch (Exception e) {
					throw new CommandInvokeException(msg.what, e);
				}
			}
		}

	};

	public static Command invoke(Runnable run) {
		Command cmd = new Command();
		cmd.msg.what = RUN;
		cmd.msg.obj = run;
		return cmd;
	}

	public static Command invoke(int progId) {
		Command cmd = new Command();
		cmd.msg.what = progId;
		return cmd;
	}

	public static void revoke(int progId) {
		Commander.removeMessages(progId);
	}

	private final Message msg;

	private Command() {
		msg = Message.obtain(Commander);
	}

	public Command args(Object... args) {
		msg.obj = args;
		return this;
	}
	
	public Command only() {
		Commander.removeMessages(msg.what);
		return this;
	}
	
	public Command exclude(int progId) {
		Commander.removeMessages(progId);
		return this;
	}

	public Message getMessage() {
		return msg;
	}

	public void send() {
		Commander.sendMessage(msg);
	}

	public void sendAtTime(long timeMillis) {
		Commander.sendMessageAtTime(msg, timeMillis);
	}

	public void sendDelayed(long delay) {
		Commander.sendMessageDelayed(msg, delay);
	}

}
