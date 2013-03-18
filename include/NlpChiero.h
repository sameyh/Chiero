#ifndef NLP_CHIERO_H
#define NLP_CHIERO_H

#include "rain_algorithm.h"
#include "FastLM.h"
#include "Data.h"
#include "Grammar.h"
#include "Graph.h"
#include "Decoder.h"
#include "Config.h"
#include "utils.h"
#include <sys/time.h>
#include <algorithm>
using namespace std;

class NlpChiero
{
public:
	NlpChiero(void);
	~NlpChiero(void);


	bool Initialize();
	void Translate(const string & src, ofstream * fou);
	ofstream * fou;
	//FastLM*             pflm;
	//FastLM*				sflm; 
	GrammarTree*		gmr_trie;
	CTemplate*          Patent;
	/*int                 START;
	int                 END;
	map<int, int>		Punc_map;*/
	//vector<Feature*>    FEATS;
};

#endif //NLP_CHIERO_H
