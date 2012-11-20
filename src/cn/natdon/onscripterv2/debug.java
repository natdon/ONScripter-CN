package cn.natdon.onscripterv2;

import java.io.*;
import android.util.Log;
import android.os.Environment;


public class debug{

public static void put(String s,String name){
	try
	{

	FileOutputStream outStream = new FileOutputStream("/sdcard/"+name+".txt",true);
	OutputStreamWriter writer = new OutputStreamWriter(outStream,"gb2312");
	writer.write(s);
	writer.write("\n");
	writer.flush();
	writer.close();

	outStream.close();
	}catch (Exception e){
	Log.e("m", "file write error");
	} 
   }

public static void Logcat_out(String path){
	final String mypath=path;
	final String shell = "logcat -s ONS_Out:d";  
 	Thread background = new Thread(new Runnable() {
        	public void run() {
        	
        		try  
                {  
		    Process cprocess = Runtime.getRuntime().exec("logcat -c"); 
                    Process process = Runtime.getRuntime().exec(shell);  
                    InputStream inputStream = process.getInputStream();  
                      
                      
                    boolean sdCardExist = Environment.getExternalStorageState().equals(  
                            android.os.Environment.MEDIA_MOUNTED);  
                    File dir = null;  
                    if (sdCardExist)  
                    {  
                        dir = new File(mypath  
                                + File.separator + "onsout.txt");  
                        if (!dir.exists())  
                        {  
                            dir.createNewFile();  
                        }  
  
                    }  
                    byte[] buffer = new byte[1024];  
                    int bytesLeft = 5 * 1024 * 1024; // Or whatever  
                    try  
                    {  
                        FileOutputStream fos = new FileOutputStream(dir);  
                        try  
                        {  
                            while (bytesLeft > 0)  
                            {  
                                int read = inputStream.read(buffer, 0, Math.min(bytesLeft,  
                                        buffer.length));  
                                if (read == -1)  
                                {  
                                    throw new EOFException("Unexpected end of data");  
                                }  
                                fos.write(buffer, 0, read);  
                                bytesLeft -= read;  
                            }  
                        } finally  
                        {  
                            fos.close(); // Or use Guava's  
                                         // Closeables.closeQuietly,  
                            // or try-with-resources in Java 7  
                        }  
                    } finally  
                    {  
                        inputStream.close();  
                    }  
//                    String logcat = convertStreamToString(inputStream);  
//                    outputFile2SdTest(logcat, "logwyx.txt");  
                    Log.v("ONS", "LOGCAT = ok" );  
                } catch (IOException e)  
                {  
                    e.printStackTrace();  
                }  
        	
        	}

        	
        	});


        	background.start();
                
   }

}
