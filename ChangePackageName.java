
import java.io.*;

public class ChangePackageName {
	
	private static String ReadTextFile(String path)
	{
		String txt = null;
		
		FileInputStream   fis = null;
		InputStreamReader isr = null;
		BufferedReader    br  = null;
		try {
            fis = new FileInputStream(path);
            isr = new InputStreamReader(fis, "UTF-8");
			br  = new BufferedReader(isr);
			
			StringBuffer buf = new StringBuffer();
			String linesp = System.getProperty("line.separator");
			
			String line;
            while((line = br.readLine()) != null){
				buf.append(line);
				buf.append(linesp);
			}
			
			txt = buf.toString();
        } catch(IOException e) {
			e.printStackTrace();
        } finally {
			if (br  != null) try { br.close();  } catch (IOException e) {}
			if (isr != null) try { isr.close(); } catch (IOException e) {}
			if (fis != null) try { fis.close(); } catch (IOException e) {}
		}
		
		return txt;
	}
	
	private static void WriteTextFile(String path, String txt)
	{
		FileOutputStream   fos  = null;
		OutputStreamWriter osw = null;
		BufferedWriter     bw  = null;
		try {
            fos = new FileOutputStream(path);
            osw = new OutputStreamWriter(fos, "UTF-8");
			bw  = new BufferedWriter(osw);
			
			bw.write(txt);

			bw.flush();
        } catch(IOException e) {
			e.printStackTrace();
        } finally {
			if (bw  != null) try { bw.close();  } catch (IOException e) {}
			if (osw != null) try { osw.close(); } catch (IOException e) {}
			if (fos != null) try { fos.close(); } catch (IOException e) {}
		}
	}
	
	public static void main(String[] args)
	{
		if(args.length == 0){
			System.out.println("Usage: java ChangePackageName xxx.xxx.xxx");
			return;
		}
		
		String packagename = args[0];
		
		String path;
		String txt;
		
		path = "AndroidManifest.xml";
		txt = ReadTextFile(path);
		txt = txt.replaceFirst("package=\"[A-Za-z0-9\\.]*\"", "package=\"" + packagename + "\"");
		WriteTextFile(path, txt);
		
		path = "build.xml";
		txt = ReadTextFile(path);
		txt = txt.replaceFirst("project name=\"[A-Za-z0-9\\.]*\"", "project name=\"" + packagename.substring(packagename.lastIndexOf(".") + 1) + "\"");
		WriteTextFile(path, txt);
		
		path = "jni/Android.mk";
		txt = ReadTextFile(path);
		txt = txt.replaceFirst("SDL_JAVA_PACKAGE_PATH.*",    "SDL_JAVA_PACKAGE_PATH    := " + packagename.replace(".","_"));
		txt = txt.replaceFirst("SDL_ANDROID_PACKAGE_NAME.*", "SDL_ANDROID_PACKAGE_NAME := " + packagename);
		WriteTextFile(path, txt);
		
		try {
			File searchDirFile = new File("src");
			
			File[] fileArray = searchDirFile.listFiles();
			for(File file : fileArray){
				if(file.isDirectory()){
					continue;
				}
				
				String name = file.getName();
				int lastdotpos = name.lastIndexOf(".");
				if(lastdotpos <= 0){
					continue;
				}
				if(!name.substring(lastdotpos).equals(".java")){
					continue;
				}
				
				path = file.getAbsolutePath();
				txt = ReadTextFile(path);
				txt = txt.replaceFirst("package [A-Za-z0-9\\.]*;", "package " + packagename + ";");
				WriteTextFile(path, txt);
			}
		} catch(Exception e){
			e.printStackTrace();
		}
	}
}
