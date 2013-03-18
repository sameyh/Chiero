#include "NlpChiero.h"
#include "mr.h"
using namespace rain;


int main(int argc, const char* argv[])
{
    assert(argc == 3 && "cmd config.py dev|tst");
	//OPEN_WFILE(fou, "kbest");
	//NlpChiero decoder;
	//decoder.Initialize(argv[1], argv[2], &fou);
	Data::initialize(argv[1], argv[2]);
	struct timeval beg_time;
    gettimeofday(&beg_time,NULL); 
    VecStr sents;
	vector<string> dict_rules;
	string tgt;
    Data::get_sents(Data::SRC, sents);
    cout << "sent size: "<< sents.size() << endl;
    //fou << sents.size() <<  endl;
	//vector<string> dict_rules;
    //ofstream out_file("result.txt");
    //string tgt;
    //string assist_str;
    //string forest_str;

    //OOVmap * _oovmap = new OOVmap(Data::pflm);
    /*for (int i = 0; i < (int)sents.size(); ++i)
    {
		decoder.Translate(sents[i], dict_rules, tgt);
		cout << "trans: [" << tgt << "]" << endl << endl;
     }*/
     cout << "thread [" << Data::THREAD_NUM << "]" << endl;
	 PRun prun(sents, Data::THREAD_NUM);
     struct timeval end_time;
     gettimeofday(&end_time, NULL);
     double timeuse = (double)(end_time.tv_sec - beg_time.tv_sec) + ((double) (end_time.tv_usec - beg_time.tv_usec)) / 1000000;
     cout << "Total_TIMEUSE = [" << timeuse << "]" << endl;
	return 0;
}

