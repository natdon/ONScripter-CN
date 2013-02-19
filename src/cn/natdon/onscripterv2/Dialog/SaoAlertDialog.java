package cn.natdon.onscripterv2.Dialog;

import android.app.AlertDialog;
import android.content.Context;
import android.os.Bundle;
import cn.natdon.onscripterv2.ONScripter;
import cn.natdon.onscripterv2.R;

public class SaoAlertDialog extends AlertDialog {

	public SaoAlertDialog(Context context) {
		super(context,R.style.AliDialog);
		// TODO Auto-generated constructor stub
	}


	@Override
	protected void onCreate(Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		super.onCreate(savedInstanceState);
		//setContentView(R.layout.sao_dialog);
	}


	public void setItems(String[] items, ONScripter onScripter) {
		// TODO Auto-generated method stub
		
	}

	

}
