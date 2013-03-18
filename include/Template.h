#ifndef TEMPLATE_H
#define TEMPLATE_H

#include "rain_algorithm.h"
#include "utils.h"
using namespace rain;

// add by yuheng  2011/5/30  read NE , and generate str for rules and original sentence
bool ReadNeTemplate(const string & src, vector<string> & rule_str, string & ori_sen)
{
	vector<string> vecWord;
	string::size_type current = 0;
	string::size_type temp_current = 0;
	string::size_type beg_words = 0;
	string::size_type end_words = 0;
	vector<string>::size_type beg_vec_index = 0;
	vector<string>::size_type end_vec_index = 0;
	vector<string> vecStr;
    //cout << "src [" << src << "]" << endl;
	while (true)
	{
		beg_words = current;
		temp_current = current;
		current = src.find("Chinese=\"", current);
		//vector<string> pre_words = split(src.substr(temp_current, current - temp_current - 1));
		pair<int, int> NE_span;
		if (current == string::npos)
		{
			current = src.find("english=\"", temp_current);
			if (current == string::npos)
			{
				break;
			}			
		}
        //NE_span.first = pre_words.size(); 
		end_words = src.rfind("<", current);

		// not NE
		string str_sub = src.substr(beg_words, end_words - beg_words);
		trim(str_sub);
		if (!str_sub.empty())
		{
			vecStr.clear();
			vecStr = split(str_sub);
			vecWord.insert(vecWord.end(), vecStr.begin(), vecStr.end() );				
		}
        NE_span.first = vecWord.size();
		beg_vec_index = vecWord.size();

		//获取英文翻译和中文词语
		string::size_type eleft = current + 9;
		string::size_type eright = src.find_first_of("\"", eleft);
		string english = src.substr(eleft, eright - eleft);
		string::size_type cleft = src.find_first_of(">", eright);
		string::size_type cright = src.find_first_of("<", eleft);
		string chinese = src.substr(cleft + 1, cright - cleft - 1);
        string::size_type end_NE_Type = src.find(">", cright);

		//去掉chinese的空格
		trim(chinese);
		vecStr.clear();
		vecStr = split(chinese);
		NE_span.second = NE_span.first + vecStr.size() -1 ;
		vecWord.insert(vecWord.end(), vecStr.begin(), vecStr.end() );
		end_vec_index = vecWord.size() - 1;

		//对english进行处理
		vecStr.clear();
		vecStr = split(english, "|");
		vector<string>::iterator iterVec;
		for (iterVec = vecStr.begin(); iterVec != vecStr.end(); ++iterVec)
		{
			trim(*iterVec);
			//lowerStr(*iterVec);
			string span_str = to_string(NE_span.first) + ":" + to_string(NE_span.second);
			string _NE_rule_str = span_str + " ||| " + chinese + " ||| " + *iterVec + " ||| 0 0 0 0";
			rule_str.push_back(_NE_rule_str);
		}
		//mapNETrans[chinese] = vecStr;

		current = end_NE_Type + 1;
	}
	if (beg_words != src.size())
	{
		string str_sub = src.substr(beg_words);
		trim(str_sub);
		vecStr.clear();
		vecStr = split(str_sub);
		vecWord.insert(vecWord.end(), vecStr.begin(), vecStr.end() );
	}
	for(size_t i = 0; i < vecWord.size(); ++i)
	{
		if(i != 0)
			ori_sen += " ";
		ori_sen += vecWord[i];
	}
	return true;
}

void GetNErulematch(vector<string> & rule_str, vector<RuleMatch> & ne_rulematch, Graph & graph)
{
	for(size_t i = 0; i < rule_str.size(); ++i)
	{
		pair<int, int> rule_span;
		// because of add head/tail tags in the ori_sen, so here all the span should +1
		vector<string> rule_str_set = split(rule_str[i], " ||| ");
		if(rule_str_set.size() == 0)
			continue;
		
		size_t pos = rule_str_set[0].find(":");
		if(pos == string::npos)
			continue;
		rule_span.first = to_int(rule_str_set[0].substr(0, pos).c_str());
		rule_span.second = to_int(rule_str_set[0].substr(pos+1, rule_str_set[0].size()-pos-1).c_str());
		RuleMatch tmp_rule;
        size_t pos1 = rule_str[i].find("|||");
		size_t pos2 = rule_str[i].size();
		if(rule_str_set.size() > 4)
		{
			pos2 = rule_str[i].rfind("|||");
		}
        //cout << "rule_str ["<< rule_str[i] << "]" << endl;
        //cout << "rule [ " << rule_str[i].substr(pos1+4, rule_str[i].size()-pos1-4) << "]" << endl;
		tmp_rule.gnode = graph.tmp_trie->add_rule('X', rule_str[i].substr(pos1+4, pos2-pos1-4));
		tmp_rule.span = rule_span;
		if(rule_str_set.size() > 4)
		{
			vector<Xtran> tmp_xtrans;
			//Xtran::UnSerializeVXtran(rule_str_set[4], tmp_xtrans);
			tmp_rule.xes = tmp_xtrans;
		}
		if(tmp_rule.gnode != NULL)
			ne_rulematch.push_back(tmp_rule);
	}
}


string AnalyTemplateStr(const string & str)
{
	vector<string> words = split(str, " ");
	bool tag = false;
	for(size_t i = 0; i < words.size(); ++i)
	{
		size_t pos = words[i].find("@@");
		if(pos != string::npos)
		{
			if(tag == false)
			{
				words[i] = "#X1#";
				tag = true;
			}
			else
				words[i] = "#X2#";
		}
	}
	string r_str = "";
	for(size_t i = 0; i < words.size(); ++i)
	{
		if(i != 0)
			r_str = r_str + " ";
		r_str += words[i];	
	}
    trim(r_str);
	return r_str;
}


bool SpanReplace(vector<string> & vec, vector<pair<int, int> > & span_set)
{
    vector<string> _new_vec;
    /*if(whole_span.second >= vec.size())
        whole_span.second = vec.size()-1;
    if(whole_span.first < 0)
        whole_span.first = 0;*/
    if(span_set.size() < 2)
        return false;
    //int offset = vec.size() - span_set[0].second + span_set[0].first;
	for(int i = span_set[0].first; i <= span_set[0].second; ++i)
	{
		if(i >= span_set[1].first && i <= span_set[1].second)
        {
            _new_vec.push_back("#X1#");
            continue;
        }
        if(span_set.size() == 3)
        {
            if(i >= span_set[2].first && i <= span_set[2].second)
            {
                _new_vec.push_back("#X2#");
                continue;
            }
        }
		_new_vec.push_back(vec[i]);
	}
    vec = _new_vec;
    return true;
}

void ReplaceX(vector<string> & str)
{
    bool x1_tag = false;
    bool x2_tag = false;
    vector<string> _new_str;
    for(size_t i = 0; i< str.size(); ++i)
    {
        if(str[i] == "#X1#")
        {
            if(x1_tag == false)
            {
                _new_str.push_back(str[i]);
                x1_tag = true;
            }
            else
                continue;
        }
        else if(str[i] == "#X2#")
        {
            if(x2_tag == false)
            {
                _new_str.push_back(str[i]);
                x2_tag = true;
            }
            else
                continue;
        }
        else
            _new_str.push_back(str[i]);
    }
    str = _new_str;
}

void FindXtrans(const vector<string> & _vec, vector<Xtran> & xtrans)
{
    for(size_t i = 0; i < _vec.size(); ++i)
    {
        if(_vec[i] == "#X1#")
        {
            Xtran x(i, i);
            //cout << "xtrans [" << i << "]" << endl;
            xtrans.push_back(x);
        }
        else if(_vec[i] == "#X2#")
        {
            Xtran x(i, i);
            //cout << "xtrans [" << i << "]" << endl;
            xtrans.push_back(x);
        }
        else
            continue;
    }
}

bool IwistTemplateMatch(const string & line, vector<RuleMatch> & rule_matches, Graph & graph, CTemplate* Patent)
{
	int tag;
	string result;
	vector<int> subid;
	vector<CTemplate::MatchResultType> mat_temp;

	tag = Patent->Match_Single(line,result,subid, mat_temp);
	if (tag)
	{
        //cout <<"Template "<< line << endl;
		for(size_t i = 0;i < mat_temp.size(); ++i)
		{
            vector<pair<int, int> > span_set;
			pair<int, int> whole_span;
			whole_span.first = mat_temp[i].start;
			whole_span.second = mat_temp[i].end;

			//word index: w0 w1 w2 w3   NOT space-index:  0 w 1 w 2 w 3 w 4 
			//Chiero: span = [i, j]

			//cout << "span [" << whole_span.first << "-" << whole_span.second << "]" << endl;
            span_set.push_back(whole_span);
            string trg= AnalyTemplateStr(mat_temp[i].body);
			vector<string> src_words = split(line, " ");
            vector<Xtran> tmp_xtrans;
			size_t pos = mat_temp[i].body.find("@@");
			while(pos != string::npos)
			{
				size_t pos2 = mat_temp[i].body.find(" ", pos);
				size_t pos3 = mat_temp[i].body.find(",", pos);
				if(pos2 != string::npos && pos3 != string::npos)
				{
					pair<int, int> tmp_span;
					tmp_span.first = to_int(mat_temp[i].body.substr(pos+2, pos3-pos-2));
					tmp_span.second = to_int(mat_temp[i].body.substr(pos3+1, pos2-pos3-1));
					span_set.push_back(tmp_span);
					Xtran tmp_xtran;
					tmp_xtran.i = tmp_span.first;
					tmp_xtran.j = tmp_span.second;
					tmp_xtrans.push_back(tmp_xtran);
				}
				pos = mat_temp[i].body.find("@@", pos+1);
			}
            SpanReplace(src_words, span_set);
            ReplaceX(src_words);
            string src = "";
			for(size_t j = 0; j < src_words.size(); ++j)
			{
				if(j != 0)
					src += " ";
				src += src_words[j];
			}

			string rule_str = src + " ||| " + trg + " ||| 20 20 20 20";
            RuleMatch tmp_rule;
			tmp_rule.gnode = graph.tmp_trie->add_rule('X', rule_str , true);
            //cout << "rule[" << rule_str << "]" << endl;
            tmp_rule.span = whole_span;
			tmp_rule.xes = tmp_xtrans;
            if(tmp_rule.gnode != NULL)
			    rule_matches.push_back(tmp_rule);
		}
	}
    return true;
}


#endif
