/*
    modified date: 5-26-2011
    author: Tian Xia, Daqi Zheng
 */

#ifndef FAST_LM_H 
#define FAST_LM_H 

#include "rain_algorithm.h"
#include "flmsri.h"
using namespace rain;

inline ostream& operator << (ostream& oss, const LmStr& s)
{
    copy(s.begin(), s.end(), ostream_iterator<int>(oss, " "));
    return oss;
}

class FastLM
{
public:
	FastLM(string lm_file, int order): nwords_id(0), m_lm(NULL), m_order(order)
    {
        CountTime ct("loading language model...");
        if ((m_lm = sriLoadLM(lm_file.c_str(),0, m_order)) == NULL)
        {
            cerr << "can't open " << lm_file << endl;
            exit(1);
        }
    }

	~FastLM()
    {
        if(m_lm != NULL)
        {
            cerr << "unload language model..." << endl;
            sriUnloadLM(m_lm);
            cout << "added new words: " << -nwords_id << endl;
        }
    }

    double get_str_prob(const LmStr& s, const LmStr& ctx = LmStr())
    {
        LmStr _s[2] = {s, ctx};
        for (int i = 0; i < 2; ++i)
        {
            for (int p = 0; p < (int)_s[i].size(); ++p)
                _s[i][p] = _s[i][p] < 0 ? 0 : _s[i][p];
        }
        return kenContextStrProb(m_lm, _s[0], _s[1]);
    }


    LmStr str2ids(const VecStr& str, bool add = true)
    {
        LmStr r;
        for (int i = 0; i < (int)str.size(); ++i)
            r += word2id(str[i], add);
        return r;
    }
    
    VecStr ids2str(const LmStr& ids)
    {
        VecStr r;
        for (int i = 0; i < (int)ids.size(); ++i)
            r.push_back(id2word(ids[i]));
        return r;
    }

	// H.M: add secondary language model
    int word2id(const string& word, bool add = true )
    {
        int& value = tword2ids[word];
        if (value == 0)
        {
            int id = kenWord2Idx(m_lm, word);
            if (id != 0)
            {
                tid2words[id] = word;
                value = id;
            }
            else if (add)
            {
                id = --nwords_id; 
                value = id; 
                tid2words[value] = word;
            }
			else
			{
				value = -100;
			}
        }
        return value;
    }

    string id2word(int id)
    {
        map<int, string>::const_iterator ite = tid2words.find(id);
        if (ite != tid2words.end())
            return ite->second;
        else
        {
            cerr << "can't find id in dictionary: " << id << endl;
            //exit(1);
			return "UNK";
        }
    }

    map<int, string>        tid2words;         // total words;
    map<string, int>        tword2ids;         
    int                     nwords_id;
	void*                   m_lm;   
    int                     m_order;
};


class OOVmap
{
public:
	OOVmap(FastLM * _pflm):pflm(_pflm),nwords_id(_pflm->nwords_id)
	{
	}

	~OOVmap()
	{

	}
	
	LmStr str2ids(const VecStr& str)
    {
        LmStr r;
        for (int i = 0; i < (int)str.size(); ++i)
            r += word2id(str[i]);
        return r;
    }

    VecStr ids2str(const LmStr& ids)
    {
        VecStr r;
        for (int i = 0; i < (int)ids.size(); ++i)
            r.push_back(id2word(ids[i]));
        return r;
    }

    int word2id(const string& word)
    {
        map<string, int>::const_iterator iter = pflm->tword2ids.find(word);
        int value;
        if (iter == pflm->tword2ids.end())
        {
			value = oov_word2ids[word];
			if(value == 0)
			{
				int id = kenWord2Idx(pflm->m_lm, word);
				if (id != 0)
				{
					oov_id2words[id] = word;
					value = id;
				}
				else
				{
                    
					id = --nwords_id; 
					value = id;
					oov_id2words[value] = word;
				}
			} 
        }
        else
            value = iter->second;
        return value;
    }

    string id2word(int id)
    {
        map<int, string>::const_iterator ite = pflm->tid2words.find(id);
        if (ite != pflm->tid2words.end())
            return ite->second;
        else
		{
			map<int, string>::const_iterator iter = oov_id2words.find(id);
			if(iter != oov_id2words.end())
            {
				return iter->second;
            }
			else
			{
				cerr << "oov map can't find id in dictionary: " << id << endl;
				//exit(1);
                return "UNK";
			}
		}
    }
    
    FastLM * pflm;

	map<int, string>        oov_id2words;         // total words;
    map<string, int>        oov_word2ids;        
    int                     nwords_id;
};

#endif

