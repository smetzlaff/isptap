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
#ifndef _FUNCTIONCALLGRAPH_OBJECT_HPP_
#define _FUNCTIONCALLGRAPH_OBJECT_HPP_

#include "global.h"
#include "graph_structure.h"
#include "cfgobj.hpp"

struct function_graph_t {
	uint32_t address;
	string name;
	ControlFlowGraphObject *cfg;
};


typedef map<uint32_t, FGVertex> FuncVertexMap;
typedef map<uint32_t, ControlFlowGraphObject*> FuncCfgMap;

class FunctionCallGraphObject {
	public:
		FunctionCallGraphObject();
		virtual ~FunctionCallGraphObject();

		void addFunc(function_graph_t funct);

		uint32_t getCodeSize(void);
		uint32_t getFunctionCount(void);

		bool isFinished(void);

		void setStartLabel(addr_label_t entry_function);
		addr_label_t getStartLabel(void);

		FunctionCallGraph getFCG(void);

		vector<uint32_t> getFunctions(void);
		string getFunctionName(uint32_t address);
		uint32_t getFunctionSize(uint32_t address);
		uint32_t getFunctionAddress(string string);


		ControlFlowGraphObject* getCFGOForFunction(uint32_t address);
		ControlFlowGraphObject* getCFGOForStartFunction(void);

	private:
		FGVertex setCaller(uint32_t function_address, string function_name, uint32_t size);
		FGVertex getCallee(uint32_t function_address);

		void eraseFunctionFromIncompleteList(uint32_t addr);
		void pushCalleeToIncompleteList(uint32_t addr);

		uint32_t code_size;
		uint32_t function_count;

		FunctionCallGraph fg;
		FuncVertexMap funcVMap;
		FuncCfgMap funcCMap;

		vector<uint32_t> incomplete_nodes;
		vector<uint32_t> completed_nodes;

		addr_label_t start_label;

		property_map<FunctionCallGraph, function_name_t>::type funcNameNProp;
		property_map<FunctionCallGraph, function_addr_t>::type funcAddrNProp;
		property_map<FunctionCallGraph, function_size_t>::type funcSizeNProp;
		property_map<FunctionCallGraph, caller_points_t>::type funcCallerEProp;

		static LoggerPtr logger;
};


#endif
