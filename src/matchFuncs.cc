


#include "utils.h"
#include <iostream>
//#include <strstream>
#include <fstream>
#include <sstream>
using namespace std;

typedef CTemplate::TmpltType TmpltType;
//typedef CTemplate::VarType VarType;




//获取模板
TmpltType CTemplate::GetTemplate( const size_t index )
{
	TmpltType t;
	t=AllTemp[index];
	return t;
}

//遍历	
//获取得一条模板的索引号;
size_t CTemplate::GetFirstIndex()
{
	return 0;
}

//获得index的下一条模板的索引号;
inline size_t CTemplate::GetNextIndex( const size_t index )
{
	return index+1;
}

//判断遍历模板库的时候是否已经走到模板库的末尾
bool CTemplate::IsEnd( const size_t index )
{
	return !(index<AllTemp.size());
}


//单个模板匹配;
bool CTemplate::SingleTempateMatch( const string& srcSent, const size_t tmpltIndex,const vector<string>& tempvec, MatchResultType& match_result )
{//向量中需要去掉netrans标记

	
	TmpltType using_tmplt = GetTemplate( tmpltIndex );
	//cmatch mat;
	//regex reg( using_tmplt.chTmplt);
	//bool r=regex_search( srcSent.c_str(), mat, reg);
	CRegexpT <char> szReg(using_tmplt.chTmplt.c_str());
	MatchResult mat=szReg.Match(srcSent.c_str());

	bool r=false;
	if(mat.IsMatched()) //如果匹配成功
	{
		r=true;
		
		struct temp_var elem;
		//int tag=0;
		//对ne trans 标记处理,记录下标记内汉语所在span
		//例子：我 想 我们 将 在 <TIME english="three o'clock|three o'clock p.m." believable=true>下午 三点 </TIME> 左右 登记 。
		//cmatch mat_ne;
		//string ne_tag="english=(.*)?believable=[true|false]> ";
		//regex reg_ne( ne_tag);
		//bool rr=regex_search( srcSent.c_str(), mat_ne, reg_ne);

		//判断是否有netrans的标记,如果有，并且标记包含在模板匹配范围内，或在模板匹配上部分的前面，都要处理，其它情况不管。
		if(srcSent.find("english=")!= string::npos)//如果有netrans的标记
		{
				string src_no_ne;
				//cmatch::iterator itr=mat.begin();
				
	
				//       指向子串对应首位置        指向子串对应尾位置          子串内容
				//cout << itr->first-srcSent.c_str() << ' ' << itr->second-srcSent.c_str() << ' ' << *itr << endl;
				//cout << mat.GetGroupStart(0) << ' ' << mat.GetGroupEnd(0) << ' ' << srcSent.substr(mat.GetGroupStart(0),mat.GetGroupEnd(0)-mat.GetGroupStart(0)) << endl;
				//int start0 = itr->first-srcSent.c_str();
				//int end0 = itr->second-srcSent.c_str();
				int start0=mat.GetGroupStart(0);
				int end0=mat.GetGroupEnd(0);
				string str=srcSent.substr(start0,end0-start0);
				//elem.c=*itr;
				elem.c=str;
				//一个句子中可能不只一个标记
				int i=0,count_cha=0;//模板前面有你netrans标记时，标记占的字符数
				while(i<int(srcSent.length()))
				{
					int st_s=srcSent.find("<",i);
					if(st_s!=-1)
					{
						int st_e=srcSent.find(">",st_s);

						int end_s=srcSent.find("</",st_e);
						int end_e=srcSent.find(">",end_s);

						if(start0>end_e)//模板前面有你netrans标记
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
				//计算模板的开始位置
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
					int i=0,count_cha_e=0;//模板前面有你netrans标记时，标记占的字符数
					while(i<int(srcSent.length()))
					{
						int st_s=srcSent.find("<",i);
						if(st_s!=-1)
						{
							int st_e=srcSent.find(">",st_s);

							int end_s=srcSent.find("</",st_e);
							int end_e=srcSent.find(">",end_s);

							if(end0>end_e)//模板前面有你netrans标记
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
					//计算模板的结束位置
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

					//再计算出每个变量内部的netrans标记所占字符

					for(int itr=1; itr<mat.MaxGroupNumber()+1; ++itr)//查看模板内的每个变量是否有标记
					{
						//       指向子串对应首位置        指向子串对应尾位置          子串内容
						//cout << itr->first-srcSent.c_str() << ' ' << itr->second-srcSent.c_str() << ' ' << *itr << endl;
						int starti=mat.GetGroupStart(itr);
						int endi=mat.GetGroupEnd(itr);
						//cout << mat.GetGroupStart(itr) << ' ' << mat.GetGroupEnd(itr) << ' ' << srcSent.substr(starti,endi-starti) << endl;
						//int starti = itr->first-srcSent.c_str();
						//int endi = itr->second-srcSent.c_str();
						
						elem.c=srcSent.substr(starti,endi-starti);
						//elem.c=*itr;
						int i=0,count_cha=0;//模板前面有你netrans标记时，标记占的字符数
						while(i<int(srcSent.length()))
						{
							int st_s=srcSent.find("<",i);
							if(st_s!=-1)
							{
								int st_e=srcSent.find(">",st_s);

								int end_s=srcSent.find("</",st_e);
								int end_e=srcSent.find(">",end_s);

								if(starti>end_e)//模板前面有你netrans标记
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
						//计算变量的开始位置
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
							int i=0,count_cha_e=0;//模板前面有netrans标记时，标记占的字符数
							while(i<int(srcSent.length()))
							{
								int st_s=srcSent.find("<",i);
								if(st_s!=-1)
								{
									int st_e=srcSent.find(">",st_s);

									int end_s=srcSent.find("</",st_e);
									int end_e=srcSent.find(">",end_s);

									if(endi>end_e)//模板前面有你netrans标记
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
							//计算变量的结束位置
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
				
		//记录所有子串,没有netrans标记时
		for(int itr=0; itr<mat.MaxGroupNumber()+1; ++itr)
		{
			//       指向子串对应首位置        指向子串对应尾位置          子串内容
			int starti=mat.GetGroupStart(itr);
			int endi=mat.GetGroupEnd(itr);
			//cout << mat.GetGroupStart(itr) << ' ' << mat.GetGroupEnd(itr) << ' ' << srcSent.substr(starti,endi-starti) << endl;
			
			//int starti = itr->first-srcSent.c_str();
			//int endi = itr->second-srcSent.c_str();
			
			//cout<<starti << "  "<<endi<<"  "<<*itr<<endl;
			//由子串开始结尾的字符数，计算出所在span
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
			
			
			//把模板用英文替换掉
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

//搜索可能的模板以匹配句子: 此为搜索算法的主控函数；
bool CTemplate::MatchSent( const string &oristr, string &retstring, vector<int>& indexVec, vector<MatchResultType> & mat_temp)
{
	bool success = false;			//存在与句子oristr相匹配的模板,匹配成功
	string currentResult = "";		//当前模板匹配成功后的结果串
	string str_body_ch;

	//////////////////////////////////////////////////////////////////////////

	string ref_oristr = oristr;		//不对oristr做处理,只对其副本ref_oristr做变换处理;
	//{将待匹配的句子做预处理:祛除空格,半角标点转换为全角标点;
	DelAllSpace(ref_oristr);
	//DelSubstr(ref_oristr,"$$","");
	DelSubstr(ref_oristr,":","：");
	DelSubstr(ref_oristr,";","；");
	DelSubstr(ref_oristr,",","，");	
	//预处理}

	//storeStr是用来存储存放在tempvec中的字串,因为中文标点与英文标点所在字节数不同,
	//所以都转换成中文标点后存储下来,以便在FindClas中计算长度.
	string storeStr = oristr;		
	DelSubstr(storeStr,":","：");
	DelSubstr(storeStr,";","；");
	DelSubstr(storeStr,",","，");
	vector<string> tempvec;
	//storeStr="我想 会议 在 下午 三点 开始 。会议 是";
	Str2Vec(storeStr,tempvec);			//***

	bool thisTurnMatched = false;	//在本轮查找匹配时,找到可匹配的模板时为true,否则为false;
	size_t index = GetFirstIndex();
	//size_t first_match_index =  0;

	//遍历模板库,检索是否存在可以匹配的模板
	while( !IsEnd( index ) )
	{
		
		MatchResultType match_result;			//模板成功匹配后的结果存放于此；应该放在一个向量中(所有匹配上的模板)
	
		if ( SingleTempateMatch( ref_oristr, index, tempvec, match_result ) )
		{

			//找到第一个匹配上的模板
			if ( !thisTurnMatched )
			{
				success = true;
			
			} 
			mat_temp.push_back(match_result);
		
		}
		//取得下一条模板的索引：
		index = GetNextIndex( index );
	}

	if( !success )
	{
		retstring = oristr;
		Trim(retstring);
	}

	return success;
	
}
