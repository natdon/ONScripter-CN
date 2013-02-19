package cn.natdon.onscripterv2;

import java.util.ArrayList;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.view.animation.Animation;
import android.widget.ArrayAdapter;
import android.widget.ImageView;
import android.widget.ListAdapter;
import android.widget.TextView;
import cn.natdon.onscripterv2.anim.AnimationAutomata;
import cn.natdon.onscripterv2.anim.AnimationBuilder;
import cn.natdon.onscripterv2.anim.AutomataAction;
import cn.natdon.onscripterv2.anim.StateIO;
import cn.natdon.onscripterv2.anim.StateRunner;
import cn.natdon.onscripterv2.command.Command;
import cn.natdon.onscripterv2.command.CommandHandler;

public class GameAdapter extends ArrayAdapter<Game> implements ListAdapter {

	private static <T> T $(View v, int id) {
		return U.$(v, id);
	}

	// Workaround to make delay load of creation animations {{{
	private long gadLastGetTime = 0;
	private long gadCount = 0;
	private long getJustAddAnimationDelay() {
		long current = System.currentTimeMillis();
		if(current - gadLastGetTime > 200) {
			gadCount = 0;
			gadLastGetTime = current;
		}
		return gadCount++ * 50;
	}

	private int getAnimationInitialState() {
		if(gadLastGetTime == 0) return STATE_CREATED;
		if(System.currentTimeMillis() - gadLastGetTime < 200) return STATE_CREATED;
		return STATE_NORMAL;
	}
	// }}}

	private void setListItemStyle(View v, boolean selected) {
		final ImageView icon = $(v, R.id.icon);
		final TextView caption = $(v, R.id.caption);
		if(!selected) {
			icon.setImageResource(R.drawable.test_icon_0);
			caption.setTextColor(getContext().getResources().getColor(R.color.sao_grey));
			v.setBackgroundColor(getContext().getResources().getColor(R.color.sao_transparent_white));
		} else {
			icon.setImageResource(R.drawable.test_icon_1);
			caption.setTextColor(getContext().getResources().getColor(R.color.sao_white));
			v.setBackgroundColor(getContext().getResources().getColor(R.color.sao_orange));
		}
	}

	public static final int STATE_SELECTED_PANEL = 2;
	public static final int STATE_SELECTED = 1;
	public static final int STATE_NORMAL = 0;
	public static final int STATE_CREATED = -1;

	// When load view with different Item, these state will be used for non-animated switch
	// actions will be immediately taken to switch the state to SELECTED or NORMAL
	public static final int STATE_SELECTED_MIDDLE = -2;
	public static final int STATE_NORMAL_MIDDLE = -3;

	private void buildAutomataForListItem(final View v, StateIO StateHolder) {
		final TextView caption = $(v, R.id.caption);
		final View panel = $(v, R.id.start_panel);
		final View btn_play = $(v, R.id.btn_play);
		final View btn_config = $(v, R.id.btn_config);


		AnimationAutomata.refer(StateHolder).target(v)
		.edit(STATE_CREATED, STATE_NORMAL)
		.setAnimation(AnimationBuilder.create()
				.decelerated(3.0f)
				.alpha(0.0f, 0.8f).pending(getJustAddAnimationDelay()).animateFor(500)
				.valtype(Animation.RELATIVE_TO_SELF)
				.translate(0.0f, 0.0f, 1.7f, 0.0f).animateFor(500)
				.build())
		.edit(STATE_NORMAL, STATE_SELECTED)
		.setAnimation(AnimationBuilder.create()
				.alpha(0.8f, 1.0f).animateFor(200)
				.build())
		.addAction(new AutomataAction() {
			public void onStateChanged(int from, int to) {
				setListItemStyle(v, true);
			}
		})
		.edit(STATE_SELECTED, STATE_SELECTED_PANEL)
		.edit(STATE_SELECTED_PANEL, STATE_SELECTED)
		.edit(STATE_SELECTED, STATE_NORMAL)
		.setAnimation(AnimationBuilder.create()
				.alpha(1.0f, 0.8f).animateFor(200)
				.build())
		.addAction(new AutomataAction() {
			public void onStateChanged(int from, int to) {
				setListItemStyle(v, false);
			}
		})
		.edit(STATE_SELECTED_PANEL, STATE_NORMAL)
		.setAnimation(STATE_SELECTED, STATE_NORMAL)
		.setAction(STATE_SELECTED, STATE_NORMAL)
		.edit(STATE_NORMAL, STATE_SELECTED_MIDDLE)
		.setAnimation(AnimationBuilder.create()
				.alpha(1.0f)
				.build())
		.addAction(new AutomataAction() {
			public void onStateChanged(int from, int to) {
				getAutomata().gotoState(STATE_SELECTED_MIDDLE, STATE_SELECTED);
			}
		})
		.edit(STATE_SELECTED, STATE_NORMAL_MIDDLE)
		.setAnimation(AnimationBuilder.create()
				.alpha(0.8f)
				.build())
		.addAction(new AutomataAction() {
			public void onStateChanged(int from, int to) {
				getAutomata().gotoState(STATE_NORMAL_MIDDLE, STATE_NORMAL);
			}
		})
		.edit(STATE_SELECTED_PANEL, STATE_NORMAL_MIDDLE)
		.setAnimation(STATE_SELECTED, STATE_NORMAL_MIDDLE)
		.setAction(STATE_SELECTED, STATE_NORMAL_MIDDLE)
		.edit(STATE_NORMAL_MIDDLE, STATE_NORMAL)
		.setAction(STATE_SELECTED, STATE_NORMAL)
		.edit(STATE_SELECTED_MIDDLE, STATE_SELECTED)
		.setAction(STATE_NORMAL, STATE_SELECTED)
		;

		AnimationAutomata.refer(StateHolder).target(caption)
		.edit(STATE_SELECTED, STATE_SELECTED_PANEL)
		.setAnimation(AnimationBuilder.create()
				.alpha(1.0f, 0.5f).animateFor(500).accelerated()
				.build())
		.edit(STATE_SELECTED_PANEL, STATE_SELECTED)
		.setAnimation(AnimationBuilder.create()
				.alpha(0.5f, 1.0f).animateFor(500).accelerated()
				.build())
		.edit(STATE_SELECTED_PANEL, STATE_NORMAL)
		.setAnimation(STATE_SELECTED_PANEL, STATE_SELECTED)
		.edit(STATE_SELECTED_PANEL, STATE_NORMAL_MIDDLE)
		.setAnimation(AnimationBuilder.create()
				.alpha(0.5f, 1.0f).animateFor(100)
				.build())
		.edit(STATE_SELECTED, STATE_NORMAL_MIDDLE)
		.setAnimation(STATE_SELECTED_PANEL, STATE_NORMAL_MIDDLE)
		;

		AnimationAutomata.refer(StateHolder).target(panel)
		.edit(STATE_SELECTED, STATE_SELECTED_PANEL)
		.setAnimation(AnimationBuilder.create()
				.alpha(0.0f, 1.0f).animateFor(500).accelerated()
				.build())
		.addAction(new AutomataAction() {
			public void Before(Animation animation) {
				btn_play.setClickable(true);
				btn_config.setClickable(true);
				panel.setClickable(true);
				btn_play.setOnClickListener(mOnPlayClickListener);
				btn_config.setOnClickListener(mOnConfigClickListener);
				panel.setVisibility(View.VISIBLE);
				btn_play.setVisibility(View.VISIBLE);
				btn_config.setVisibility(View.VISIBLE);
			}
		})
		.edit(STATE_SELECTED_PANEL, STATE_SELECTED)
		.setAnimation(AnimationBuilder.create()
				.alpha(1.0f, 0.0f).animateFor(500).accelerated()
				.build())
		.addAction(new AutomataAction() {
			public void After(Animation animation) {
				btn_play.setClickable(false);
				btn_config.setClickable(false);
				panel.setClickable(false);
				btn_play.setOnClickListener(null);
				btn_config.setOnClickListener(null);
				panel.setVisibility(View.GONE);
				btn_play.setVisibility(View.GONE);
				btn_config.setVisibility(View.GONE);
			}
		})
		.edit(STATE_SELECTED_PANEL, STATE_NORMAL)
		.setAnimation(STATE_SELECTED_PANEL, STATE_SELECTED)
		.setAction(STATE_SELECTED_PANEL, STATE_SELECTED)
		.edit(STATE_SELECTED_PANEL, STATE_NORMAL_MIDDLE)
		.setAnimation(AnimationBuilder.create()
				.alpha(0.0f)
				.build())
		.setAction(STATE_SELECTED_PANEL, STATE_SELECTED)
		;

	}


	// Class for storage state in Tag of correspoinding View
	public class Payload {

		Game Item;
		StateRunner StateHolder;

		Payload(final View v) {
			StateHolder = new StateRunner(getAnimationInitialState());

			setListItemStyle(v, false);

			// Ugly set Alpha for compaticity with lower API version
			v.startAnimation(AnimationBuilder.create().alpha(0.8f).animateFor(30).build());

			buildAutomataForListItem(v, StateHolder);
		}

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

	public Game getSelectedItem() {
		if(selectedPos == -1) return null;
		return getItem(selectedPos);
	}

	private OnClickListener mOnConfigClickListener, mOnPlayClickListener;

	public void setOnConfigClickListener(OnClickListener listener) {
		mOnConfigClickListener = listener;
	}

	public void setOnPlayClickListener(OnClickListener listener) {
		mOnPlayClickListener = listener;
	}

	private View mSelectedView = null;

	private View getSelectedView() {
		return mSelectedView;
	}

	public static Payload getLoad(View v) {
		Object o = v.getTag();
		return (o instanceof Payload)?(Payload) o:null;
	}

	public View getView(final int position, View convertView, ViewGroup parent) {
		View v = convertView;
		if (v == null) {
			LayoutInflater vi = (LayoutInflater)getContext().getSystemService(Context.LAYOUT_INFLATER_SERVICE);
			v = vi.inflate(textViewResourceId, null);
			v.setTag(new Payload(v));
			getLoad(v).StateHolder.gotoState(STATE_NORMAL);
		}
		Payload load = getLoad(v);
		Game o = getItem(position);
		if (o != null) {
			@SuppressWarnings("unused")
			ImageView icon = $(v, R.id.icon);
			TextView caption = $(v, R.id.caption);
			caption.setText(o.title);
			if(selectedPos != position) {
				if(load.Item != null && load.Item != o) {
					load.StateHolder.gotoState(STATE_SELECTED, STATE_NORMAL_MIDDLE);
					load.StateHolder.gotoState(STATE_SELECTED_PANEL, STATE_NORMAL_MIDDLE);
				}else{
					load.StateHolder.gotoState(STATE_SELECTED, STATE_NORMAL);
					load.StateHolder.gotoState(STATE_SELECTED_PANEL, STATE_NORMAL);
				}
			}else{
				if(load.Item != null && load.Item != o) {
					load.StateHolder.gotoState(STATE_NORMAL, STATE_SELECTED_MIDDLE);
				}else{
					load.StateHolder.gotoState(STATE_NORMAL, STATE_SELECTED);
				}
				mSelectedView = v;
			}
			load.Item = o;
		}
		return v;
	}

	// Async Operation Block {{{

	static {
		// Register Async Operation
		cn.natdon.onscripterv2.command.Command.register(GameAdapter.class);
	}

	public static final int STATE_CONTROL_COND = 209;

	@CommandHandler(id = STATE_CONTROL_COND)
	public static void STATE_CONTROL_COND(StateIO sio, int cond, int to) {
		sio.gotoState(cond, to);
	}

	public static final int SHOW_PANEL = 210;

	@CommandHandler(id = SHOW_PANEL)
	public static void SHOW_PANEL(GameAdapter list) {
		View v = list.getSelectedView();
		if(v != null) {
			final Payload load = getLoad(v);
			Command.revoke(STATE_CONTROL_COND);
			Command.invoke(STATE_CONTROL_COND).args(load.StateHolder,
					STATE_SELECTED, STATE_SELECTED_PANEL).send();
			Command.invoke(STATE_CONTROL_COND).args(load.StateHolder, 
					STATE_SELECTED_PANEL, STATE_SELECTED).sendDelayed(5000);
		}
	}

	// }}}

}