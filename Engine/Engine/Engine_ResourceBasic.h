#pragma once
#ifndef ENGINE_RESOURCEBASIC_H
#define ENGINE_RESOURCEBASIC_H

#include <string>
#include <vector>
#include <boost/weak_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

class EngineResource{
	private:
		boost::shared_ptr<std::string> m_Name;
	public:
		EngineResource(std::string = "");
		~EngineResource();

		std::string& name();
		boost::shared_ptr<std::string>& namePtr();
		void setName(std::string);
};


struct skey final{
	std::string* s;boost::weak_ptr<std::string> w;bool d;
	bool operator==(const skey& o)const{
		if(d){if(o.d){ return *(s) == *(o.s); } else{ return *(s) == *(o.w.lock().get()); }}
		else{if(o.d){ return *(w.lock().get()) == *(o.s); } else{ return *(w.lock().get()) == *(o.w.lock().get()); }}
		return *(s) == *(o.s);
	}
	bool operator<(const skey& o)const{
		if(d){if(o.d){ return *(s) < *(o.s); } else{ return *(s) < *(o.w.lock().get()); }}
		else{if(o.d){ return *(w.lock().get()) < *(o.s); } else{ return *(w.lock().get()) < *(o.w.lock().get()); }}
		return *(s) < *(o.s);
	}
	skey(std::string _key = ""){ s = new std::string(_key);d = true;}
	skey(EngineResource* o){ s = nullptr; w = boost::dynamic_pointer_cast<std::string>(o->namePtr()); d = false;}
	~skey(){if(d){delete s;d = false;}}
	void lock(boost::shared_ptr<std::string>& p){ delete s;w = boost::dynamic_pointer_cast<std::string>(p); d = false; }
	void lock(EngineResource* o){ delete s;w = boost::dynamic_pointer_cast<std::string>(o->namePtr()); d = false; }
};
struct skh final{
	size_t operator() (skey const& key) const{
		size_t hash = 0;
		if(key.d){for(size_t i = 0; i < (key.s)->size(); i++) hash += (71*hash + (key.s)->at(i)) % 5;}
		else{for(size_t i = 0; i < (key.w.lock().get())->size(); i++) hash += (71*hash + (key.w.lock().get())->at(i)) % 5;}
		return hash;
    }
};
struct skef final{
    bool operator() (skey const& t1,skey const& t2) const{
		if(t1.d){if(t2.d){return !((*t1.s).compare((*t2.s)));}else{return !((*t1.s).compare((*t2.w.lock().get())));}}
		else{if(t2.d){return !((*t1.w.lock().get()).compare((*t2.s)));}else{return !((*t1.w.lock().get()).compare((*t2.w.lock().get())));}}
		return !((*t1.s).compare((*t2.s)));
	}
};

struct sksortlessthan final{ //for sorting skey by its string value (alphabetical order)
    bool operator() (const skey& t1, const skey& t2){
		if(t1.d){if(t2.d){ return *(t1.s) < *(t2.s); }else{ return *(t1.s) < *(t2.w.lock().get()); }}
		else{if(t2.d){ return *(t1.w.lock().get()) < *(t2.s); }else{ return *(t1.w.lock().get()) < *(t2.w.lock().get()); }}
		return *(t1.s) < *(t2.s);
    }
};

#endif