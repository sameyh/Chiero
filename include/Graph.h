#ifndef GRAPH_H 
#define GRAPH_H

class Deduction;
typedef vector<Deduction*>  VecDed;
class Span
{
public:
    Span(int i = -1, int j = -1, Deduction* d = NULL): i(i), j(j), antis(d)
    {}
    int         i;
    int         j;
    Deduction*  antis;
};

class Xtran
{
public:
    Xtran(int i = -1, int j = -1, int k = 0): i(i), j(j), k(k)
    {}

    friend ostream& operator << (ostream& oss, const Xtran& ot)
    {
        return (oss << "(" << ot.i << ", " << ot.j << ", " << ot.k << ")");
    }

    int i;
    int j;
    int k;
};

class Deduction
{
public: 
    Deduction(GrammarNode* gnode = NULL, const Span& span = Span()): gnode(gnode), span(span)
    {}

    void analyze_x(vector<Xtran>& xes)
    {
        //assert(gnode->tm == 'X');
        const LmStr& le = gnode->le;
        Deduction* antis = span.antis;
        int j = span.j;

        xes.clear();
        for (int i = (int)le.size() - 1; i >= 1; --i)
        {
            if (le[i] == Data::idX1 || le[i] == Data::idX2)
            {
                xes.insert(xes.begin(), Xtran(antis->span.j + 1, j));
                j = antis->span.j;
            }
            else
                --j;
            antis = antis->span.antis;
        }
        if (le[0] == Data::idX1 || le[0] == Data::idX2)
            xes.insert(xes.begin(), Xtran(span.i, j));
    }

    GrammarNode*    gnode;
    Span            span;
};

class Lattice
{
public:
    typedef pair<int, double>  EdgePair;

    Lattice(int size): size(size)
    {
        edges.resize(size, VVec(EdgePair)(size));
    }

    Lattice(const string& sent)
    {
        VecStr wds = split(sent);
        size = (int)wds.size();
        edges.resize(size, VVec(EdgePair)(size));
        for (int i = 0; i < size; ++i)
        {
			int word_id = Data::pflm->word2id(wds[i]);
            if(Data::Punc_map.find(word_id) != Data::Punc_map.end())
                punc_vec.push_back(word_id);
            edges[i][i].push_back(make_pair(word_id, 0.0));
        }
    }
    int size;
    VVec(vector<EdgePair>)  edges;
    vector<int> punc_vec;
};

class RuleMatch
{
public:
    RuleMatch(): gnode(NULL)
    {}

    friend ostream& operator << (ostream& oss, const RuleMatch& rule)
    {
        //oss << rule.gnode->rulebin.at(0);
        //return oss;

        oss << "RuleMatch--------------" << '\n';
        //oss << join(" ", Data::pflm->ids2str(rule.gnode->le)) << " | ";
        oss <<  rule.span.first << "-" << rule.span.second << " | ";
        for (int xid = 0; xid < (int)rule.xes.size(); ++xid)
            oss << rule.xes[xid] << ", ";
        oss << '\n';
        oss << "--------------RuleMatch" << endl;
        return oss;
    }

    bool check()
    {
        try
        {
            if (gnode->tm == '\0')
                throw int();
            if (xes.size() != gnode->le_xes.size())
                throw int();
            for (int xi = 0; xi < (int)xes.size(); ++xi)
            {
                Xtran& x = xes[xi];
                if (!(span.first <= x.i && x.j <= span.second))
                    throw int();
            }
            return true;
        }
        catch (...)
        {
            cerr << *this << endl;
            return false;
        }
    }

    GrammarNode*    gnode;
    pair<int, int>  span;
    vector<Xtran>   xes;
};

class Graph
{
public:
    Graph(const Lattice& lattice, GrammarTree * gmr_trie, OOVmap * _oov_map): src_size(lattice.size)
    {
        root = gmr_trie->root;
        tmp_trie = new GrammarTree("", _oov_map);
        dotrules.resize(src_size, VVec(Deduction*)(src_size));

        for (int len = 1; len <= src_size; ++len)
        {
            for (int i = 0, j; (j = i + len - 1) < src_size; ++i)
            {
                j = i + len - 1;
                if (len <= MAX_RULE_LEN)
                    deal_cell(i, j, lattice);

                if (i == j && !lattice.edges[i][j].empty() && get_rules(i, j) == 0)      
                {
                    GrammarNode* p;
                    for (int wi = 0; wi < (int)lattice.edges[i][j].size(); ++wi)
                    {
						string wd = Data::pflm->id2word(lattice.edges[i][j][wi].first);
                        string rule;
                        rule = Format("{w} ||| {w} ||| -100 -100 -100 -100")("{w}", wd)();
                        //GrammarTree::gmr_trie->add_rule('X', rule, true);
                        p = tmp_trie->add_rule('X', rule, true);
                        Deduction *d = new Deduction(p, Span(i, j, NULL));
                        dotrules[i][j].push_back(d);
                    }
                    if ((p = root->down(Data::idX)) != NULL)
                        dotrules[i][j].push_back(new Deduction(p, Span(i, j, NULL)));
                }
            }
        }
    }

    ~Graph()
    {
        for (int i = 0; i < src_size; ++i)
        {
            for (int j = 0; j < src_size; ++j)
            {
                for (int k = 0; k < (int)dotrules[i][j].size(); ++k)
                    delete dotrules[i][j][k];
                dotrules[i][j].clear();
            }
        }
        delete tmp_trie;
    }

    void get_rules(vector<RuleMatch>& rules)
    {
        rules.clear();
        for (int i = 0; i < src_size; ++i)
        {
            for (int j = i; j < src_size; ++j)
            {
                for (int di = 0; di < (int)dotrules[i][j].size(); ++di)
                {
                    Deduction* d = dotrules[i][j][di];
                    if (d->gnode->tm == 'X')
                    {
                        RuleMatch rule;
                        rule.gnode = d->gnode;
                        rule.span = make_pair(i, j);
                        d->analyze_x(rule.xes);
                        rules.push_back(rule);
                    }
                }
            }
        }
    }
    
    GrammarTree*        tmp_trie;

private:  
    int get_rules(int i, int j)
    {
        int num = 0;
        for (int di = 0; di < (int)dotrules[i][j].size(); ++di)
            num = dotrules[i][j][di]->gnode->tm == 'X' ? num + 1 : num;
        return num;
    }

    bool step(Deduction* d, int m, int j, VecDed& r, const Lattice& lattice)
    {
        Deduction* xded = get_xdeds(m, j);
        GrammarNode* node;
        bool full = false;

        if (xded != NULL && (node = d->gnode->down(Data::idX)) != NULL)
        {
            r.push_back(new Deduction(node, Span(d->span.i, j, d)));
            full = node->tm == 'X' ? true : false;
        }

        const vector<pair<int, double> >& edges = lattice.edges[m][j];
        for (int ei = 0; ei < (int)edges.size(); ++ei)
        {
            if ((node = d->gnode->down(edges[ei].first)) != NULL)
            {
                r.push_back(new Deduction(node, Span(d->span.i, j, d)));
                full = node->tm == 'X' ? true : false;
            }
        }
        return full;
    }

    Deduction* get_xdeds(int i, int j)
    {
        for (int k = 0; k < (int)dotrules[i][j].size(); ++k)
        {
            if (dotrules[i][j][k]->gnode->tm == 'X')
                return dotrules[i][j][k];
        }
        return NULL;
    }

    void deal_cell(int i, int j, const Lattice& lattice)
    {
        GrammarNode* p;
        bool full = false;  
        if (!lattice.edges[i][j].empty())
        {
            const vector<pair<int, double> >& edges = lattice.edges[i][j];
            for (int ei = 0; ei < (int)edges.size(); ++ei)
            {
                if ((p = root->down(edges[ei].first)) != NULL)
                {
                    Deduction *d = new Deduction(p, Span(i, j, NULL));
                    dotrules[i][j].push_back(d);
                    full = p->tm == 'X' ? true : full;
                }
            }
        }
        for (int k = i; k < j; ++k)
        {
            VecDed& deds = dotrules[i][k];
            for (int id = 0; id < (int)deds.size(); ++id)
                full = step(deds[id], k + 1, j, dotrules[i][j], lattice) ? true : full;
        }
        if (full && (p = root->down(Data::idX)) != NULL)
            dotrules[i][j].push_back(new Deduction(p, Span(i, j, NULL)));
    }

    int                 src_size;
    VVec(VecDed)        dotrules;
    GrammarNode*        root;
};

#endif

