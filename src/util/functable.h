#ifndef _BRO_INSTRUMENTATION_FUNCTABLE_H
#define _BRO_INSTRUMENTATION_FUNCTABLE_H

#include "Func.h"

namespace plugin
{
namespace Instrumentation
{

struct FunctionEntry
{
	std::string name;
	std::string file;
	uint32_t    line;
	bool        active;

	FunctionEntry()
	: name(""), file(""), line(0), active(false) { }

	FunctionEntry(const Func* func, const Location* loc)
	{
		name   = std::string(func->Name());
		file   = loc->filename;
		line   = loc->first_line;
		active = true;
	}

	bool operator <(const FunctionEntry& f2)
	{
		if(name < f2.name) {
			return true;
		}
		return file < f2.file;

	}

	bool operator >(const FunctionEntry& f2)
	{
		// Name takes precendence
		if(name > f2.name) {
			return true;
		}

		return file > f2.file;		
	}

	bool operator ==(const FunctionEntry& f2)
	{
		return 
			name == f2.name 
			&& file == f2.file
			&& line == f2.line 
			&& active == f2.active;
	}

	bool operator !=(const FunctionEntry& f2)
	{
		return ! FunctionEntry::operator==(f2);
	}
};

class FunctionTable
{
public:

	uint32_t add(const Func *func, uint32_t offset, const Location *loc)
	{
		uint32_t func_id = func->GetUniqueFuncID();
		if(functions.size() <= func_id)
		{
			functions.resize(func_id + 1);
		}
		if(functions[func_id].size() <= offset)
		{
			functions[func_id].resize(offset + 1);
		}
		if(!functions[func_id][offset].active)
		{
			functions[func_id][offset] = FunctionEntry(func, loc);			
		}

		// compose an ID that's easier to work with ...
		// NOTE: caps max number of functions to 24 bits, and
		// max number of offsets to 8 bits

		return ( (func_id & 0xFFFFFF) << 24) | (offset & 0xFF);
	}

	const FunctionEntry& lookup(const Func* func, uint32_t offset) 
	{ 
		uint32_t func_id = func->GetUniqueFuncID();
		return functions[func_id][offset];
	}

	void reset() 
	{ 
		functions.clear(); 
	}

private:
	std::vector<std::vector<FunctionEntry> > functions;
};

}
}

#endif
