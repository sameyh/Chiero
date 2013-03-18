


#include "utils.h"
//#include <strstream>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <sstream>
#include <list>

CTemplate CTemplate::m_TransTemp;

typedef string::size_type size_type;
//��str1�еĿո�ȥ�����ѷֹ��ʵľ��Ӻϲ���
/*
void DelSpace(string &str,struct sent &elem)
{
	;
}*/




/**
*  ����:	��str�Կո�Ϊ�ָ������зָ�������vec��ȥ��������ȡģ���к���Լ���е�����.
*  ����:
*		@str:	���ָ����ַ�����
*		@vec:	��ŷָ�������ַ�����
*  ����ֵ:
* 		0:	û�д���
*		g_errno��	��������
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
*  [����]:
*		ɾ��str�е����пո�
*  [����]:
*		@str:	��ɾ���ո���ַ�����
*  [����ֵ]:
*		�ޣ�
*/
void CTemplate::DelAllSpace(string& str)
{
	str.erase(std::remove(str.begin(),str.end(),' '),str.end());
}

/**
*  ����:
*		ɾ��str��ͷ�����β�Ŀո�
*  ����:
*		@str:	��ɾ���ո���ַ�����
*  ����ֵ:
*		�ޣ�
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
*  ����:
*		��str�Կո�Ϊ�ָ������зָ�������vec��ȥ��
*  ����:
*		@str:	���ָ����ַ�����
*		@vec:	��ŷָ�������ַ�����
*  ����ֵ:
* 		true:	str�ǿգ�
*		false�� strΪ�գ�
*/
bool CTemplate::Str2Vec(const string& str, vector<string> &vec)
{
	if(str.empty()) return false;
	vec.clear();

	//�����netrans��ǣ�ȥ��������ݺ� ����vec.
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
*  ����:
*		��str�����е�src������sub�������档
*  ����:
*		@str:	���str����sub�滻src��Ľ����
*		@sub:	 �����滻���Ӵ���
*		@src:	���滻���ַ�����
*  ����ֵ:
*		�ޣ�
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
//ģ����������:�ܿغ���;
bool CTemplate::DoMatch( const string &line, string &retstring, vector<int>& index, vector<MatchResultType> & mat_temp)
{

	bool matched_flag=false;	//�ɹ�ƥ����ĳЩģ��;
	string sentence = line;	//������ģ��ƥ��ľ���;

	mat_temp.clear();
	bool mt = MatchSent(sentence, retstring, index, mat_temp);
	//���������ƥ��:������������Ϊһ������ȥ����ģ���ƥ��.
	if(mt)
	{		
		matched_flag=true;	

	}

	//��ӡ�����õ���ģ�壻

	//ģ��ƥ�����,modify by caojie, ����ŵ�ִ����DoMatch()ִ������(�ں���Match_Single��)��
	//DelSubstr( retstring, "$$", "" );
	//qjtobj(retstring);

	//LeaveCriticalSection(&cs);

	//{����滻ջ
	//��tagnumber��ʼ��:
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
