
/*����ģ��ƥ�����غ������ṹ��ȫ�ֱ���*/
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
	*  ����:����ģ�����Ψһ��ʵ����
	*  ����:
	*		��
	*  ����ֵ:
	*		CTemplate���һ��ʵ����
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
	TmpltType;								//ģ�屻ʹ��ʱ��״̬:

	vector<TmpltType> AllTemp;

	struct temp_var
	{
		int s;
		int e;
		string c;
	}VarType;
	typedef struct
	{
		//string	srcSent;					//Դ����;
		//size_t	tmplt_index;				//ģ���������;

		string	body;					//ƥ��ɹ��Ĵ�:ģ���Ѿ�����Ӣ�Ĳ����滻;

		size_t start;//ģ���ڵڼ����ʿ�ʼyuhui
		size_t end;//ģ���ڵڼ����ʽ���
		int pri;
		vector<struct temp_var> var_result;
		int temp_id;

	}
	MatchResultType;	


	//vector<MatchResultType> mat_temp;//�������ƥ���ϵ�ģ����Ϣ


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
	bool Str2Vec(const string& str, vector<string> &vec);//��ʼ���vec,�����netrans��ǣ�ȥ��
	
	//��ȡģ��
	TmpltType GetTemplate( const size_t index );
	//����	
	//��ȡ��һ��ģ���������;
	size_t GetFirstIndex();
	//���index����һ��ģ���������;
	size_t GetNextIndex( const size_t index );
	//�жϱ���ģ����ʱ���Ƿ��Ѿ��ߵ�ģ����ĩβ
	bool IsEnd( const size_t index );
	//ƥ����������:�ܿغ���;
	//����ģ��ƥ��;
	bool SingleTempateMatch( const string&srcSent, const size_t tmpltIndex, const vector<string>& tempvec, MatchResultType& match_result );

	bool DoMatch( const string &line, string &retstring, vector<int>& index, vector<MatchResultType> & mat_temp);
	//�������ܵ�ģ����ƥ����ӣ�
	bool MatchSent( const string &oristr, string &retstring, vector<int>& indexVec, vector<MatchResultType> & mat_temp);

	//deal the single sentence and return the matched result
	int Match_Single(const string &line, string &retstring, vector<int>& subind, vector<MatchResultType> & mat_temp);

};
#endif
