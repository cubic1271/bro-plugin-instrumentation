#ifndef _BRO_INSTRUMENTATION_FUNCTION_H
#define _BRO_INSTRUMENTATION_FUNCTION_H

#include <string>
#include <iostream>
#include <fstream>

#include "syshooks/syshook-io.h"
#include "syshooks/syshook-malloc.h"

namespace plugin
{
namespace Instrumentation
{
	struct CounterSet {
		uint64_t cycles;
		void Read();
		CounterSet operator-(const CounterSet& c2);
		CounterSet& operator-= (const CounterSet& c2);
		CounterSet operator+(const CounterSet& c2);
		CounterSet& operator+= (const CounterSet& c2);

		CounterSet()
		: cycles(0) { }
	};

	struct FunctionCounterSet {
		double network_time;
		std::string name;
		std::string location;
		uint64_t count;
		MemoryInfo memory;
		ReadWriteInfo io;
		CounterSet perf;

		FunctionCounterSet(const double network_time, MemoryInfo memory, ReadWriteInfo io)
		: network_time(network_time), name("-"), location("-"), count(0), memory(memory), io(io) { perf.Read(); }

		FunctionCounterSet()
		: network_time(0.0), name("-"), location("-"), count(0)
		{ }

		static FunctionCounterSet Create(double network_time);
		static void ConfigWriter(std::ofstream& target);
		void Write(std::ofstream& target);
		FunctionCounterSet operator -(const FunctionCounterSet& s2)
			{
			FunctionCounterSet tmp;
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
			FunctionCounterSet tmp;
			tmp.network_time = this->network_time + s2.network_time;
			tmp.name = this->name;
			tmp.location = this->location;
			tmp.memory = this->memory + s2.memory;
			tmp.io = this->io + s2.io;
			tmp.count = this->count + s2.count;
			tmp.perf = this->perf + s2.perf;
			return tmp;
			}

		FunctionCounterSet& operator +=(const FunctionCounterSet& s2)
			{
			this->network_time += s2.network_time;
			this->name = s2.name;
			this->location = s2.location;
			this->memory += s2.memory;
			this->io += s2.io;
			this->count += s2.count;
			this->perf += s2.perf;
			return *this;
			}

		FunctionCounterSet& operator -=(const FunctionCounterSet& s2)
			{
			this->network_time -= s2.network_time;
			this->name = s2.name;
			this->location = s2.location;
			this->memory -= s2.memory;
			this->io -= s2.io;
			this->count -= s2.count;
			this->perf -= s2.perf;
			return *this;
			}
	};
}
}
#endif