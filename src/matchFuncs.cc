


#include "utils.h"
#include <iostream>
//#include <strstream>
#include <fstream>
#include <sstream>
using namespace std;

typedef CTemplate::TmpltType TmpltType;
//typedef CTemplate::VarType VarType;




//��ȡģ��
TmpltType CTemplate::GetTemplate( const size_t index )
{
	TmpltType t;
	t=AllTemp[index];
	return t;
}

//����	
//��ȡ��һ��ģ���������;
size_t CTemplate::GetFirstIndex()
{
	return 0;
}

//���index����һ��ģ���������;
inline size_t CTemplate::GetNextIndex( const size_t index )
{
	return index+1;
}

//�жϱ���ģ����ʱ���Ƿ��Ѿ��ߵ�ģ����ĩβ
bool CTemplate::IsEnd( const size_t index )
{
	return !(index<AllTemp.size());
}


//����ģ��ƥ��;
bool CTemplate::SingleTempateMatch( const string& srcSent, const size_t tmpltIndex,const vector<string>& tempvec, MatchResultType& match_result )
{//��������Ҫȥ��netrans���

	
	TmpltType using_tmplt = GetTemplate( tmpltIndex );
	//cmatch mat;
	//regex reg( using_tmplt.chTmplt);
	//bool r=regex_search( srcSent.c_str(), mat, reg);
	CRegexpT <char> szReg(using_tmplt.chTmplt.c_str());
	MatchResult mat=szReg.Match(srcSent.c_str());

	bool r=false;
	if(mat.IsMatched()) //���ƥ��ɹ�
	{
		r=true;
		
		struct temp_var elem;
		//int tag=0;
		//��ne trans ��Ǵ���,��¼�±���ں�������span
		//���ӣ��� �� ���� �� �� <TIME english="three o'clock|three o'clock p.m." believable=true>���� ���� </TIME> ���� �Ǽ� ��
		//cmatch mat_ne;
		//string ne_tag="english=(.*)?believable=[true|false]> ";
		//regex reg_ne( ne_tag);
		//bool rr=regex_search( srcSent.c_str(), mat_ne, reg_ne);

		//�ж��Ƿ���netrans�ı��,����У����ұ�ǰ�����ģ��ƥ�䷶Χ�ڣ�����ģ��ƥ���ϲ��ֵ�ǰ�棬��Ҫ��������������ܡ�
		if(srcSent.find("english=")!= string::npos)//�����netrans�ı��
		{
				string src_no_ne;
				//cmatch::iterator itr=mat.begin();
				
	
				//       ָ���Ӵ���Ӧ��λ��        ָ���Ӵ���Ӧβλ��          �Ӵ�����
				//cout << itr->first-srcSent.c_str() << ' ' << itr->second-srcSent.c_str() << ' ' << *itr << endl;
				//cout << mat.GetGroupStart(0) << ' ' << mat.GetGroupEnd(0) << ' ' << srcSent.substr(mat.GetGroupStart(0),mat.GetGroupEnd(0)-mat.GetGroupStart(0)) << endl;
				//int start0 = itr->first-srcSent.c_str();
				//int end0 = itr->second-srcSent.c_str();
				int start0=mat.GetGroupStart(0);
				int end0=mat.GetGroupEnd(0);
				string str=srcSent.substr(start0,end0-start0);
				//elem.c=*itr;
				elem.c=str;
				//һ�������п��ܲ�ֻһ�����
				int i=0,count_cha=0;//ģ��ǰ������netrans���ʱ�����ռ���ַ���
				while(i<int(srcSent.length()))
				{
					int st_s=srcSent.find("<",i);
					if(st_s!=-1)
					{
						int st_e=srcSent.find(">",st_s);

						int end_s=srcSent.find("</",st_e);
						int end_e=srcSent.find(">",end_s);

						if(start0>end_e)//ģ��ǰ������netrans���
						{
							count_cha=count_cha+st_e-st_s+end_e-end_s+2;
							i=end_e;
						}
						else
							break;
					}
					else
						break;
						
				}
				//����ģ��Ŀ�ʼλ��
				start0=start0-count_cha;
				int count_s=0;
                //int count_e=0;
				for(size_t i = 0; i<tempvec.size();i++)
				{
					if(start0==count_s)
					{
						elem.s=i;
						break;
					}
					if(start0>count_s)
					{
						count_s = count_s+tempvec[i].length();
						if(start0==count_s)
						{
							elem.s=i+1;
							break;
						}

					}
					if(start0<count_s)
					{
						r=false;
						break;
					}
				}
				if(r)
				{
					int i=0,count_cha_e=0;//ģ��ǰ������netrans���ʱ�����ռ���ַ���
					while(i<int(srcSent.length()))
					{
						int st_s=srcSent.find("<",i);
						if(st_s!=-1)
						{
							int st_e=srcSent.find(">",st_s);

							int end_s=srcSent.find("</",st_e);
							int end_e=srcSent.find(">",end_s);

							if(end0>end_e)//ģ��ǰ������netrans���
							{
								count_cha_e=count_cha_e+st_e-st_s+end_e-end_s+2;
								i=end_e;
							}
							else
								break;
						}
						else
							break;
						
					}
					//����ģ��Ľ���λ��
					//end0=end0-count_cha_e-count_cha;
					end0=end0-count_cha_e;
					int count_s = 0;
                    //int count_e=0;
					for(size_t i = 0; i<tempvec.size();i++)
					{
						if(end0==count_s)
						{
							elem.e=i-1;
							break;
						}
						if(end0>count_s)
						{
							count_s = count_s+tempvec[i].length();
							if(end0==count_s)
							{
								elem.e=i;
								break;
							}

						}
						if(end0<count_s)
						{
							r=false;
							break;
						}
					}
					match_result.var_result.push_back(elem);

				}
				
				if(r)
				{

					//�ټ����ÿ�������ڲ���netrans�����ռ�ַ�

					for(int itr=1; itr<mat.MaxGroupNumber()+1; ++itr)//�鿴ģ���ڵ�ÿ�������Ƿ��б��
					{
						//       ָ���Ӵ���Ӧ��λ��        ָ���Ӵ���Ӧβλ��          �Ӵ�����
						//cout << itr->first-srcSent.c_str() << ' ' << itr->second-srcSent.c_str() << ' ' << *itr << endl;
						int starti=mat.GetGroupStart(itr);
						int endi=mat.GetGroupEnd(itr);
						//cout << mat.GetGroupStart(itr) << ' ' << mat.GetGroupEnd(itr) << ' ' << srcSent.substr(starti,endi-starti) << endl;
						//int starti = itr->first-srcSent.c_str();
						//int endi = itr->second-srcSent.c_str();
						
						elem.c=srcSent.substr(starti,endi-starti);
						//elem.c=*itr;
						int i=0,count_cha=0;//ģ��ǰ������netrans���ʱ�����ռ���ַ���
						while(i<int(srcSent.length()))
						{
							int st_s=srcSent.find("<",i);
							if(st_s!=-1)
							{
								int st_e=srcSent.find(">",st_s);

								int end_s=srcSent.find("</",st_e);
								int end_e=srcSent.find(">",end_s);

								if(starti>end_e)//ģ��ǰ������netrans���
								{
									count_cha=count_cha+st_e-st_s+end_e-end_s+2;
									i=end_e;
								}
								else
									break;
							}
							else
								break;							

						}
						//��������Ŀ�ʼλ��
						starti=starti-count_cha;
						int count_s=0;
                        //int count_e=0;
						for(size_t i = 0; i<tempvec.size();i++)
						{
							if(starti==count_s)
							{
								elem.s=i;
								break;
							}
							if(starti>count_s)
							{
								count_s = count_s+tempvec[i].length();
								if(starti==count_s)
								{
									elem.s=i+1;
									break;
								}

							}
							if(starti<count_s)
							{
								r=false;
								break;
							}
						}
						if(r)
						{
							int i=0,count_cha_e=0;//ģ��ǰ����netrans���ʱ�����ռ���ַ���
							while(i<int(srcSent.length()))
							{
								int st_s=srcSent.find("<",i);
								if(st_s!=-1)
								{
									int st_e=srcSent.find(">",st_s);

									int end_s=srcSent.find("</",st_e);
									int end_e=srcSent.find(">",end_s);

									if(endi>end_e)//ģ��ǰ������netrans���
									{
										count_cha_e=count_cha_e+st_e-st_s+end_e-end_s+2;
										i=end_e;
									}
									else
										break;
								}
								else
									break;
								
							}
							//��������Ľ���λ��
							endi=endi-count_cha_e;
							int count_s=0;
                            //int count_e=0;
							for(size_t i = 0; i<tempvec.size();i++)
							{
								if(endi==count_s)
								{
									elem.e=i-1;
									break;
								}
								if(endi>count_s)
								{
									count_s = count_s+tempvec[i].length();
									if(endi==count_s)
									{
										elem.e=i;
										break;
									}

								}
								if(endi<count_s)
								{
									r=false;
									break;
								}
							}
							match_result.var_result.push_back(elem);

						}


					}
				}
			
		}
		
		else{
				
		//��¼�����Ӵ�,û��netrans���ʱ
		for(int itr=0; itr<mat.MaxGroupNumber()+1; ++itr)
		{
			//       ָ���Ӵ���Ӧ��λ��        ָ���Ӵ���Ӧβλ��          �Ӵ�����
			int starti=mat.GetGroupStart(itr);
			int endi=mat.GetGroupEnd(itr);
			//cout << mat.GetGroupStart(itr) << ' ' << mat.GetGroupEnd(itr) << ' ' << srcSent.substr(starti,endi-starti) << endl;
			
			//int starti = itr->first-srcSent.c_str();
			//int endi = itr->second-srcSent.c_str();
			
			//cout<<starti << "  "<<endi<<"  "<<*itr<<endl;
			//���Ӵ���ʼ��β���ַ��������������span
			int count_s=0,count_e=0;
			for(size_t i = 0; i<tempvec.size();i++)
			{
				if(starti==count_s)
				{
					elem.s=i;
					break;
				}
				if(starti>count_s)
				{
					count_s = count_s+tempvec[i].length();
					if(starti==count_s)
					{
						elem.s=i+1;
						break;
					}

				}
				if(starti<count_s)
				{
					r=false;
					break;
				}
			}
			if(r)
			{
				for(size_t i = 0; i<tempvec.size();i++)
				{
					if(endi==count_e)
					{
						elem.e=i;
						break;
					}
					if(endi>count_e)
					{
						count_e = count_e+tempvec[i].length();
						if(endi==count_e)
						{
							elem.e=i;
							break;
						}

					}

					if(endi<count_e)
					{
						r=false;
						break;
					}
				}
			}
			if(r==false)
				break;
			//elem.c=*itr;
			elem.c=srcSent.substr(starti,endi-starti);
			match_result.var_result.push_back(elem);
		
		}
		}
		
		if(r)
		{
			match_result.start=match_result.var_result[0].s;
			match_result.end=match_result.var_result[0].e;
			
			
			//��ģ����Ӣ���滻��
			string en=using_tmplt.eTmplt;
			//vector<struct temp_var> var_result;

			for(int i=1;i<int(match_result.var_result.size());i++)
			{
				char num[10];
				//itoa(match_result.var_result[i].s, num, 10);		 
				sprintf(num,"%d",match_result.var_result[i].s);
                string var_span="@@";
				var_span=var_span+num;		
				//itoa(match_result.var_result[i].e, num, 10);
				 sprintf(num,"%d",match_result.var_result[i].e);
                var_span=var_span+","+num;	

				char str1[10];
			    sprintf(str1,"%d",i);
                //string str2=itoa(i,str1,10);
			    string str2=str1;
                string tag="##"+str2;
				int p=en.find(tag,0);


				//int q=en.find
				string sub1=en.substr(0,p);
				string sub2=en.substr(p+3,en.length()-p-3);
				en=sub1+var_span+sub2;

			}
			match_result.body=en;
			
			match_result.pri=using_tmplt.pri;
			match_result.temp_id=tmpltIndex;
		}
	

	}

	return r;

}

//�������ܵ�ģ����ƥ�����: ��Ϊ�����㷨�����غ�����
bool CTemplate::MatchSent( const string &oristr, string &retstring, vector<int>& indexVec, vector<MatchResultType> & mat_temp)
{
	bool success = false;			//���������oristr��ƥ���ģ��,ƥ��ɹ�
	string currentResult = "";		//��ǰģ��ƥ��ɹ���Ľ����
	string str_body_ch;

	//////////////////////////////////////////////////////////////////////////

	string ref_oristr = oristr;		//����oristr������,ֻ���丱��ref_oristr���任����;
	//{����ƥ��ľ�����Ԥ����:����ո�,��Ǳ��ת��Ϊȫ�Ǳ��;
	DelAllSpace(ref_oristr);
	//DelSubstr(ref_oristr,"$$","");
	DelSubstr(ref_oristr,":","��");
	DelSubstr(ref_oristr,";","��");
	DelSubstr(ref_oristr,",","��");	
	//Ԥ����}

	//storeStr�������洢�����tempvec�е��ִ�,��Ϊ���ı����Ӣ�ı�������ֽ�����ͬ,
	//���Զ�ת�������ı���洢����,�Ա���FindClas�м��㳤��.
	string storeStr = oristr;		
	DelSubstr(storeStr,":","��");
	DelSubstr(storeStr,";","��");
	DelSubstr(storeStr,",","��");
	vector<string> tempvec;
	//storeStr="���� ���� �� ���� ���� ��ʼ ������ ��";
	Str2Vec(storeStr,tempvec);			//***

	bool thisTurnMatched = false;	//�ڱ��ֲ���ƥ��ʱ,�ҵ���ƥ���ģ��ʱΪtrue,����Ϊfalse;
	size_t index = GetFirstIndex();
	//size_t first_match_index =  0;

	//����ģ���,�����Ƿ���ڿ���ƥ���ģ��
	while( !IsEnd( index ) )
	{
		
		MatchResultType match_result;			//ģ��ɹ�ƥ���Ľ������ڴˣ�Ӧ�÷���һ��������(����ƥ���ϵ�ģ��)
	
		if ( SingleTempateMatch( ref_oristr, index, tempvec, match_result ) )
		{

			//�ҵ���һ��ƥ���ϵ�ģ��
			if ( !thisTurnMatched )
			{
				success = true;
			
			} 
			mat_temp.push_back(match_result);
		
		}
		//ȡ����һ��ģ���������
		index = GetNextIndex( index );
	}

	if( !success )
	{
		retstring = oristr;
		Trim(retstring);
	}

	return success;
	
}
