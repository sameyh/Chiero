


#include "utils.h"
//#include <strstream>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <sstream>
#include <list>

CTemplate CTemplate::m_TransTemp;

typedef string::size_type size_type;
//把str1中的空格去掉（把分过词的句子合并）
/*
void DelSpace(string &str,struct sent &elem)
{
	;
}*/




/**
*  功能:	将str以空格为分隔符进行分隔，存入vec中去。用于提取模板中含词约束中的内容.
*  参数:
*		@str:	待分隔的字符串；
*		@vec:	存放分隔结果的字符串；
*  返回值:
* 		0:	没有错误；
*		g_errno：	发生错误；
*/
bool CTemplate::SpecVec(const string& str,vector<string> &vec)
{
	if(str.empty())
	{
		
		return false;
	}
	if(str == "0")
	{
		vec.push_back("NULL");
		return true;
	}
	string first = str.substr(0,1);
	string line = str.substr(1);
	stringstream istr(line.c_str());
	string seg;
	while(istr>>seg)
	{
		vec.push_back(seg);
	}
	if(first == "-")
	{vec.push_back("-");}
	else if(first == "+")
	{vec.push_back("+");}
	else
	{
		
		return false;
	}
	return true;
}

/*
*  [功能]:
*		删除str中的所有空格；
*  [参数]:
*		@str:	待删除空格的字符串；
*  [返回值]:
*		无；
*/
void CTemplate::DelAllSpace(string& str)
{
	str.erase(std::remove(str.begin(),str.end(),' '),str.end());
}

/**
*  功能:
*		删除str开头的与结尾的空格；
*  参数:
*		@str:	待删除空格的字符串；
*  返回值:
*		无；
*/
void CTemplate::Trim( string& str )
{
	if ( str.empty() )
		return;

	string::size_type nStart = str.find_first_not_of( ' ' );
	string::size_type nEnd = str.find_last_not_of( ' ' );

	if ( nStart == string::npos )
	{
		//str.erase(0,str.end());
		str = "";
	}
	else 
	{
		str = str.substr( nStart, nEnd-nStart+1 );
	}
}

/**
*  功能:
*		将str以空格为分隔符进行分隔，存入vec中去。
*  参数:
*		@str:	待分隔的字符串；
*		@vec:	存放分隔结果的字符串；
*  返回值:
* 		true:	str非空；
*		false： str为空；
*/
bool CTemplate::Str2Vec(const string& str, vector<string> &vec)
{
	if(str.empty()) return false;
	vec.clear();

	//如果有netrans标记，去掉标记内容后 存入vec.
	int count=0;
	string str0="";
	while(count<int(str.length()))
	{
		int count1=str.find("<",count);
		if(count1==-1)
		{
			str0=str0+" "+str.substr(count,count1-count);
			break;
		}
		//if(count1==0)
			//str0=str0+" "+str.substr(count,count1-count-1);
		if(count1>0)
		{
			string stri=str.substr(count,count1-count);
			str0=str0+" "+stri;
		}
		int count2=str.find(">",count1);
		int count3=str.find("</",count2);
		int count4=str.find(">",count3);
		string str1=str.substr(count2+1,count3-count2-1);

		stringstream iss(str1.c_str());
		string seg,com_str1="";

		while(iss>>seg)
		{
			com_str1=com_str1+seg;
		}

		str0=str0+" "+com_str1;
		count=count4+1;

	}
	//str=str0;
	stringstream istr(str0.c_str());
	string seg;

	while(istr>>seg)
	{
			
		vec.push_back(seg);	
	}
	return true;
}

/**
*  功能:
*		将str中所有的src串都用sub串来代替。
*  参数:
*		@str:	存放str中用sub替换src后的结果；
*		@sub:	 用来替换的子串；
*		@src:	被替换的字符串；
*  返回值:
*		无；
*/
void CTemplate::DelSubstr(string& str,const string& src,const string& sub)
{
	string::size_type pos = str.find(src);
	if(pos==string::npos) return;
	while(pos!=string::npos)
	{
		str.replace(pos,src.size(),sub);
		pos=str.find(src,pos);
	}
}
//模板整个句子:总控函数;
bool CTemplate::DoMatch( const string &line, string &retstring, vector<int>& index, vector<MatchResultType> & mat_temp)
{

	bool matched_flag=false;	//成功匹配上某些模板;
	string sentence = line;	//待进行模板匹配的句子;

	mat_temp.clear();
	bool mt = MatchSent(sentence, retstring, index, mat_temp);
	//进行整句的匹配:以整个句子做为一个整体去进行模板的匹配.
	if(mt)
	{		
		matched_flag=true;	

	}

	//打印所有用到的模板；

	//模板匹配后处理,modify by caojie, 后处理放到执行完DoMatch()执行完做(在函数Match_Single中)。
	//DelSubstr( retstring, "$$", "" );
	//qjtobj(retstring);

	//LeaveCriticalSection(&cs);

	//{清空替换栈
	//将tagnumber初始化:
	//InitTagnumber();
	

	return matched_flag;
	
}
int CTemplate::Match_Single(const string &line, string &retstring, vector<int>& subind, vector<MatchResultType> & mat_temp)
{

	bool ret = false;


	
	ret = DoMatch( line, retstring, subind, mat_temp);


	return ret;
}


bool CTemplate::Init(const string & tempfile)
{
	ifstream temp(tempfile.c_str());
	string line = "";
	AllTemp.clear();
	while(getline(temp,line))
	{
		string str1 = line;
        //cout << "template [" << line << "]" << endl;		
		CTemplate::TmpltType Tmplt;
		//string str2 = ;
		Tmplt.pri = atoi((str1.substr(0,1)).c_str());
		int p = str1.find("|||",2);
		string str2 = str1.substr(2,p-2);
		Tmplt.chTmplt = str2;
		Tmplt.eTmplt = str1.substr(p+3,str1.length()-p-3);
		AllTemp.push_back(Tmplt);
	}
	temp.close();
	return true;
}
