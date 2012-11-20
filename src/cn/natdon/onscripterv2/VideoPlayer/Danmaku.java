package cn.natdon.onscripterv2.VideoPlayer;

import android.app.Application;

public class Danmaku extends Application {

	/*dead code*/
	// protected boolean initialize() {
	// // prepare, orz
	// String root = super.getCacheDir().getAbsolutePath();
	// try {
	// ArrayList<String> list = new ArrayList<String>();
	// AssetManager asset = getAssets();
	// InputStream is = asset.open("index.txt");
	// InputStreamReader ir = new InputStreamReader(is);
	// BufferedReader br = new BufferedReader(ir);
	// try {
	// while (br.ready()) {
	// String line = br.readLine();
	// list.add(line);
	// }
	// } finally {
	// br.close();
	// ir.close();
	// is.close();
	// }
	// for (String file : list) {
	// String path = String.format("%s/%s", root, file);
	// String parent = path.substring(0, path.lastIndexOf('/'));
	// File test = new File(parent);
	// if (!test.isDirectory()) {
	// test.mkdirs();
	// if (!test.isDirectory())
	// return false;
	// }
	// test = new File(path);
	// // if (test.exists())
	// // continue;
	// byte[] buffer = new byte[32768];
	// is = asset.open(file);
	// OutputStream os = new FileOutputStream(path);
	// try {
	// while (true) {
	// int rc = is.read(buffer);
	// if (rc <= 0)
	// break;
	// os.write(buffer, 0, rc);
	// }
	// } finally {
	// os.close();
	// is.close();
	// }
	// }
	// } catch (IOException e) {
	// return false;
	// }
	// // start VLC
	// String libd = String.format("%s/lib", root);
	// // VLC.setenv("VLC_PLUGIN_PATH", libd, true);
	// String conf = String.format("%s/etc/vlcrc", root);
	// String aout = String.format("aout_android");
	// String vout = String.format("vout_android");
	// // XXX: --intf, --aout, --vout don't make sense here
	// // VLC.getInstance().create(
	// // new String[] { "--verbose", "3", "--no-ignore-config",
	// // "--config", conf, "--no-plugins-cache", "--intf",
	// // "reporter", "--aout", aout, "--vout", vout });
	// // start VLM
	// // VLM.getInstance()
	// // .create(new String[] { "127.0.0.1", "21178", "20001" });
	//
	// return true;
	// }
	//
	// protected void finalize() {
	// // stop VLM
	// // VLM.getInstance().destroy();
	// // stop VLC
	// // VLC.getInstance().destroy();
	// }

	@Override
	public void onCreate() {
		super.onCreate();

		System.setProperty("java.net.preferIPv6Addresses", "false");

	}

	@Override
	public void onTerminate() {
		super.onTerminate();

	}

}
