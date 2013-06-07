/*
2012/7 Created by AKIZUKI Katane
*/

package cn.natdon.onscripterv2;

import android.app.Activity;
import android.content.pm.ActivityInfo;
import android.content.Context;
import java.util.TreeMap;
import java.util.HashMap;
import java.util.ArrayList;
import android.view.KeyEvent;

import android.os.Environment;

class Locals {
	
	//App Setting

	public static String AppModuleName = Globals.APP_MODULE_NAME_ARRAY[0]; //do not change
	
	public static String AppCommandOptions = "--force-button-shortcut";

	public static Boolean gDisableRescale = false;
	public static Boolean gWideScreen = false;
	public static Boolean gFullScreen = false;
	public static Boolean gScaleFullScreen = false;
	public static Boolean gWindowScreen = false;
	public static Boolean gDisableVideo = false;
	public static Boolean gOtherPL = false;
	public static Boolean gKeepON = false;
	public static Boolean mylanguage = true;
	public static Boolean Orientation = false;
	public static Boolean ScreenMove = false;
	public static Boolean ScreenHide = false;
	public static Boolean Logout = false;
	public static Boolean HideClick = false;
	public static Boolean ShowCursor = false;
	public static Boolean gFontSize = false;
	public static Boolean gFontColor = false;
	public static Boolean VideoMode = false;
	public static Boolean isxsystem = false;
	
	//Environment

	public static HashMap<String,String> EnvironmentMap = new HashMap<String,String>(); //do not change
	static {
		//EnvironmentMap.put("EXAMPLE_KEY", "ExampleValue");
	}
	
	//Screen Setting
	
	public static int ScreenOrientation = ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE; //ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE or ActivityInfo.ORIENTATION_PORTRAIT	
	
	//Video Setting
	
	public static int VideoXPosition = 0;  //-1:Left 0:Center 1:Right
	public static int VideoYPosition = 0; //-1:Top  0:Center 1:Bottom
	public static int VideoXMargin = 0;
	public static int VideoYMargin = 0;
	
	public static int VideoXRatio = 4;     //r <= 0:FULL;
	public static int VideoYRatio = 3;     //r <= 0:FULL;
	public static int VideoDepthBpp = 16;  //16 or 32
	public static boolean VideoSmooth = true;
	
	//Touch Mode Setting
	
	public static String TouchMode = "Touch";
	//"Invalid"
	//"Touch"
	//"TrackPad"
	//"GamePad"
	
	//Button Setting
	
	public static boolean ButtonLeftEnabled   = false;
	public static boolean ButtonRightEnabled  = true;
	public static boolean ButtonTopEnabled    = false;
	public static boolean ButtonBottomEnabled = true;

	//AppConfig Setting
	
	public static boolean AppLaunchConfigUse = true;
	
	//Run Static Initializer
	
	public static boolean Run = false; //do not change
}
