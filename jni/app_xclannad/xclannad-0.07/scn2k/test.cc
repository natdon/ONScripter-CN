#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>

#include<SDL.h>
#include<vector>

#include"system/file.h"
#include"system/system_config.h"
#include"window/widget.h"
#include"window/system.h"

#include"music2/music.h"

#include"scn2k.h"
#include"scn2k_impl.h"

using namespace std;

extern char* diag;
extern char* diag2;

int main(void) {
	AyuSysConfig config;
	printf("%s\n",setlocale(LC_ALL,""));
	//file_searcher.InitRoot("/home/uenok/pb");
	file_searcher.InitRoot("/mnt/KEY/CLANNAD");
	config.LoadInitFile();

	MuSys mu(config);
	mu.InitMusic();
	SetFont("msgothic.ttc");
	
	SDL_Init(SDL_INIT_VIDEO);
	SDL_SetVideoMode(640, 480, 0, SDL_HWSURFACE /*| SDL_FULLSCREEN */);
	System::Main main;

	PicContainer* main_panel = main.root.create_node(Rect(0, 0, main.root.width, main.root.height), 0);
	main_panel->show();
	Scn2k scn(main.event, *main_panel, mu, config);
	// MouseCursor* cursor = new MouseCursor
	// scn.OpenScript(1002, -19482); // ラブ生〜
	//scn.OpenScript(1002, -18240); // ラブ生シーン開始
	// scn.OpenScript(1002, -59528); // ラブ生シーン開始
	// scn.OpenScript(2, 0);
	// scn.OpenScript(320, 0);
	//scn.OpenScript(320, -4269); // オープニング(Chap.2)
	//scn.OpenScript(320, -2523);
	//scn.OpenScript(320, -7549);
	// scn.OpenScript(310, 0); // エンディング(Wed.day)
	//scn.OpenScript(311, 0); // エンディング
	// scn.OpenScript(100, -2244); // 選択肢
	// scn.OpenScript(100, -9008); // 選択肢
	//scn.OpenScript(1001, -118520 ); // 選択肢：葛城クン
	// scn.OpenScript(1, 0); // 開始
	// scn.OpenScript(414,380); // 開始
	// scn.OpenScript(9142,0); // 開始
	// config.SetParam("#SEEN_START", 1, 9140);
	//scn.show_textwindow(0);
	
	main.Mainloop();

	mu.FinalizeMusic();

	SDL_Quit();
}

