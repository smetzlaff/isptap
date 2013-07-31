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
#include "report_generator.hpp"

ReportGenerator::ReportGenerator(string file_name, bool file_append, mem_type_t memory, replacement_policy_t r_policy, analysis_metric_t use_metric, bool use_istats) : report_append(file_append), blocks_assigned_solution_type(SolutionNotCalculated), functions_assigned_solution_type(SolutionNotCalculated), wc_cost(0), wc_cost_solution_type(SolutionNotCalculated), wc_cost_improved(0),  wc_cost_improved_solution_type(SolutionNotCalculated), mem_cost(0), mem_cost_solution_type(SolutionNotCalculated), mem_size(0), mem_size_used(0), dfa_state_count(0), dfa_state_representation_count(0), dfa_mem_used(0), dfa_maintained_references(0), replacement_policy(r_policy), metric(use_metric), generate_instruction_statistics(use_istats)
{
	report_file = file_name;
	memory_type = memory;

	cache_hm_stats.hits=0;
	cache_hm_stats.misses=0;
	cache_hm_stats.ncs=0;
}

ReportGenerator::~ReportGenerator()
{
	blocks_assigned.clear();
	functions_assigned.clear();
}

void ReportGenerator::setCodeSize(uint32_t codeSize)
{
	code_size = codeSize;
}

void ReportGenerator::setBlockAssignment(vector<uint32_t> assigned_blocks)
{
	blocks_assigned = assigned_blocks;
	blocks_assigned_solution_type = OptimalSolution;
}


void ReportGenerator::setBlockAssignment(vector<uint32_t> assigned_blocks, lp_solution_t solution_type)
{
	blocks_assigned = assigned_blocks;
	blocks_assigned_solution_type = solution_type;
}

void ReportGenerator::setFunctionAssignment(vector<addr_label_t> assigned_functions)
{
	functions_assigned = assigned_functions;
	functions_assigned_solution_type = OptimalSolution;
}

void ReportGenerator::setWCCostValue(uint64_t wc_cost_value)
{
	wc_cost = wc_cost_value;
	wc_cost_solution_type = OptimalSolution;
}

void ReportGenerator::setWCCostValue(uint64_t wc_cost_value, lp_solution_t solution_type)
{
	wc_cost = wc_cost_value;
	wc_cost_solution_type = solution_type;
}

void ReportGenerator::setWCCostValue(uint64_t wc_cost_value, uint64_t wc_cost_value_improved)
{
	wc_cost = wc_cost_value;
	wc_cost_solution_type = OptimalSolution;
	wc_cost_improved = wc_cost_value_improved;
	wc_cost_improved_solution_type = OptimalSolution;
}

void ReportGenerator::setWCCostValue(uint64_t wc_cost_value, uint64_t wc_cost_value_improved, lp_solution_t solution_type, lp_solution_t solution_type_improved)
{
	wc_cost = wc_cost_value;
	wc_cost_solution_type = solution_type;
	wc_cost_improved = wc_cost_value_improved;
	wc_cost_improved_solution_type = solution_type_improved;
}

void ReportGenerator::setMemCostValue(uint64_t mem_cost_value)
{
	mem_cost = mem_cost_value;
	mem_cost_solution_type = OptimalSolution;
}

void ReportGenerator::setMemCostValue(uint64_t mem_cost_value, lp_solution_t solution_type)
{
	mem_cost = mem_cost_value;
	mem_cost_solution_type = solution_type;
}

void ReportGenerator::setMemSize(uint32_t size, uint32_t used)
{
	mem_size = size;
	mem_size_used = used;
}

void ReportGenerator::setMemSize(uint32_t size)
{
	mem_size = size;
}


void ReportGenerator::setDFAStatistics(uint64_t state_count, uint64_t state_representation_count, uint64_t used_mem, uint64_t maintained_references)
{
	dfa_state_count = state_count;
	dfa_state_representation_count = state_representation_count;
	dfa_mem_used = used_mem;
	dfa_maintained_references = maintained_references;
}

void ReportGenerator::setCacheHMStats(cache_hm_stat_t cache_stat)
{
	cache_hm_stats = cache_stat;
}


void ReportGenerator::setWCPathInstructionStatistics(instr_stat_t istats)
{
	i_stats = istats;
}

void ReportGenerator::generate_header(void)
{
	ofstream report;
	if(report_append)
	{
		report.open(report_file.c_str(), ios_base::app);
	}
	else
	{
		report.open(report_file.c_str());
	}

	assert(report.is_open());

	if(report_append)
	{
		report << endl;
	}

	report << "code size: " << code_size << "\t\t";

	if(memory_type == BBSISP)
	{
		report << "memory: BBSISP";
	}
	else if(memory_type == BBSISP_JP)
	{
		report << "memory: BBSISP_JP";
	}
	else if(memory_type == BBSISP_WCP)
	{
		report << "memory: BBSISP_WCP";
	}
	else if(memory_type == BBSISP_JP_WCP)
	{
		report << "memory: BBSISP_JP_WCP";
	}
	else if(memory_type == FSISP)
	{
		report << "memory: FSISP";
	}
	else if(memory_type == FSISP_WCP)
	{
		report << "memory: FSISP_WCP";
	}
	else if(memory_type == FSISP_OLD)
	{
		report << "memory: FSISP_OLD";
	}
	else if (memory_type == DISP)
	{
		report << "memory: DISP";
	}
	else if (memory_type == ICACHE)
	{
		report << "memory: ICACHE";
	}
	else
	{
		assert(false);
	}

	if(IS_DYNAMIC_MEM(memory_type))
	{
		report << "\tReplacement policy: ";
		switch(replacement_policy)
		{
			case FIFO:
				{
					report << "FIFO";
					break;
				}
			case LRU:
				{
					report << "LRU";
					break;
				}
			case DIRECT_MAPPED:
				{
					report << "DIRECT_MAPPED";
					break;
				}
			case STACK:
				{
					report << "STACK-BASED";
					break;
				}
			default:
				{
					report << "UNKNOWN";
				}
		}
		report << " DFA Stats:\t\t";
	}

	if(generate_instruction_statistics)
	{
		report << "\tInstruction stats:";
	}

	report << endl;

	if(metric == WCET)
	{
		report << "Size:\tWCET:\tSTwcet:\tMemCost:\tSTmem:";
		if(IS_DYNAMIC_MEM(memory_type))
		{
			report << "\tDFAStates:\tDFAStateRepresentations:\tDFAUsedMem:\tDFAMaintainedReferences:";
		}

		if(memory_type == ICACHE)
		{
			report << "\tICWCPHits:\tICWCPMisses:\tICWCPNcs:";
		}

		if(generate_instruction_statistics)
		{
			report << "\tALL:\tN:\tBR:\tCBR:\tCALL:\tRET:\tLD:\tST:\tSY:\tOTH:\tDBG:\tNC:";
		}


		if((memory_type == BBSISP) || (memory_type == BBSISP_JP) || (memory_type == BBSISP_WCP) || (memory_type == BBSISP_JP_WCP))
		{
			report << "\tUsedSize:\tSTassignment:\tAssignedBlocks:";
		}
		else if((memory_type == FSISP) || (memory_type == FSISP_WCP) || (memory_type == FSISP_OLD))
		{
			report << "\tUsedSize:\tAssignedFunctions:";
		}

		report << endl;
	}
	else
	{
		assert(false); // not implemented yet
	}

	report.close();
}


void ReportGenerator::generate_line(void)
{
	ofstream report;
	// always append since a line is written
	report.open(report_file.c_str(), ios_base::app);

	assert(report.is_open());


	if(metric == WCET)
	{
		report << dec << mem_size << "\t" << ((wc_cost_improved!=0)?(wc_cost_improved):(wc_cost)) << "\t" << ((wc_cost_improved!=0)?(printSolutionType(wc_cost_improved_solution_type)):(printSolutionType(wc_cost_solution_type))) << "\t" << mem_cost << "\t" << printSolutionType(mem_cost_solution_type);
		
		if(IS_DYNAMIC_MEM(memory_type))
		{
			report << "\t" << dfa_state_count << "\t" << dfa_state_representation_count << "\t" << dfa_mem_used << "\t" << dfa_maintained_references;
		}

		if(memory_type == ICACHE)
		{
			report << "\t" << cache_hm_stats.hits << "\t" << cache_hm_stats.misses << "\t" << cache_hm_stats.ncs;
		}

		if(generate_instruction_statistics)
		{
			report << "\t" << i_stats.instructions << "\t" << i_stats.arithmetic_instructions << "\t" << i_stats.branch_instructions << "\t" << i_stats.condbranch_instructions << "\t" << i_stats.call_instructions << "\t" << i_stats.return_instructions << "\t" << i_stats.load_instructions << "\t" << i_stats.store_instructions << "\t" << i_stats.sync_instructions << "\t" << i_stats.other_instructions << "\t" << i_stats.debug_instructions << "\t" << i_stats.unknown_instructions;
		}

		if((memory_type == BBSISP) || (memory_type == BBSISP_JP) || (memory_type == BBSISP_WCP) || (memory_type == BBSISP_JP_WCP))
		{
			report << "\t" << mem_size_used << "\t";
			report << printSolutionType(blocks_assigned_solution_type) << "\t";
			for(uint32_t i=0; i < blocks_assigned.size(); i++)
			{
				report << "0x" << hex << blocks_assigned[i] << " ";
			}
		}
		else if((memory_type == FSISP) || (memory_type == FSISP_WCP) || (memory_type == FSISP_OLD))
		{
			report << "\t" << mem_size_used << "\t";
			for(uint32_t i=0; i < functions_assigned.size(); i++)
			{
				report << functions_assigned[i].label << " ";
			}
		}

		report <<endl;
	}
	else
	{
		assert(false); // not implemented yet
	}

	// TODO: Add also wcpath instruction statistics.

	report.close();
}


void ReportGenerator::generate(void)
{
	ofstream report;
	if(report_append)
	{
		report.open(report_file.c_str(), ios_base::app);
	}
	else
	{
		report.open(report_file.c_str());
	}

	assert(report.is_open());


	if(report_append)
	{
		report << endl;
	}

	report << "Code size: " << code_size << " ";

	if((memory_type == BBSISP) || (memory_type == BBSISP_JP) || (memory_type == BBSISP_WCP) || (memory_type == BBSISP_JP_WCP))
	{
		string s_mem;

		switch(memory_type)
		{
			case BBSISP:
				s_mem = "BBSISP";
				break;
			case BBSISP_JP:
				s_mem = "BBSISP_JP";
				break;
			case BBSISP_WCP:
				s_mem = "BBSISP_WCP";
				break;
			case BBSISP_JP_WCP:
				s_mem = "BBSISP_JP_WCP";
				break;
			default:
				assert(false);
		}
		report << "Memory: " << s_mem << ", Size: " << mem_size <<  " used: " << mem_size_used << endl;
		report << "Block assignment: " << endl;
		for(uint32_t i=0; i < blocks_assigned.size(); i++)
		{
			report << "0x" << hex << blocks_assigned[i] << "\t";
		}
		report << endl;



	}
	else if((memory_type == FSISP) || (memory_type == FSISP_WCP) || (memory_type == FSISP_OLD))
	{
		string s_mem;
		switch(memory_type)
		{
			case FSISP:
				s_mem = "FSISP";
				break;
			case FSISP_WCP:
				s_mem = "FSISP_WCP";
				break;
			case FSISP_OLD:
				s_mem = "FSISP_OLD";
				break;
			default:
				assert(false);
		}
		report << "Memory: " << s_mem << ", Size: " << mem_size << " used: " << mem_size_used << endl;
		report << "Function assignment: " << endl;
		for(uint32_t i=0; i < functions_assigned.size(); i++)
		{
			report << "0x" << hex << functions_assigned[i].address << " " << functions_assigned[i].label << "\t";
		}
		report << endl;


	}
	else if (memory_type == DISP)
	{
		report << "Memory: DISP, Size: " << mem_size << endl;
	}
	else if (memory_type == ICACHE)
	{
		report << "Memory: ICACHE, Size: " << mem_size << endl;
	}
	else
	{
		report << endl;
		//assert(false);
	}

	switch(metric)
	{
		case WCET:
		case WCET_RATIO_FILES:
			{
				if(wc_cost_improved != 0)
				{
					report << "WCET is: " << dec << wc_cost << " cycles (" << printSolutionType(wc_cost_solution_type) << "). Improved WCET is: " << wc_cost_improved << " cycles (" << printSolutionType(wc_cost_improved_solution_type) << "). Improvement: " << (wc_cost - wc_cost_improved) << " cycles";
				}
				else
				{
					report << "WCET is: " << dec << wc_cost << " (" << printSolutionType(wc_cost_solution_type)  << ")";
				}

				if(mem_cost != 0)
				{
					report << " MEM cost is: " << dec << mem_cost << " (" << printSolutionType(mem_cost_solution_type)  << ")";
				}
				break;
			}
		case MDIC:
			{
				report << "WCIC is: " << dec << wc_cost << "instructions (" << printSolutionType(wc_cost_solution_type)  << ")."; // number of instructions on the longest path (with dynamic instruciton count as metric)
				break;
			}
		case MPL:
			{
				report << "WCPL is: " << dec << wc_cost << " byte (" << printSolutionType(wc_cost_solution_type)  << "). "; // longest path
				break;
			}
		default:
			{
				assert(false);
			}
	}

	if(IS_DYNAMIC_MEM(memory_type))
	{
		report << endl << "DFA state count: " << dfa_state_count << " DFA state representations count: " << dfa_state_representation_count << " DFA mem used: " << dfa_mem_used << " bytes" << " DFA maintained referecnes: " << dfa_maintained_references;
	}

	if(memory_type == ICACHE)
	{
		report << endl << "Cache hits on WCP: " << cache_hm_stats.hits << " Cache misses on WCP: " << cache_hm_stats.misses << " Cache ncs on WCP: " << cache_hm_stats.ncs;
	}

	report << endl;

	if(generate_instruction_statistics)
	{
		report << "Istats:\nInstr:\tN:\tBR:\tCBR:\tCALL:\tRET:\tLD:\tST:\tSY:\tOTH:\tDBG:\tNC:" << endl;
		report << i_stats.instructions << "\t" << i_stats.arithmetic_instructions << "\t" << i_stats.branch_instructions << "\t" << i_stats.condbranch_instructions << "\t" << i_stats.call_instructions << "\t" << i_stats.return_instructions << "\t" << i_stats.load_instructions << "\t" << i_stats.store_instructions << "\t" << i_stats.sync_instructions << "\t" << i_stats.other_instructions << "\t" << i_stats.debug_instructions << "\t" << i_stats.unknown_instructions;
	}

	report << endl;

	report.close();
}

void ReportGenerator::setMappingFileName(string mappingFileName)
{
	size_t dot_pos = mappingFileName.find_first_of(".", 0);

	if(dot_pos == string::npos)
	{
		mapping_file_stub = mappingFileName;
	}
	else
	{
		mapping_file_stub = mappingFileName.substr(0, dot_pos);
	}
//	LOG_DEBUG(logger, "Setting static mapping file: " << mapping_file_stub);
}

void ReportGenerator::generateStaticFunctionMapping(void)
{
	if((memory_type == FSISP) || (memory_type == FSISP_WCP) || (memory_type == FSISP_OLD))
	{
		char mapping_file[100];
		sprintf(mapping_file, "%s_%i.cfg", mapping_file_stub.c_str(), mem_size);
		char hs_file[100];
		sprintf(hs_file, "%s_%i_start.h", mapping_file_stub.c_str(), mem_size);
		char he_file[100];
		sprintf(he_file, "%s_%i_end.h", mapping_file_stub.c_str(), mem_size);

		ofstream mapping_stream;
		ofstream header_start_stream, header_end_stream;
		mapping_stream.open(mapping_file);
		header_start_stream.open(hs_file);
		header_end_stream.open(he_file);
//		LOG_DEBUG(logger, "Opening mapping file for write: " << mapping_file);

		assert(mapping_stream.is_open());
		assert(header_start_stream.is_open());
		assert(header_end_stream.is_open());
		for(uint32_t i=0; i < functions_assigned.size(); i++)
		{
			mapping_stream << functions_assigned[i].label << endl;
			header_start_stream << "#define " << functions_assigned[i].label <<  " __attribute__ ((section (\".sisp\"))) " << functions_assigned[i].label << endl;
			header_end_stream << "#undef " << functions_assigned[i].label <<  endl;
		}
		mapping_stream.close();
		header_start_stream.close();
		header_end_stream.close();
	}
}

string ReportGenerator::printSolutionType(lp_solution_t solution_type)
{
	switch(solution_type)
	{
		case OptimalSolution:
			{
				return "O";
			}
		case SuboptimalSolution:
			{
				return "S";
			}
		case ErrorWhileSolving:
			{
				return "E";
			}
		default:
			{
				return "?";
			}
	}
	return "?";
}
