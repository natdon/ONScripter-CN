package cn.natdon.onscripterv2;

import io.vov.vitamio.MediaPlayer;
import io.vov.vitamio.MediaPlayer.OnCompletionListener;
import io.vov.vitamio.Vitamio;
import android.app.Activity;
import android.content.Intent;
import android.content.res.Configuration;
import android.os.AsyncTask;
import android.os.Bundle;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Toast;
import cn.natdon.onscripterv2.widget.MediaController;
import cn.natdon.onscripterv2.widget.VideoView;

public class VitamioPlayer extends Activity{
	private VideoView mVideoView;
	private String videofile;
	private static volatile boolean isVideoInitialized = false;

	@Override
	public void onCreate(Bundle icicle) {
		super.onCreate(icicle);
		isInit();
		Intent in=getIntent();
		videofile=in.getStringExtra("one"); 
		setContentView(R.layout.game_videoview);
		if(isVideoInitialized)
			Playvideo(videofile);
		
	}
	
	
	void isInit()
	{
		if(!Vitamio.isInitialized(this)) {
			
			new AsyncTask<Object, Object, Boolean>() {
				@Override
				protected void onPreExecute() {
					isVideoInitialized = false;
					
				}

				@Override
				protected Boolean doInBackground(Object... params) {
					Thread.currentThread().setPriority(Thread.MIN_PRIORITY);
					boolean inited = Vitamio.initialize(VitamioPlayer.this);
					Thread.currentThread().setPriority(Thread.NORM_PRIORITY);
					return inited;
				}

				@Override
				protected void onPostExecute(Boolean inited) {
					if (inited) {
						isVideoInitialized = true;
						Playvideo(videofile);
						
					}
				}

			}.execute();
			Toast.makeText(this, "此为第一次播放,正在后台解压解码器,期间勿进行其他操作, 完成后开始播放...", Toast.LENGTH_LONG).show();
		}else{
			isVideoInitialized = true;
		}
	}
	
	void Playvideo(String path)
	{
		if(isVideoInitialized){
			mVideoView = (VideoView) findViewById(R.id.surface_view);
			mVideoView.setVideoPath(path);
			mVideoView.setVideoQuality(MediaPlayer.VIDEOQUALITY_HIGH);
			mVideoView.setMediaController(new MediaController(this));
			mVideoView.setOnClickListener(new OnClickListener() {
				
				public void onClick(View v) {
					if (mVideoView.isPlaying())
						mVideoView.toggleMediaControlsVisiblity();
					
				}
			});
			mVideoView.setOnCompletionListener(new OnCompletionListener() {
				
				public void onCompletion(MediaPlayer arg0) {
					mVideoView.stopPlayback();
					finish();
				}
			});
		}
	}


	@Override
	public void onConfigurationChanged(Configuration newConfig) {
		if (mVideoView != null)
			mVideoView.setVideoLayout(VideoView.VIDEO_LAYOUT_SCALE, 0);
		super.onConfigurationChanged(newConfig);
	}

	@Override
	protected void onDestroy() {
		if(mVideoView.isPlaying())
			mVideoView.stopPlayback();
		super.onDestroy();
	}

	@Override
	protected void onPause() {
		if(mVideoView.isPlaying())
			mVideoView.stopPlayback();
		super.onPause();
	}

	@Override
	protected void onStop() {
		if(mVideoView.isPlaying())
			mVideoView.stopPlayback();
		super.onStop();
	}
	
	
	
	

}
