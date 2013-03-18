#ifndef DECODER_H
#define DECODER_H

class XSpan
{
public:
    XSpan(int i = -1, int j = -1): i(i), j(j)
    {}
    int         i;
    int         j;
    bool operator < (const XSpan & span)const
    {
	if(i != span.i)
	    return (i<span.i);
	else
	    return (j<span.j);
    }
};

class Cube
{
public:   
    Cube(const LmStr& maxs): maxs(maxs)
    {}

    bool less(const LmStr& s1, const LmStr& s2) const
    {
        int p = 0;
        while (p < (int)s1.size() && s1[p] < s2[p])
            ++p;
        return p == (int)s1.size();
    }

    bool duplicate(int rule_id, const vector<Xtran>& xes)
    {
        LmStr v(1, rule_id);
        for (int i = 0; i < (int)xes.size(); ++i)
            v += xes[i].k;
        if(v.size() != maxs.size())
		{
			cerr << "fatal error, v size not equal to maxs" << endl;
			exit(0);
		}
        if (!less(v, maxs))
            return true;    
        bool& vi = in[v];
        return vi ? true : (vi = true, false);
    }

    map<LmStr, bool> in;
    LmStr            maxs;   
};

class Hyp
{
public:
    Hyp(): gnode(NULL), rule_id(-1), feats(Data::WEIGHT_NUM, 0), cube(NULL), neighbor(NULL),punc_distort_tag(false)
    {
	     //lm_loss = 0;
		//sec_lm_loss = 0;
	}

    ~Hyp()
    {
        delete neighbor;
    }

    LmStr border() const
    {
        return (int)btran[0].size() < Data::LM_ORDER - 1 ? btran[0] : btran[0] + btran[1];
    }
	
	LmStr sec_border() const
	{
		return (int)sec_btran[0].size() < Data::LM_ORDER - 1 ? sec_btran[0] : sec_btran[0] + sec_btran[1];
	}
    const Rule& rule() const
    {
        return gnode->rulebin.at(rule_id);
    }
	friend ostream& operator << (ostream& oss, const Hyp* hyp)
	{
		oss << "Hyp [" ;
		for(size_t i=0;i < hyp->rule().ri.size(); ++i)
		{
			oss << hyp->rule().ri[i] << " ";
		}
		oss << "]" << endl;
		return oss;
	}
    GrammarNode*        gnode;
    int                 rule_id;
    vector<Xtran>       xes;          
    LmStr               btran[2];
	// H.M: add secondary language model
	LmStr				sec_btran[2];

    VecDbe              feats;
    double              score;
    double              lm_loss;

	// H.M: add secondary language model
	double				sec_lm_loss;

    Cube*               cube; 
    Hyp*                neighbor;
	bool				punc_distort_tag;
};

typedef vector<Hyp*>    VecHyp;

class SortPHypQueue
{
public:
    bool operator () (const Hyp *t1, const Hyp *t2) const
    {
        return t1->score < t2->score;
    }
};

class Decoder
{
public:
    Decoder(Lattice& plat, vector<RuleMatch>& rules, ofstream* pfou, string& result, GrammarTree* _gmr_trie): plat(plat), rules(rules), pfou(pfou), gmr_trie(_gmr_trie)
    {
        CountTime ct("decoding...");
        x_hyps.resize(plat.size, VVec(Hyp*)(plat.size));
        for (int len = 1; len <= plat.size; ++len)
        {
            for (int i = 0, j; (j = i + len - 1) < plat.size; ++i)
                cube_pruning(i, j);
        }
#ifdef TIMES_TO_PLUS
		// do nothing
#else
		trace_nbest();
#endif
		VecDbe feats;
		result = construct_tran(x_hyps[0][plat.size-1].at(0), feats, false);
#if DEBUG >= 1
        /*cout << "Print Whole Chart ==========" << endl;
        PrintChart();
        cout << "=====end======" << endl;
        cout << endl;
        cout << "Print 1best derivation =========" << endl;
        Print1bestRuleDerivation(x_hyps[0][plat.size-1].at(0), 0, plat.size-1);
        cout << "=====end======" << endl;*/
#endif
    //pfou->close();
    }

    ~Decoder()
    {
        for (int i = 0; i < plat.size; ++i)
        {
            for (int j = i; j < plat.size; ++j)
            {
                for (int k = 0; k < (int)x_hyps[i][j].size(); ++k)
                    delete x_hyps[i][j][k];
            }
        }
    }

    void trace_nbest()
    {
        basic_string<Hyp*> stack;
        for (int hi = 0; hi < (int)x_hyps[0][plat.size - 1].size(); ++hi)
        {
            for (Hyp* hyp = x_hyps[0][plat.size - 1][hi]; hyp != NULL; hyp = hyp->neighbor)
                stack += hyp;
        }
        sort(stack.rbegin(), stack.rend(), SortPHypQueue());
		
        int nbest = min((int)stack.size(), Data::NBEST);
#ifdef RAND_NBEST
        int exnbest = nbest / 2;
        *pfou << nbest + exnbest << '\n';
#else
        *pfou << nbest << '\n';
#endif       
        for (int hi = 0; hi < nbest; ++hi)
        {
            Hyp* top = stack[hi];
            VecDbe feats(top->feats);
            *pfou << construct_tran(top, feats, false) << " ||| ";
            copy(top->feats.begin(), top->feats.end(), ostream_iterator<double>(*pfou, " "));
            //copy(feats.begin(), feats.end(), ostream_iterator<double>(*pfou, ", "));
            *pfou << " ||| " << top->score << '\n';
        }

#ifdef RAND_NBEST
        for (int hi = 0; hi < exnbest; ++hi)
        {
            Hyp* top = stack[hi];
            VecDbe feats(top->feats);
            *pfou << construct_tran(top, feats, true) << " ||| ";
            copy(feats.begin(), feats.end(), ostream_iterator<double>(*pfou, " "));
            *pfou << " ||| " << inner_product(feats.begin(), feats.end(), Data::WEIGHT.begin(), 0.0) << '\n';
        }
#endif
        pfou->flush();
    }

	bool SpanAllCover(const vector<Xtran> &xes, const XSpan &fspan)
	{
        	int count = 0;
        	for(size_t i = 0 ; i < xes.size(); ++i)
        	{
            		count += xes[i].j - xes[i].i + 1;
        	}
        	return (count == fspan.j- fspan.i+1);
	}
   
    string construct_tran(Hyp* hlink, VecDbe& feats, bool random)
    {
        Hyp* tran = hlink;
        if (random)
        {
            basic_string<Hyp*> buf;
            for (Hyp* hyp = hlink; hyp != NULL; hyp = hyp->neighbor)
                buf += tran;
            tran = buf[rand() % buf.size()];
            VecDbe tfs(hlink->feats.begin(), hlink->feats.end());
            feats -= tfs;
            tfs.assign(tran->feats.begin(), tran->feats.end());
            feats += tfs;
        }

        VecStr r; 
        const LmStr& ri = tran->rule().ri;
        for (int i = 0; i < (int)ri.size(); ++i)
        {
            if (ri[i] != Data::idX1 && ri[i] != Data::idX2)
				r.push_back(Data::pflm->id2word(ri[i]));
            else
            {
                const Xtran& x = ri[i] == Data::idX1 ? tran->xes[0] : tran->xes[1];
                Hyp* xstr = x_hyps[x.i][x.j][x.k];
                r.push_back(construct_tran(xstr, feats, random));
            }
        }
        string res = join(" ", r);
        return replace(replace(res, "<s> ", ""), " </s>", "");
        //return join(" ", r);
    }

    double update_bdy(const LmStr& ri, LmStr bt[2])
    {
        bt[0] = ri.substr(0, min(Data::LM_ORDER - 1, (int)ri.size()));
        bt[1] = ri.substr(max((int)ri.size() - (Data::LM_ORDER - 1), 0));
        return 0.0;
    }
    
	bool ContainPunc(Lattice&  plat , const vector<Xtran>& xes)
	{
		for(size_t k = 0; k < xes.size(); ++k)
		{
            for(size_t i = 0; i < plat.punc_vec.size(); ++i)
            {
                if(xes[k].i <= plat.punc_vec[i] && xes[k].j >= plat.punc_vec[i])
                    return true;
            }
        }
        return false;
	}
    double update_bdy(const Rule& rule, const vector<Xtran>& xes, double& sec_lm_loss, LmStr bt[2], LmStr sec_bt[2], bool & distort_tag)
    {
        double loss1 = rule.deltalm; // rule.feats[7]; use deltalm instead  20110707 Haitao Mi
		
		double sec_loss1 = rule.sec_deltalm; // H.M: add secondary language model

        double loss2 = 0;
		double sec_loss2 = 0; // H.M: add secondary language model

        LmStr str;

		// H.M: add secondary language model
		LmStr sec_str;

		//bool distort_tag = false;
		//bool punc_tag = false;
		int x1_index = -1;
		int x2_index = -1;
        for (int i = 0; i < (int)rule.ri.size(); ++i)
        {
            if (rule.ri[i] != Data::idX1 && rule.ri[i] != Data::idX2)
			{
				str += rule.ri[i];
				sec_str += rule.sLM_ri[i];
			}				
            else
            {
				Xtran x;
				if(rule.ri[i] == Data::idX1)
				{
					x = xes[0];
					x1_index = i;
				}
				else
				{
					x = xes[1];
					x2_index = i;
				}
                //const Xtran& x = rule.ri[i] == Data::idX1 ? xes[0] : xes[1];
                LmStr b = x_hyps[x.i][x.j][x.k]->border();
				loss1 += Data::pflm->get_str_prob(b);
				str += b;

				// H.M: add secondary language model
				LmStr sec_b = x_hyps[x.i][x.j][x.k]->sec_border();
				if(Data::sflm != NULL)
					sec_loss1 += Data::sflm->get_str_prob(sec_b);
				sec_str += sec_b;

            }
        }
		if(x1_index > x2_index)
			distort_tag = true;
        loss2 = Data::pflm->get_str_prob(str);

		// H.M: add secondary language model
		if(Data::sflm != NULL)
			sec_loss2 = Data::sflm->get_str_prob(sec_str);

        update_bdy(str, bt);

		// H.M: add secondary language model
		update_bdy(sec_str, sec_bt);
		sec_lm_loss = (-sec_loss1 + sec_loss2);
		//cout << "primary:\t" << loss1 << " + " << loss2 << endl;
		//cout << "secondary:\t" << sec_loss1 << " + " << sec_loss2 << endl;
        return -loss1 + loss2;
    }

    void get_next(Hyp* cur, vector<Hyp*>& r, int i, int j)
    {
        vector<Xtran> xes = cur->xes;
        Cube* cube = cur->cube;
        r.clear();

        Hyp *tran;
        if (!cube->duplicate(cur->rule_id + 1, xes))
        {
            tran = get_tran(cur->gnode, cur->rule_id + 1, xes);
            tran->cube = cube;
            r.push_back(tran);
        }

        for (int xi = 0; xi < (int)xes.size(); ++xi)
        {
            ++xes[xi].k;
            if (!cube->duplicate(cur->rule_id, xes))
            {
                tran = get_tran(cur->gnode, cur->rule_id, xes);
                tran->cube = cube;
                r.push_back(tran);
            }
            --xes[xi].k;
        }
    }

    Hyp* get_tran(GrammarNode* gnode, int rule_id, const vector<Xtran>& xes)
    { 
        Hyp* tran = new Hyp();
        const Rule& rule = gnode->rulebin.at(rule_id);
		bool distort_tag = false;
		bool punc_tag = false;

		// H.M: update both lm_loss and sec_lm_loss
        tran->lm_loss = update_bdy(rule, xes, tran->sec_lm_loss, tran->btran, tran->sec_btran, distort_tag);
		if(distort_tag == true)
		{
			punc_tag = ContainPunc(plat, xes);
		}
		tran->punc_distort_tag = punc_tag;
        tran->gnode = gnode;
        tran->rule_id = rule_id;
        tran->xes = xes;
#ifdef TIMES_TO_PLUS
		// do nothing
#else
		// copy feature vector
        tran->feats = rule.feats;
        for (int xi = 0; xi < (int)xes.size(); ++xi)
            tran->feats += x_hyps[xes[xi].i][xes[xi].j][xes[xi].k]->feats;
       	if(Data::sflm == NULL)
			tran->feats[Data::WEIGHT_NUM - 1] += tran->lm_loss;  
		else
		{
			tran->feats[Data::WEIGHT_NUM - 2] += tran->lm_loss;
			tran->feats[Data::WEIGHT_NUM - 1] += tran->sec_lm_loss;
		}
#endif

        tran->score = rule.score;
        for (int xi = 0; xi < (int)xes.size(); ++xi)
            tran->score += x_hyps[xes[xi].i][xes[xi].j][xes[xi].k]->score;

#ifdef TIMES_TO_PLUS
	if(sflm == NULL) 
		tran->score += tran->lm_loss;
	else
	{
		tran->score += tran->lm_loss;
		// only the weight of primary lm is 1!
		tran->score += tran->sec_lm_loss * Data::WEIGHT[WEIGHT_NUM - 1]; 
	}
#else
	if(Data::sflm == NULL)
		tran->score += tran->lm_loss * Data::WEIGHT[Data::WEIGHT_NUM - 1];
	else
	{
        	tran->score += tran->lm_loss * Data::WEIGHT[Data::WEIGHT_NUM - 2];
		tran->score += tran->sec_lm_loss * Data::WEIGHT[Data::WEIGHT_NUM - 1];
	}

#endif
        return tran;
    }

    void cube_pruning(int i, int j)
    {
        vector<Cube*> cubes;
        VecHyp first_trans;
        for (int t = 0; t < (int)rules.size(); ++t)
        {
			bool tag = false;
            RuleMatch& rule = rules[t];
            if (rule.span.first != i || rule.span.second != j)
                continue;
            LmStr maxs(1, rule.gnode->rulebin.size());
            for (int xi = 0; xi < (int)rule.xes.size(); ++xi)
            {
                Xtran& x = rule.xes[xi];
                maxs += x_hyps[x.i][x.j].size();
				if(x_hyps[x.i][x.j].size() == 0)
				{
					tag = true;
					break;
				}
            }
			if(tag == true)
				continue;
            cubes.push_back(new Cube(maxs));
            
            Hyp* tran = get_tran(rule.gnode, 0, rule.xes);
            tran->cube = cubes.back();
            first_trans.push_back(tran);
        }
        if (i == 0)
        {
            for (int k = 0; k < j; ++k)
            {
                if (!x_hyps[0][k].empty() && !x_hyps[k + 1][j].empty())
                {
                    vector<Xtran> xes;
                    xes.push_back(Xtran(0, k, 0));
                    xes.push_back(Xtran(k + 1, j, 0));   
                    LmStr maxs(1, 1);
                    maxs += LmStr(1, x_hyps[0][k].size()) + LmStr(1, x_hyps[k + 1][j].size());
                    cubes.push_back(new Cube(maxs));
                    Hyp* tran = get_tran(gmr_trie->glue_node, 0, xes);
                    tran->cube = cubes.back();
                    first_trans.push_back(tran);
                }
            }
        }

        get_nbest(i, j, first_trans, x_hyps[i][j]);

        for (int ci = 0; ci < (int)cubes.size(); ++ci)
            delete cubes[ci];

    }

	void PrintChart()
	{
        VecDbe feats;
		for(int len = 0; len < plat.size; ++len)
		{
			for (int hi = 0; hi+len < plat.size; ++hi)
			{
				cout << "Span [" << hi << "-" << hi+len << "]" << endl;
				for(int i = 0; i < (int)x_hyps[hi][hi+len].size(); ++i)
					cout << "hyp "<< i <<": ["<< construct_tran(x_hyps[hi][hi+len][i], feats, false) << "]" << endl;
				cout<< endl;
			}
		}
	}

    void Print1bestRuleDerivation(Hyp * top, int start, int end)    
    {        
        //Hyp * top = x_hyps[0][plat.size-1].at(0);
        cout << "Span [" << start << ":" << end << "] ||| " <<  join(" ", Data::pflm->ids2str(*(top->rule().le))) << "->" << join(" ", Data::pflm->ids2str(top->rule().ri)) << " ||| " << join(" ", top->feats) << endl;
        for(size_t i = 0; i < top->xes.size(); ++i)
        {
            Print1bestRuleDerivation(x_hyps[top->xes[i].i][top->xes[i].j][top->xes[i].k], top->xes[i].i, top->xes[i].j);
        }
        return;
    }

    void get_nbest(int i, int j, VecHyp& first_trans, VecHyp &rst)
    {
        rst.clear();
        priority_queue<Hyp*, vector<Hyp*>, SortPHypQueue> pqueue;
        for (int t = 0; t < (int)first_trans.size(); ++t)
            pqueue.push(first_trans[t]);

        double optscore = -MAXV(double);
        int pop_num = 0;
        map<LmStr, Hyp*> mp;
        vector<Hyp*> next;
        vector<Hyp*> hyps_not_use;
        while (!pqueue.empty())
        {
            if (++pop_num > Data::BEAM_SIZE * BETA)
            {
                break;
            }

            Hyp* top = pqueue.top();
            pqueue.pop();
			//checking: 
			//<s> must appear at the beginning of each hypotheses
			//</s> must be at the end of each hypotheses
            //if(i == 0  && top->btran[0][0] != Data::START)
			//{
            //   hyps_not_use.push_back(top);
			//}
			//else if(j == plat.size-1 && top->btran[1][top->btran[1].size()-1]!= Data::END)
			//{
            //    hyps_not_use.push_back(top);
			//}
			if(top->punc_distort_tag == true)
			{
                hyps_not_use.push_back(top);
			}
			else
			{
                //empty = false;
				sorted_insert(mp[top->border()], top);
				if ((int)mp.size() > Data::BEAM_SIZE)
				{
					break;
				}

				optscore = max(optscore, top->score);
#ifdef TIMES_TO_PLUS
				if (optscore - Data::NEWALPHA > top->score)
				{
					//++Decoder::stat_cube_pruning.cut_max_min;
					break;
				}
#else
				if (optscore - ALPHA > top->score)
				{
					//++Decoder::stat_cube_pruning.cut_max_min;
					break;
				}
#endif
			}
            get_next(top, next, i, j);
            for (int t = 0; t < (int)next.size(); ++t)
                pqueue.push(next[t]);
        }
        while (!pqueue.empty())
        {
            delete pqueue.top();
            pqueue.pop();
        }

        for(size_t i = 0; i < hyps_not_use.size(); ++i)         
            delete hyps_not_use[i];
        /*if(mp.size() == 0){
            if(hyps_bak.size() > 0){
                rst.push_back(hyps_bak[0]);
            }
            for(size_t i = 1; i < hyps_bak.size(); ++i){
                delete hyps_bak[i];
            }
        }
        else{
            for(size_t i = 0; i < hyps_bak.size(); ++i){
                delete hyps_bak[i];
            }
        }*/
        for (map<LmStr, Hyp*>::iterator ite = mp.begin(); ite != mp.end(); ++ite)
            rst.push_back(ite->second);
        sort(rst.rbegin(), rst.rend(), SortPHypQueue());
    }

    void sorted_insert(Hyp*& link_head, Hyp* nhyp)
    {
        if (link_head == NULL)
            link_head = nhyp;
        else if (link_head->score > nhyp->score)
            sorted_insert(link_head->neighbor, nhyp);
        else
        {
            nhyp->neighbor = link_head;
            link_head = nhyp;
        }
    }
    
	Lattice&            plat;
    VVec(VecHyp)        x_hyps;
    vector<RuleMatch>&  rules;
    ofstream*           pfou;
    //vector<Feature*>    feats;
    //OOVmap * oov_map;
    //FastLM * pflm;
    //FastLM * sflm;
    GrammarTree*		gmr_trie;
};

#endif

