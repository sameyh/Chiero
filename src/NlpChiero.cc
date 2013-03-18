#include "NlpChiero.h"
#include "Template.h"
using namespace rain;

//GrammarTree*        GrammarTree::gmr_trie;

vector<Feature*>    Rule::FEATS;
int                 Data::THREAD_NUM;
int					Data::WEIGHT_NUM;
int                 Data::LM_ORDER;
string              Data::LM_FILE;
string		    Data::IWSTTEMPLATE;
int					Data::idX1;
int					Data::idX2;
int					Data::idX;
int					Data::START;
int					Data::END;
map<int, int>		Data::Punc_map;
// H.M: add secondary language model
string				Data::SEC_LM_FILE;

FastLM*				Data::pflm;
FastLM*				Data::sflm;

string              Data::GRAMMAR;
int                 Data::NBEST;
string              Data::SRC;
int                 Data::BEAM_SIZE;
VecDbe              Data::WEIGHT;

double				Data::LMWeight;
double				Data::SLMWeight; //H.M add secondary language model
double				Data::NEWALPHA;

void Data::ReadConfig(string configfe, string flag)
{
    Config config(configfe);
    config.get_value("LM_FILE", LM_FILE);
    remove_sides(LM_FILE);

	//H.M add secondary language model
	config.get_value("SEC_LM_FILE", SEC_LM_FILE);
	if(SEC_LM_FILE != "")
		remove_sides(SEC_LM_FILE);

    config.get_value("LM_ORDER", LM_ORDER);
    config.get_value("NBEST", NBEST);
    config.get_value("BEAM_SIZE", BEAM_SIZE);
    config.get_value("THREAD_NUM", THREAD_NUM);
    config.get_value("IWSTTEMPLATE", IWSTTEMPLATE);
	remove_sides(IWSTTEMPLATE);
    string weights;
    config.get_value("WEIGHTS", weights);
    assert((*weights.begin() == '[' && *weights.rbegin() == ']') || !(cerr << "weights: " << weights << endl));
    remove_sides(weights);
    vector<string> tmp = split(weights, ",");
	WEIGHT_NUM = tmp.size();
    //assert(tmp.size() == WEIGHT_NUM || !(cerr << weights << endl));
    for (int id = 0; id < Data::WEIGHT_NUM; ++id)
        WEIGHT.push_back(to_double(tmp.at(id)));

#ifdef TIMES_TO_PLUS

	cout << "[times to plus] is active!" << endl;
	LMWeight = WEIGHT[Data::WEIGHT_NUM-2]; // 1]; //H.M add secondary language model

	NEWALPHA = ALPHA / Data::LMWeight;
	for(int id = 0; id < Data::WEIGHT_NUM; ++id)    
    {       
        if( LMWeight != 0)
        {
            WEIGHT[id] = WEIGHT[id] / LMWeight;
            //cout << "weights [" << id << "] = " << WEIGHT[id] << '\t';
        }
		cout << endl;
    }
	SLMWeight = WEIGHT[Data::WEIGHT_NUM-1];

#else
	for(int id = 0; id < Data::WEIGHT_NUM; ++id)
		cout << "weight[" << id << "] = " << WEIGHT[id] << "\t";
	cout << endl;
	//do nothing
#endif

	//for(int id = 0; id < WEIGHT_NUM; ++id)        
	//	cout << "weights [" << id << "] = " << WEIGHT[id] << '\t';	
	//cout << endl;

    if (flag == "dev")
    {
        config.get_value("DEV_SRC", SRC);
        config.get_value("DEV_RULES", GRAMMAR);
    }
    else if (flag == "tst")
    {
        config.get_value("TST_SRC", SRC);
        config.get_value("TST_RULES", GRAMMAR);
    }
    //config.get_value("SRC", SRC);
    //config.get_value("RULES", GRAMMAR);
    remove_sides(SRC);
    remove_sides(GRAMMAR);
}

void Data::initialize(string config, string flag)
{
    ReadConfig(config, flag);
   
}

void Data::finalize()
{
    //for (int fid = 0; fid < (int)Rule::FEATS.size(); ++fid)
     //   delete Rule::FEATS[fid];
}

void Data::remove_sides(string& s)
{
    assert(s.size() >= 2 || !(cerr << "can't remove_sides: " << s << endl));
    s = s.substr(1, s.size() - 2);
}

void Data::get_sents(const string& src, vector<string>& sents)
{
    OPEN_RFILE(fin, src);
    string line;
    sents.clear();
    while (getline(fin, line)) 
    {
        if (startswith(line, "<seg")) 
        {
            if (line.find("</seg>") != string::npos)
            {
                int p1 = (int)line.find(">");
                int p2 = (int)line.rfind("<");
                string sent = line.substr(p1 + 1, p2 - (p1 + 1));
                sents.push_back(sent);
            }
            else 
            {
                cerr << "error input: can't find </src>" << endl;
                //assert(false);
            }
        }
    }
}



NlpChiero::NlpChiero(void)
{

}

NlpChiero::~NlpChiero(void)
{
	//fou->close();
	delete gmr_trie;
    delete Data::pflm;
	if(Data::sflm != NULL)
		delete Data::sflm;
	delete Patent;
	for (int fid = 0; fid < (int)Rule::FEATS.size(); ++fid)
		delete Rule::FEATS[fid];
    
    //Data::finalize();
}


bool NlpChiero::Initialize()
{
	//string conf_path = config_path;
	//fou = kbest;
	//Data::initialize(conf_path, flag);i
	{
    CountTime ct("loading primary lm...");
    double memory = get_memory();
    Data::pflm = new FastLM(Data::LM_FILE, Data::LM_ORDER);
    Data::idX1 = Data::pflm->word2id(X1);
    Data::idX2 = Data::pflm->word2id(X2);
    Data::idX  = Data::pflm->word2id(X);
        
    Data::START = Data::pflm->word2id("<s>");
    Data::END = Data::pflm->word2id("</s>");
	Data::Punc_map[Data::pflm->word2id("£¬")] = 1;
	Data::Punc_map[Data::pflm->word2id("¡£")] = 1;
	Data::Punc_map[Data::pflm->word2id("£º")] = 1;
	Data::Punc_map[Data::pflm->word2id("¡°")] = 1;
	Data::Punc_map[Data::pflm->word2id("¡±")] = 1;
	Data::Punc_map[Data::pflm->word2id("¡®")] = 1;
	Data::Punc_map[Data::pflm->word2id("¡¯")] = 1;
	Data::Punc_map[Data::pflm->word2id("£»")] = 1;
	//  English
	Data::Punc_map[Data::pflm->word2id(",")] = 1;
	Data::Punc_map[Data::pflm->word2id(".")] = 1;
	Data::Punc_map[Data::pflm->word2id(":")] = 1;
	Data::Punc_map[Data::pflm->word2id("\"")] = 1;
	Data::Punc_map[Data::pflm->word2id("'")] = 1;
	Data::Punc_map[Data::pflm->word2id(";")] = 1;


	//H.M add secondary language model
	ifstream sec_file(Data::SEC_LM_FILE.c_str());
	if(sec_file.good())
	{
		sec_file.close();
		Data::sflm = new FastLM(Data::SEC_LM_FILE, Data::LM_ORDER);
	}
	else
		Data::sflm = NULL;    
	
	Rule::FEATS.push_back(new PhraseFeature(0));        // P(e|c)
    Rule::FEATS.push_back(new PhraseFeature(1));        // Lex(e|c)            
    Rule::FEATS.push_back(new PhraseFeature(2));        // P(c|e)
    Rule::FEATS.push_back(new PhraseFeature(3));        // Lex(c|e)
    Rule::FEATS.push_back(new GlueFeat());              // glue rule.
    Rule::FEATS.push_back(new RulePenalty());           // rule number penalty.
    Rule::FEATS.push_back(new WordPenalty());           // word number penalty.
    Rule::FEATS.push_back(new LMfeat(Data::pflm));            // language model score.
	if(Data::WEIGHT_NUM > 8)
		Rule::FEATS.push_back(new LMfeat(Data::sflm, true)); 
	
	Patent = new CTemplate();
    Patent->Init(Data::IWSTTEMPLATE);
    cout << "memory used on LM: " << get_memory() - memory << "MB" << endl;
	}
	{
    CountTime ct("load grammars...");
    double memory = get_memory();
    gmr_trie = new GrammarTree(Data::GRAMMAR);
    cout << "memory used on grammars: " << get_memory() - memory << "MB" << endl;
	}
	{
    CountTime ct("recursively sort grammars tree");
	gmr_trie->root->recursive_ensure_sort();
	}

	cout << "Decoder Initialize done" << endl;
	return true;
}

void NlpChiero::Translate(const string & src, ofstream * fou)
{
	string tgt = "";
	fou = fou;
	string ori_sen;
	vector<string> rule_str;
	OOVmap * _oovmap = new OOVmap(Data::pflm);
	// add start and end tages for each input sentences
	//string cur_sent = "<s> " + replace(src, "|||", "THREENULLNULL") + " </s>";
	string cur_sent = replace(src, "|||", "THREENULLNULL");
    ReadNeTemplate(cur_sent, rule_str, ori_sen);
	Lattice _lattice(ori_sen);
	cout << "src [" << ori_sen << "]" << endl;
	//ofstream fou("kbest", ios::app);

	vector<RuleMatch> rules;
    Graph graph(_lattice, gmr_trie, _oovmap);
    graph.get_rules(rules);

	vector<RuleMatch> ne_rulematch;
    GetNErulematch(rule_str, ne_rulematch, graph); // get NE rules
	for(size_t i = 0; i< ne_rulematch.size(); ++i)
	{
		rules.push_back(ne_rulematch[i]);
	}

	vector<RuleMatch>::iterator mi = rules.begin();
	while (mi != rules.end())
	{
		if (!mi->check())
			mi = rules.erase(mi);
		else
			mi++;
	}
	
    //struct timeval end_time;
    //gettimeofday(&end_time,NULL);
    //double timeuse = (double)(end_time.tv_sec - beg_time.tv_sec) + ((double) (end_time.tv_usec - beg_time.tv_usec)) / 1000000;
    //cout << "BEFOR_DECODE_TIMEUSE = [" << timeuse << "]" << endl;
    (*fou) << "sent: " << ori_sen << endl;
	Decoder decode(_lattice, rules, fou, tgt, gmr_trie);
    string tmp_tgt = replace(tgt, "THREENULLNULL", "|||");
	
	// replace the start and end tags
	//tgt = replace(replace(tmp_tgt, "<s>", ""), "</s>", "");
	cout << "trans [" << tmp_tgt << "]" << endl;
	delete(_oovmap);
    //fou.close();
}
