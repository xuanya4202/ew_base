#include "ewa_base/util/regex.h"
#include "regex_parser.h"
#include "regex_impl.h"

EW_ENTER

Regex::Regex()
{

}

Regex::Regex(const Regex& r):pimpl(r.pimpl)
{

}

Regex::Regex(const String& s,int f)
{
	assign(s,f);
}


bool Regex::assign(const String& s,int f)
{
	RegexParser parser(f);
	pimpl.reset(parser.parse(s));
	return pimpl!=NULL;
}

bool Regex::match(const String& s)
{
	return execute(s,NULL,true);
}


bool Regex::execute(const String& s,Match* pres,bool match_mode)
{

	if(!pimpl) return false;
	regex_item_root* seq=static_cast<regex_item_root*>(pimpl.get());

	if(pres)
	{
		Match& res(*pres);
		regex_impl<regex_policy_char> impl;

		res.matchs.clear();
		res.orig_str=s;
		res.orig_reg=*this;

		const char* q1=res.orig_str.c_str();
		const char* q2=q1+res.orig_str.length();

		bool flag=impl.execute(seq,q1,q2,match_mode);

		if(!flag) return false;

		impl.update_match_results(res);

		return true;	
	}
	else
	{
		regex_impl<regex_policy_char_match_only> impl;
		const char* q1=s.c_str();
		const char* q2=q1+s.length();

		bool flag=impl.execute(seq,q1,q2,match_mode);
		return flag;
	}

}

bool Regex::match(const String& s,Match& m)
{
	return execute(s,&m,true);
}

bool Regex::search(const String& s,Match& m)
{
	return execute(s,&m,false);
}



bool regex_match(const String& s,Regex& re)
{
	return re.execute(s,NULL,true);
}

bool regex_match(const String& s,Match& res,Regex& re)
{
	return re.execute(s,&res,true);
}

bool regex_search(const String& s,Match& res,Regex& re)
{
	return re.execute(s,&res,false);
}

String regex_replace(const String& s,Regex& re,const String& p)
{
	Match res;
	re.match(s,res);
	return res.replace(p);
}



bool Match::search_next()
{
	if(matchs.empty()||!orig_reg.pimpl)
	{
		return false;
	}

	const char* q1=orig_str.c_str();
	const char* q2=q1+orig_str.length();

	const char* p1=matchs[0][0].it_end;

	matchs.clear();

	if(p1<q1||p1>=q2)
	{
		return false;
	}

	regex_impl<regex_policy_char> impl;

	regex_item_root* seq=static_cast<regex_item_root*>(orig_reg.pimpl.get());
	bool flag=impl.execute(seq,p1,q2,false);

	if(!flag) return false;

	impl.update_match_results(*this);

	return true;

}

String Match::replace(const String& dst) const
{
	StringBuffer<char> sb;
	const char* p1=dst.c_str();
	while(*p1)
	{
		if(*p1!='$' )
		{
			sb.push_back(*p1++);
			continue;
		}
		if(p1[1]>='0'&&p1[1]<='9')
		{
			int n=p1[1]-'0';
			if(n<(int)matchs.size())
			{
				sb<<matchs[n];
			}
			p1+=2;
		}
		else
		{
			sb.push_back(p1[1]);	
			p1+=2;
		}
	}
	return sb;
}

Match::item_array::operator String() const 
{
	if(empty()) return "";
	const_iterator it=cbegin();
	String result(*it++);
	while(it!=cend()) result+="\t"+*it++;
	return result;
}


bool RegexEx::prepare(const String& name,const String& expr,int f)
{
	RegexParser parser(f);
	DataPtrT<ObjectData>& ptr(named_regex[name]);
	ptr.reset(parser.parse(expr));
	return ptr;
}


bool RegexEx::execute(const String& s,Match* pres,bool match_mode)
{
	if(!pimpl) return false;
	regex_item_root* seq=static_cast<regex_item_root*>(pimpl.get());
	
	regex_impl<regex_policy_char_recursive> impl;
	for(bst_map<String,DataPtrT<ObjectData> >::iterator it=named_regex.begin();it!=named_regex.end();++it)
	{
		impl.item_map.insert(std::make_pair((*it).first,static_cast<regex_item_root*>((*it).second.get())));;
	}
	
	if(pres)
	{
		Match& res(*pres);

		res.matchs.clear();
		res.orig_str=s;
		res.orig_reg=*this;

		const char* q1=res.orig_str.c_str();
		const char* q2=q1+res.orig_str.length();

		bool flag=impl.execute(seq,q1,q2,match_mode);

		if(!flag) return false;

		impl.update_match_results(res);
		return true;	
	}
	else
	{
		const char* q1=s.c_str();
		const char* q2=q1+s.length();

		bool flag=impl.execute(seq,q1,q2,match_mode);

		return flag;
	}

}



EW_LEAVE


