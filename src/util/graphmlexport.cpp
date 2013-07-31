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
#include "graphmlexport.hpp"


GraphMLExport::GraphMLExport(string filename)
{
	export_filename = filename;
}

GraphMLExport::~GraphMLExport()
{
	if(export_file_stream.is_open())
	{
		export_file_stream.close();
	}
}

bool GraphMLExport::exportGraph(ControlFlowGraph cfg)
{

	if(!export_file_stream.is_open())
	{
		export_file_stream.open(export_filename.c_str());
	}
	if(!export_file_stream.is_open())
	{
		return false;
	}

	dynamic_properties dp;
    dp.property("label", get(startaddr_t(), cfg));
    dp.property("end_label", get(endaddr_t(), cfg));
    dp.property("call_label", get(calllabel_t(), cfg));
//    dp.property("description", get(bbcode_t(), cfg));
    dp.property("weigth", get(cost_t(), cfg));
    dp.property("circ", get(circ_t(), cfg));

	write_graphml(export_file_stream, cfg, dp, true);

	return true;
}

bool GraphMLExport::exportGraph(FunctionCallGraph fcg)
{

	if(!export_file_stream.is_open())
	{
		export_file_stream.open(export_filename.c_str());
	}
	if(!export_file_stream.is_open())
	{
		return false;
	}

	dynamic_properties dp;
    dp.property("label", get(function_name_t(), fcg));
    dp.property("address", get(function_addr_t(), fcg));
    dp.property("callers", get(caller_points_t(), fcg));

	write_graphml(export_file_stream, fcg, dp, true);

	return true;
}

bool GraphMLExport::exportGraph(MemoryStateGraph msg)
{

	if(!export_file_stream.is_open())
	{
		export_file_stream.open(export_filename.c_str());
	}
	if(!export_file_stream.is_open())
	{
		return false;
	}

	dynamic_properties dp;
    dp.property("cfg_vertex", get(cfg_vertex_t(), msg));

	write_graphml(export_file_stream, msg, dp, true);

	return true;
}

bool GraphMLExport::exportGraph(AbsStackMemGraph asmg)
{

	if(!export_file_stream.is_open())
	{
		export_file_stream.open(export_filename.c_str());
	}
	if(!export_file_stream.is_open())
	{
		return false;
	}

	dynamic_properties dp;
    dp.property("cfg_vertex", get(name_t(), asmg));

	write_graphml(export_file_stream, asmg, dp, true);

	return true;
}

