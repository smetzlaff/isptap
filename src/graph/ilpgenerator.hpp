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
#ifndef _ILP_GENERATOR_HPP_
#define _ILP_GENERATOR_HPP_


#include "global.h"
#include "constants.h"
#include "graph_structure.h"
#include "configuration.hpp"
#include "flowfact_reader.hpp"
#include "lpsolver.hpp"
#include "cfgcost_calc.hpp"
#include <iostream>
#include <fstream>

//#include <lpsolve/lp_lib.h>


typedef map<CFGEdge, uint32_t> EdgeFlowVarMap;
typedef map<uint32_t, CFGEdge> FlowVarEdgeMap;

struct obj_func_entry_t {
	uint32_t weight;
	uint32_t var_id;
};

struct flow_equation_t {
	vector<uint32_t> left;
	vector<uint32_t> right;
};

struct context_flow_var_id_t {
	uint32_t context_addr;
	uint32_t context_id;
	uint32_t called_function_addr;
	uint32_t flow_var_id;
};

struct flow_var_id_match_t {
	uint32_t left;
	uint32_t right;
};

struct flow_fact_t {
	vector<uint32_t> loop_edges;
	vector<uint32_t> inducting_edges;
	uint32_t loop_bound;
};

struct s_flow_fact_t {
	flowc_type_t flow_type;
	uint32_t flow_bound;
	vector<uint32_t> edges;
	uint16_t id;
};


class ILPGenerator {

	public:
		ILPGenerator(ControlFlowGraph cfgraph, CFGVertex cfg_entry, CFGVertex cfg_exit);
		virtual ~ILPGenerator();
		void createILP(void);
		void writeILPFile(string filename);
		ControlFlowGraph getILPCFG(void);
		void setFlowFactHandler(FlowFactReader *ff);
		lp_solution_t solveILP(void);
		lp_solution_t getSolutionType(void);
		uint64_t getWCCostValue(void);
		void setWCCostValue(uint64_t wcost);
		/*!
		 * Notice for DISP: the mem cost are _additional_ wait cycles needed for the DISP (the number of cycles the DISP handles hits and misses that are in the slipstream of the call/ret handling are not contained)
		 */
		uint64_t getMemCostValue(void);
		void setAssignedBlocks(vector<uint32_t> assigned_blocks);
		uint32_t getSizeOfBlocks(vector<uint32_t> blocks);
		void updateObjectiveFunction(void);
		void setDetectedFunctions(vector<addr_label_t> detected_functions);
		CFGVertex getEntry(void);
		CFGVertex getExit(void);
		cache_hm_stat_t getCacheHMStatsForWCP(void);
	private:
		void addBackEdge(void);
		void buildObjectFunction(bool createFlowVarMappings);
		void extractFlowConservation(void);
		void extractFlowConservationForFunctionCalls(void);
		void getLoopBounds(void);
		bool isBackEdge(CFGEdge e);
		void setFlowValue(string flowvar, uint32_t flowvar_value);
		void setFlowValue(uint32_t flowvar_id, uint32_t flowvar_value);
		inline uint32_t getWeightOfEdge(CFGEdge e);
		inline uint32_t getMemPenaltyOfEdge(CFGEdge e);

		uint64_t getGraphCost(void);
		uint64_t getGraphMemCost(void);

		void createILPFormulation(void);

		ControlFlowGraph cfg;
		CFGVertex entry, exit;

		EdgeFlowVarMap efvMap;
		FlowVarEdgeMap fevMap;
		uint32_t next_flow_var_id;
		uint32_t flow_id_for_back_edge;
		vector<obj_func_entry_t> object_function;
		vector<flow_equation_t> flow_conservation;
		vector<flow_var_id_match_t> flow_conservation_for_functions;
		vector<flow_fact_t> flow_facts;
		vector<s_flow_fact_t> s_flow_facts;

		vector<addr_label_t> function_table;

		uint32_t wc_cost_value;
		lp_solution_t lp_solution_type;

		string ilp_file;
		string ilp_formulation;


		property_map<ControlFlowGraph, edgetype_t>::type edgeTypeEProp;
		property_map<ControlFlowGraph, cost_t>::type costEProp;
		property_map<ControlFlowGraph, cost_onchip_t>::type costOnChipEProp;
		property_map<ControlFlowGraph, cost_offchip_t>::type costOffChipEProp;
		property_map<ControlFlowGraph, mem_penalty_t>::type memPenaltyEProp;
		property_map<ControlFlowGraph, capacityl_t>::type capacitylEProp;
		property_map<ControlFlowGraph, capacityh_t>::type capacityhEProp;
		property_map<ControlFlowGraph, circ_t>::type circEProp;
		property_map<ControlFlowGraph, activation_t>::type actEProp;
		property_map<ControlFlowGraph, static_flow_t>::type sflowEProp;
		property_map<ControlFlowGraph, edgename_t>::type edgeNameEProp;
		property_map<ControlFlowGraph, nodetype_t>::type nodeTypeNProp;
		property_map<ControlFlowGraph, startaddr_t>::type startAddrNProp;
		property_map<ControlFlowGraph, startaddrstring_t>::type startAddrStringNProp;
		property_map<ControlFlowGraph, endaddr_t>::type endAddrNProp;
		property_map<ControlFlowGraph, context_id_t>::type contextIDNProp;
		property_map<ControlFlowGraph, calllabel_t>::type callLabelNProp;
		property_map<ControlFlowGraph, bbsize_t>::type bbSizeNProp;
		property_map<ControlFlowGraph, cache_hits_t>::type cacheHitsNProp;
		property_map<ControlFlowGraph, cache_misses_t>::type cacheMissesNProp;
		property_map<ControlFlowGraph, cache_ncs_t>::type cacheNCsNProp;


		Configuration *conf;


		static LoggerPtr logger;
};

#endif
