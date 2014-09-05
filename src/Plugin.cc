
#include "Plugin.h"
#include "syshooks/syshook-malloc.h"
#include "syshooks/syshook-io.h"

#include <dlfcn.h>
#include <papi.h>
#include <iostream>
#include <fstream>

namespace plugin { namespace Instrumentation { Plugin plugin; } }

using namespace plugin::Instrumentation;

double Plugin::_network_time = 0.0;

plugin::Configuration Plugin::Configure()
	{
	plugin::Configuration config;
	config.name = "Instrumentation::Instrumentation";
	config.description = "Plugin that adds low-level instrumentation to a running bro process.";
	config.version.major = 1;
	config.version.minor = 0;
	return config;
	}

static int _papi_event_set = 0;
static double _last_network_update = 0.0;
static double _stats_timer = 0.0;
static uint64_t _last_count = 0;
static uint64_t _stats_count = 0;
static std::string _stats_target = "";
static std::ofstream _stats_ofstream;

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

Val* Plugin::HookCallFunction(const Func* func, Frame *parent, val_list* args)
	{
		return NULL;
	}

void Plugin::InitPreScript()
	{
	printf("[INFO] Initializing instrumentation plugin...\n");
	plugin::Plugin::InitPreScript();

	EnableHook(HOOK_UPDATE_NETWORK_TIME);
	EnableHook(HOOK_CALL_FUNCTION);

	printf("[INFO] Instrumentation initialization completed.\n");
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
	                << std::endl;
	_stats_ofstream << "#types" 
					<< " double int int int int"
					<< " int int int int int int"
					<< " int int int" 
					<< std::endl;
	_stats_ofstream << "#separator \\x20" << std::endl;
	}

void Plugin::WriteCollection()
	{
	assert(_stats_ofstream.good());
	MemoryInfo info = GetMemoryCounts();
	ReadWriteInfo rwinfo = GetReadWriteCounts();
	_stats_ofstream << _network_time << " " << info.malloc_count << " " << info.free_count << " ";
	_stats_ofstream << info.malloc_sz << " " << rwinfo.fopen_count << " " << rwinfo.open_count << " ";
	_stats_ofstream << rwinfo.fwrite_count << " " << rwinfo.write_count << " " << rwinfo.fwrite_sz << " ";
	_stats_ofstream << rwinfo.write_sz << " " << rwinfo.fread_count << " " << rwinfo.read_count << " ";
	_stats_ofstream << rwinfo.fread_sz << " " << rwinfo.read_sz << "\n"; 
	}

void Plugin::FlushCollection()
	{
	assert(_stats_ofstream.good());
	_stats_ofstream.flush();
	}
