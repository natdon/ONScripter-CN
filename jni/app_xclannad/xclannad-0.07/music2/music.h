#ifndef __MUSIC__ /* __MUSIC__ */
#define __MUSIC__

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include<sys/types.h>
#include<sys/time.h>

#define MIX_PCM_BGM	0 /* 4 */
#define MIX_PCM_EFFEC	0 /* 5 */
#define MIX_PCM_KOE	1 /* 6 */
#define MIX_PCM_SIZE	2

#define DEFAULT_AUDIOBUF	4096

enum KoeType { koe_unknown, koe_nwk, koe_ovk};
typedef struct {
	FILE* stream;
	int length;
	int offset;
	int rate;
	KoeType type;
}AvgKoeInfo;

extern int pcm_enable;

/* koedec.cc */
extern AvgKoeInfo OpenKoeFile(const char* path);
extern char* decode_koe(AvgKoeInfo info, int* len);
extern char* decode_koe_nwa(AvgKoeInfo info, int* len);
extern const char* MakeWavHeader(int rate, int ch, int bps, int size);
extern AvgKoeInfo FindKoe(int file_number, int index);

#include<unistd.h>
struct MuSys {
	class AyuSysConfig& config;
	char cdrom_track[128]; char effec_track[128];
	int movie_id;
	int music_enable;
	MuSys(AyuSysConfig& _config);

	void PlayCDROM(char* name, int play_count);
	void StopCDROM(int time);
	void PlaySE(const char* name, int loop_flag=0, int channel=0);
	void PlaySE(int number);
	void StopSE(int time = 0);
	bool IsStopSE(void);
	void PlayKoe(const char* fname);
	void StopKoe(int time);
	void PlayMovie(const char* fname, int x1, int y1, int x2, int y2, int loop_count);
	void StopMovie(void);
	bool IsStopMovie(void);
	void InitMusic(void);
	void FinalizeMusic(void);
};

#endif /* __MUSIC__ */
