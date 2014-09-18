#include "counters.h"

namespace plugin
{
namespace Instrumentation
{
	using std::ios;

	FunctionCounterSet FunctionCounterSet::Create(double network_time)
		{
		FunctionCounterSet set(network_time, GetMemoryCounts(), GetReadWriteCounts());
		return set;
		}

	void FunctionCounterSet::ConfigWriter(std::ofstream& target)
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

	void FunctionCounterSet::Write(std::ofstream& target)
		{
		target << network_time << " \"" << name << "\" \"" << location << "\" " << count << " "; 
		target << memory.malloc_count << " " << memory.free_count << " ";
		target << memory.malloc_sz << " " << io.fopen_count << " " << io.open_count << " ";
		target << io.fwrite_count << " " << io.write_count << " " << io.fwrite_sz << " ";
		target << io.write_sz << " " << io.fread_count << " " << io.read_count << " ";
		target << io.fread_sz << " " << io.read_sz << " ";
		target << perf.cycles << "\n";
		}

	void CounterSet::Read()
		{
		#if defined(__amd64__) || defined(__i686__)
	        uint32_t high = 0, low = 0;
	        uint32_t cpu1, cpu2, cpu3, cpu4;
	        // serializing instruction
	        // all registers may be modified.  load 0 into EAX to fetch vendor string.
	        // asm volatile("cpuid" : /*no output*/ : "a"(0) : "eax", "ebx", "ecx", "edx");
	        // read cycle count
	        asm volatile("rdtsc" : "=a" (low), "=d" (high));
	        this->cycles = ((uint64_t(high) << uint64_t(32)) | low);
	    #else
			#warning "[instrumentation] Unsupported platform: cycles will always be 0!"
	        this->cycles = 0;
		#endif
		}

	CounterSet CounterSet::operator+ (const CounterSet& c2)
	{
		CounterSet tmp;
		tmp.cycles = this->cycles + c2.cycles;
		return tmp;
	}

	CounterSet CounterSet::operator- (const CounterSet& c2)
	{
		CounterSet tmp;
		tmp.cycles = (this->cycles > c2.cycles) ? this->cycles - c2.cycles : 0;
		return tmp;
	}

	CounterSet& CounterSet::operator+= (const CounterSet& c2)
	{
		this->cycles = this->cycles + c2.cycles;
		return *this;
	}

	CounterSet& CounterSet::operator-= (const CounterSet& c2)
	{
		this->cycles = (this->cycles > c2.cycles) ? this->cycles - c2.cycles : 0;
		return *this;
	}
}
}
