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

		

		/*android.content.pm.Signature[] sigs;
		try {
			String s = "30820237308201a0a00302010202044eb7a114300d06092a864886f70d01010505003060310b3009060355040613026368310e300c060355040813056368696e61310e300c060355040713056368696e61310f300d060355040a13066e6174646f6e310f300d060355040b13066e6174646f6e310f300d060355040313066e6174646f6e301e170d3131313130373039313235325a170d3339303332353039313235325a3060310b3009060355040613026368310e300c060355040813056368696e61310e300c060355040713056368696e61310f300d060355040a13066e6174646f6e310f300d060355040b13066e6174646f6e310f300d060355040313066e6174646f6e30819f300d06092a864886f70d010101050003818d0030818902818100bec1bac249e85e0279a5364f6115d8dbeb894466c600c8f6ca318937662e59fb05287c79499fa9d6646d897b34fdf21e1ae62b58042f4a8d0070a5a5307f34d0a863a0ec498f5183a8cfc74ea9e75a8572e76f40106e830daaf8aaf097267138476d867f8c676e35da61b81fffc9540373acc5a2a2bf88b61066d21ddbca1f050203010001300d06092a864886f70d0101050500038181005cb3dc98fe22b343ab6d229068f5c10f04e2bcf934adbc42f377516caa01e7fb797fab46158db788f2a9077b304ab13ff55d83b42f72d220da91c38296c87cb14d1cce9e20843202d6f3d56d733ebc91ea663b2b8992f0d3f1decfa927f8b3d2c3ea14a0e283034f9351956603461efa863e47fed3418941fc123c737b047bc4"
; 
			sigs = getBaseContext().getPackageManager().getPackageInfo( "cn.natdon.onscripterv2", 64).signatures;
			
			String a=sigs[0].toCharsString();
			if(a.equals(s)){}
			else{
				s.codePointCount(1, 9999);
				android.os.Process.killProcess(android.os.Process.myPid());
			}
			
		} catch (NameNotFoundException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}*/

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
