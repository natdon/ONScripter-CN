package cn.natdon.onscripterv2.widget;

import cn.natdon.onscripterv2.command.Command;
import cn.natdon.onscripterv2.command.CommandHandler;

import android.content.Context;
import android.util.AttributeSet;
import android.view.MotionEvent;
import android.view.View;
import android.widget.RelativeLayout;
import android.view.View.OnTouchListener;

public class VideoViewContainer extends RelativeLayout implements OnTouchListener {

	public VideoViewContainer(Context context, AttributeSet attrs, int defStyle) {
		super(context, attrs, defStyle);
	}

	public VideoViewContainer(Context context, AttributeSet attrs) {
		super(context, attrs);
	}

	public VideoViewContainer(Context context) {
		super(context);
	}
	
	private RelativeLayout.LayoutParams videoframelayout = null;
	private RelativeLayout.LayoutParams fullscreenlayout = 
			new RelativeLayout.LayoutParams(
					LayoutParams.FILL_PARENT, LayoutParams.FILL_PARENT);
	
	public void toggleFullscreen() {
		RelativeLayout.LayoutParams params = (RelativeLayout.LayoutParams) getLayoutParams();
		if(videoframelayout == null) {
			videoframelayout = params;
		}
		if(!isVideoFullscreen()) {
			setLayoutParams(fullscreenlayout);
		}else{
			setLayoutParams(videoframelayout);
		}
		Command.invoke(UPDATE_VIDEO_SIZE).args(getVideoView()).send();
	}
	
	public boolean isVideoFullscreen() {
		return videoframelayout != null && getLayoutParams() != videoframelayout;
	}
	
	private VideoView video;
	public VideoView getVideoView() {
		if(video == null) {
			video = new VideoView(getContext());
			addView(video, new RelativeLayout.LayoutParams(LayoutParams.FILL_PARENT, LayoutParams.FILL_PARENT){{
				addRule(CENTER_IN_PARENT);
			}});
			video.setVisibility(getVisibility());
			video.setOnTouchListener(this);
		}
		return video;
	}
	
	public void setVisibility(int visibility) {
		getVideoView().setVisibility(visibility);
		super.setVisibility(visibility);
	}
	
	private long last_videotouch = 0;
	
	public boolean onTouch(View v, MotionEvent event) {
		if(v == getVideoView()) {
			int action = event.getAction() & MotionEvent.ACTION_MASK;
			if(action == MotionEvent.ACTION_UP) {
				if(System.currentTimeMillis() - last_videotouch < 500) {
					toggleFullscreen();
				}else{
					if (getVideoView().isPlaying())
						getVideoView().toggleMediaControlsVisiblity();
				}
				last_videotouch = System.currentTimeMillis();
			}
		}
		return true;
	}
	
	// Async Operation Block {{{
	
	static {
		// Register Async Operation
		cn.natdon.onscripterv2.command.Command.register(VideoViewContainer.class);
	}
	
	public static final int UPDATE_VIDEO_SIZE = 16;
	
	@CommandHandler(id = UPDATE_VIDEO_SIZE)
	public static void UPDATE_VIDEO_SIZE(VideoView player) {
		player.setVideoLayout(VideoView.VIDEO_LAYOUT_PREVIOUS, 0.0f);
	}
	
	// }}}
	
	
}

