#ifndef LMSRI_H_2008_03_25
#define LMSRI_H_2008_03_25
#include <vector>
#include <string>

using namespace std;

typedef basic_string<int> LmStr;

void*   sriLoadLM(const char * fn, int arpa=0, int order=3, int unk=0, int tolow=0);
//void* sriLoadMemLM(const char * str, int order=3, int unk=0, int tolow=0);
void    sriUnloadLM(void * plm);
//int  sriSetOrder(void * plm, int o);
double  sriPerplexity(void * plm, const char * sentence);
double  sriWordProb(void * plm, const char * word, const char * context);

int     kenWord2Idx(void * plm, const string& word);
double  kenContextStrProb(void * plm, const LmStr& str, const LmStr& context);
double  kenContextStrProb(void * plm, const string & str, const string & context, const string & delimit);

/**
  @brief eg. split("   a   b", " ") will return [a, b].
  @param[in] delimit view delimit as a whole string.
  */
vector<string> private_split(const string& src, string delimit);

//double sriWordProbSplit(int n, void * *plms, const char * word, const char * context);
//int sriWordProbFill(void *plm, const char * word, const char * context, double vs[]);

#endif

