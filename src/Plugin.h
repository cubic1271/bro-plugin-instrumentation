
#ifndef BRO_PLUGIN_INSTRUMENTATION
#define BRO_PLUGIN_INSTRUMENTATION

#include "syshooks/syshook-malloc.h"
#include "syshooks/syshook-io.h"

#include <map>
#include <plugin/Plugin.h>

#include "Func.h"

#include <signal.h>
#include <stdio.h>
#include <inttypes.h>
#include <mach/mach_time.h>

namespace plugin {
namespace Instrumentation {

class Plugin : public ::plugin::Plugin
{
public:
	struct CounterSet {
		uint64_t cycles;
		void Read();
		CounterSet operator-(const CounterSet& c2);
		CounterSet operator+(const CounterSet& c2);
	};

	struct FunctionCounterSet {
		double network_time;
		std::string name;
		std::string location;
		uint64_t count;
		MemoryInfo memory;
		ReadWriteInfo io;
		CounterSet perf;

		FunctionCounterSet()
		: network_time(0.0), name("-"), location("-"), count(0)
		{ }

		static FunctionCounterSet Create();
		static void ConfigWriter(ofstream& target);
		void Write(ofstream& target);
		FunctionCounterSet operator -(const FunctionCounterSet& s2)
			{
			Plugin::FunctionCounterSet tmp;
			tmp.network_time = this->network_time - s2.network_time;
			tmp.name = this->name;
			tmp.location = this->location;
			tmp.memory = this->memory - s2.memory;
			tmp.io = this->io - s2.io;
			tmp.count = this->count - s2.count;
			tmp.perf = this->perf - s2.perf;
			return tmp;
			}

		FunctionCounterSet operator +(const FunctionCounterSet& s2)
			{
			Plugin::FunctionCounterSet tmp;
			tmp.network_time = this->network_time + s2.network_time;
			tmp.name = this->name;
			tmp.location = this->location;
			tmp.memory = this->memory + s2.memory;
			tmp.io = this->io + s2.io;
			tmp.count = this->count + s2.count;
			tmp.perf = this->perf + s2.perf;
			return tmp;
			}

	};

    virtual void HookUpdateNetworkTime(const double network_time);
    virtual void InitPreScript();
    virtual Val* HookCallFunction(const Func* func, Frame *parent, val_list* args);

	static void SetCollectionTimer(const double target);
	static void SetCollectionCount(const uint64_t target);
	static void SetCollectionTarget(const std::string target);
	static void WriteCollection();
	static void FlushCollection();

	static void SetFunctionDataTarget(const std::string target);
	static void WriteFunctionData();
	static void FlushFunctionData();


protected:
	virtual plugin::Configuration Configure();
	static Val* CallBroFunction(const BroFunc* func, Frame *parent, val_list* args);
	static Val* CallBuiltinFunction(const BuiltinFunc* func, Frame *parent, val_list* args);
};

extern Plugin plugin;

}
}

#endif
