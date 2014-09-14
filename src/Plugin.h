
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
#include "util/counters.h"

namespace plugin {
namespace Instrumentation {

class Plugin : public ::plugin::Plugin
{
public:
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

	static void SetChainDataTarget(const std::string target);
	static void WriteChainData();

protected:
	virtual plugin::Configuration Configure();
	static Val* CallBroFunction(const BroFunc* func, Frame *parent, val_list* args);
	static Val* CallBuiltinFunction(const BuiltinFunc* func, Frame *parent, val_list* args);
};

extern Plugin plugin;

}
}

#endif
