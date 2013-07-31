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
#ifndef _GRAPHMLEXPORT_HPP_
#define _GRAPHMLEXPORT_HPP_

#include "global.h"
#include "graph_structure.h"

#include <boost/graph/graphml.hpp>

/**
 * Export object for ControlFlogGraphs
 * GraphML file including all vertex and edge properties is created
 * Note: It is recommended to use instead of GraphML the Graphviz exporter: GraphVizExport
 */

class GraphMLExport {
	public:
		GraphMLExport(string filename);
		virtual ~GraphMLExport();

		bool exportGraph(ControlFlowGraph cfg);
		bool exportGraph(FunctionCallGraph fcg);
		bool exportGraph(MemoryStateGraph msg);
		bool exportGraph(AbsStackMemGraph asmg);

	private:
		string export_filename;
		ofstream export_file_stream;
};

#endif
