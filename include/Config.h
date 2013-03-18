#ifndef CONFIG_H
#define CONFIG_H

#include "rain_algorithm.h"
using namespace rain;

class Config
{
public:
    explicit Config(string config)
    {
        ifstream fin(config.c_str());
        assert(fin.good() || !(cerr << "can't open " << config << endl));
        string line;
        string key;
        string value;

        while (getline(fin, line))
        {
            uncomment(line);
            if (extract_key_value(line, key, value))
                m_value[key] = value;
        }
    }

    void get_value(const string &token, string &value)
    {
        map<string, string>::iterator ite;
        if ((ite = m_value.find(token)) == m_value.end())
		{
            cout << "error in get_value:" << token << endl;
			value = "";
		}
        else
            value = ite->second;
    }

    void get_value(const string &token, double &value)
    {
        string v;
        get_value(token, v);
        value = to_double(v);
    }

    void get_value(const string &token, int &value)
    {
        string v;
        get_value(token, v);
        value = to_int(v);
    }


private:
    void uncomment(string &line)
    {
        int p = line.rfind("//");
        if (p != (int)string::npos)
            line = line.substr(0, p);
    }

    bool extract_key_value(const string &line, string &key, string &value)
    {
        int p = line.find("=");
        if (p == (int)string::npos)
            return false;
        else
        {
            key = line.substr(0, p);
            value = line.substr(p + 1);
            key = strip(key, "\t ");
            value = strip(value, "\t ");
            return true;
        }
    }

protected:
    map<string, string>     m_value;
};

#endif

