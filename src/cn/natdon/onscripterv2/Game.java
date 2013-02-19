package cn.natdon.onscripterv2;

import java.io.File;

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

	// Optional Path/To/Audio/File
	public String audio;
	
	public void readJSON(JSONObject json) throws JSONException {
		readJSON(json, false);
	}
	
	public void readJSON(JSONObject json, boolean overlay) throws JSONException {
		if(json.has("title"))
		if(overlay || title == null)
		title = json.getString("title");
		if(json.has("cover"))
		if(overlay || cover == null)
		cover = json.getString("cover");
		if(json.has("description"))
		if(overlay || description == null)
		description = json.getString("description");
		if(json.has("background"))
		if(overlay || background == null)
		background = json.getString("background");
		if(json.has("icon"))
		if(overlay || icon == null)
		icon = json.getString("icon");
		if(json.has("video"))
		if(overlay || video == null)
		video = json.getString("video");
		if(json.has("audio"))
		if(overlay || audio == null)
		audio = json.getString("audio");
	}

	public static Game fromBundle(Bundle bundle) {
		Game g = new Game();
		g.title = bundle.getString("title");
		g.cover = bundle.getString("cover");
		g.description = bundle.getString("description");
		g.background = bundle.getString("background");
		g.icon = bundle.getString("icon");
		g.video = bundle.getString("video");
		g.audio = bundle.getString("audio");
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
		b.putString("audio", audio);
		return b;
	}
	
	public static Game scanGameDir(File gamedir) {
		Game g = new Game();
		g.title = gamedir.getName();
		File media = new File(gamedir, "media.json");
		if(media.exists()) {
			try {
				JSONObject data = new JSONObject(U.read(media));
				g.readJSON(data, true);
			} catch (Exception e) {}
		}
		if(	g.cover != null && g.background != null && g.video != null && 
			g.icon != null && g.audio != null) return g;
		String[] files = gamedir.list();
		for(String file: files) {
			String name = file.toLowerCase();
			if(name.equals("cover.jpg") || name.equals("cover.png")) {
				if(g.cover == null) 
					g.cover = new File(gamedir, file).getAbsolutePath();
			}
			if(name.equals("background.jpg") || name.equals("background.png") ||
			   name.equals("bkg.jpg") || name.equals("bkg.png")) {
				if(g.background == null) 
					g.background = new File(gamedir, file).getAbsolutePath();
			}
			if(name.equals("preview.mp4") || name.equals("preview.avi") || name.equals("preview.mpg") ||
			   name.equals("pv.mp4") || name.equals("pv.avi") || name.equals("pv.mpg")||
			   name.equals("op.mp4") || name.equals("op.avi") || name.equals("op.mpg")) {
				if(g.video == null) 
					g.video = new File(gamedir, file).getAbsolutePath();
			}
			if(name.equals("theme.mp3") || name.equals("theme.flac") || name.equals("theme.ogg") ||
			   name.equals("theme.wma") ||
			   name.equals("track.mp3") || name.equals("track.flac") || name.equals("track.ogg") || 
			   name.equals("track.wma")) {
				if(g.audio == null) 
					g.audio = new File(gamedir, file).getAbsolutePath();
			}
			if(name.equals("icon.jpg") || name.equals("icon.png")) {
				if(g.icon == null) 
					g.icon = new File(gamedir, file).getAbsolutePath();
			}
		}
		if(g.cover != null && g.video != null && g.audio != null) return g;
		for(String file: files) {
			String name = file.toLowerCase();
			if(name.endsWith(".jpg") || name.endsWith(".png")) {
				if(g.cover == null) 
					g.cover = new File(gamedir, file).getAbsolutePath();
			}
			if(U.supportAudioMedia(name)) {
				if(g.audio == null) 
					g.audio = new File(gamedir, file).getAbsolutePath();
			}
			if(name.startsWith("preview.") && U.supportVideoMedia(name)) {
				if(g.video == null) 
					g.video = new File(gamedir, file).getAbsolutePath();
			}
		}
		if(g.video != null) return g;
		for(String file: files) {
			String name = file.toLowerCase();
			if(U.supportVideoMedia(name)) {
				if(g.video == null) 
					g.video = new File(gamedir, file).getAbsolutePath();
			}
		}
		return g;
	}
	
}
