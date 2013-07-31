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
#ifndef _FLOWFACT_ENRICHMENT_HPP_
#define _FLOWFACT_ENRICHMENT_HPP_

#include "global.h"
#include "constants.h"
#include "graph_structure.h"
#include "configuration.hpp"
#include "flowfact_reader.hpp"

struct flow_fact_edges_t {
	vector<CFGEdge> loop_edges;
	CFGEdge inducting_edge;
	uint32_t loop_bound;
};

/**
 * \brief This class adds flow fact information for loops to the control flow graph.
 * The bound of the loop (added by user) is stored in the circ_t-Edge property of the loop inducting edge. A loop bound of
 * n represents one initial loop body execution plus n further loop body executions. 
 * Notice that a loop bound of 0 means that a loop body is executed only once (there is no iteration).
 */
class FlowFactEnricher {
	public:
		/**
		 * \brief Contructor delivering the graph that will be enriched.
		 * \param cfgraph Control flow graph to which the flow fact values will be added.
		 * \param cfg_entry Entry node of the control flow graph.
		 * \param cfg_exit Exit node of the control flow graph.
		 */
		FlowFactEnricher(ControlFlowGraph cfgraph, CFGVertex cfg_entry, CFGVertex cfg_exit);
		/**
		 * \brief Default destructor.
		 */
		virtual ~FlowFactEnricher();
		/**
		 * \brief Obtains the flow facts of the loop bound and writes them to the circ_t property of the loop inducting edge.
		 * This method uses the FlowFactReader class, which needs the names and addresses of all functions that may appear in the cfg. Therfore these information should be passed to this class by the setDetectedFunctions().
		 */
		void getLoopBounds(void);
		/**
		 * \brief Returns the enriched graph.
		 * \returns The original control flow graph with flow fact information (and also adjusted edgename_t property of the altered edges).
		 */
		ControlFlowGraph getGraphWithFlowFacts(void);
		/**
		 * \brief Returns the set of injecting edges, loop edges and maximum flow for the loop bounds.
		 * \returns Vector of edges that characterizes the loop and the loop bound.
		 */
		vector<flow_fact_edges_t> getFlowFacts(void);
		/**
		 * \brief Returns the entry node of the control flow graph.
		 * \returns The entry node of the control flow graph.
		 */
		CFGVertex getEntry(void);
		/**
		 * \brief Returns the exit node of the control flow graph.
		 * \returns The exit node of the control flow graph.
		 */
		CFGVertex getExit(void);
		/**
		 * \brief Sets all detected functions with label and address, needed by the FlowFactReader.
		 * \param detected_functions All addresses and labels of the functions, that may appear in the control flow graph that is to be enriched.
		 */
		void setDetectedFunctions(vector<addr_label_t> detected_functions);
	private:
		/**
		 * \brief Helper class for parsing the flow fact file.
		 */
		FlowFactReader flowfact;
		/**
		 * \brief Vector that stores the edges and maximum loop bpund for all loop bounds.
		 */
		vector<flow_fact_edges_t> flow_facts;
		/**
		 * \brief The function table of all functions, that may appear in the cfg. Needed for the FlowFactReader.
		 */
		vector<addr_label_t> function_table;
		/**
		 * \brief Control flow graph that is to be enriched with loop bound informations.
		 */
		ControlFlowGraph cfg;
		/**
		 * \brief Entry node of the control flow graph.
		 */
		CFGVertex entry;
		/**
		 * \brief Exit node of the control flow graph.
		 */
		CFGVertex exit;
		/**
		 * \brief ID to distinguish different static flow constraints.
		 * The ID is necessary to merge the same static flow constraints that were split by loop unrolling during VIVU creation in the ILP generator (ILPGenerator::getLoopBounds()).
		 */
		uint16_t sflow_id;

		// Properties of the control flow graph.
		// TODO document!
		property_map<ControlFlowGraph, edgetype_t>::type edgeTypeEProp;
		property_map<ControlFlowGraph, circ_t>::type circEProp;
		property_map<ControlFlowGraph, static_flow_t>::type sflowEProp;
		property_map<ControlFlowGraph, edgename_t>::type edgeNameEProp;
		property_map<ControlFlowGraph, nodetype_t>::type nodeTypeNProp;
		property_map<ControlFlowGraph, startaddr_t>::type startAddrNProp;
		property_map<ControlFlowGraph, startaddrstring_t>::type startAddrStringNProp;
		property_map<ControlFlowGraph, endaddr_t>::type endAddrNProp;


		/**
		 * \brief Pointer to the global configuration object.
		 */
		Configuration *conf;
		/**
		 * \brief Pointer to the LOGCXX logger object.
		 */
		static LoggerPtr logger;
};

#endif
