package cn.natdon.onscripterv2.Class;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.security.PublicKey;
import java.util.Enumeration;
import java.util.zip.ZipEntry;
import java.util.zip.ZipException;
import java.util.zip.ZipFile;

import android.os.Environment;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import cn.natdon.onscripterv2.ONSVariable;
import cn.natdon.onscripterv2.ONScripter;

public class UnZip {
	private static String ASSETS_NAME = "test.zip";
	private static String DB_PATH = Environment.getExternalStorageDirectory()
			+ "/ons/mytest/";
	private static String DB_TOPATH = Environment.getExternalStorageDirectory()
			+ "/ons/mytest/";
	private static String DB_NAME = "test.zip";

	public static void StartUnZip(final ONScripter mActivity) {
		
		
		final Handler mHandler = new Handler() {

			@Override
			public void handleMessage(Message msg) {
				super.handleMessage(msg);
				switch (msg.what) {
				case 1:
					mActivity.loadCurrentDirectory();
					break;

				default:
					break;
				}
			}
		};

		Thread zipbackground = new Thread(new Runnable() {
			public void run() {

				Looper.prepare();

				try {
					copyDataBase(mActivity);
				} catch (IOException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}

				// unzip
				String path = DB_PATH + DB_NAME;
				File zipFile = new File(path);
				try {
					upZipFile(zipFile, DB_TOPATH);
					Message msg = new Message();
					msg.what = 1;
					mHandler.sendMessage(msg);
				} catch (ZipException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				} catch (IOException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}

				Looper.loop();

			}
		});

		zipbackground.start();
		 
		
	}

	private static void copyDataBase(ONScripter mActivity) throws IOException {
		// Path to the just created empty db
		String outFileName = DB_PATH + DB_NAME;
		// 判断目录是否存在。如不存在则创建一个目录
		File file = new File(DB_PATH);
		if (!file.exists()) {
			file.mkdirs();
		}
		file = new File(outFileName);
		if (!file.exists()) {
			file.createNewFile();
		}
		// Open your local db as the input stream
		InputStream myInput = mActivity.getAssets().open(ASSETS_NAME);
		// Open the empty db as the output stream128
		OutputStream myOutput = new FileOutputStream(outFileName);
		// transfer bytes from the inputfile to the outputfile130
		byte[] buffer = new byte[1024];
		int length;
		while ((length = myInput.read(buffer)) > 0) {
			myOutput.write(buffer, 0, length);
		}
		// Close the streams136
		myOutput.flush();
		myOutput.close();
		myInput.close();
	}

	/**
	 * 解压缩一个文件
	 * 
	 * @param zipFile
	 *            要解压的压缩文件
	 * @param folderPath
	 *            解压缩的目标目录
	 * @throws IOException
	 *             当解压缩过程出错时抛出
	 */
	public static void upZipFile(File zipFile, String folderPath)
			throws ZipException, IOException {
		File desDir = new File(folderPath);
		if (!desDir.exists()) {
			desDir.mkdirs();
		}

		ZipFile zf = new ZipFile(zipFile);
		for (Enumeration<?> entries = zf.entries(); entries.hasMoreElements();) {
			ZipEntry entry = ((ZipEntry) entries.nextElement());
			InputStream in = zf.getInputStream(entry);
			String str = folderPath + File.separator + entry.getName();
			str = new String(str.getBytes("8859_1"), "GB2312");
			File desFile = new File(str);
			if (!desFile.exists()) {
				File fileParentDir = desFile.getParentFile();
				if (!fileParentDir.exists()) {
					fileParentDir.mkdirs();
				}
				desFile.createNewFile();
			}
			OutputStream out = new FileOutputStream(desFile);
			byte buffer[] = new byte[1024];
			int realLength;
			while ((realLength = in.read(buffer)) > 0) {
				out.write(buffer, 0, realLength);
			}
			in.close();
			out.close();
		}
	}

}