/*******************************************************************************
 * ISPTAP - Instruction Scratchpad Timing Analysis Program
 * Copyright (C) 2013 Stefan Metzlaff, University of Augsburg, Germany
 * URL: <https://github.com/smetzlaff/isptap>
 * 
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program, see <LICENSE>. If not, see
 * <http://www.gnu.org/licenses/>.
 ******************************************************************************/
#ifndef _GRAPHEXPORTER_HPP_
#define _GRAPHEXPORTER_HPP_

#include "global.h"
#include "constants.h"

#include "util/configuration.hpp"
#include "util/graphmlexport.hpp"
#include "util/graphvizexport.hpp"

class GraphExporter {
	public:
		static GraphExporter* getInstance(void);
		void exportGraphsToFile(string name, FunctionCallGraph fcg);
		void exportGraphsToFile(string name, ControlFlowGraph cfg);
		void exportGraphsToFile(string name, MemoryStateGraph msg);

	private:
		GraphExporter();
		virtual ~GraphExporter();
		GraphExporter(const GraphExporter&);                 // Prevent copy-construction
		GraphExporter& operator=(const GraphExporter&);      // Prevent assignment
	
		static GraphExporter *singleton;
		static LoggerPtr logger;
		Configuration *conf;
};

#endif
