
/*定义模板匹配的相关函数，结构和全局变量*/
#ifndef TEMPLATE_MATCH


#define TEMPLATE_MATCH

#include <vector>
#include <string>
#include <map>
#include <fstream>

#include <cstdlib>
#include <stdlib.h>
//#include <boost/regex.hpp>
#include "deelx.h"
#undef max
#undef min
#include <stack>


using namespace std;


typedef struct tempstruc{
	string	eTmplt;
	string  chTmplt;
	int     pri;
}TempStruct;

typedef pair< string, string > MatchedPart;



class CTemplate{
private:
	static	CTemplate m_TransTemp;
public:
	/**
	*  功能:返回模板类的唯一的实例。
	*  参数:
	*		无
	*  返回值:
	*		CTemplate类的一个实例。
	*				
	*/
	inline static CTemplate * Instance()
	{
		return &m_TransTemp;
	}

	typedef struct  
	{
		string	eTmplt;
		string  chTmplt;
		int     pri;

	}
	TmpltType;								//模板被使用时的状态:

	vector<TmpltType> AllTemp;

	struct temp_var
	{
		int s;
		int e;
		string c;
	}VarType;
	typedef struct
	{
		//string	srcSent;					//源句子;
		//size_t	tmplt_index;				//模板的索引号;

		string	body;					//匹配成功的串:模板已经用其英文部分替换;

		size_t start;//模板在第几个词开始yuhui
		size_t end;//模板在第几个词结束
		int pri;
		vector<struct temp_var> var_result;
		int temp_id;

	}
	MatchResultType;	


	//vector<MatchResultType> mat_temp;//存放所有匹配上的模板信息


	bool Init(const string & tempfile);

	//push back a string line into a vector
	bool SpecVec(const string& str,vector<string> &vec);
	//delete all substrings in the string
	void DelSubstr(string& str,const string& src,const string& sub);	
	//delete all spaces in the string
	void DelAllSpace(string& str);
	//trim the space on the beginning or end of the line
	static void Trim(string& str);
	//convert a string line into a vector
	bool Str2Vec(const string& str, vector<string> &vec);//开始清空vec,如果有netrans标记，去掉
	
	//获取模板
	TmpltType GetTemplate( const size_t index );
	//遍历	
	//获取得一条模板的索引号;
	size_t GetFirstIndex();
	//获得index的下一条模板的索引号;
	size_t GetNextIndex( const size_t index );
	//判断遍历模板库的时候是否已经走到模板库的末尾
	bool IsEnd( const size_t index );
	//匹配整个句子:总控函数;
	//单个模板匹配;
	bool SingleTempateMatch( const string&srcSent, const size_t tmpltIndex, const vector<string>& tempvec, MatchResultType& match_result );

	bool DoMatch( const string &line, string &retstring, vector<int>& index, vector<MatchResultType> & mat_temp);
	//搜索可能的模板以匹配句子；
	bool MatchSent( const string &oristr, string &retstring, vector<int>& indexVec, vector<MatchResultType> & mat_temp);

	//deal the single sentence and return the matched result
	int Match_Single(const string &line, string &retstring, vector<int>& subind, vector<MatchResultType> & mat_temp);

};
#endif
