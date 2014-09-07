
#ifndef BRO_PLUGIN_INSTRUMENTATION
#define BRO_PLUGIN_INSTRUMENTATION

#include <map>
#include <plugin/Plugin.h>

#include "pcm/cpucounters.h"
#include "Func.h"

namespace plugin {
namespace Instrumentation {

class Plugin : public ::plugin::Plugin
{
public:
	struct FunctionCounterSet {
		uint64_t malloc_count;
		uint64_t free_count;
		uint64_t malloc_sz;
	};

    virtual void HookUpdateNetworkTime(const double network_time);
    virtual void InitPreScript();
    virtual Val* HookCallFunction(const Func* func, Frame *parent, val_list* args);

	static void SetCollectionTimer(const double target);
	static void SetCollectionCount(const uint64_t target);
	static void SetCollectionTarget(const std::string target);
	static void WriteCollection();
	static void FlushCollection();

protected:
	static double _network_time;
	// transient state needed to keep track of counter start points while functions are executing
	static std::vector<FunctionCounterSet> _counter_stack;
	// persistent counter state
	static std::map<std::string, FunctionCounterSet> _counters;
	virtual plugin::Configuration Configure();
	static Val* CallBroFunction(const BroFunc* func, Frame *parent, val_list* args);
	static Val* CallBuiltinFunction(const BuiltinFunc* func, Frame *parent, val_list* args);
};

extern Plugin plugin;

}
}

#endif
