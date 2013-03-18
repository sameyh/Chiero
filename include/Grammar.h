#ifndef GRAMMAR_H
#define GRAMMAR_H

class Feature;

class Rule
{
public:
    Rule(): tm('\0'), le(NULL), feats(Data::WEIGHT_NUM, 0)
    {
    } 

    friend ostream& operator << (ostream& oss, const Rule& rule) 
    {
        for (int fi = 0; fi < Data::WEIGHT_NUM; ++fi)
            oss << rule.feats[fi] << " ";
        oss << endl;
        return oss;
    }

    bool operator < (const Rule& rule) const
    {
        return score < rule.score;
    }

    bool operator == (const Rule& rule) const
    {
        return *le == *rule.le && ri == rule.ri;
    }
    
    static vector<Feature*> FEATS;

    char            tm;						// nonterminal label
    LmStr*          le;						// the source side
    LmStr           ri;						// the target side
	LmStr			sLM_ri;		// H.M: add secondary language model
    VecDbe          feats;                  // feature value, not weighted.
	double			deltalm;				// the delta lm score 
	// H.M: add secondary language model
	double			sec_deltalm;			// the delat lm score for secondary lm
    double          score;
    LmStr           ri_xes;                 // sequential position record.
};   

class Feature
{
public:
    virtual ~Feature()
    {}

    virtual double transmit(const Rule& rule) = 0;
};

class GrammarNode
{
public:
    GrammarNode(): tm('\0'), sorted(false)
    {}

    ~GrammarNode()
    {
        for (map<int, GrammarNode*>::iterator ite = addr.begin(); ite != addr.end(); ++ite)
            delete ite->second;
    }

    void ensure_sort(bool tag = false)
    {
        sorted = true;
        for (int i = 0; i < (int)rulebin.size(); ++i)
        {
            Rule& rule = rulebin[i];
            for (int j = 0; j < Data::WEIGHT_NUM; ++j)
	    {
                rule.feats[j] = Rule::FEATS[j]->transmit(rule);
	    }
			rule.score = inner_product(rule.feats.begin(), rule.feats.end(), Data::WEIGHT.begin(), 0.0);
			// add a new classmember  20110707 Haitao Mi
		if(Data::sflm == NULL)
			rule.deltalm = rule.feats[Data::WEIGHT_NUM-1];
		else
		{	
			rule.deltalm = rule.feats[Data::WEIGHT_NUM-2];  // primary lm 
			
				//H.M: add secondary language model
			rule.sec_deltalm = rule.feats[Data::WEIGHT_NUM-1]; // add secondary lm score 
		}
			//clear feature vector and release some memory  
#ifdef TIMES_TO_PLUS
            if(tag == false)
                rule.feats.clear();    
#endif
		}
        sort(rulebin.rbegin(), rulebin.rend());
        rulebin.erase(unique(rulebin.begin(), rulebin.end()), rulebin.end());
		//int rule_size = min(int(rulebin.size()), TOP_RULE_COUNT);
		//rulebin.resize(rule_size);
}

    void recursive_ensure_sort()
    {
        ensure_sort();
        for (map<int, GrammarNode*>::iterator ite = addr.begin(); ite != addr.end(); ++ite)
            ite->second->recursive_ensure_sort();
    }

    GrammarNode* down(int word)
    {
        map<int, GrammarNode*>::iterator ite = addr.find(word);
        return ite == addr.end() ? NULL : ite->second;
    }

    char                        tm;
    LmStr                       le;
    LmStr                       le_xes;
    vector<Rule>                rulebin;
    map<int, GrammarNode*>      addr;
    bool                        sorted;
};

class GrammarTree
{

public:
    GrammarTree(string grammar): root(new GrammarNode()), rule_num(0)
    {
        oov_map = new OOVmap(Data::pflm);
	//	pflm = _pflm;
	//	sflm = _sflm;
        if (grammar != "")
        {
            OPEN_RFILE(fin, Data::GRAMMAR);
            string line;
            while (getline(fin, line))
            {
                add_rule('X', line);
                if (rule_num % 100000 == 0)
                    cout << rule_num << endl;
            }
        }
        string glue = Format("{X1} {X2} ||| {X1} {X2} ||| 0 0 0 0")("{X1}", X1)("{X2}", X2)(); 
        add_rule('S', glue);
        glue_node = root->addr[Data::idX]->addr[Data::idX];
    }
    GrammarTree(string grammar, OOVmap * _pflm): root(new GrammarNode()), rule_num(0)
    {
        oov_map = _pflm; 
    }
    ~GrammarTree()
    {
        delete root;
    }

    /*  note, X is not permitted in a rule, which must be indexed.  eg.  
        S--><X1 X2, X1 X2>    legal
     */
    GrammarNode* add_rule(char nonterm, const string& srule, bool sort = false)
    {    
        // not accounting for rule duplication.
        try
        {
            vector<string> rb = split(srule, " ||| ");
            if(rb.size() < 3)
                throw 0;
            GrammarNode* node = root;

			// H.M: analyze the source side of a given rule
			// H.M: and construct grammar tree
            vector<string> syms = split(rb[0]);
            map<int, GrammarNode*>::iterator ite;		
            for (int i = 0; i < (int)syms.size(); ++i)
            {
                string sym = (syms[i] == X1 || syms[i] == X2) ? X : syms[i];
                int symid;
                if(sort == true)
                    symid = oov_map->word2id(sym);
                else
                    symid = Data::pflm->word2id(sym);
                ite = node->addr.find(symid);
                if (ite == node->addr.end())
                    ite = node->addr.insert(ite, make_pair(symid, new GrammarNode()));
                node = ite->second;
            }
            if (node->tm == '\0')
            {
                node->tm = nonterm;
                if(sort == true)
                    node->le = oov_map->str2ids(split(rb[0]));
                else
                    node->le = Data::pflm->str2ids(split(rb[0]));
                analyze_x(node->le, node->le_xes);
            }
			
			// H.M: analyze the target side of a given rule
			// H.M: and construct a rule
            Rule rule;
            rule.tm = node->tm;
            rule.le = &node->le;
            if(sort == true)
                rule.ri = oov_map->str2ids(split(rb[1]));
            else
                rule.ri = Data::pflm->str2ids(split(rb[1]));

			// H.M: add secondary language model
			if(Data::sflm != NULL)
			{
				rule.sLM_ri = Data::sflm->str2ids(split(rb[1]),false);
			}
			// H.M: analyze the feats
            vector<string> sp = split(rb[2]);
            if (sp.size() < 4)
                throw 0;
            for (size_t i = 0; i < sp.size(); ++i)
                rule.feats[i] = to_double(sp[i]);
            //transform(sp.begin(), sp.end(), rule.feats.begin(), ptr_fun<const string&, double>(to_double));

			analyze_x(rule.ri, rule.ri_xes);
            if (node->le_xes.size() != rule.ri_xes.size())
                throw 0;
            node->rulebin.push_back(rule);
            if (sort)
			{
                node->ensure_sort(true);
			}
            ++rule_num;
            return node;
        }
        catch (...)
        {
            cerr << "there is an error in adding rule: " << srule << endl;
            return NULL;
        }
    }
    
    void analyze_x(const LmStr& vs, LmStr& xes_pos)
    {
        xes_pos.clear();
        for (int i = 0; i < (int)vs.size(); ++i)
        {
            if (vs[i] == Data::idX1 || vs[i] == Data::idX2)
                xes_pos += i;
        }
        if (xes_pos.size() > 2)
		{
			xes_pos.resize(2);
            throw 0;
		}
    }
    
    GrammarTree*		gmr_trie;
    GrammarNode*        root;
    GrammarNode*        glue_node;
    int                 rule_num;
	//vector<Feature*>  FEATS;
	//FastLM * pflm;
	//FastLM * sflm;
	OOVmap * oov_map;
};

class PhraseFeature: public Feature
{
public:
    PhraseFeature(int col): Feature(), col(col)
    {}

    double transmit(const Rule& rule)
    {
        return rule.feats[col];
    }

private:
    int col;
};

class GlueFeat: public Feature
{
public:
    double transmit(const Rule& rule)
    {
        return rule.tm == 'S' ? 1 : 0;
	}
};

class RulePenalty: public Feature
{
public:
    double transmit(const Rule &rule)
	{
        return 1;
	}
};

class WordPenalty: public Feature
{
public:
    double transmit(const Rule& rule)
    {
        return double(rule.ri.size() - rule.ri_xes.size());
	}
};

class LMfeat: public Feature
{
public:
	
	// H.M: add secondary language model
    LMfeat(FastLM* lm, bool secondary = false): Feature(), lm(lm), secondary(secondary)
    {}

    double transmit(const Rule& rule)
    {
		// H.M: add secondary language model
		const LmStr& ristr = (secondary == false) ? rule.ri : rule.sLM_ri;
       		if(lm == NULL)
			return 0; 
		double r = 0.0;
        LmStr p = LmStr(1, -1) + rule.ri_xes + LmStr(1, rule.ri.size()); 

        for (int i = 0; i < (int)p.size() - 1; ++i)
        {
            if (p[i + 1] > p[i] + 1)
			{
				if(lm != NULL)
                	r += lm->get_str_prob(ristr.substr(p[i] + 1, p[i + 1] - p[i] - 1));
			}
        }
        return r;
    }

private:
    FastLM* lm;
	bool secondary;
};

#endif

