/*
    File                 : trace.h
    Project              : LabPlot
    Description          : Function and macros related to performance and debugging tracing
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2017 Alexander Semke <alexander.semke@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef TRACE_H
#define TRACE_H

#include "backend/lib/macros.h"
#include <chrono>

class PerfTracer {
public:
	explicit PerfTracer(const char* m) {
		msg = m;
		start = std::chrono::high_resolution_clock::now();
	};
	~PerfTracer() {
		auto end = std::chrono::high_resolution_clock::now();
		auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
		std::cout << msg << ": " << diff << " ms" << std::endl;
	}

private:
	std::chrono::high_resolution_clock::time_point start;
	std::string msg;
};

#define PERFTRACE_ENABLED 1

#define PERFTRACE_CURVES 1
#define PERFTRACE_LIVE_IMPORT 1

#ifdef PERFTRACE_ENABLED
#define PERFTRACE(msg) PerfTracer tracer(msg)
#else
#define PERFTRACE(msg) DEBUG(msg)
#endif


#ifndef HAVE_WINDOWS

#include <execinfo.h> //backtrace
#include <dlfcn.h>    //dladdr
#include <cxxabi.h>   //__cxa_demangle

#include <cstdio>
#include <cstdlib>
#include <string>
#include <sstream>

/*!
 * this function prints the current call stack and helps to figure out why a certain (e.g. performance critical) function
 * is called multiple times and from where without involving the debugger.
 * To get the callstack, simple include \c print_callstack() in the function of interest.
 */
static inline void print_callstack() {
	//get the current call stack
	const int max_frames_count = 10 + 1; //print the last 10 frames (+1 because of this function frame)
	void* callstack[max_frames_count];
	const int frames_count = backtrace(callstack, max_frames_count);

	//get the symbols
	char **symbols = backtrace_symbols(callstack, frames_count);

	std::ostringstream out;
	char buf[1024];

	// iterate over the frames, skip the first one (frame of this function call)
	for (int i = 1; i < frames_count; i++) {
		Dl_info info;
		if (dladdr(callstack[i], &info) && info.dli_sname) {
			char* demangled_name = nullptr;
			const char* name_to_print = nullptr;
			int status = -1;
			if (info.dli_sname[0] == '_')
				demangled_name = abi::__cxa_demangle(info.dli_sname, nullptr, nullptr, &status);

			if (status == 0)
				name_to_print = demangled_name;
			else {
				if (info.dli_sname == nullptr)
					name_to_print = symbols[i];
				else
					name_to_print = info.dli_sname;
			}

			snprintf(buf, sizeof(buf), "%-3d %*p %s + %zd\n",
					 i,
					 int(2 + sizeof(void*) * 2),
					 callstack[i],
					 name_to_print,
					 (char*)callstack[i] - (char*)info.dli_saddr);

			free(demangled_name);
		} else {
			snprintf(buf, sizeof(buf), "%-3d %*p %s\n",
						i, int(2 + sizeof(void*) * 2), callstack[i], symbols[i]);
		}
		out << buf;
	}
	free(symbols);

	std::cout << "stack trace:\n" <<  out.str();
}
#endif //  #ifndef HAVE_WINDOWS

#endif //TRACE_H
