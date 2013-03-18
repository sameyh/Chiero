#ifndef MR_H
#define MR_H

#include "NlpChiero.h"
#include <unistd.h>
#include <sys/wait.h>
#include "Data.h"

class Worker
{
public:
    Worker(int _id, NlpChiero& _decoder, const vector<string>& _sents): id(_id), decoder(&_decoder), sents(_sents)
    {}

    void run()
    {
        ofstream fou(get_kbest_file().c_str());
#ifdef OUTPUT_FOREST
        ofstream fst(get_forest_file().c_str());
#endif
        for (int i = 0; i < (int)sents.size(); ++i)
        {
            //fou << "sent: " << sents[i] << endl;
            cout << "thread: " << id << ", sent ID: " << i << endl;
#ifdef OUTPUT_FOREST            
            decoder->decode(sents[i], &fou, &fst);
            ofstream fhv(get_hashv_file().c_str());
            for (set<UINT64>::iterator ite = decoder->rules_used.begin(); ite != decoder->rules_used.end(); ++ite)
                fhv << *ite << '\n';
            fhv.close();
#else
            decoder->Translate(sents[i], &fou);
#endif
            cout << endl;
        }
        fou.close();
    }

    string get_kbest_file() const
    {
        return Format("__thread.{id}.kbest")("{id}", id)();
    }

    string get_forest_file() const
    {
        return replace(get_kbest_file(), "kbest", "forest");
    }

    string get_hashv_file() const
    {
        return replace(get_kbest_file(), "kbest", "hashv");
    }

    void dump_to_file(string fe_kbest, string fe_fst)
    {
        Format fmt("cat {s1} >> {s2}");
        string cmd1 = fmt("{s1}", get_kbest_file())("{s2}", fe_kbest)();
        assert(system(cmd1.c_str()) == 0);

#ifdef OUTPUT_FOREST
        string cmd2 = fmt("{s1}", get_forest_file())("{s2}", fe_fst)();
        assert(system(cmd2.c_str()) == 0);
#endif
    }

    int         id;
    NlpChiero*    decoder;
    vector<string>      sents;
};

class PRun
{
public:
    PRun(const vector<string>& _sents, int _td_size): sents(_sents), td_size(_td_size)
    {
        //Graph graph;
        NlpChiero decoder;
	decoder.Initialize();
	vector<Worker> workers;
        vector< vector<string> > vsents;
        split_sents(sents, vsents, td_size);
       /* {
            CountTime ct("pre-analyzing grammars");
            for (int id = 0; id < (int)sents.size(); ++id)
            {
                graph.analyze(sents[id]);
                graph.post_analyze();
            }
        }

        {
            CountTime ct("recursively sort grammars");
            graph.root->recursive_ensure_sort(decoder.feats);
        }*/
        for (int id = 0; id < td_size && id < int(vsents.size()); ++id)
	{	
	    //Worker k(id, decoder, vsents[id]);
            //workers.push_back(k);
	    workers.push_back(Worker(id, decoder, vsents[id]));
	}
        start_workers(workers);

        string cmd = Format("echo {num} > kbest")("{num}", int(sents.size()))();
        assert(system(cmd.c_str()) == 0);
        assert(system("echo > __forest") == 0);

        for (int wid = 0; wid < td_size && wid < int(sents.size()); wid++)
            workers[wid].dump_to_file("kbest", "__forest");
    }
   
private:
    void start_workers(vector<Worker>& args)
    {
        for (int i = 1; i < (int)args.size(); ++i)
        {
            if (!fork())
            { 
                args[i].run();
                //cout << "Worker " << args[i].wid << " finishes\n";
                exit(0);
            }
        }
        //Do the first chunck myself
        args[0].run();
        mywait((int)args.size() - 1);
    }
    
    void mywait(int worker_num)
    {
        for (int i = 0; i < worker_num; i++)
        {
            int status;
            wait(&status);
            if (!WIFEXITED(status))
                cerr << "Warning: abnormal terminated!" << endl;
        }
    }

    void split_sents(const vector<string>& sents, VVec(string)& out, int thread_num)
    {
        int size = (int)sents.size();
        int pos = 0;
        int chunk_size = size % thread_num == 0 ? size / thread_num : size / thread_num + 1;
        out.clear();
        while (pos < size)
        {
            out.resize(out.size() + 1);
            vector<string>& chunk = out.back();
            for (int i = 0, j = pos; i < chunk_size && j < size; ++i, ++j)
                chunk.push_back(sents[j]);
            pos += chunk_size;
        }
    }

    vector<string>      sents;
    int         td_size;
};

#endif

