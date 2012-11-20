package cn.natdon.onscripterv2;

import org.json.JSONException;
import org.json.JSONObject;

import android.os.Bundle;

public class Game {

	// Game Title
	public String title;

	// Path/To/Cover/File
	public String cover;

	// Description of the game
	public String description;

	// Optional Path/To/Background/File || blur from cover
	public String background;

	// Optional Path/To/Icon/File
	public String icon;

	// Optional Path/To/Video/File
	public String video;
	
	public void readJSON(JSONObject json) throws JSONException {
		readJSON(json, false);
	}
	
	public void readJSON(JSONObject json, boolean overlay) throws JSONException {
		if(overlay || title == null)
		title = json.getString("title");
		if(overlay || cover == null)
		cover = json.getString("cover");
		if(overlay || description == null)
		description = json.getString("description");
		if(overlay || background == null)
		background = json.getString("background");
		if(overlay || icon == null)
		icon = json.getString("icon");
		if(overlay || video == null)
		video = json.getString("video");
	}

	public static Game fromBundle(Bundle bundle) {
		Game g = new Game();
		g.title = bundle.getString("title");
		g.cover = bundle.getString("cover");
		g.description = bundle.getString("description");
		g.background = bundle.getString("background");
		g.icon = bundle.getString("icon");
		g.video = bundle.getString("video");
		return g;
	}
	
	public Bundle toBundle() {
		Bundle b = new Bundle();
		b.putString("title", title);
		b.putString("cover", cover);
		b.putString("description", description);
		b.putString("background", background);
		b.putString("icon", icon);
		b.putString("video", video);
		return b;
	}
	
}
