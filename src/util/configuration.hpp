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
#ifndef _CONFIGURATION_HPP_
#define _CONFIGURATION_HPP_

#include "global.h"
#include "constants.h"
#include <map>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <boost/regex.hpp>
#include <log4cxx/helpers/properties.h>
#include "console_string_printer.hpp"
#include "help_message_types.hpp"

#define CONF_ARCHITECTURE "architecture"
#define CONF_DUMP_FILE  "dump_file"
#define CONF_RATIO_FILE_ONCHIP  "ratio_file_onchip"
#define CONF_RATIO_FILE_OFFCHIP  "ratio_file_offchip"
#define CONF_FLOWFACT_FILE  "flow_fact_file"
#define CONF_USE_METRIC  "use_metric" 
#define CONF_USE_FLOWFACT_FILE  "use_flow_fact_files"
#define CONF_USE_FLOWFACT_GRAPH_ENRICHMENT  "use_flow_fact_graph_enrichment"
#define CONF_ENTRY_FUNCTION  "entry_function"
#define CONF_NUMBER_OF_CORES "cores"
#define CONF_ENTRY_FUNCTIONS  "entry_functions"
#define CONF_HRT_CORES "hrt_cores"
#define CONF_MEMORY_TYPE  "memory_type"
#define CONF_MEMORY_SIZE  "memory_size"
#define CONF_MEMORY_START_SIZE  "memory_start_size"
#define CONF_MEMORY_STEP_SIZE  "memory_step_size"
#define CONF_MEMORY_SIZE_STEPPING  "memory_size_stepping"
#define CONF_MEMORY_CACHE_BBS "memory_cache_BBs"
#define CONF_MEMORY_REPLACEMENT_POLICY "memory_replacement_policy"
#define CONF_MEMORY_DISP_IGNORE_OUTSIZED_FUNCTIONS "memory_disp_ignore_outsized_functions"
#define CONF_MEMORY_BBSISP_ADD_JUMP_PENALTIES_TO_WCET "memory_bbsisp_add_jump_penalties_to_wcet" // If any BBSISP flavour is used for finding a memory assignment, the WCET calculation of the assignment will take the penalies into account when connecting jumps have to be added.
#define CONF_USE_MEMORY_BUDGET "use_memory_budget" // determines if instead of cache or scratchpad sizes a global memory budget is to be used (inluding all additional helper/tag memory sizes) is to be used
#define CONF_MEMORY_BUDGET "memory_budget"
#define CONF_EXPORT_FORMAT "export_format"
#define CONF_EXPORT_FUNCTION_CFGS "export_function_cfgs"
#define CONF_EXPORT_FUNCTION_CALL_GRAPH "export_function_call_graph"
#define CONF_EXPORT_SCFG	"export_scfg"
#define CONF_EXPORT_FLOW_SCFG "export_flow_scfg"
#define CONF_EXPORT_SOLVED_FLOW_SCFG "export_solved_flow_scfg"
#define CONF_EXPORT_SOLVED_FLOW_SCFG_WITH_ASSIGNMENT "export_solved_flow_scfg_with_assignment"
#define CONF_EXPORT_VIVU_GRAPH "export_vivu_graph"
#define CONF_EXPORT_FUNCTION_TABLE "export_function_table"
#define CONF_FUNCTION_TABLE_FILE "function_table_file"
#define CONF_OLD_FUNCTION_TABLE_FORMAT "old_function_table_format"
#define CONF_REPORT_FILE "report_file"
#define CONF_REPORT_APPEND "report_append"
#define CONF_STATIC_MAPPING_REPORT "static_mapping_report"
#define CONF_STATIC_MAPPING_REPORT_FILE "static_mapping_report_file"
#define CONF_EXPORT_WC_PATH "export_wcpath"
#define CONF_EXPORT_WC_PATH_HIST "export_wcpath_hist"
#define CONF_EXPORT_WC_PATH_FILEPREFIX "wcpath_fileprefix"
#define CONF_EXPORT_WC_PATH_INSTR_STATS "export_wcpath_instr_stats"
#define CONF_EXPORT_BB_COST "export_bb_cost_from_graph"
#define CONF_EXPORT_BB_COST_FILEPREFIX "export_bb_cost_fileprefix"
#define CONF_EXPORT_ILPS "export_ilps"
#define CONF_WRITE_RAW_FILE "write_raw_file"
#define CONF_RAW_FILE "raw_file"
#define CONF_DISP_INSTRUMENTATION "disp_instrumentation"
#define CONF_USE_ARCH_CFG_FILE "use_architecture_config_file"
#define CONF_ARCH_CFG_FILE "architecture_config_file"
#define CONF_LOG_FILE "log_file"
#define CONF_LOG_PROP_FILE "log_properties_file"
#define CONF_LP_SOLVE_PARAMETERS "lp_solve_parameters"
#define CONF_LP_SOLVE_DONT_DELETE_TEMPFILES "lp_solve_dont_delete_tempfiles"
#define CONF_BBSISP_WCP_SHRINK_ILP_FORMULATION "bbsisp_wcp_shrink_ilp_formulation" // Simplifies the WCP-sensitive ILP formulation used to find the basic block assignment, if the original formulation (which is human readable) is to complex to be solved quickly. (Setting this option simplifies the ILP for WCP-sensitive formulation by inlining as much as possible constraints, like the edge costs.)
#define CONF_BBSISP_WCP_FILL_ISP_UP "bbsisp_wcp_fill_isp_up" // Also adds BBs to the BBSISP, even if they have no impact on the WCET. This is done by adding the used scratchpad size to objective function weighted with 1e-10, to fill the scratchpad as much as possible. Notice due to the weighting of the scratchpad size the assignment of usefull bocks shall not be affected.

// this config parameter is used to check if the output of the isptap changes, due to bugfixes or feature extension
// thus it is used for verification and testing
#define CONF_OUTPUT_WCET "output_wcet"
#define CONF_OUTPUT_MEM_COST "output_mem_cost"
#define CONF_OUTPUT_WCET_WO_OPT "output_wcet_wo_optimization" // for static mems the wcet without optimization
#define CONF_OUTPUT_MEM_COST_WO_OPT "output_mem_cost_wo_optimization" // for static mems the memory cost without optimization
#define CONF_OUTPUT_WCET_STEP "output_wcet_step"
#define CONF_OUTPUT_MEM_COST_STEP "output_mem_cost_step"

// contains the real execution time of the simulator for that benchmark and configuration
#define CONF_OUTPUT_ET_SIM "output_sim"


// supported architectures
#define CONF_ARCHITECTURE_CARCORE "CARCORE"
#define CONF_ARCHITECTURE_ARMV6M "ARMV6M"
enum architecture_t { CARCORE=0, ARMV6M, UNKNOWN_ARCHICTECTURE};


enum mem_type_t { BBSISP=0, BBSISP_JP, BBSISP_WCP, BBSISP_JP_WCP, FSISP_OLD, FSISP, FSISP_WCP, DISP, ICACHE, NO_MEM, VIVU_TEST, UNKNOWN_MEM};
#define CONF_MEMORY_TYPE_BBSISP "BBSISP"
#define CONF_MEMORY_TYPE_BBSISP_JP "BBSISP_JP" // as BBSISP but with adding the jump penalty in knapsack optimization and WCET calculation
#define CONF_MEMORY_TYPE_BBSISP_WCP "BBSISP_WCP" // using optimal WCP-aware assignment as proposed by SuMi05 and FaKl09/FaLo10 without jump and size penalties
#define CONF_MEMORY_TYPE_BBSISP_JP_WCP "BBSISP_JP_WCP" // using optimal WCP-aware assignment as proposed by SuMi05 and FaKl09/FaLo10 including jump and size penalties

#define CONF_MEMORY_TYPE_FSISP_OLD "FSISP_OLD"
#define CONF_MEMORY_TYPE_FSISP "FSISP"
#define CONF_MEMORY_TYPE_FSISP_WCP "FSISP_WCP"
#define CONF_MEMORY_TYPE_DISP "DISP"
#define CONF_MEMORY_TYPE_ICACHE "ICACHE"
#define CONF_MEMORY_TYPE_NOMEM "NONE"
// the VIVU_TEST creates a vivu graph, but then no DFA for dynamic memory is performed. On the vivu graph the ilp is solved. The result should be the same as for memory type NONE
#define CONF_MEMORY_TYPE_VIVU_TEST "VIVU_TEST"


enum replacement_policy_t { FIFO=0, LRU, DIRECT_MAPPED, STACK, UNKNOWN_RP};
#define CONF_MEMORY_REPLACEMENT_POLICY_FIFO "FIFO"
#define CONF_MEMORY_REPLACEMENT_POLICY_LRU "LRU"
#define CONF_MEMORY_REPLACEMENT_POLICY_DIRECT_MAPPED "DIRECT_MAPPED"
#define CONF_MEMORY_REPLACEMENT_POLICY_STACK "STACK"

enum export_graph_format_t { GRAPHVIZ=0, GRAPHML, UNKNOWN_GRAPHOUTPUT};
#define CONF_EXPORT_FORMAT_GRAPHVIZ "GRAPHVIZ"
#define CONF_EXPORT_FORMAT_GRAPHML "GRAPHML"

enum analysis_metric_t { 
	WCET = 0, // Use the WCET as metric, for basic block cost the internal timing model is used
	MDIC, // Use the maximum dynamic instruction count as metric
	MPL, //  Use the length of the worst case path as metric (this will not work for the BBSISP, FSISP is supported only)
	WCET_RATIO_FILES, // Use the WCET as metric, for basic block the OTAWA ratio files are used
	UNKNOWN_METRIC
};
#define CONF_METRIC_WCET "WCET"
#define CONF_METRIC_MDIC "MDIC"
#define CONF_METRIC_MPL "MPL"
#define CONF_METRIC_WCET_RATIO_FILES "wcet_ratio_files"

#define IS_STATIC_MEM(a) ((a == BBSISP) || (a == BBSISP_JP) || (a == BBSISP_WCP) || (a == BBSISP_JP_WCP) || (a == FSISP_OLD) || (a == FSISP) || (a == FSISP_WCP))
#define IS_BBSISP(a) ((a == BBSISP) || (a == BBSISP_JP) || (a == BBSISP_WCP) || (a == BBSISP_JP_WCP))
#define IS_FSISP(a) ((a == FSISP_OLD) || (a == FSISP) || (a == FSISP_WCP))
#define IS_DYNAMIC_MEM(a) ((a == DISP) || (a == ICACHE))

enum loglevel_t { DEBUG=0, INFO, WARN, ERROR};

using namespace boost;

typedef map<const string, string> prop_val_map_t;
typedef map<const string, string> val_map_t;
typedef map<const string, val_map_t> prop_vals_map_t;

class Configuration {
	public:
		static Configuration* getInstance(void);
		static Configuration* getInstanceWithHelpMessages(void);

		void parseConfigFile(const string &config_file); 
		void initializeLogging(void);

		void setProperty(const string property, string value);
		void setProperty(const string property, bool value);

		bool getBool(string property);
		int32_t getInt(string property);
		uint32_t getUint(string property);
		uint32_t getUint(string property, string value);
		uint32_t getUint(string property, uint32_t value);
		vector<uint32_t> getUints(string property);
		uint32_t getHex(string property);
		string getString(string property);
		string getString(string property, string value);
		string getString(string property, uint32_t value);
		vector<string> getStrings(string property);

		string getHelpMessage(void);

	private:
		// function called by public members: getInstance(void) and getInstanceWithHelpMessages()
		static Configuration* getInstance(bool initialise_help_messages);
		Configuration(bool initialise_help_messages);
		Configuration();
		virtual ~Configuration();
		Configuration(const Configuration&);                 // Prevent copy-construction
		Configuration& operator=(const Configuration&);      // Prevent assignment

		bool isCfgAssignmentLine(string line);
		void storeConfigurationEntry(string line);

		void addLogProps(string logprop_filename);
		bool isLogAssignmentLine(string line);

		bool usesMap(string property);

		static Configuration *singleton;

		static regex re_cfg_assignment;
		static regex re_log_assignment;

		prop_val_map_t config_map;
		prop_vals_map_t config_maps;
		
		vector<help_message_t> config_properties;

		static LoggerPtr logger;

		vector<pair<loglevel_t, string> > log_output;

		helpers::Properties logProp;
};

#endif
