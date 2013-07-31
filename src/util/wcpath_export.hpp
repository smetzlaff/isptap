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
#ifndef _WCPAT_EXPORT_HPP_
#define _WCPAT_EXPORT_HPP_


#include "global.h"
#include "constants.h"
#include "graph_structure.h"
#include "context_stack.hpp"
#include "cfgloophelper.hpp"
#include "dumpline_parser.hpp"
#include <iostream>
#include <fstream>
#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>


struct instr_stat_t {
	uint64_t instructions;
	uint64_t arithmetic_instructions;
	uint64_t branch_instructions;
	uint64_t condbranch_instructions;
	uint64_t call_instructions;
	uint64_t return_instructions;
	uint64_t load_instructions;
	uint64_t store_instructions;
	uint64_t sync_instructions;
	uint64_t debug_instructions;
	uint64_t other_instructions;
	uint64_t unknown_instructions;
};

// #define PRINT_CACHE_STATS

/*!
 * \brief Exports the worst case path found by the ILPGenerator.
 * The ILPGenerator marks the nodes on the WC-path by setting the activation_t property (the number of activations on the WC-Path is stored) for the edges that connect nodes on the WC-path.
 * The WCPathExporter traverses the WC-path by unrolling all loops on the path and thus creating a trace of nodes and instructions (for basic blocks) of the WC-path. 
 * The exported WC-path then can be compared to the path that was executed by the processor/simulator.
 */
class WCPathExporter : CFGLoopHelper {
	public:
		/*!
		 * \brief Constructor.
		 * \param cfggraph The graph for which the WC-path will be created. Notice that the graph has to be annotated with activation_t properties by the ILPGenerator that mark the WC-path.
		 * \param cfgentry The entry node of the graph (usually the &lt;SUPER_ENTRY&gt; node).
		 * \param cfgexit The exit node of the graph (usually the &lt;SUPER_EXIT&gt; node).
		 * \param wcpath_out_file The name of the file in which the trace of the WC-path is written.
		 * \param wchist_out_file The name of the file in which the trace of the basic block wc histogram is written.
		 */
		WCPathExporter(ControlFlowGraph cfggraph, CFGVertex cfgentry, CFGVertex cfgexit, string wcpath_out_file, string wchist_out_file);
		/*!
		 * \brief Default destructor.
		 */
		virtual ~WCPathExporter();
		/*!
		 * \brief Initiates the export of the WC-path trace.
		 * If the path should not be printed (determined by CONF_EXPORT_WC_PATH == 0) the worst case path is only traversed and all instructions on this path are counted and categorized.
		 */
		void traverseAndPrintWCPath(void);
		/*!
		 * \brief Initiates the export of the WC-path histogram.
		 */
		void printWCHist(void);

		/*!
		 * \brief Creates the categorization of the instructions on the WC-path.
		 * \returns The statistic containing the number of occurences of the different instruction classes.
		 */
		instr_stat_t getInstrStats(void);
	private:
		/*!
		 * \brief Prints the WC-path for a sequential code part. On finding a back edge it is checked if it creates loop. If so the WC-path of loop body is printed (as often as the loop body is executed) by recursivly calling traverseWCPathForSequentialCode().
		 * \param start The start node of the sequential code part (resp. loop body).
		 * \param end The end node of the sequential code pard (resp. loop body).
		 * \param count The number of times this code part is executed in the worst case (defined by the flow value of the loop injecting edge).
		 * \param context Pointer to the context stack that ensures that after calling functions from different call points the right return point is selected on function exit.
		 */
		void traverseWCPathForSequentialCode(CFGVertex start, CFGVertex end, uint32_t count, ContextStack *context);
		/*!
		 * \brief Prints node id and code to export file.
		 * \param v The node to print.
		 */
		void printNode(CFGVertex v);

		/*!
		 * \brief Categorizes an instruction provided in a code line of the basic block and increases the number of occurences for this instruction class by a given number.
		 * \param instr_stats A pointer to the structure that holds the number of occurences of the different instruction classes on the WC-path.
		 * \param instruction A code line from the dump file (has to be a valid code line, checked by DumpLineParser::isCodeLine())
		 * \param count The number of occurences of this instruction on the WC-path, i.e. this is the activation count of the basic block to which the instruction belongs.
		 */
		void countAndCategorizeInstruction(instr_stat_t *instr_stats, string instruction, uint32_t count);

		/*!
		 * \brief Class for parsing lines from the dump file.
		 */
		DumpLineParser *dlp;

		/*!
		 * \brief The file in which the trace of the WC-path is written.
		 */
		ofstream wcpath_output_file;
		/*!
		 * \brief The name of the file in which the trace of the WC-path is written.
		 */
		string wcpath_output_file_name;
		/*!
		 * \brief The file in which the histogram of the WC-path is written.
		 */
		ofstream wchist_output_file;
		/*!
		 * \brief The name of the file in which the histogram of the WC-path is written.
		 */
		string wchist_output_file_name;

		// Node and edge properties of the ControlFlowGraph
		property_map<ControlFlowGraph, bbcode_t>::type bbCodeNProp;
		property_map<ControlFlowGraph, cache_hits_t>::type cacheHitsNProp;
		property_map<ControlFlowGraph, cache_misses_t>::type cacheMissesNProp;
		property_map<ControlFlowGraph, cache_ncs_t>::type cacheNCsNProp;

		property_map<ControlFlowGraph, activation_t>::type actEProp;

		static LoggerPtr logger;
};

#endif
