#ifndef _BRO_INSTRUMENTATION_GRAPH_H
#define _BRO_INSTRUMENTATION_GRAPH_H

#include <vector>
#include <unordered_map>
#include <limits.h>

namespace plugin {
namespace Instrumentation {

class CallChain {
public:
	uint64_t count;

	void add(uint32_t item) {
		_entries.push_back(item);
		update();
	}

	size_t size() const {
		return _entries.size();
	}

	void pop() {
		if(!_entries.empty()) {
			_entries.pop_back();			
		}
		update();
	}

	void clear() {
		_entries.clear();
	}

	std::vector<uint32_t> entries() {
		return _entries;
	}

	CallChain()
	: _hash_incr(0) { }

	void update() {
		char *key = (char *)(_entries.data());
		size_t len = _entries.size() * sizeof(uint32_t);

		// http://en.wikipedia.org/wiki/Jenkins_hash_function
		uint32_t hash, i;
	    for(hash = i = 0; i < len; ++i)
	    {
	        hash += key[i];
	        hash += (hash << 10);
	        hash ^= (hash >> 6);
	    }
	    hash += (hash << 3);
	    hash ^= (hash >> 11);
	    hash += (hash << 15);
	    _hash_incr = hash;
	}

	size_t hash() const {
		return _hash_incr;
	}

	bool operator== (const CallChain& c2) const {
		if(this->size() != c2.size()) {
			return false;
		}

		for(int i = 0; i < this->size(); ++i) {
			if(this->_entries[i] != c2._entries[i]) {
				return false;
			}
		}

		return true;
	}

	struct ChainHash {
		size_t operator()(const CallChain& k) const
		{
			return k.hash();
		}
	};

private:
	std::vector<uint32_t> _entries;
	uint32_t _hash_incr;
};

class FunctionCallChain {
public:
	FunctionCallChain()
	: edge(false) { }

	typedef std::unordered_map<CallChain, uint64_t, CallChain::ChainHash> ChainContainer;
	void add(uint32_t dst) {
		active.add(dst);
		edge = true;
	}

	void end() {
		// only a new chain if we've added at least one entry since the last time we ended the chain
		if(edge) {
			ChainContainer::iterator iter = chains.find(active);
			if(iter == chains.end()) {
				chains[active] = 1;
			}
			else {
				iter->second++;
			}
			edge = false;
		}
		active.pop();
	}

	std::vector<CallChain> list() {
		std::vector<CallChain> items;
		for(ChainContainer::iterator iter = chains.begin();
			iter != chains.end(); ++iter) {
			CallChain c = iter->first;
			c.count = iter->second;
			items.push_back(c);
		}
		return items;
	}
private:
	bool edge;	
	CallChain active;
	ChainContainer chains;
};

}
}

#endif
