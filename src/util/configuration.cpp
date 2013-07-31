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
#include "configuration.hpp"
#include <boost/algorithm/string.hpp>

regex Configuration::re_cfg_assignment("^([[:space:]]*)isptap\\.[[:alnum:]]([[:alnum:]]|\\_|/|\\.)+[[:space:]]*=[[:space:]]*([[:alnum:]]|\\_|/|\\.)*", regex::icase);
regex Configuration::re_log_assignment("^([[:space:]]*)log4j\\.[[:alnum:]]([[:alnum:]]|\\_|/|\\.)+[[:space:]]*=[[:space:]]*([[:alnum:]]|\\_|/|\\.)*", regex::icase);

LoggerPtr Configuration::logger(Logger::getLogger("Configuration"));

Configuration *Configuration::singleton = NULL;


Configuration* Configuration::getInstance(void)
{
	return getInstance(false);
}

Configuration* Configuration::getInstanceWithHelpMessages(void)
{
	return getInstance(true);
}


Configuration* Configuration::getInstance(bool initialise_help_messages)
{
	if(singleton == NULL)
	{
		singleton = new Configuration(initialise_help_messages);
	}
	return singleton;
}

#define HELP_CONF_ARCHITECTURE "Specifies the architecture model that is used for timing analysis. Supported architecutes are: \""CONF_ARCHITECTURE_CARCORE"\" and \""CONF_ARCHITECTURE_ARMV6M"\"."
#define HELP_CONF_DUMP_FILE "The dump file to parse."
#define HELP_CONF_RATIO_FILE_ONCHIP  "[deprecated]"
#define HELP_CONF_RATIO_FILE_OFFCHIP  "[deprecated]"
#define HELP_CONF_FLOWFACT_FILE  "The name and the relative path to the flow-fact file containing additional flow information of the program, including loop bounds, absolute flow information, and indirect jump and call targets. For help on the format and usage of the flow-fact file read the help message by using the command line parameter --help-flow-facts."
#define HELP_CONF_USE_METRIC  "The used analysis metric. The default metric is the worst-case execution time in which the path with the longest execution time of the program is determined. Other metrics are MDIC, which selects the path of the program with the Maximum Dynamic Instruction Count, and MPL (Maximum Path Length), which selects the longest path of the program (w.r.t. the number of instructions). The latter metrics do not apply a processor timing model and are added just for debug reasons." 
#define HELP_CONF_USE_FLOWFACT_FILE  "If set to '1', the analysis uses a flow-fact file providing information about loop bounds, other flow constraints, and indirect jump or call targets."
#define HELP_CONF_USE_FLOWFACT_GRAPH_ENRICHMENT  "If set to '1', the information of the flow-fact file will be added to the graph representation of the program. This is necessary to respect the flow information in the analysis of the program."
#define HELP_CONF_ENTRY_FUNCTION  "The top function that is to be analysed. All functions called by this function are included into the analysis."
#define HELP_CONF_NUMBER_OF_CORES "Defines the number of cores of a multi/many core architecture. Currently not implemented."
#define HELP_CONF_ENTRY_FUNCTIONS  "List of the entry functions per core. Currently not implemented."
#define HELP_CONF_HRT_CORES "List of the cores with hard real-time tasks. Currently not implemented."
#define HELP_CONF_MEMORY_TYPE  "Sets the used instruction memory type. Supported are the following memory types: Static instruction scratchpads with basic block assignment policy: BBSISP (Knapsack algorithm), BBSISP_JP (Knapsack algorithm with jump and size penalties), BBSISP_WCP (WCET path sensitive algorithm, without jump and size penalties), and BBSISP_JP_WCP (WCET path sensitive algorithm with jump and size penalties); Static instruction scratchpads with function assignment policy: FSISP (Knapsack algorithm) and FSISP_WCP (WCET path sensitive algorithm); Instruction caches: ICACHE with different replacement policies, see memory_replacement_policy; Dynamic instruction scratchpad: DISP with different replacement policies, see memory_replacement_policy; No on-chip instruction memory: NONE (all fetches are handled by the off-chip memory)."
#define HELP_CONF_MEMORY_SIZE  " Size of the memory in bytes."
#define HELP_CONF_MEMORY_START_SIZE  "Start memory size in bytes for the memory range analysis mode (see \""CONF_MEMORY_SIZE_STEPPING"\")."
#define HELP_CONF_MEMORY_STEP_SIZE  "Size of the steps in bytes used in memory range analysis mode (see \""CONF_MEMORY_SIZE_STEPPING"\")."
#define HELP_CONF_MEMORY_SIZE_STEPPING  "If set to '1', multiple analyses with a range of memory sizes are performed for the chosen memory type and application. The analysis starts with the size defined in \""CONF_MEMORY_START_SIZE"\" and increases in steps defined in \""CONF_MEMORY_STEP_SIZE"\" until the value defined in \""CONF_MEMORY_SIZE"\" is reached."
#define HELP_CONF_MEMORY_CACHE_BBS "Sets the cache line granularity to basic blocks (i.e. a basic block fits always a cache line and a cache line contains always only one basic block)."
#define HELP_CONF_MEMORY_REPLACEMENT_POLICY "The replacement policy of the dynamic memories, DISP and cache. Supported policies are: FIFO, LRU, DIRECT_MAPPED (cache only), and STACK (DISP only)."
#define HELP_CONF_MEMORY_DISP_IGNORE_OUTSIZED_FUNCTIONS "If set to '1', the DISP ignores functions that are larger than its memory size. In that case these functions will be fetched from the off-chip memory."
#define HELP_CONF_MEMORY_BBSISP_ADD_JUMP_PENALTIES_TO_WCET "This option is used for BBSISP flavours that do not consider any penalties for additional jumps on assignment of basic blocks to the scratchpad (i.e. BBSISP and BBSISP_WCP). If the parameter is set to '1', the penalties for connecting the chosen basic blocks by additional jumps are added before the final WCET calculation. Thus, the provided WCET will be correct, concerning the necessary jump penalties, but these penalties were not considered during the finding of the scratchpad assignments." 
#define HELP_CONF_USE_MEMORY_BUDGET "If set to '1', instead a memory size a global memory budget for the on-chip instruction memory is used. This includes the tag memory for a cache and the helper memories for the DISP." 
#define HELP_CONF_MEMORY_BUDGET "The global memory budget for the on-chip instruction memory. Depending on the configuration of the memory type, replacement policies, etc. the size of the on-chip instruction memory is calculated. Replaces \""CONF_MEMORY_SIZE"\", if \""CONF_USE_MEMORY_BUDGET"\" set."
#define HELP_CONF_EXPORT_FORMAT "Output format of graphs: \""CONF_EXPORT_FORMAT_GRAPHVIZ"\" or \""CONF_EXPORT_FORMAT_GRAPHML"\"."
#define HELP_CONF_EXPORT_FUNCTION_CFGS "If set to '1', the control flow graphs of all functions are exported."
#define HELP_CONF_EXPORT_FUNCTION_CALL_GRAPH "If set to '1', the call graph (of the entry function) is exported."
#define HELP_CONF_EXPORT_SCFG	"If set to '1', the super control flow graph (CFG of the entry function in which all calls are inlined) is exported."
#define HELP_CONF_EXPORT_FLOW_SCFG "If set to '1', the super control flow graph enriched with the flow facts is exported."
#define HELP_CONF_EXPORT_SOLVED_FLOW_SCFG "If set to '1', the super control flow graph with the calculated flows of the worst-case path is exported."
#define HELP_CONF_EXPORT_SOLVED_FLOW_SCFG_WITH_ASSIGNMENT "If set to '1', the super control flow graph with the calculated flows of the worst-case path is exported. Additionally, the code (basic blocks) that is assigned to the scratchpad (BBSISP or FSISP) is highlighted."
#define HELP_CONF_EXPORT_VIVU_GRAPH "If set to '1', the VIVU translated super control flow graph is exported. The VIVU graph can only created for cache or DISP."
#define HELP_CONF_EXPORT_FUNCTION_TABLE "If set to '1', a function table containing the name, start address, and size of each function is exported."
#define HELP_CONF_FUNCTION_TABLE_FILE "The file name of the function table to export."
#define HELP_CONF_OLD_FUNCTION_TABLE_FORMAT "If set to '1', the old function table format (that contains only name and start address) is used for export."
#define HELP_CONF_REPORT_FILE "The file name of the report file."
#define HELP_CONF_REPORT_APPEND "If set to '1', the report file is not overwritten and the report is appended."
#define HELP_CONF_STATIC_MAPPING_REPORT "If set to '1', the functions that are assigned for the FSISP are exported. Also header files that assign the selected functions to the SPM section are generated for the analysed application."
#define HELP_CONF_STATIC_MAPPING_REPORT_FILE "The file name prefix for the reports and headers of the FSISP assignment."
#define HELP_CONF_EXPORT_WC_PATH "If set to '1', a basic block address trace of the WCET critical path is exported."
#define HELP_CONF_EXPORT_WC_PATH_HIST "If set to '1', a basic block histogram of the WCET critical path is exported."
#define HELP_CONF_EXPORT_WC_PATH_FILEPREFIX "The prefix for the file names for the exported WCET critical path statistics."
#define HELP_CONF_EXPORT_WC_PATH_INSTR_STATS "If set to '1', the statistics of the different instruction types on the WCET critical path are exported."
#define HELP_CONF_EXPORT_BB_COST "If set to '1', the cost and activation count of every basic block on the WCET critical path is exported."
#define HELP_CONF_EXPORT_BB_COST_FILEPREFIX "The file prefix of the basic block cost file."
#define HELP_CONF_EXPORT_ILPS "If set to '1', the generated ILPs needed to calculate the estimate are exported."
#define HELP_CONF_WRITE_RAW_FILE "[deprecated]"
#define HELP_CONF_RAW_FILE "[deprecated]"
#define HELP_CONF_DISP_INSTRUMENTATION "[deprecated]]"
#define HELP_CONF_USE_ARCH_CFG_FILE "If set to '1', an architectural configuration file is used. Otherwise a default architecture is assumed."
#define HELP_CONF_ARCH_CFG_FILE " The file that configures the architecture that is analysised. For further information see --help-architecture option."
#define HELP_CONF_LOG_FILE "The file name of the output log file."
#define HELP_CONF_LOG_PROP_FILE "The used log4cxx log property file, which defines the log level."
#define HELP_CONF_LP_SOLVE_PARAMETERS "Additional parameters for the ILP solver lp_solve."
#define HELP_CONF_LP_SOLVE_DONT_DELETE_TEMPFILES "If set to '1', the temp files used by the ILP solver are not deleted after their use (for debugging)."
#define HELP_CONF_BBSISP_WCP_SHRINK_ILP_FORMULATION "Set this option to '1', if the ILPs of BBSISP_WCP and BBSISP_JP_WCP (which is human readable) are too complex to be solved quickly. Then the WCP-sensitive ILP formulation used to find the basic block assignment will be simplified. This is done by inlining simple constraints, like the edge costs. Notice that the provided scratchpad assignment is not affected by this option." 
#define HELP_CONF_BBSISP_WCP_FILL_ISP_UP "This option allows the BBSISP assignment algorithms to add basic blocks to the BBSISP, even if they have no positive impact on the WCET. This is done by adding the used scratchpad size to objective function weighted with 1e-10, which fills the scratchpad as much as possible. Notice due to the weighting of the used scratchpad size the assignment of useful bocks shall not be affected." 
#define HELP_CONF_OUTPUT_WCET "Reference WCET value. It is used for a validation check with the calculated WCET bound (debug option)."
#define HELP_CONF_OUTPUT_MEM_COST "Reference memory cost value. It is used for a validation check with the calculated memory cost (debug option)."
#define HELP_CONF_OUTPUT_WCET_WO_OPT "Reference WCET value before static scratchpad assignment. It is used for a validation check with the calculated WCET bound (debug option)." 
#define HELP_CONF_OUTPUT_MEM_COST_WO_OPT "Reference memory cost value before static scratchpad assignment. It is used for a validation check with the calculated memory cost (debug option)." 
#define HELP_CONF_OUTPUT_WCET_STEP "If the memory range analysis mode is active, a list of reference WCET values can be provided for validation (debug option)."
#define HELP_CONF_OUTPUT_MEM_COST_STEP "If the memory range analysis mode is active, a list of reference memory cost values can be provided for validation (debug option)."
#define HELP_CONF_OUTPUT_ET_SIM "Real execution time of the application. It is used to calculate the pessimism/overhead of the calculated WCET."

Configuration::Configuration(bool initialise_help_messages)
{
	PREPARE_OPTION_STRING(config_map, CONF_ARCHITECTURE, "carcore", initialise_help_messages);
	PREPARE_OPTION_STRING(config_map, CONF_DUMP_FILE, "main.dump", initialise_help_messages);
	PREPARE_OPTION_STRING(config_map, CONF_RATIO_FILE_ONCHIP, "main_ONCHIP.ratio", initialise_help_messages);
	PREPARE_OPTION_STRING(config_map, CONF_RATIO_FILE_OFFCHIP, "main_OFFCHIP.ratio", initialise_help_messages);
	PREPARE_OPTION_STRING(config_map, CONF_FLOWFACT_FILE, "main.ff", initialise_help_messages);
	PREPARE_OPTION_STRING(config_map, CONF_USE_FLOWFACT_FILE, "0", initialise_help_messages);
	PREPARE_OPTION_STRING(config_map, CONF_USE_FLOWFACT_GRAPH_ENRICHMENT, "0", initialise_help_messages);
	PREPARE_OPTION_STRING(config_map, CONF_NUMBER_OF_CORES, "1", initialise_help_messages);
	PREPARE_OPTION_STRING(config_map, CONF_ENTRY_FUNCTION, "main", initialise_help_messages);
	PREPARE_OPTION_STRING(config_map, CONF_MEMORY_TYPE, CONF_MEMORY_TYPE_NOMEM, initialise_help_messages);
	PREPARE_OPTION_STRING(config_map, CONF_MEMORY_REPLACEMENT_POLICY, "UNKNOWN", initialise_help_messages);
	PREPARE_OPTION_STRING(config_map, CONF_MEMORY_SIZE, "0", initialise_help_messages);
	PREPARE_OPTION_STRING(config_map, CONF_MEMORY_START_SIZE, "0", initialise_help_messages);
	PREPARE_OPTION_STRING(config_map, CONF_MEMORY_STEP_SIZE, "0", initialise_help_messages);
	PREPARE_OPTION_STRING(config_map, CONF_MEMORY_SIZE_STEPPING, "0", initialise_help_messages);
	PREPARE_OPTION_STRING(config_map, CONF_MEMORY_CACHE_BBS, "0", initialise_help_messages);
	PREPARE_OPTION_STRING(config_map, CONF_MEMORY_DISP_IGNORE_OUTSIZED_FUNCTIONS, "0", initialise_help_messages);
	PREPARE_OPTION_STRING(config_map, CONF_MEMORY_BBSISP_ADD_JUMP_PENALTIES_TO_WCET, "0", initialise_help_messages);
	// TODO: Use other step sizes for memory budget stepping mode.
	PREPARE_OPTION_STRING(config_map, CONF_USE_MEMORY_BUDGET, "0", initialise_help_messages);
	PREPARE_OPTION_STRING(config_map, CONF_MEMORY_BUDGET, "0", initialise_help_messages);
	PREPARE_OPTION_STRING(config_map, CONF_EXPORT_FORMAT, "0", initialise_help_messages); // GRAPHVIZ is default;
	PREPARE_OPTION_STRING(config_map, CONF_EXPORT_FUNCTION_CFGS, "0", initialise_help_messages);
	PREPARE_OPTION_STRING(config_map, CONF_EXPORT_FUNCTION_CALL_GRAPH, "0", initialise_help_messages);
	PREPARE_OPTION_STRING(config_map, CONF_EXPORT_SCFG, "0", initialise_help_messages);
	PREPARE_OPTION_STRING(config_map, CONF_EXPORT_FLOW_SCFG, "0", initialise_help_messages);
	PREPARE_OPTION_STRING(config_map, CONF_EXPORT_SOLVED_FLOW_SCFG, "0", initialise_help_messages);
	PREPARE_OPTION_STRING(config_map, CONF_EXPORT_SOLVED_FLOW_SCFG_WITH_ASSIGNMENT, "0", initialise_help_messages);
	PREPARE_OPTION_STRING(config_map, CONF_EXPORT_VIVU_GRAPH, "0", initialise_help_messages);
	PREPARE_OPTION_STRING(config_map, CONF_EXPORT_FUNCTION_TABLE, "0", initialise_help_messages);
	PREPARE_OPTION_STRING(config_map, CONF_EXPORT_ILPS, "0", initialise_help_messages);
	PREPARE_OPTION_STRING(config_map, CONF_FUNCTION_TABLE_FILE, "isptap_functiontable.ftab", initialise_help_messages);
	PREPARE_OPTION_STRING(config_map, CONF_OLD_FUNCTION_TABLE_FORMAT, "0", initialise_help_messages);
	PREPARE_OPTION_STRING(config_map, CONF_REPORT_FILE, "report.log", initialise_help_messages);
	PREPARE_OPTION_STRING(config_map, CONF_REPORT_APPEND, "0", initialise_help_messages);
	PREPARE_OPTION_STRING(config_map, CONF_STATIC_MAPPING_REPORT, "0", initialise_help_messages);
	PREPARE_OPTION_STRING(config_map, CONF_STATIC_MAPPING_REPORT_FILE, "report_smapping.cfg", initialise_help_messages);
	PREPARE_OPTION_STRING(config_map, CONF_EXPORT_WC_PATH, "0", initialise_help_messages);
	PREPARE_OPTION_STRING(config_map, CONF_EXPORT_WC_PATH_HIST, "0", initialise_help_messages);
	PREPARE_OPTION_STRING(config_map, CONF_EXPORT_WC_PATH_FILEPREFIX, "isptap_wcpath", initialise_help_messages);
	// enabling CONF_EXPORT_WC_PATH_INSTR_STATS by default will cause rather high analysis time
	PREPARE_OPTION_STRING(config_map, CONF_EXPORT_WC_PATH_INSTR_STATS, "1", initialise_help_messages);
	PREPARE_OPTION_STRING(config_map, CONF_EXPORT_BB_COST, "0", initialise_help_messages);
	PREPARE_OPTION_STRING(config_map, CONF_EXPORT_BB_COST_FILEPREFIX, "isptap_bbcost", initialise_help_messages);
	PREPARE_OPTION_STRING(config_map, CONF_WRITE_RAW_FILE, "0", initialise_help_messages);
	PREPARE_OPTION_STRING(config_map, CONF_RAW_FILE, "main.raw", initialise_help_messages);
	PREPARE_OPTION_STRING(config_map, CONF_DISP_INSTRUMENTATION, "0", initialise_help_messages);
	PREPARE_OPTION_STRING(config_map, CONF_USE_ARCH_CFG_FILE, "0", initialise_help_messages);
	PREPARE_OPTION_STRING(config_map, CONF_ARCH_CFG_FILE, "configs/carcore.prop", initialise_help_messages);
	PREPARE_OPTION_STRING(config_map, CONF_LOG_PROP_FILE, "configs/baselog.prop", initialise_help_messages);
	PREPARE_OPTION_STRING(config_map, CONF_LOG_FILE, "isptap_default.log", initialise_help_messages);
	PREPARE_OPTION_STRING(config_map, CONF_LP_SOLVE_PARAMETERS, "", initialise_help_messages);
	PREPARE_OPTION_STRING(config_map, CONF_LP_SOLVE_DONT_DELETE_TEMPFILES, "0", initialise_help_messages);
	PREPARE_OPTION_STRING(config_map, CONF_BBSISP_WCP_SHRINK_ILP_FORMULATION, "0", initialise_help_messages);
	PREPARE_OPTION_STRING(config_map, CONF_BBSISP_WCP_FILL_ISP_UP, "0", initialise_help_messages);
	
	// wcet path length as metric activated by default
	PREPARE_OPTION_STRING(config_map, CONF_USE_METRIC, "WCET", initialise_help_messages);
	PREPARE_OPTION_STRING(config_map, CONF_OUTPUT_WCET, "0", initialise_help_messages);
	PREPARE_OPTION_STRING(config_map, CONF_OUTPUT_WCET_WO_OPT, "0", initialise_help_messages);
	PREPARE_OPTION_STRING(config_map, CONF_OUTPUT_MEM_COST, "0", initialise_help_messages);
	PREPARE_OPTION_STRING(config_map, CONF_OUTPUT_MEM_COST_WO_OPT, "0", initialise_help_messages);

	PREPARE_OPTION_STRING(config_map, CONF_OUTPUT_ET_SIM, "0", initialise_help_messages);

	{
		val_map_t tmp;
		config_maps.insert(pair<string, val_map_t>(CONF_OUTPUT_WCET_STEP, tmp));
		ADD_HELP_MESSAGE(CONF_OUTPUT_WCET_STEP, initialise_help_messages);
	}
	{
		val_map_t tmp;
		config_maps.insert(pair<string, val_map_t>(CONF_OUTPUT_MEM_COST_STEP, tmp));
		ADD_HELP_MESSAGE(CONF_OUTPUT_MEM_COST_STEP, initialise_help_messages);
	}
	{
		val_map_t tmp;
		config_maps.insert(pair<string, val_map_t>(CONF_ENTRY_FUNCTIONS, tmp));
		ADD_HELP_MESSAGE(CONF_ENTRY_FUNCTIONS, initialise_help_messages);
	}
	{
		val_map_t tmp;
		config_maps.insert(pair<string, val_map_t>(CONF_HRT_CORES, tmp));
		ADD_HELP_MESSAGE(CONF_HRT_CORES, initialise_help_messages);
	}
}

Configuration::~Configuration()
{
	config_map.clear();

	prop_vals_map_t::iterator it;
	for(it = config_maps.begin(); it != config_maps.end(); it++)
	{
		it->second.clear();
	}
	config_maps.clear();

	log_output.clear();

	config_properties.clear();
}

void Configuration::parseConfigFile(const string &config_file)
{
	ifstream config_file_s;

	// buffer log output until logger is set up
	stringstream s;
	s << "Opening config file: " << config_file;
	log_output.push_back(make_pair(DEBUG, s.str()));

	config_file_s.open(config_file.c_str());

	assert(config_file_s.is_open());

	while(!config_file_s.eof())
	{
		string str;
		getline(config_file_s, str, '\n');
		storeConfigurationEntry(str);
	}

	config_file_s.close();
}

void Configuration::initializeLogging(void)
{
	// configure log properties
	addLogProps(getString(CONF_LOG_PROP_FILE));
	logProp.setProperty("log4j.appender.A1.File", getString(CONF_LOG_FILE));

	PropertyConfigurator::configure(logProp);

	for(uint32_t i = 0; i < log_output.size(); i++)
	{
		switch(log_output[i].first)
		{
			case DEBUG: 
				{
					LOG_DEBUG(logger, log_output[i].second);
					break;
				}
			case INFO: 
				{
					LOG_INFO(logger, log_output[i].second);
					break;
				}
			case WARN: 
				{
					LOG_WARN(logger, log_output[i].second);
					break;
				}
			default:
				{
					LOG_ERROR(logger, log_output[i].second);
					break;
				}
		}
	}

	// free written log entries
	log_output.clear();

}

bool Configuration::getBool(string property)
{
	prop_val_map_t::iterator pos;
	pos = config_map.find(property);
	if(pos != config_map.end())
	{
		// return true if value is 1, "TRUE", or "true", else false
		if(((pos->second).compare("1")==0) || ((pos->second).compare("TRUE")==0)||((pos->second).compare("true")==0))
		{
			return true;
		}
	}
	return false;
}

int32_t Configuration::getInt(string property)
{
	prop_val_map_t::iterator pos;

	pos = config_map.find(property);

	if(pos != config_map.end())
	{
		return strtol((pos->second).c_str(), NULL,10);
	}

	return 0;
}

uint32_t Configuration::getUint(string property)
{
	prop_val_map_t::iterator pos;

	pos = config_map.find(property);

	if(pos != config_map.end())
	{
		return strtoul((pos->second).c_str(), NULL,10);
	}

	return 0;
}


uint32_t Configuration::getUint(string property, uint32_t value)
{
	stringstream s;

	s << value;

	return getUint(property, s.str());
}

uint32_t Configuration::getUint(string property, string value)
{
	prop_vals_map_t::iterator pos;

	pos = config_maps.find(property);

	if((pos != config_maps.end()) && usesMap(property))
	{
		val_map_t::iterator v_pos;

		v_pos = pos->second.find(value);
		if(v_pos != pos->second.end())
		{
			return strtoul((v_pos->second).c_str(), NULL,10);
		}
	}
	
	return 0;
}

vector<uint32_t> Configuration::getUints(string property)
{
	vector<uint32_t> values;

	vector<string> s_values = getStrings(property);

	for(vector<string>::iterator it = s_values.begin(); it != s_values.end(); it++)
	{
		values.push_back(strtoul((*it).c_str(), NULL,10));
	}

	return values;
}


uint32_t Configuration::getHex(string property)
{
	prop_val_map_t::iterator pos;

	pos = config_map.find(property);

	if(pos != config_map.end())
	{
		return strtoul((pos->second).c_str(), NULL,16);
	}
	return 0;
}

string Configuration::getString(string property)
{
	prop_val_map_t::iterator pos;

	pos = config_map.find(property);

	if(pos != config_map.end())
	{
		if(property == CONF_ARCHITECTURE)
		{
			architecture_t arch = (architecture_t)strtoul((pos->second).c_str(), NULL,10);
			switch(arch)
			{
				case CARCORE:
					return CONF_ARCHITECTURE_CARCORE;
				case ARMV6M:
					return CONF_ARCHITECTURE_ARMV6M;
				case UNKNOWN_ARCHICTECTURE:
					return "UNKNOWN_ARCHICTECTURE";
				default:
					assert(false);
			}
		} 
		else if(property == CONF_MEMORY_TYPE)
		{
			mem_type_t m_type = (mem_type_t)strtoul((pos->second).c_str(), NULL,10);
			switch(m_type)
			{
				case BBSISP:
					return "BBSISP";
				case BBSISP_JP:
					return "BBSISP_JP";
				case BBSISP_WCP:
					return "BBSISP_WCP";
				case BBSISP_JP_WCP:
					return "BBSISP_JP_WCP";
				case FSISP_OLD:
					return "FSISP_OLD";
				case FSISP:
					return "FSISP";
				case FSISP_WCP:
					return "FSISP_WCP";
				case DISP:
					return "DISP";
				case ICACHE:
					return "ICACHE";
				case NO_MEM:
					return "NO_MEM";
				case VIVU_TEST:
					return "VIVU_TEST";
				case UNKNOWN_MEM:
					return "UNKNOWN_MEM";
				default:
					assert(false);
			}
		} 
		else if(property == CONF_MEMORY_REPLACEMENT_POLICY)
		{
			replacement_policy_t r_pol = (replacement_policy_t)strtoul((pos->second).c_str(), NULL,10);
			switch(r_pol)
			{
				case FIFO:
					return "FIFO";
				case LRU:
					return "LRU";
				case DIRECT_MAPPED:
					return "DIRECT_MAPPED";
				case STACK:
					return "STACK";
				case UNKNOWN_RP:
					return "UNKNOWN_RP";
				default:
					assert(false);
			}
		}
		else
		{
			return pos->second;
		}
	}
	
	return string("");
}

string Configuration::getString(string property, uint32_t value)
{
	stringstream s;

	s << value;

	return getString(property, s.str());
}

string Configuration::getString(string property, string value)
{
	prop_vals_map_t::iterator pos;

	pos = config_maps.find(property);

	if((pos != config_maps.end()) && usesMap(property))
	{
		val_map_t::iterator v_pos;

		v_pos = pos->second.find(value);
		if(v_pos != pos->second.end())
		{
			return v_pos->second;
		}
	}

	return string("");
}

vector<string> Configuration::getStrings(string property)
{
	vector<string> values;

	prop_vals_map_t::iterator pos;

	pos = config_maps.find(property);

	if((pos != config_maps.end()) && usesMap(property))
	{
		val_map_t vmap = pos->second;

		for(val_map_t::iterator it = vmap.begin(); it != vmap.end(); it++)
		{
			values.push_back(it->second);
		}
	}

	return values;
}

bool Configuration::isCfgAssignmentLine(string line)
{
	return regex_search(line, re_cfg_assignment);
}

bool Configuration::isLogAssignmentLine(string line)
{
	return regex_search(line, re_log_assignment);
}

void Configuration::storeConfigurationEntry(string line)
{
	// buffer log output until logger is set up
	stringstream s;
//	s << "Inspecting line: " << line;
//	log_output.push_back(make_pair(DEBUG, s.str()));

	if(isCfgAssignmentLine(line))
	{

		string line_wo_prefix = regex_replace(line, regex("isptap\\.", regex::icase), "", match_default | format_first_only);
		string line_no_spaces = regex_replace(line_wo_prefix, regex("[[:space:]]"), "", match_default | format_all);
		string line_no_comment = regex_replace(line_no_spaces, regex("(#|//)[[:print:]]*"), "", match_default | format_all);

		uint32_t eq_char = line_no_comment.find("=");

		string property = line_no_comment.substr(0,eq_char);
		string value = line_no_comment.substr(eq_char+1, line_no_comment.length());

//		LOG_DEBUG(logger, "Line wo comments is: |" << line_no_comment << "| property detected: |" << property << "| value: |" << value << "|");

		// buffer log output until logger is set up
		stringstream s;
		s << "Line wo comments is: |" << line_no_comment << "| property detected: |" << property << "| value: |" << value << "|";
		log_output.push_back(make_pair(DEBUG, s.str()));

		// convert architecture from string to architecture_t
		if(property.compare(CONF_ARCHITECTURE) == 0)
		{
			stringstream new_value;
			if(to_upper_copy(value).compare(CONF_ARCHITECTURE_CARCORE) == 0)
			{
				new_value << CARCORE;
			}
			else if(to_upper_copy(value).compare(CONF_ARCHITECTURE_ARMV6M) == 0)
			{
				new_value << ARMV6M;
			}
			else
			{
				// default is carcore
				new_value << UNKNOWN_ARCHICTECTURE;
			}
			value = new_value.str();

			// buffer log output until logger is set up
//			stringstream s;
//			s << "Value is now: " << value;
//			log_output.push_back(make_pair(DEBUG, s.str()));
		}

		// convert memory types from string to mem_type_t
		if(property.compare(CONF_MEMORY_TYPE) == 0)
		{
			stringstream new_value;
			if(to_upper_copy(value).compare(CONF_MEMORY_TYPE_BBSISP) == 0)
			{
				new_value << BBSISP;
			}
			else if(to_upper_copy(value).compare(CONF_MEMORY_TYPE_BBSISP_JP) == 0)
			{
				new_value << BBSISP_JP;
			}
			else if(to_upper_copy(value).compare(CONF_MEMORY_TYPE_BBSISP_WCP) == 0)
			{
				new_value << BBSISP_WCP;
			}
			else if(to_upper_copy(value).compare(CONF_MEMORY_TYPE_BBSISP_JP_WCP) == 0)
			{
				new_value << BBSISP_JP_WCP;
			}
			else if(to_upper_copy(value).compare(CONF_MEMORY_TYPE_FSISP_OLD) == 0)
			{
				new_value << FSISP_OLD;
			}
			else if(to_upper_copy(value).compare(CONF_MEMORY_TYPE_FSISP) == 0)
			{
				new_value << FSISP;
			}
			else if(to_upper_copy(value).compare(CONF_MEMORY_TYPE_FSISP_WCP) == 0)
			{
				new_value << FSISP_WCP;
			}
			else if(to_upper_copy(value).compare(CONF_MEMORY_TYPE_DISP) == 0)
			{
				new_value << DISP;
			}
			else if(to_upper_copy(value).compare(CONF_MEMORY_TYPE_ICACHE) == 0)
			{
				new_value << ICACHE;
			}
			else if(to_upper_copy(value).compare(CONF_MEMORY_TYPE_NOMEM) == 0)
			{
				new_value << NO_MEM;
			}
			else if(to_upper_copy(value).compare(CONF_MEMORY_TYPE_VIVU_TEST) == 0)
			{
				new_value << VIVU_TEST;
			}
			else
			{
				new_value << UNKNOWN_MEM;
			}
			value = new_value.str();

			// buffer log output until logger is set up
//			stringstream s;
//			s << "Value is now: " << value;
//			log_output.push_back(make_pair(DEBUG, s.str()));

		}

		// convert memory types from string to replacement_policy_t
		if(property.compare(CONF_MEMORY_REPLACEMENT_POLICY) == 0)
		{
			stringstream new_value;
			if(to_upper_copy(value).compare(CONF_MEMORY_REPLACEMENT_POLICY_FIFO) == 0)
			{
				new_value << FIFO;
			}
			else if(to_upper_copy(value).compare(CONF_MEMORY_REPLACEMENT_POLICY_LRU) == 0)
			{
				new_value << LRU;
			}
			else if(to_upper_copy(value).compare(CONF_MEMORY_REPLACEMENT_POLICY_DIRECT_MAPPED) == 0)
			{
				new_value << DIRECT_MAPPED;
			}
			else if(to_upper_copy(value).compare(CONF_MEMORY_REPLACEMENT_POLICY_STACK) == 0)
			{
				new_value << STACK;
			}
			else
			{
				new_value << UNKNOWN_RP;
			}
			value = new_value.str();
		
			// buffer log output until logger is set up
//			stringstream s;
//			s << "Value is now: " << value;
//			log_output.push_back(make_pair(DEBUG, s.str()));
		}

		// convert export graph format types from string to export_graph_format_t
		if(property.compare(CONF_EXPORT_FORMAT) == 0)
		{
			stringstream new_value;
			if(to_upper_copy(value).compare(CONF_EXPORT_FORMAT_GRAPHVIZ) == 0)
			{
				new_value << GRAPHVIZ;
			}
			else if(to_upper_copy(value).compare(CONF_EXPORT_FORMAT_GRAPHML) == 0)
			{
				new_value << GRAPHML;
			}
			else
			{
				// default is graphviz
				new_value << GRAPHVIZ;
			}
			value = new_value.str();

			// buffer log output until logger is set up
//			stringstream s;
//			s << "Value is now: " << value;
//			log_output.push_back(make_pair(DEBUG, s.str()));
		}

		// convert metric types from string to analysis_metric_t
		if(property.compare(CONF_USE_METRIC) == 0)
		{
			stringstream new_value;
			if(to_upper_copy(value).compare(CONF_METRIC_WCET) == 0)
			{
				new_value << WCET;
			}
			else if(to_upper_copy(value).compare(CONF_METRIC_MDIC) == 0)
			{
				new_value << MDIC;
			}
			else if(to_upper_copy(value).compare(CONF_METRIC_MPL) == 0)
			{
				new_value << MPL;
			}
			else if(to_upper_copy(value).compare(CONF_METRIC_WCET_RATIO_FILES) == 0)
			{
				new_value << WCET_RATIO_FILES;
			}
			else
			{
				new_value << UNKNOWN_METRIC;
			}
			value = new_value.str();
		
			// buffer log output until logger is set up
//			stringstream s;
//			s << "Value is now: " << value;
//			log_output.push_back(make_pair(DEBUG, s.str()));
		}

		if((property.compare(CONF_LP_SOLVE_PARAMETERS) == 0))
		{
			// add for each - an additional space to support multiple lp_solve parameters
			value = regex_replace(value, regex("-"), " -", match_default | format_all);
			// the timout parameter needs a blank 
			value = regex_replace(value, regex("-timeout", regex::icase), "-timeout ", match_default | format_all);
		}

		if(!usesMap(property))
		{
			prop_val_map_t::iterator pos;
			pos = config_map.find(property);

			if(pos != config_map.end())
			{
				// overwrite the value for this property
				pos->second = value;
			}
			else
			{
				// buffer log output until logger is set up
				stringstream s;
				s <<"Property: " << property << " was not initialized!";
				log_output.push_back(make_pair(WARN, s.str()));

				config_map.insert(pair<string, string>(property, value));
			}
		}
		else
		{
			prop_vals_map_t::iterator pos;
			pos = config_maps.find(property);

			assert(pos != config_maps.end());
			val_map_t *val_map = &pos->second;

			if((property.compare(CONF_OUTPUT_WCET_STEP) == 0) || (property.compare(CONF_OUTPUT_MEM_COST_STEP) == 0))
			{
				sregex_token_iterator i(value.begin(), value.end(), regex("\\|"), -1);
				sregex_token_iterator end;

				while(i != end)
				{
					stringstream s;
					
					string str(*i);
					uint32_t assign_char = str.find(":");
					string value_one = str.substr(0,assign_char);
					string value_two = str.substr(assign_char+1, str.length());

					s << " assignment: " << value_one << " = " << value_two;
					log_output.push_back(make_pair(INFO, s.str()));

					val_map->insert(pair<string, string>(value_one, value_two));

					i++;
				}
			} 
			else if(property.compare(CONF_ENTRY_FUNCTIONS) == 0)
			{
				sregex_token_iterator i(value.begin(), value.end(), regex("\\|"), -1);
				sregex_token_iterator end;

				while(i != end)
				{
					stringstream s;
					
					string str(*i);
					uint32_t assign_char = str.find(":");
					string value_one = str.substr(0,assign_char);
					string value_two = str.substr(assign_char+1, str.length());

					s << " assignment: " << value_one << " = " << value_two;
					log_output.push_back(make_pair(INFO, s.str()));

					val_map->insert(pair<string, string>(value_one, value_two));

					// set the property CONF_ENTRY_FUNCTION if core 0 is selected
					if(value_one.compare("0") == 0)
					{
						prop_val_map_t::iterator pos;
						pos = config_map.find(CONF_ENTRY_FUNCTION);

						if(pos != config_map.end())
						{
							// buffer log output until logger is set up
							stringstream s;
							s <<"Overwritten entry of: " << CONF_ENTRY_FUNCTION << " with value of " << CONF_ENTRY_FUNCTIONS << " 0:" << value_two;
							log_output.push_back(make_pair(WARN, s.str()));
							// overwrite the value for this property
							pos->second = value_two;
						}
						else
						{
							// buffer log output until logger is set up
							stringstream s;
							s <<"Property: " << CONF_ENTRY_FUNCTION << " was not initialized!";
							log_output.push_back(make_pair(WARN, s.str()));

							config_map.insert(pair<string, string>(CONF_ENTRY_FUNCTION, value_two));
						}
					}

					i++;
				}
			}
			else if(property.compare(CONF_HRT_CORES) == 0)
			{
				sregex_token_iterator i(value.begin(), value.end(), regex("\\|"), -1);
				sregex_token_iterator end;

				uint32_t entry_nr = 0;

				while(i != end)
				{
					stringstream s;
					
					string value(*i);
					stringstream st;
					st << entry_nr;
					string entry = st.str();

					s << " assignment: " << entry << " = " << value;
					log_output.push_back(make_pair(INFO, s.str()));

					val_map->insert(pair<string, string>(entry, value));

					i++;
					entry_nr++;
				}
			}
		}
	}
	else
	{
//		LOG_DEBUG(logger, "Wrong line format for config property assignment: " << line);
	}
}

void Configuration::addLogProps(string logprop_filename)
{
	ifstream logprop_file;
	logprop_file.open(logprop_filename.c_str());

	assert(logprop_file.is_open());

	while(!logprop_file.eof())
	{
		string str;
		getline(logprop_file, str);

		if(isLogAssignmentLine(str))
		{
			string line_no_spaces = regex_replace(str, regex("[[:space:]]"), "", match_default | format_all);
			string line_no_comment = regex_replace(line_no_spaces, regex("(#|//)[[:print:]]*"), "", match_default | format_all);

			uint32_t eq_char = line_no_comment.find("=");
			string property = line_no_comment.substr(0,eq_char);
			string value = line_no_comment.substr(eq_char+1, line_no_comment.length());

			logProp.setProperty(property, value);
		}
	}

	logprop_file.close();
}

bool Configuration::usesMap(string property)
{
	if((property.compare(CONF_OUTPUT_WCET_STEP) == 0) || (property.compare(CONF_OUTPUT_MEM_COST_STEP) == 0) || (property.compare(CONF_ENTRY_FUNCTIONS) == 0) || (property.compare(CONF_HRT_CORES) == 0))
	{
		return true;
	}
	return false;
}


void Configuration::setProperty(const string property, string value)
{
	if(!usesMap(property))
	{
		prop_val_map_t::iterator pos;
		pos = config_map.find(property);

		if(pos != config_map.end())
		{
			// overwrite the value for this property
			pos->second = value;

			stringstream s;
			s <<"Overwriting property: " << property << " with " << value;
			log_output.push_back(make_pair(WARN, s.str()));
		}
		else
		{
			// buffer log output until logger is set up
			stringstream s;
			s <<"Property: " << property << " was not initialized!";
			log_output.push_back(make_pair(WARN, s.str()));

			config_map.insert(pair<string, string>(property, value));
		}
	}
	else
	{
		// Properties with maps are not yet supported for manual overwrite
		assert(false);
	}
}

void Configuration::setProperty(const string property, bool value)
{
	setProperty(property, ((value)?(string("1")):(string("0"))));
}

string Configuration::getHelpMessage(void)
{
	ConsoleStringPrinter csp;

	assert(!config_properties.empty()); // ensure that the config_properties was initialised

	stringstream s;
	s << endl;
	s << "Configuration file help" << endl;
	s << "=======================" << endl;
	s << csp.chopToLineLength("This tool is usually started with a configuration file that provides all necessary information about the program under analysis and the modelled architecture including the memory configuration. Furthermore, the configuration file allows to set up logging and export options.") << endl;
	s << endl;
	s << csp.chopToLineLength("The configuration file supports the following properties. In the configuration files the prefix \"isptap.\" has to be used for each property definition.") << endl;
	s << endl;

	// print the help messages of the registered properties
	for(vector<help_message_t>::iterator it = config_properties.begin(); it != config_properties.end(); ++it)
	{
		string p = it->property + ": (default:" + ((it->default_value.compare("")!=0)?(it->default_value):("--none--")) + ") " + it->help_message;
		s << csp.chopToLineLength(p, " ") << endl;
	}

	return s.str();
}
