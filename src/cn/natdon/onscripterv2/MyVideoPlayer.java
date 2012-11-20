package cn.natdon.onscripterv2;

import java.io.IOException;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.content.Intent;
import android.media.AudioManager;
import android.media.MediaPlayer;
import android.media.MediaPlayer.OnBufferingUpdateListener;
import android.media.MediaPlayer.OnCompletionListener;
import android.media.MediaPlayer.OnErrorListener;
import android.os.Bundle;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.KeyEvent;
import android.view.Window;
import android.view.WindowManager;
import android.app.AlertDialog;
import android.content.DialogInterface;

@SuppressLint("NewApi")
public class MyVideoPlayer extends Activity implements
		OnBufferingUpdateListener, OnCompletionListener,
		MediaPlayer.OnPreparedListener, SurfaceHolder.Callback {

	private MediaPlayer mediaPlayer;

	private SurfaceView surfaceView;

	private SurfaceHolder surfaceHolder;

	private int videoWidth;

	private int videoHeight;
	
	private String videofile;

	/** Called when the activity is first created. */
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		requestWindowFeature(Window.FEATURE_NO_TITLE);
		getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager.LayoutParams.FLAG_FULLSCREEN);
		Intent in=getIntent(); //获取intent
		videofile=in.getStringExtra("one"); //获取�?		setContentView(R.layout.video);
		this.surfaceView = (SurfaceView) this.findViewById(R.id.surface);
		this.surfaceHolder = this.surfaceView.getHolder();
		this.surfaceHolder.addCallback(this);
		this.surfaceHolder.setType(SurfaceHolder.SURFACE_TYPE_PUSH_BUFFERS);
		Log.v("mplayer", ">>>create ok.");
		this.surfaceView.setOnClickListener(new View.OnClickListener() {
			
			public void onClick(View v) {
				// TODO Auto-generated method stub
				stop();
			}
		});
	}

	private void stop()
	{
		if (mediaPlayer != null) {
			getWindow().clearFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
			mediaPlayer.stop();
			mediaPlayer.release();
			mediaPlayer = null;
			finish();
			overridePendingTransition(android.R.anim.fade_in,android.R.anim.fade_out);
		}
	}
	
	private void playVideo(String path) throws IllegalArgumentException,
			IllegalStateException, IOException {
		getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
		this.mediaPlayer = new MediaPlayer();
		this.mediaPlayer.setDataSource(path);
		this.mediaPlayer.setDisplay(this.surfaceHolder);
		this.mediaPlayer.prepare();
		this.mediaPlayer.setOnBufferingUpdateListener(this);
		this.mediaPlayer.setOnPreparedListener(this);
		this.mediaPlayer.setAudioStreamType(AudioManager.STREAM_MUSIC);
		Log.v("mplayer", ">>>play video");
		mediaPlayer.setOnErrorListener(new OnErrorListener() {

			public boolean onError(MediaPlayer mp, int what, int extra) {
				// TODO Auto-generated method stub
				new AlertDialog.Builder(MyVideoPlayer.this)
						.setTitle("错误")
						.setMessage("视频格式不正确，播放已停止")
						.setPositiveButton("知道了",
		new AlertDialog.OnClickListener() {

		public void onClick(DialogInterface dialog,int which) {
				stop();
			}

			}).setCancelable(false).show();
				return false;
			}
		});
		mediaPlayer.setOnCompletionListener(new MediaPlayer.OnCompletionListener() {
			public void onCompletion(MediaPlayer mp) {
				stop();
			}
		});
	}

	
	public void onBufferingUpdate(MediaPlayer mp, int percent) {
		// TODO Auto-generated method stub

	}

	
	public void onCompletion(MediaPlayer mp) {
		// TODO Auto-generated method stub

	}

	
	public void onPrepared(MediaPlayer mp) {
		this.videoWidth = this.mediaPlayer.getVideoWidth();
		this.videoHeight = this.mediaPlayer.getVideoHeight();

		if (this.videoHeight != 0 && this.videoWidth != 0) {
			this.surfaceHolder.setFixedSize(this.videoWidth, this.videoHeight);
			this.mediaPlayer.start();
		}
	}

	public void surfaceChanged(SurfaceHolder holder, int format, int width,
			int height) {
		Log.v("mplayer", ">>>surface changed");
	}

	public void surfaceCreated(SurfaceHolder holder) {
		try {
			this.playVideo(videofile);
		} catch (Exception e) {
			Log.e("mplayer", ">>>error", e);
		}
		Log.v("mplayer", ">>>surface created");
	}

	public void surfaceDestroyed(SurfaceHolder holder) {
		Log.v("mplayer", ">>>surface destroyed");
	}

	@Override
	protected void onDestroy() {
		super.onDestroy();
		if (this.mediaPlayer != null) {
			getWindow().clearFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
			this.mediaPlayer.release();
			this.mediaPlayer = null;
		}
	}
	
	@Override
	 protected void onPause() {
	 if (this.mediaPlayer != null) {
	   stop();
	}
	  super.onPause();
	 }

}
