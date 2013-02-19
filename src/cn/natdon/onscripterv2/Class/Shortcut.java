package cn.natdon.onscripterv2.Class;

import java.lang.ref.WeakReference;

import android.app.Activity;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.Intent.ShortcutIconResource;
import android.graphics.Bitmap;
import android.graphics.Bitmap.Config;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Matrix;
import android.graphics.Paint;
import android.graphics.Rect;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.view.Display;
import android.view.WindowManager;
import cn.natdon.onscripterv2.R;
import cn.natdon.onscripterv2.start;

public class Shortcut {
	private static final String SHORT_CUT_EXTRAS = "cn.natdon.onscripterv2";

	public static void addShortcut(String name, String path, Activity mActivity) {
		Intent shortcut = new Intent(
				"com.android.launcher.action.INSTALL_SHORTCUT");
		shortcut.putExtra(Intent.EXTRA_SHORTCUT_NAME, name);
		shortcut.putExtra("duplicate", false);
		ComponentName comp = new ComponentName(mActivity.getPackageName(), "."
				+ mActivity.getLocalClassName());
		shortcut.putExtra(Intent.EXTRA_SHORTCUT_INTENT, new Intent(
				Intent.ACTION_MAIN).setComponent(comp));
		Intent shortcutIntent = new Intent(Intent.ACTION_MAIN);
		shortcutIntent.setClass(mActivity, start.class);
		shortcutIntent.putExtra(SHORT_CUT_EXTRAS, path);
		shortcutIntent.putExtra("setting", name);
		shortcut.putExtra(Intent.EXTRA_SHORTCUT_INTENT, shortcutIntent);

		String iconPath = path + "/ICON.PNG";
		Drawable d = Drawable.createFromPath(iconPath);
		if (d != null) {
			shortcut.putExtra(
					Intent.EXTRA_SHORTCUT_ICON,
					generatorContactCountIcon(((BitmapDrawable) (mActivity
							.getResources().getDrawable(R.drawable.icon)))
							.getBitmap(), d, mActivity));
		} else {
			ShortcutIconResource iconRes = Intent.ShortcutIconResource
					.fromContext(mActivity, R.drawable.icon);
			shortcut.putExtra(Intent.EXTRA_SHORTCUT_ICON_RESOURCE, iconRes);
		}
		mActivity.sendBroadcast(shortcut);
	}

	public static Bitmap generatorContactCountIcon(Bitmap icon, Drawable d,
			Activity mActivity) {

		Display disp = ((WindowManager) mActivity
				.getSystemService(Context.WINDOW_SERVICE)).getDefaultDisplay();
		@SuppressWarnings("deprecation")
		int dh = disp.getHeight();
		int iconSize = 32;

		if (dh <= 240)
			iconSize = 48;
		else if (dh < 500 || dh > 500) {
			if (dh == 640)
				iconSize = 96;
			else
				iconSize = 72;

		}

		Bitmap contactIcon = Bitmap.createBitmap(iconSize, iconSize,
				Config.ARGB_8888);
		Canvas canvas = new Canvas(contactIcon);
		BitmapDrawable bd = (BitmapDrawable) d;
		Bitmap bm = bd.getBitmap();

		Paint iconPaint = new Paint();
		iconPaint.setDither(true);
		iconPaint.setFilterBitmap(true);
		Rect src = new Rect(0, 0, iconSize, iconSize);
		Rect dst = new Rect(0, 0, iconSize, iconSize);
		canvas.drawBitmap(contactIcon, src, dst, iconPaint);

		Paint countPaint = new Paint(Paint.ANTI_ALIAS_FLAG
				| Paint.FILTER_BITMAP_FLAG);

		canvas.drawBitmap(CreatMatrixBitmap(bm, iconSize, iconSize), src, dst,
				countPaint);

		return contactIcon;
	}

	private static Bitmap CreatMatrixBitmap(Bitmap bitMap, float scr_width,
			float res_height) {

		int bitWidth = bitMap.getWidth();
		int bitHeight = bitMap.getHeight();
		float scaleWidth = scr_width / (float) bitWidth;
		float scaleHeight = res_height / (float) bitHeight;
		Matrix matrix = new Matrix();
		matrix.postScale(scaleWidth, scaleHeight);
		bitMap = Bitmap.createBitmap(bitMap, 0, 0, bitWidth, bitHeight, matrix,
				true);
		return bitMap;
	}

}