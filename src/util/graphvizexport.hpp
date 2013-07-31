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
#ifndef _GRAPHVIZEXPORT_HPP_
#define _GRAPHVIZEXPORT_HPP_

#include "global.h"
#include "graph_structure.h"

#include <boost/graph/graphviz.hpp>

/**
 * Export object for ControlFlogGraphs
 * Graphviz file is created, but the vertex and edge preferences are currently not written.
 */


class GraphVizExport {
	public:
		GraphVizExport(string filename);
		virtual ~GraphVizExport();

		bool exportGraph(ControlFlowGraph cfg);
		bool exportGraph(ControlFlowGraph cfg, startaddrstring_t,  edgename_t);
		bool exportGraph(FunctionCallGraph fcg);
		bool exportGraph(MemoryStateGraph msg);
		bool exportGraph(AbsStackMemGraph asmg);

	private:
		string export_filename;
		ofstream export_file_stream;
};

#endif
