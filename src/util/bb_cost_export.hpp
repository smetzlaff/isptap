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
#ifndef _BB_COST_EXPORT_HPP_
#define _BB_COST_EXPORT_HPP_

#include "global.h"
#include "constants.h"
#include "graph_structure.h"
#include "configuration.hpp"

class BBCostExport {
	public:
		BBCostExport(ControlFlowGraph cfgraph);
		virtual ~BBCostExport();

		/*!
		 * \brief Prints the costs of the basic blocks that are currently assigned to the graph, to a file.
		 * \param filename_extension The extension of the file name where the costs are print to. The base file name is defined by property: CONF_EXPORT_BB_COST_FILEPREFIX
		 */
		void exportCostToFile(string filename_extension);

	private:
		ControlFlowGraph cfg;
		Configuration *conf;
		static LoggerPtr logger;

		property_map<ControlFlowGraph, nodetype_t>::type nodeTypeNProp;
		property_map<ControlFlowGraph, startaddr_t>::type startAddrNProp;
		property_map<ControlFlowGraph, cost_t>::type costEProp;
		property_map<ControlFlowGraph, mem_penalty_t>::type memPenaltyEProp;
		property_map<ControlFlowGraph, activation_t>::type actEProp;

};



#endif
