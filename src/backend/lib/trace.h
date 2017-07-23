/***************************************************************************
    File                 : trace.h
    Project              : LabPlot
    Description          : Function and macros related to performance and debugging tracing
    --------------------------------------------------------------------
    Copyright            : (C) 2017 Alexander Semke (alexander.semke@web.de)

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                           *
 *                                                                         *
 ***************************************************************************/

#include "backend/lib/macros.h"
#include <chrono>

class PerfTracer {
	public:
		PerfTracer(const char* m) {
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

#ifdef PERFTRACE_ENABLED
#define PERFTRACE(msg) PerfTracer tracer(msg)
#else
#define PERFTRACE(msg) DEBUG(msg)
#endif
