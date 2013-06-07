#include<string>

/* CD Track 名 <-> Track 番号の変換を行う */
class TrackName {
	char** track;
	int* track_num;
	char** track_wave;
	int* track_start;
	int deal;
	void Expand(void);
	char** se_track;
	int se_deal;
	void ExpandSE(int num);
public:
	TrackName(void);
	~TrackName(void);
	void AddCDROM(char* name, int track);
	void AddWave(char* name, char* wave, int start_pt);
	void AddSE(int num, char* se);
	int CDTrack(char* name);
	int TrackStart(char* name);
	const char* WaveTrack(char* name);
	const char* SETrack(int num);
};
/* gameexe.ini で設定されるパラメータ */
/* まず初めに、設定項目を SetOrigPara* でセットする
** ただし、設定名は255文字以下である必要がある。
**
** SetPara* で設定項目は変更できる
** また、GetPara* で設定項目を得られる。
*/

class AyuSysConfig {
	friend class Conf2; /* テスト用のクラス */
	int change_flag;
	int dirty_flag;
	class AyuSysConfigString* str_config;
	class AyuSysConfigIntlist* int_config;

public:
	TrackName track_name;

public:
	AyuSysConfig(void);
	bool LoadInitFile(void);
	/* パラメータを検索する */
	/* str なら 1, int なら 2, 見つからないなら 0 */
	int SearchParam(const char* name) const;
	/* パラメータを得る */
	const char* GetParaStr(const char* name) const; /* str */
	int GetParam(const char* name, int deal, ...) const; /* int, error -> return -1, no error -> return 0 */
	int GetOriginalParam(const char* name, int deal, ...) const; /* int, error -> return -1, no error -> return 0 */
	int GetParaInt(const char* name) const {
		int n;
		if (GetParam(name,1,&n)) return 0;
		return n;
	}
	const int* GetParamArray(const char* name, int& deal) const;
	/* パラメータを変更する */
	void SetParaStr(const char* name, const char* var); /* str */
	void SetParam(const char* name, int deal, ...); /* int */
private:
	friend class AyuSys;
	/* 元設定を行う */
	/* AyuSys からのみ可能 */
	void SetOrigParaStr(const char* name, const char* var); /* str */
	void SetOrigParam(const char* name, int para_deal, ...); /* int */
	void SetOrigParamArray(const char* name, int deal, int* array); /* 上とおなじ */
public:

	/* オリジナルの設定関係
	** SetOriginal : 全ての設定を初めの状態に戻す
	** DiffOriginal : 初めの状態と現在の状態の変更分を得る
	** PatchOriginal: DiffOriginal で得た文字列を引数に
	**   渡す。DiffOriginal 呼び出し時の状態に戻す
	*/
	void SetOriginal(void);
	void DiffOriginal(std::string&);
	const char* PatchOriginal(const char*);
	/* config の内容を表示する */
	void Dump(FILE* f) const;
};

