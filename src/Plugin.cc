
#include "Plugin.h"
#include "syshooks/syshook-malloc.h"
#include "syshooks/syshook-io.h"

#include "Func.h"
#include "Stats.h"
#include "Stmt.h"

#include <dlfcn.h>
#include <papi.h>
#include <iostream>
#include <fstream>

namespace plugin { namespace Instrumentation { Plugin plugin; } }

using namespace plugin::Instrumentation;

double Plugin::_network_time = 0.0;

static int _papi_event_set = 0;
static double _last_network_update = 0.0;
static double _stats_timer = 0.0;
static uint64_t _last_count = 0;
static uint64_t _stats_count = 0;
static std::string _stats_target = "";
static std::ofstream _stats_ofstream;

static PCM *_pcm_state;
static SystemCounterState _original_state;

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
	++_last_count;

	// trigger on number of packets
	if(_stats_count > 0 && _last_count >= _stats_count)
		{
			_last_count = 0;
			WriteCollection();
		}
	// trigger on network timestamps
	else if(_stats_timer > 0.001 && network_time - _last_network_update > _stats_timer) 
		{
			_last_network_update = network_time;
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

    /*
    SegmentProfiler(segment_logger, func->GetLocationInfo());

    if ( sample_logger )
        sample_logger->FunctionSeen(func);
	*/

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

    /*
    if ( g_trace_state.DoTrace() )
        {
        ODesc d;
        func->DescribeDebug(&d, args);

        g_trace_state.LogTrace("%s called: %s\n",
            func->FType()->FlavorString().c_str(), d.Description());
        }
    */
    loop_over_list(*args, i)
        f->SetElement(i, (*args)[i]);

    stmt_flow_type flow = FLOW_NEXT;

    Val* result = 0;

    for ( size_t i = 0; i < bodies.size(); ++i )
        {
        /*
        if ( sample_logger )
            sample_logger->LocationSeen(
                bodies[i].stmts->GetLocationInfo());
		*/

        Unref(result);

        try
            {
            result = bodies[i].stmts->Exec(f, flow);
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
    /*
    if(result) {
	    ODesc d;
	    result->Describe(&d);
	    printf("Method %s returned %p: %s\n", func->Name(), result, d.Bytes());    	
    }
    else {
    	printf("Method %s returned NULL\n", func->Name());
    }
    */
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

	_pcm_state = PCM::getInstance();
	_pcm_state->program (PCM::DEFAULT_EVENTS, NULL);

	if (_pcm_state->program() != PCM::Success)
		{
		reporter->Info("[instrumentation] unable to initialize PCM ...");
		}

	_original_state = getSystemCounterState();

	reporter->Info("[instrumentation] initialization completed.\n");
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
	// set up our stream ...
	_stats_ofstream.setf(ios::fixed, ios::floatfield);
	_stats_ofstream.setf(ios::showpoint);
	_stats_ofstream.precision(6);
	// write CSV headers
	_stats_ofstream << "#fields"
					<< " network_time malloc_count free_count malloc_sz fopen_count" 
	                << " open_count fwrite_count write_count fwrite_sz write_sz fread_count" 
	                << " read_count fread_sz read_sz"
	                << " instructions_retired l2_hit l2_miss l3_hit l3_miss mem_read mem_write"
	                << std::endl;
	_stats_ofstream << "#types" 
					<< " double int int int int"
					<< " int int int int int int"
					<< " int int int int" 
					<< " int int int int int int int"
					<< std::endl;
	_stats_ofstream << "#separator \\x20" << std::endl;
	}

void Plugin::WriteCollection()
	{
	assert(_stats_ofstream.good());
	MemoryInfo info = GetMemoryCounts();
	ReadWriteInfo rwinfo = GetReadWriteCounts();
		// method call to be profiled goes here!
	SystemCounterState curr_state = getSystemCounterState();

	_stats_ofstream << _network_time << " " << info.malloc_count << " " << info.free_count << " ";
	_stats_ofstream << info.malloc_sz << " " << rwinfo.fopen_count << " " << rwinfo.open_count << " ";
	_stats_ofstream << rwinfo.fwrite_count << " " << rwinfo.write_count << " " << rwinfo.fwrite_sz << " ";
	_stats_ofstream << rwinfo.write_sz << " " << rwinfo.fread_count << " " << rwinfo.read_count << " ";
	_stats_ofstream << rwinfo.fread_sz << " " << rwinfo.read_sz;
	_stats_ofstream << getInstructionsRetired(_original_state, curr_state) << " " 
	                << getL2CacheHits(_original_state, curr_state) << " "
	                << getL2CacheMisses(_original_state, curr_state) << " "
	                << getL3CacheHits(_original_state, curr_state) << " "
	                << getL3CacheMisses(_original_state, curr_state) << " "
	                << getBytesReadFromMC(_original_state, curr_state) << " "
	                << getBytesWrittenToMC(_original_state, curr_state) << "\n";

	}

void Plugin::FlushCollection()
	{
	assert(_stats_ofstream.good());
	_stats_ofstream.flush();
	}
