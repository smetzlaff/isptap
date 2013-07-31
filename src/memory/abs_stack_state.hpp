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
#ifndef _ABS_STACK_STATE_HPP_
#define _ABS_STACK_STATE_HPP_

#include "global.h"
#include "constants.h"
#include "graph_structure.h"
#include "mem_state_maintainer.hpp"
#include "fcgobj.hpp"
#include "util/graphmlexport.hpp"
#include "util/graphvizexport.hpp"

#define EMPTY 0


class AbsStackMemState {
	public:
		AbsStackMemState(uint32_t memsize, uint32_t blocksize, FunctionCallGraphObject* fcgo, analysis_type_t analysistype);
		AbsStackMemState(const AbsStackMemState& other);

		virtual ~AbsStackMemState();

		void exportGraph(string node_id);

		bool isInState(uint32_t func_addr);
		void insertFunction(uint32_t function_addr, activation_type_t activation, uint32_t previous_function);
		void activateFunction(uint32_t function_addr, activation_type_t activation, uint32_t previous_function);
//		AbsStackMemState& operator+(AbsStackMemState &other);
		AbsStackMemState operator+(AbsStackMemState const other);
		AbsStackMemState join(AbsStackMemState const other);
		void printMemSet(ostringstream *os, string node_id, bool export_graph);

	private:
		AbsStackMemState();

		void setGraphProperties(void);
		uint32_t getFunctionMemSize(uint32_t address);


		ASMVertex getNodeForFunction(uint32_t func_addr);



		uint32_t getSizeOfFwPath(ASMVertex start);
		uint32_t getSizeOfBwPath(ASMVertex start);
		vector<ASMVertex> inline getSuccessors(ASMVertex node);
		vector<ASMVertex> inline getPredecessors(ASMVertex node);
		ASMVertex addSuccessorNode(ASMVertex active, uint32_t func_addr);
		ASMVertex addPredecessorNode(ASMVertex active, uint32_t func_addr);
		void evictStartNodeFromPath(ASMVertex node);
		void evictEndNodeFromPath(ASMVertex node);
		pair<ASMVertex, bool> overwriteFwNodes(ASMVertex active, ASMVertex evictNode, uint32_t function_addr);
		pair<ASMVertex, bool> overwriteBwNodes(ASMVertex active, ASMVertex evictNode, uint32_t function_addr);
		ASMVertex inline addNode(uint32_t func_addr, uint32_t func_size);
		void inline deleteNode(ASMVertex node);
		ASMEdge inline connectNodes(ASMVertex first, ASMVertex second);


		analysis_type_t analysis;
		uint32_t mem_size;
		uint32_t block_size;
		FunctionCallGraphObject* functions;
		AbsStackMemGraph asmg;

		bool empty_graph;

		property_map<AbsStackMemGraph, function_addr_t>::type funcAddrNProp;
		property_map<AbsStackMemGraph, function_size_t>::type funcSizeNProp;
		property_map<AbsStackMemGraph, name_t>::type nameNProp;

		property_map<AbsStackMemGraph, asm_id_t>::type edgeIdEProp;

		/**
		 * \brief Pointer to the LOGCXX logger object.
		 */
		static LoggerPtr logger;


};

#endif
