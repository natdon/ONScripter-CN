package cn.natdon.onscripterv2;

import java.util.Timer;
import java.util.TimerTask;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.graphics.PixelFormat;
import android.media.AudioManager;
import android.media.MediaPlayer;
import android.media.MediaPlayer.OnBufferingUpdateListener;
import android.media.MediaPlayer.OnCompletionListener;
import android.media.MediaPlayer.OnErrorListener;
import android.media.MediaPlayer.OnPreparedListener;
import android.media.MediaPlayer.OnVideoSizeChangedListener;
import android.os.Bundle;
import android.os.Handler;
import android.util.Log;
import android.view.Gravity;
import android.view.KeyEvent;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup.LayoutParams;
import android.widget.LinearLayout;
import android.widget.Toast;

public class HWPlayer extends Activity {
	private static final String TAG = "PlayVideo";

	private LinearLayout.LayoutParams linlp;
	private LinearLayout linear;

	private boolean isCenter = false;

	private static Preview mPreview;

	private String videofile;

	private long exitTime = 0;

	/** Called when the activity is first created. */
	@Override
	public void onCreate(Bundle icicle) {
		super.onCreate(icicle);

		Intent in = getIntent();
		videofile = in.getStringExtra("one");

		getWindow().setFormat(PixelFormat.UNKNOWN);// PixelFormat.TRANSLUCENT
		try {
			mPreview = new Preview(this);
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
		linear = new LinearLayout(this);

		linear.setOrientation(LinearLayout.HORIZONTAL);

		linlp = new LinearLayout.LayoutParams(LayoutParams.FILL_PARENT,
				LayoutParams.FILL_PARENT);

		linlp.gravity = Gravity.CENTER_HORIZONTAL;

		linear.addView(mPreview, linlp);
		setContentView(linear);

	}

	@Override
	protected void onResume() {

		super.onResume();
		// mPreview.resume();
	}

	@Override
	protected void onPause() {

		super.onPause();
		//mPreview.stopPlay();

	}

	@Override
	protected void onDestroy() {
		// TODO Auto-generated method stub
		super.onDestroy();
		//mPreview.stopPlay();
	}

	public boolean onKeyDown(int keyCode,KeyEvent msg) {
		if (keyCode == KeyEvent.KEYCODE_BACK) {

			if ((System.currentTimeMillis() - exitTime) > 2000) {
				Toast.makeText(this, "再按一次退出播放", Toast.LENGTH_SHORT).show();
				exitTime = System.currentTimeMillis();
				return false;
			} else {

				mPreview.stopPlay();
				
			}
		}

		return super.onKeyDown(keyCode, msg);
	}

	class Preview extends SurfaceView implements OnErrorListener,
			OnBufferingUpdateListener, OnCompletionListener,
			OnPreparedListener, OnVideoSizeChangedListener,
			SurfaceHolder.Callback, OnClickListener {
		private SurfaceHolder mHolder;
		private MediaPlayer mp;

		private int mVideoWidth;
		private int mVideoHeight;
		private boolean mIsVideoSizeKnown = false;
		private boolean mIsVideoReadyToBePlayed = false;

		Preview(Context context) throws InterruptedException {
			super(context);

			mHolder = getHolder();
			mHolder.addCallback(this);

			mHolder.setFixedSize(320, 240);
			mHolder.setType(SurfaceHolder.SURFACE_TYPE_PUSH_BUFFERS);

		}

		// =========================================================
		public boolean onError(MediaPlayer mediaPlayer, int what, int extra) {
			Log.e(TAG, "onError--->   what:" + what + "    extra:" + extra);
			if (mp != null) {
				mp.stop();
				mp.release();
				Intent in = new Intent();
				in.putExtra("one", videofile);
				in.setClass(HWPlayer.this, VitamioPlayer.class);
				HWPlayer.this.startActivity(in);
			}
			return true;
		}

		// --------------------------------------------------
		public void onBufferingUpdate(MediaPlayer arg0, int percent) {
			Log.d(TAG, "onBufferingUpdate percent:" + percent);
		}

		public void onCompletion(MediaPlayer arg0) {
			Log.d(TAG, "onCompletion called");
			stopPlay();
		}

		public void onVideoSizeChanged(MediaPlayer mp, int width, int height) {
			Log.v(TAG, "onVideoSizeChanged called");
			if (width == 0 || height == 0) {
				Log.e(TAG, "invalid video width(" + width + ") or height("
						+ height + ")");
				return;
			}
			mIsVideoSizeKnown = true;
			mVideoWidth = width;
			mVideoHeight = height;
			if (mIsVideoReadyToBePlayed && mIsVideoSizeKnown) {
				startVideoPlayback();
			}
		}

		public void onPrepared(MediaPlayer mediaplayer) {
			Log.d(TAG, "onPrepared called");
			mIsVideoReadyToBePlayed = true;
			if (mIsVideoReadyToBePlayed && mIsVideoSizeKnown) {
				startVideoPlayback();
			}

		}

		public void onClick(View v) {

			int displayWidth = getWindowManager().getDefaultDisplay()
					.getWidth();
			int displayHeight = getWindowManager().getDefaultDisplay()
					.getHeight();
			int targetWidth = -1;
			int targetHeight = -1;

			if (!isCenter) {
				targetWidth = 16;
				targetHeight = 9;
				isCenter = true;
			} else {
				targetWidth = -1;
				targetHeight = -1;
				isCenter = false;
			}

			if (targetWidth > 0 && targetHeight > 0) {
				double ard = (double) displayWidth / (double) displayHeight;
				double art = (double) targetWidth / (double) targetHeight;
				if (ard > art) {
					displayWidth = displayHeight * targetWidth / targetHeight;
				} else {
					displayHeight = displayWidth * targetHeight / targetWidth;
				}
			}
			LayoutParams lp = Preview.this.getLayoutParams();
			lp.width = displayWidth;
			lp.height = displayHeight;

			Preview.this.setLayoutParams(lp);
			mPreview.mHolder.setSizeFromLayout();

			linlp.gravity = Gravity.CENTER;

			linear.updateViewLayout(mPreview, linlp);

			Preview.this.invalidate();
		}

		private void startVideoPlayback() {
			Log.v(TAG, "startVideoPlayback");
			mPreview.mHolder.setFixedSize(mVideoWidth, mVideoHeight);

			try {
				mp.start();
			} catch (IllegalStateException e) {
				Log.i(TAG, e.toString());
				e.printStackTrace();
			}
		}

		public void surfaceCreated(SurfaceHolder holder) {
			playVideo(videofile);
		}

		public void surfaceDestroyed(SurfaceHolder holder) {

		}

		public void surfaceChanged(SurfaceHolder holder, int format, int w,
				int h) {
			// Surface size or format has changed. This should not happen in
			// this example.
		}

		// ----------------------------------------------------------------------
		private void playVideo(String path) {
			try {

				Log.v(TAG, "height: " + this.getHeight());
				Log.v(TAG, "width: " + this.getWidth());

				mp = new MediaPlayer();
				mp.setAudioStreamType(AudioManager.STREAM_MUSIC);

				mp.setOnErrorListener(this);

				mp.setDisplay(mHolder);

				try {
					mp.setDataSource(path);
				} catch (Exception e) {
					e.printStackTrace();
				}

				// mp.prepareAsync();
				mp.prepare();
				Log.v(TAG, "Duration:  ===>" + mp.getDuration());
				mp.setOnBufferingUpdateListener(this);
				mp.setOnCompletionListener(this);
				mp.setOnVideoSizeChangedListener(this);
				mp.setOnPreparedListener(this);
				Preview.this.setOnClickListener(this);

				// mp.start();
			} catch (Exception e) {
				// e.printStackTrace();
				Log.e(TAG, "error: " + e.getMessage());
				stopPlay();
			}
		}

		private void stopPlay() {
			if (mp != null) {
				mp.stop();
				mp.release();
				finish();
			}
		}
	}

}
