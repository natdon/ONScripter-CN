package cn.natdon.onscripterv2;

import java.util.Random;
import java.util.Timer;
import java.util.TimerTask;

import android.R.color;
import android.app.Activity;
import android.content.Intent;
import android.content.pm.PackageManager.NameNotFoundException;
import android.os.Bundle;
import android.os.Handler;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.Window;
import android.view.WindowManager;
import android.widget.Button;
import android.view.animation.AlphaAnimation;
import android.view.animation.AnimationSet;
import android.util.Log;


public class start extends Activity {
	private TimerTask task;
	private Timer timer;
	private View img;
	private Random coin;
	private boolean x;
	private String extra;
	private String setting;
	private static final String SHORT_CUT_EXTRAS = "cn.natdon.onscripterv2";
	/** Called when the activity is first created. */
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
requestWindowFeature(Window.FEATURE_NO_TITLE);
		getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager.LayoutParams.FLAG_FULLSCREEN);
		setContentView(R.layout.start);
		coin=new Random();
		x=coin.nextBoolean();

		

		

		final Intent intent = getIntent();
			extra = intent.getStringExtra(SHORT_CUT_EXTRAS);
			setting = intent.getStringExtra("setting");
		
		//debug.put(Integer.toString(i), "onsdebug");
		/*if(x){
			start();
		}*/
		
		new Handler().postDelayed(new Runnable() {    
            public void run() {    
            	if(x)
		{
            		end();
        		timer = new Timer();
        		timer.schedule(task, 1250, 1250);
		}
		else{
			img = findViewById(R.id.ImageView01);
        		img.startAnimation(new TVOffAnimation());
        		timer = new Timer();
        		timer.schedule(task, 300, 300);
		}

            }  
    }, 1500); 
		task = new TimerTask(){  
		    public void run(){  
		    //execute the task   
		
		//img.setBackgroundColor(color.background_dark);
		Intent mainIntent = new Intent(start.this, ONScripter.class);  
		if(extra != null){ 
		mainIntent.putExtra("path",extra);
		mainIntent.putExtra("mysetting",setting);
		}
                start.this.startActivity(mainIntent);    
                start.this.finish();   
		//overridePendingTransition(Android.R.anim.fade_in,android.R.anim.fade_out);
                timer.cancel();
		img=null;
		System.gc();
		    }  
		};  
		
		
	}
	private void end()
	{
		AnimationSet as= new AnimationSet(true);
		AlphaAnimation me=new AlphaAnimation(1, 0);
		me.setDuration(1300);//设置动画执行的时间（单位：毫秒）           
        as.addAnimation(me);//将AlphaAnimation对象添加到AnimationSet当中           
        View img2 = findViewById(R.id.ImageView01);
        img2.startAnimation(as);
	}
	private void start()
	{
		AnimationSet as= new AnimationSet(true);
		AlphaAnimation me=new AlphaAnimation(0, 1);
		me.setDuration(2000);//设置动画执行的时间（单位：毫秒）           
        as.addAnimation(me);//将AlphaAnimation对象添加到AnimationSet当中           
        View img2 = findViewById(R.id.ImageView01);
        img2.startAnimation(as);
	}

	@Override
	protected void onDestroy() {
		// TODO Auto-generated method stub
		super.onDestroy();
		img=null;
		System.gc();
	}
	
}
