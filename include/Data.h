#ifndef DATA_H 
#define DATA_H

#include "utils.h"
#define DEBUG           0

//#define WEIGHT_NUM      9 //H.M add secondary language model
#define TOP_RULE_COUNT  10 //Y.H   For the same source words, keep TOP_RULE_COUNT different translations

#define MAX_RULE_LEN    10
#define X1              "#X1#"
#define X2              "#X2#"
#define X               "#X#"
#define BETA            3.0
#define ALPHA           2.0
#define ASS_OUPUT		5

class Data
{
public:
    static void initialize(string config, string flag);
    static void finalize();
    static void get_sents(const string& src, vector<string>& sents);

    static int          THREAD_NUM;
    static int		WEIGHT_NUM;
    static int          LM_ORDER;
    static string       LM_FILE;
	static string		SEC_LM_FILE;
    static string       GRAMMAR;
    static int          NBEST;
    static string       SRC;
    static int          BEAM_SIZE;
    static VecDbe       WEIGHT;
    static FastLM*      pflm;
	static FastLM*		sflm;
    static int          idX1;
    static int          idX2;
    static int          idX;
	static string		IWSTTEMPLATE;
	//static CTemplate*	Patent;
    static int          START;
	static int			END;
	static map<int, int>	Punc_map;
	static double		LMWeight;
	static double		SLMWeight;
	static double		NEWALPHA;

private:
    static void ReadConfig(string configfe, string flag);
    static void remove_sides(string& s);
};

class Stat
{
public:
    Stat(): cut_beam(0), cut_pop(0), cut_max_min(0)
    {}

    ~Stat()
    {
        double t = cut_beam + cut_pop + cut_max_min + cut_empty;
        cout << "cut_beam:    " << cut_beam / t << endl;
        cout << "cut_pop:     " << cut_pop / t << endl;
        cout << "cut_max_min: " << cut_max_min / t << endl;
        cout << "cut_empty:   " << cut_empty / t << endl;
    }

    int     cut_beam;
    int     cut_pop;
    int     cut_max_min;
    int     cut_empty;
};

#endif

