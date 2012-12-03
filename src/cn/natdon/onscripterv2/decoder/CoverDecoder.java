package cn.natdon.onscripterv2.decoder;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Bitmap.CompressFormat;
import android.graphics.BitmapFactory;
import com.footmark.utils.TimeUnit;
import com.footmark.utils.cache.FileCache;
import com.footmark.utils.hash.IHasher;
import com.footmark.utils.hash.MD5Hasher;
import com.footmark.utils.image.decoder.BitmapDecodeException;
import com.footmark.utils.image.decoder.BitmapDecoder;

public class CoverDecoder extends BitmapDecoder {

	public static int _MAX_WIDTH;
	public static int _MAX_HEIGHT;

	private static FileCache thumbnailcache = null;
	private static IHasher hasher = new MD5Hasher();

	public static void init(Context ctx, int width, int height) {
		_MAX_WIDTH = width;
		_MAX_HEIGHT = height;
		File path = new File(ctx.getCacheDir(), "cover");
		if(! path.exists()) path.mkdir();
		thumbnailcache = new FileCache(path, TimeUnit.WEEK);
	}

	public static String getThumbernailCache(String path) {
		String id = hasher.hash(path);
		if(thumbnailcache != null && thumbnailcache.exists(id)) {
			if(!thumbnailcache.expire(id)) {
				return thumbnailcache.file(id).getAbsolutePath();
			}
		}
		return null;
	}

	public static String rTmp(String any) {
		if(any.endsWith("__tmp__")) {
			return any.substring(0, any.length() - 7);
		}
		return any;
	}

	private final int MAX_WIDTH;
	private final int MAX_HEIGHT;

	public CoverDecoder() {
		MAX_WIDTH = _MAX_WIDTH;
		MAX_HEIGHT = _MAX_HEIGHT;
	}

	public CoverDecoder(int width, int height) {
		MAX_WIDTH = width;
		MAX_HEIGHT = height;
	}

	private Bitmap $(String path) {

		Bitmap rtn = null;

		String id = hasher.hash(rTmp(path));

		FileOutputStream fout = null;

		try{

			BitmapFactory.Options options = new BitmapFactory.Options();
			options.inJustDecodeBounds = true;
			BitmapFactory.decodeFile(path, options);

			long ratio = Math.min(options.outWidth/MAX_WIDTH,
					options.outHeight/MAX_HEIGHT);
			int sampleSize = Integer.highestOneBit((int)Math.floor(ratio));

			if(thumbnailcache != null && thumbnailcache.exists(id)) {
				if(!thumbnailcache.expire(id)) {
					rtn = BitmapFactory.decodeFile(
							thumbnailcache.file(id).getAbsolutePath());
					return rtn;
				}
			}

			BitmapFactory.Options opts = new BitmapFactory.Options();
			opts.inPreferredConfig = Bitmap.Config.RGB_565;
			opts.inSampleSize = sampleSize;

			rtn = BitmapFactory.decodeFile(path, opts);

			if(thumbnailcache != null) {
				fout = thumbnailcache.put(id);
				rtn.compress(CompressFormat.PNG, 90, fout);
				fout.close();
				thumbnailcache.acknowledge(id);
			}

		} catch (OutOfMemoryError oom){
			System.gc();
		} catch (IOException e) {

		} finally {
			try {
				fout.close();
			} catch (Exception e) {

			}
		}

		return rtn;
	}

	public Bitmap decode(String path) {

		Thread.currentThread().setPriority(Thread.NORM_PRIORITY / 2);

		Bitmap rtn = $(path);

		Thread.currentThread().setPriority(Thread.NORM_PRIORITY);

		if(rtn == null)
			throw new BitmapDecodeException();

		return rtn;
	}

}
