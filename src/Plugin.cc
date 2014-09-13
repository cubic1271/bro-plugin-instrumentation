
#include "Plugin.h"

#include "Func.h"
#include "Stats.h"
#include "Stmt.h"

#include <dlfcn.h>
#include <iostream>
#include <fstream>

namespace plugin { namespace Instrumentation { Plugin plugin; } }

using namespace plugin::Instrumentation;

// A few counters to handle updates for various counters / time-based statistics
static double _last_stats_update = 0.0;
static double _stats_timer = 0.0;
static uint64_t _last_stats_count = 0;
static uint64_t _stats_count = 0;

static std::string _stats_target = "";
static std::ofstream _stats_ofstream;

static std::string _fdata_target = "";
static std::ofstream _fdata_ofstream;

static Plugin::CounterSet _original_state;

static double _network_time;
// transient state needed to keep track of counter start points while functions are executing
static std::vector<Plugin::FunctionCounterSet> _counter_stack;
// persistent counter state
static std::map<std::string, Plugin::FunctionCounterSet> _counters;
typedef std::map<std::string, Plugin::FunctionCounterSet>::iterator _counter_iterator;

plugin::Configuration Plugin::Configure()
	{
	plugin::Configuration config;
	config.name = "Instrumentation::Instrumentation";
	config.description = "Plugin that adds low-level instrumentation to a running bro process.";
	config.version.major = 1;
	config.version.minor = 0;
	return config;
	}

void Plugin::HookUpdateNetworkTime(const double network_time)
	{	
	_network_time = network_time;
	++_last_stats_count;

	// trigger on number of packets
	if(_stats_count > 0 && _last_stats_count >= _stats_count)
		{
			_last_stats_count = 0;
			WriteCollection();
		}
	// trigger on network timestamps
	else if(_stats_timer > 0.001 && network_time - _last_stats_update > _stats_timer) 
		{
			_last_stats_update = network_time;
			WriteCollection();
		}
	}

Val* Plugin::CallBroFunction(const BroFunc *func, Frame *parent, val_list *args)
	{
#ifdef PROFILE_BRO_FUNCTIONS
    DEBUG_MSG("Function: %s\n", id->Name());
#endif

    // printf("Executing bro method: %s\n", func->Name());
    std::vector<Func::Body> bodies = func->GetBodies();

    if ( bodies.empty() )
        {
        // Can only happen for events and hooks.
        assert(func->Flavor() == FUNC_FLAVOR_EVENT || func->Flavor() == FUNC_FLAVOR_HOOK);
        loop_over_list(*args, i)
            Unref((*args)[i]);

        return func->Flavor() == FUNC_FLAVOR_HOOK ? new Val(true, TYPE_BOOL) : 0;
        }

    Frame* f = new Frame(func->FrameSize(), func, args);

    // Hand down any trigger.
    if ( parent )
        {
        f->SetTrigger(parent->GetTrigger());
        f->SetCall(parent->GetCall());
        }

    g_frame_stack.push_back(f); // used for backtracing

    loop_over_list(*args, i)
        f->SetElement(i, (*args)[i]);

    stmt_flow_type flow = FLOW_NEXT;

    Val* result = 0;

    for ( size_t i = 0; i < bodies.size(); ++i )
        {

        Unref(result);

        try
            {
            _counter_stack.push_back(Plugin::FunctionCounterSet::Create());
            result = bodies[i].stmts->Exec(f, flow);
            FunctionCounterSet result = Plugin::FunctionCounterSet::Create() - _counter_stack.back();
            _counter_stack.pop_back();
            const Location* loc = bodies[i].stmts->GetLocationInfo();
            char sbuf[4096];
            snprintf(sbuf, 4096, "%s@%s:%d", func->Name(), loc->filename, loc->first_line);
            if(_counters.find(sbuf) != _counters.end())
	            {
	            _counters[sbuf] = (_counters[sbuf] + result);
    	        }
    	    else
    		    {
    		    _counters[sbuf] = result;
	    	    }
            }

        catch ( InterpreterException& e )
            {
            // Already reported, but we continue exec'ing remaining bodies.
            continue;
            }

        if ( f->HasDelayed() )
            {
            assert(! result);
            assert(parent);
            parent->SetDelayed();
            break;
            }

        if ( func->Flavor() == FUNC_FLAVOR_HOOK )
            {
            // Ignore any return values of hook bodies, final return value
            // depends on whether a body returns as a result of break statement.
            Unref(result);
            result = 0;

            if ( flow == FLOW_BREAK )
                {
                // Short-circuit execution of remaining hook handler bodies.
                result = new Val(false, TYPE_BOOL);
                break;
                }
         	}
        }

    if ( func->Flavor() == FUNC_FLAVOR_HOOK )
        {
        if ( ! result )
            result = new Val(true, TYPE_BOOL);
        }

    // Warn if the function returns something, but we returned from
    // the function without an explicit return, or without a value.
    else if ( func->FType()->YieldType() && func->FType()->YieldType()->Tag() != TYPE_VOID &&
         (flow != FLOW_RETURN /* we fell off the end */ ||
          ! result /* explicit return with no result */) &&
         ! f->HasDelayed() )
        reporter->Warning("non-void function returns without a value: %s",
                          func->Name());

    g_frame_stack.pop_back();
    Unref(f);

    // hack: since the plugin architecture can't distinguish between a NULL returned by our method
    // and a NULL returned by a function, we rely on the plugin result handler to fix things for us.
    if(NULL == result) {
    	return new Val(true, TYPE_BOOL);
    }
    return result;
	}

Val* Plugin::CallBuiltinFunction(const BuiltinFunc *func, Frame *parent, val_list *args)
	{
    Val* result = func->TheFunc()(parent, args);
    // hack: since the plugin architecture can't distinguish between a NULL returned by our method
    // and a NULL returned by a function, we rely on the plugin result handler to fix things for us.
    if(NULL == result) {
    	return new Val(true, TYPE_BOOL);
    }
    return result;
	}

Val* Plugin::HookCallFunction(const Func* func, Frame *parent, val_list* args)
	{
	if ( func->GetKind() == Func::BRO_FUNC) 
		{
		return CallBroFunction((BroFunc *)func, parent, args);
		} // end standard bro function call
	else if (func->GetKind() == Func::BUILTIN_FUNC)
		{
		return CallBuiltinFunction((BuiltinFunc *)func, parent, args);
		}
	else
		{
		reporter->Warning("[instrumentation] unable to detect function call type.  dropping through to default handler.");
		return NULL;
		}

	}

void Plugin::InitPreScript()
	{
	reporter->Info("[instrumentation] Initializing instrumentation plugin...\n");
	plugin::Plugin::InitPreScript();

	EnableHook(HOOK_UPDATE_NETWORK_TIME);
	EnableHook(HOOK_CALL_FUNCTION);

	reporter->Info("[instrumentation] initialization completed.\n");
	_original_state.Read();

	}

void Plugin::SetCollectionTimer(const double target) 
	{
	_stats_timer = target;
	}

void Plugin::SetCollectionCount(const uint64_t target)
	{
	_stats_count = target;
	}

void Plugin::SetCollectionTarget(const std::string target)
	{
	_stats_target = target;
	_stats_ofstream.open(_stats_target);
	Plugin::FunctionCounterSet::ConfigWriter(_stats_ofstream);
	}

void Plugin::SetFunctionDataTarget(const std::string target)
	{
	_fdata_target = target;
	_fdata_ofstream.open(_fdata_target);
	Plugin::FunctionCounterSet::ConfigWriter(_fdata_ofstream);
	}

void Plugin::WriteFunctionData()
	{

	}

void Plugin::WriteCollection()
	{
	assert(_stats_ofstream.good());
	Plugin::FunctionCounterSet set = Plugin::FunctionCounterSet::Create();
	set.Write(_stats_ofstream);
	}

void Plugin::FlushCollection()
	{
	assert(_stats_ofstream.good());
	_stats_ofstream.flush();
	}

Plugin::FunctionCounterSet Plugin::FunctionCounterSet::Create()
	{
	FunctionCounterSet set;
	set.memory = GetMemoryCounts();
	set.io = GetReadWriteCounts();
	set.network_time = _network_time;
	set.perf.Read();
	return set;
	}

void Plugin::FunctionCounterSet::ConfigWriter(ofstream& target)
	{
	target.setf(ios::fixed, ios::floatfield);
	target.setf(ios::showpoint);
	target.precision(6);

	target << "#fields"
		   << " network_time name location count"
		   << " malloc_count free_count malloc_sz fopen_count" 
	       << " open_count fwrite_count write_count fwrite_sz write_sz fread_count" 
	       << " read_count fread_sz read_sz"
	       << " cycles"
	       << std::endl;

	target << "#types" 
		   << " double \"string\" \"string\" int"
		   << " int int int int"
		   << " int int int int int int"
		   << " int int int int" 
		   << " int"
		   << std::endl;
	}

void Plugin::FunctionCounterSet::Write(ofstream& target)
	{
	target << network_time << " \"" << name << "\" \"" << location << "\" " << count; 
	target << memory.malloc_count << " " << memory.free_count << " ";
	target << memory.malloc_sz << " " << io.fopen_count << " " << io.open_count << " ";
	target << io.fwrite_count << " " << io.write_count << " " << io.fwrite_sz << " ";
	target << io.write_sz << " " << io.fread_count << " " << io.read_count << " ";
	target << io.fread_sz << " " << io.read_sz << " ";
	Plugin::CounterSet tmp = perf - _original_state;
	target << perf.cycles << "\n";
	}

void Plugin::CounterSet::Read()
	{
	#if defined(__amd64__) || defined(__i686__)
        uint32 high = 0, low = 0;
        uint32 cpu1, cpu2, cpu3, cpu4;
        // serializing instruction
        // all registers may be modified.  load 0 into EAX to fetch vendor string.
        asm volatile("cpuid" : /*no output*/ : "a"(0) : "eax", "ebx", "ecx", "edx");
        // read cycle count
        asm volatile("rdtsc" : "=a" (low), "=d" (high));
        this->cycles = ((uint64_t(high) << uint64_t(32)) | low);
    #else
		#warning "[instrumentation] Unsupported platform: cycles will always be 0!"
        this->cycles = 0;
	#endif
	}

Plugin::CounterSet Plugin::CounterSet::operator+ (const CounterSet& c2)
{
	Plugin::CounterSet tmp;
	tmp.cycles = this->cycles + c2.cycles;
	return tmp;
}

Plugin::CounterSet Plugin::CounterSet::operator- (const CounterSet& c2)
{
	Plugin::CounterSet tmp;
	tmp.cycles = this->cycles - c2.cycles;
	return tmp;
}
