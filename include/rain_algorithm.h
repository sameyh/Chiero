/**
* Copyright (c) rain, ICT
* All rights reserved.
*
* @date     2011-5-26 13:47
* 
* @brief    some common algorithms, only for linux platform.
* @author   summer
* @note 	
*/

#ifndef RAIN_ALGORITHM 
#define RAIN_ALGORITHM

#include <iostream>
#include <iomanip>
#include <sstream>
#include <set>
#include <stack>
#include <cstring>
#include <string>
#include <iterator>
#include <stdexcept>
#include <map>
#include <memory>
#include <list>
#include <locale>
#include <vector>
#include <queue>
#include <numeric>
#include <utility>
#include <fstream>
#include <functional>
#include <ctime>
#include <cassert>
#include <limits>	
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <algorithm>

using namespace std;

///////////////////////////////////////////////////////////////////////////////

#define PRINT_INF(x)            cout << __FILE__ << ":" << __LINE__ << ":" << x << endl;
#define VVec(type)              vector<vector<type > >
#define VecStr                  vector<string>
#define VecDbe                  vector<double>
#define VecInt                  vector<int>
#define BStr                    basic_string
#define ZERO                    1e-8
#define INT64                   long long
#define UINT64                  unsigned long long
#define OPEN_RFILE(fin, name)   ifstream fin(string(name).c_str()); assert(fin.good() || !(cerr << "can't open " << (name) << endl));
#define OPEN_WFILE(fou, name)   ofstream fou(string(name).c_str()); assert(fou.good() || !(cerr << "can't open " << (name) << endl));
#define MAXV(type)              numeric_limits<type>::max()
#define MINV(type)              numeric_limits<type>::min()

namespace rain
{
    template<class Type>
    vector<Type>& operator += (vector<Type>& v1, const vector<Type>& v2)        
    {
        for (int id = 0; id < min((int)v1.size(), (int)v2.size()); ++id)
            v1[id] += v2[id];
        return v1;
    }

    template<class Type>
    vector<Type>& operator -= (vector<Type>& v1, const vector<Type>& v2)
    {
        for (int id = 0; id < min((int)v1.size(), (int)v2.size()); ++id)
            v1[id] -= v2[id];
        return v1;
    }

    /**
      @brief used for quick sort algorithm. [begin, end)
     */
    template<class Iterator, class Type>
    inline Iterator partition(Iterator begin, Iterator end, Iterator pos)
    {
        assert(begin <= pos && pos < end);
        Type v = *pos;
        Iterator left  = begin;
        Iterator right = end - 1;

        *pos = *right;
        for (;;)
        {
            while (left < right && *left < v)
                ++left;
            if (left == right)
                break;
            else                    
            {
                *right = *left;
                --right;
            }
            while (left < right && *right >= v)
                --right;
            if (left == right)
                break;
            else
            {
                *left = *right;
                ++left;
            }
        }
        *left = v;
        return left;
    }

    inline UINT64 rand64()
    {
        return rand() ^ ((UINT64)rand() << 15) ^ ((UINT64)rand() << 30) ^ ((UINT64)rand() << 45) ^ ((UINT64)rand() << 60);
    }	

    /**
      @brief count the frequency of subs appear in str.
     */
    inline int count(const string& str, string subs)
    {
        if (subs.empty())
            return 0;
        int r = 0;
        int p = -1;
        while ((p = str.find(subs, p + 1)) != (int)string::npos)
            ++r;
        return r;
    }

    /**
      @brief erase any char in chs at the begin or end of s.
     */
    inline string strip(const string& s, const string chs = " \t\n")
    {
        if (s.empty())
            return s;
        int i = 0;
        while (i < (int)s.size() && chs.find(s[i]) != string::npos)
            ++i;
        int j = (int)s.size() - 1;
        while (j >= 0 && chs.find(s[j]) != string::npos)
            --j;
        ++j;
        return i >= j ? "" : s.substr(i, j - i);
    }

    inline string to_string(double m)
    {
            char r[64];
                int len = sprintf(r, "%.10f", m);
                    //cout << string(r) << endl;
                        while (r[len - 1] == '0')
                                    --len;                  
                                        if (r[len - 1] != '.')          
                                                    r[len] = '\0';               
                                                        else                             
                                                                    r[len - 1] = '\0';           
                                                                        return string(r);                
    }   

    template<class ConstPointer>
    inline string join(string s, ConstPointer b, ConstPointer e)
    {
        string r;
        while (b < e)
            r += *b++ + s;
        return r.empty() ? r : (r.erase(r.end() - s.size(), r.end()), r);
    }

    inline string join(string s, const vector<string>& src_vector)
    {
        return join(s, src_vector.begin(), src_vector.end());
    }
    inline string join(string s, const vector<double>& src_vector)
    {
        vector<string> tmp;
        for(size_t i = 0;i < src_vector.size(); ++i)
            tmp.push_back(to_string(src_vector[i]));
        return join(s, tmp.begin(), tmp.end());
    }
    /**
      @brief eg. split("   a   b", " ") will return [a, b].
      @param[in] delimit view delimit as a whole string.
     */
    inline vector<string> split(const string& src, string delimit)
    {
        vector<string> r;
        int f = 0;
        int p = -1;

        while ((p = (int)src.find(delimit, p + 1)) != (int)string::npos)
        {
            if (p > f)
                r.push_back(src.substr(f, p - f));				
            f = p + (int)delimit.size();
        }
        p = (int)src.size();
        if (p > f)
            r.push_back(src.substr(f, p - f));				
        return r;
    }

    /**
      @brief replace "sub" in "s" as "nstr".
     */
    inline string replace(const string& s, string sub, string nstr)
    {
        int pos = 0;
        string ns = s;
        while ((pos = ns.find(sub, pos)) != (int)string::npos)
        {
            ns = ns.replace(pos, sub.size(), nstr);
            pos += (int)nstr.size();
        }
        return ns;
    }

    /**
     * @brief split string by any blank char.
     * */
    inline vector<string> split(const string& src)
    {
        istringstream iss(src);
        vector<string> r;
        string w;
        while (iss >> w)
            r.push_back(w);
        return r;
    }

    /**
      @brief return a random sequence in [begin, end).
     */
    inline int rand_range(int begin, int end)
    {
        assert(begin < end);
        return int(rand64() % (end - begin)) + begin;
    }
    /*
    inline string to_string(double m)
    {	 
        char r[64];
        int len = sprintf(r, "%.10f", m);		
        //cout << string(r) << endl;
        while (r[len - 1] == '0')
            --len;
        if (r[len - 1] != '.')
            r[len] = '\0';
        else
            r[len - 1] = '\0';
        return string(r);
    }*/	

	/**
	  @brief erase the space in the front and back of the string
	*/
	inline void trim(string& str, string tag = " \t")
	{
		if (str.empty())
		{
			return ;
		}
		string::size_type left = str.find_first_not_of( tag );
        if (left == string::npos)
        {
            str = "";
            return ;
        }
		string::size_type right = str.find_last_not_of( tag );
		str = str.substr( left, right - left + 1 );
	}


	/** added by yuheng : 2011-5-30
	* @brief lowercase the string
	* */
	inline int lowerStr(string & str)
	{
		for (string::size_type index = 0; index < str.size(); ++index)
		{
			char &ch = str.at(index);
			if ('A' <= ch && ch <= 'Z')
			{
				ch += 'a' - 'A';
			}
		}
		return 0;
	}

    inline string to_string(int m)
    {
        return to_string((double)m);
    }

    inline string to_string(long m)
    {
	return to_string((double)m);
    }
    inline double to_double(const char* s)
    {
        return atof(s);
    }

    inline double to_double(const string& s)
    {
        return to_double(s.c_str());
    }

    inline int	to_int(const char* s)
    {
        return (int)to_double(s);
    }

    inline int	to_int(const string& s)
    {
        return to_int(s.c_str());
    }

    inline UINT64 hash(const string &s)
    {
        static bool first = true;
        static UINT64 hashbit[1024][256];

        assert(s.size() < 1024 || !(cerr << (int)s.size() << endl));
        if (first)
        {
            first = !first;
            //srand(time(NULL));
            srand(0);
            for (int pos = 0; pos < 1024; ++pos)
            {
                for (int ch = 0; ch < 256; ++ch)
                    hashbit[pos][ch] = rand64();
            }
        }

        UINT64 r = 0;
        for (int i = 0; i < (int)s.size(); ++i)
            r ^= hashbit[i][(unsigned char)s[i]];
        return r;
    }

    inline bool eq(double m, double n, double prec = ZERO)
    {
        return fabs(m - n) < prec;
    }

    inline bool ge(double m, double n, double prec = ZERO)
    {
        return m > n || eq(m, n, prec);
    }

    /**
     * @brief judge whether "str" start with "prefix", compatible with python function.
     * */
    inline bool startswith(const string &str, string prefix)
    {
        return prefix.size() <= str.size() && strncmp(str.c_str(), prefix.c_str(), prefix.size()) == 0;
    }

    /**
     * @brief judge whether "str" end with "postfix", compatible with python function.
     * */
    inline bool endswith(const string& str, string postfix)
    {
        return postfix.size() <= str.size() && strcmp(str.c_str() + (str.size() - postfix.size()), postfix.c_str()) == 0;
    }

    /**
     * @brief return the content between "le" and "ri" in "str". 
     * e.g. match_str("<summer><rain>", "<", ">") is "summer", and return the postion next
     * to match.
     * */
    inline int match_str(const string& str, string le, string ri, string& con, int fpos = 0)
    {
        int npos = (int)string::npos;
        int p1;
        int p2;
        if ((p1 = (int)str.find(le, fpos)) != npos && (p2 = (int)str.find(ri, p1 + 1)) != npos)
        {
            con = str.substr(p1 + (int)le.size(), p2 - p1 - (int)le.size());
            return p2 + (int)ri.size();
        }
        return npos;
    }

    inline string upper(const string& str)
    {
        string nstr = str;
        for (int i = 0; i < (int)nstr.size(); ++i)
            nstr[i] = toupper(nstr[i]);
        return nstr;
    }

    inline string lower(const string& str)
    {
        string nstr = str;
        for (int i = 0; i < (int)nstr.size(); ++i)
            nstr[i] = tolower(nstr[i]);
        return nstr;
    }


    /**
      * @brief return memory used, MB
      */
    inline double get_memory()    
    {
        string fe = string("/proc/") + to_string(getpid()) + "/status";
        OPEN_RFILE(fin, fe);
        string line;
        while (getline(fin, line) && !startswith(line, "VmSize"))
        {}
        if (fin.good())
            return to_int(split(line).at(1)) / 1024.0;
        else
        {
            cerr << "failed to get memory" << endl;
            return 0;
        }
    }

}

namespace rain
{
    template<class Type1, class Type2, class Type3>
    class Triple
    {
    public:
        Triple(const Type1& first = Type1(), const Type2& second = Type2(), const Type3& third = Type3()):
            first(first), second(second), third(third)
        {}

        bool operator < (const Triple& ot) const
        {
            if (first != ot.first)
                return first < ot.first;
            else if (second != ot.second)
                return second < ot.second;
            else 
                return third < ot.third;
        }

        Type1   first;
        Type2   second;
        Type3   third;
    };

    class Format
    {
    public:
        Format(const string& fmt): str(fmt)
        {}

        Format operator () (const string& sub, const string& nstr) const
        {
            return Format(replace(str, sub, nstr));
        }

        Format operator () (const string& sub, int nstr) const
        {
            return Format(replace(str, sub, to_string(nstr)));
        }

        Format operator () (const string& sub, double nstr) const
        {
            return Format(replace(str, sub, to_string(nstr)));
        }

        string operator () () const
        {
            return str;
        }

        friend ostream& operator << (ostream &out, const Format& fmt) 
        {
            return (out << fmt.str);
        }
    protected:    
        string  str;
    };

    class CountTime
    {
    public:
        CountTime(string inf = "counting time"): inf(inf)/*, start(clock())*/
        {
            start = clock();
            cout << "*****---------------------------------------" << '\n';
            cout << Format("(start - {inf})")("{inf}", inf) << endl;
        }

        ~CountTime()
        {
            finish = clock();
            double tsecond = (finish - start) / (double)CLOCKS_PER_SEC;
            int h = (int)(tsecond / (60 * 60));
            int m = (int)((tsecond - h * 60 * 60) / 60);
            double s = tsecond - h * 60 * 60 - m * 60;

            Format fmt("(finish - {inf}): {h} h {m} m {s} s");
            cout << fmt("{inf}", inf)("{h}", h)("{m}", m)("{s}", s) << '\n';
            cout << "----------------------------------------*****" << endl;
        }
    private:
        string  inf;
        time_t  start;
        time_t  finish;
    };

    template<class Type1, class Type2, class TypeValue>
    class Dict
    {
    public:
        typedef             map<Type2, TypeValue>               DictClumn;
        typedef typename    map<Type1, DictClumn>::iterator     Iterator1;
        typedef typename    DictClumn::iterator                 Iterator2;

        void clear()
        {
            dict_mps.clear();
        }

        bool exist(const Type1& w1, const Type2& w2, TypeValue& value)
        {
            Iterator1 ite1 = dict_mps.find(w1);
            Iterator2 ite2;
            return ite1 != dict_mps.end() && (ite2 = ite1->second.find(w2)) != ite1->second.end() ? (value = ite2->second, true) : false;
        }

        TypeValue get(const Type1& w1, const Type2& w2, const TypeValue& df_value)
        {
            TypeValue value;
            return exist(w1, w2, value) ? value : df_value;
        }

        TypeValue& operator () (const Type1& w1, const Type2& w2)
        {
            return dict_mps[w1][w2];
        }

        pair<Iterator1, Iterator1> iterator()
        {
            return make_pair(dict_mps.begin(), dict_mps.end());
        }

        pair<Iterator2, Iterator2> iterator(const Type1& w1)
        {
            if (dict_mps.find(w1) != dict_mps.end())
                return make_pair(dict_mps[w1].begin(), dict_mps[w1].end());
            else
                return make_pair(Iterator2(), Iterator2());
        }

    private:
        map<Type1, DictClumn>  dict_mps;
    };

	class TSpan
	{
	public:
		TSpan():begin(0), end(0)
		{
		}
		TSpan(int _b, int _e):begin(_b), end(_e)
		{}
		~TSpan()
		{}
		 bool operator < (const TSpan& sp) const
		{
			if(begin != sp.begin)
				return begin < sp.begin;
			else
				return end < sp.end;
		}

		bool operator == (const TSpan& sp) const
		{
			return begin == sp.begin && end == sp.end;
		}
		int begin;
		int end;

	};

}

#endif

