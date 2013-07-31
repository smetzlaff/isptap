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
#ifndef _COSTEXPORTER_HPP_
#define _COSTEXPORTER_HPP_

#include "global.h"
#include "constants.h"

#include "graph/graph_structure.h"
#include "util/bb_cost_export.hpp"

class CostExporter {
	public:
		static CostExporter* getInstance(void);

		void exportCostToFile(ControlFlowGraph cfg, string file_extension);

	private:
		CostExporter();
		virtual ~CostExporter();
		CostExporter(const CostExporter&);                 // Prevent copy-construction
		CostExporter& operator=(const CostExporter&);      // Prevent assignment
	
		static CostExporter *singleton;
		static LoggerPtr logger;
};

#endif
