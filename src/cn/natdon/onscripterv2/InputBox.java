package cn.natdon.onscripterv2;

import android.app.Activity;
import android.content.Intent;
import android.graphics.Color;
import android.os.Bundle;
import android.view.Gravity;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.PopupWindow;
import android.widget.TextView;
import cn.natdon.onscripterv2.Button.OkCancelButton;

public class InputBox extends Activity{
	public static PopupWindow text_popupWindow;	
	
	private String text,title;
	
	private native int nativeText(String text);
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		super.onCreate(savedInstanceState);
		final LinearLayout size_layout = new LinearLayout(this);
		size_layout.setOrientation(LinearLayout.VERTICAL);		
		size_layout.setBackgroundDrawable(getResources().getDrawable(R.drawable.config_upper));
		
		Intent in=getIntent();
		title=in.getStringExtra("title"); 
		text=in.getStringExtra("text"); 



		final TextView Text_size=new TextView(this);
		Text_size.setTextSize(21);
		Text_size.setText(" ‰»Î");
		Text_size.setGravity(Gravity.CENTER_VERTICAL);
		Text_size.setBackgroundColor(Color.argb(0, 0, 0, 0));
		Text_size.setTextColor(Color.BLACK);
		size_layout.addView(Text_size);

		final EditText editSize =new EditText(this);
		editSize.setText(new String(text));
		editSize.setSelection(editSize.length());
		editSize.setTextColor(Color.BLACK);
		editSize.setGravity(Gravity.CENTER_VERTICAL);
		editSize.setBackgroundColor(Color.argb(0, 0, 0, 0));
		size_layout.addView(editSize);
		
		final OkCancelButton size_btn=new OkCancelButton(this);
		size_btn.setBackgroundColor(Color.argb(0, 0, 0, 0));
		size_btn.setTextSize(21);
		size_btn.setText("»∑∂®");
		size_btn.setTextColor(Color.BLACK);
		size_btn.setOnClickListener(new OnClickListener() {
			public void onClick(View v) {
				debug.put(editSize.getText().toString(), "/name");
				//nativeText();
				finish();
			}
		});
		size_layout.addView(size_btn);
		
		setContentView(size_layout);
	}
	

}
