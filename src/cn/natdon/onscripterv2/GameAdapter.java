package cn.natdon.onscripterv2;

import java.util.ArrayList;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.animation.AlphaAnimation;
import android.view.animation.Animation;
import android.view.animation.AnimationSet;
import android.view.animation.DecelerateInterpolator;
import android.view.animation.TranslateAnimation;
import android.widget.ArrayAdapter;
import android.widget.ImageView;
import android.widget.ListAdapter;
import android.widget.TextView;

public class GameAdapter extends ArrayAdapter<Game> implements ListAdapter {

	private static <T> T $(View v, int id) {
		return U.$(v, id);
	}

	// Class for storage state in Tag of correspoinding View
	public class ItemViewLoad {
		Game item;
		boolean selected;
	}

	private int textViewResourceId;

	public GameAdapter(Context context, int textViewResourceId, ArrayList<Game> items) {
		super(context, textViewResourceId, items);
		this.textViewResourceId=textViewResourceId;
	}

	private int selectedPos = -1;

	public void setSelectedPosition(int position) {
		selectedPos = position;
		notifyDataSetChanged();
	}

	public int getSelectedPosition() {
		return selectedPos;
	}

	public ItemViewLoad load(View v) {
		Object o = v.getTag();
		return (o instanceof ItemViewLoad)?(ItemViewLoad) o:null;
	}

	private int viewCount = 0;

	public View getView(final int position, View convertView, ViewGroup parent) {
		View v = convertView;
		if (v == null) {
			LayoutInflater vi = (LayoutInflater)getContext().getSystemService(Context.LAYOUT_INFLATER_SERVICE);
			v = vi.inflate(textViewResourceId, null);
			v.setTag(new ItemViewLoad() {{
				item = getItem(position);
				selected = false;
				}});
		}
		Game o = getItem(position);
		if (o != null) {
			ImageView icon = $(v, R.id.icon);
			TextView caption = $(v, R.id.caption);
			caption.setText(o.title);
			if(selectedPos != position) {
				icon.setImageResource(R.drawable.test_icon_0);
				caption.setTextColor(getContext().getResources().getColor(R.color.sao_grey));
				v.setBackgroundColor(getContext().getResources().getColor(R.color.sao_transparent_white));
				// Following code implements v.setAlpha(0.8f);
				if(load(v).selected) {
					leaveSelected(v);
					load(v).selected = false;
				}
				if(convertView == null)
					flyInAnimation(v, 30 * ++viewCount, 0.8f);
			}else{
				icon.setImageResource(R.drawable.test_icon_1);
				caption.setTextColor(getContext().getResources().getColor(R.color.sao_white));
				v.setBackgroundColor(getContext().getResources().getColor(R.color.sao_orange));
				// Following code implements v.setAlpha(1.0f);
				if(!load(v).selected) {
					goSelected(v);
					load(v).selected = true;
				}
				if(convertView == null)
					flyInAnimation(v, 30 * ++viewCount, 1.0f);
			}
		}
		return v;
	}

	/**
	 * List Item Animation Generator
	 * @param v
	 * @param delay
	 * @param alpha
	 */
	private void flyInAnimation(View v, long delay, float alpha) {
		AnimationSet set = new AnimationSet(true);
		AlphaAnimation animAlpha = new AlphaAnimation(0, alpha);
		TranslateAnimation animTrans = new TranslateAnimation(
				Animation.RELATIVE_TO_PARENT, 0.7f, Animation.RELATIVE_TO_PARENT, 0f,
				Animation.RELATIVE_TO_PARENT, 0, Animation.RELATIVE_TO_PARENT, 0);
		animAlpha.setDuration(200);
		animTrans.setDuration(200);
		set.addAnimation(animAlpha);
		set.addAnimation(animTrans);
		set.setStartOffset(delay);
		set.setInterpolator(new DecelerateInterpolator(1.5f));
		set.setFillAfter(true);
		v.startAnimation(set);
	}

	private void goSelected(View v) {
		AlphaAnimation animAlpha = new AlphaAnimation(0.8f, 1.0f);
		animAlpha.setDuration(200);
		animAlpha.setFillAfter(true);
		v.startAnimation(animAlpha);
	}

	private void leaveSelected(View v) {
		AlphaAnimation animAlpha = new AlphaAnimation(0.8f, 0.8f);
		animAlpha.setFillAfter(true);
		v.startAnimation(animAlpha);
	}

}