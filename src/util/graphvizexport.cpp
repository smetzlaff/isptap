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
#include "graphvizexport.hpp"


GraphVizExport::GraphVizExport(string filename)
{
	export_filename = filename;
}

GraphVizExport::~GraphVizExport()
{
	if(export_file_stream.is_open())
	{
		export_file_stream.close();
	}
}


bool GraphVizExport::exportGraph(ControlFlowGraph cfg)
{

	if(!export_file_stream.is_open())
	{
		export_file_stream.open(export_filename.c_str());
	}
	if(!export_file_stream.is_open())
	{
		return false;
	}

	write_graphviz(export_file_stream, cfg, make_label_writer(get(startaddrstring_t(), cfg)), make_label_writer(get(cost_t(),cfg)));
 
	return true;
}

// TODO: Write universial export function, where the properties can be defined by user
bool GraphVizExport::exportGraph(ControlFlowGraph cfg, startaddrstring_t,  edgename_t)
{
	if(!export_file_stream.is_open())
	{
		export_file_stream.open(export_filename.c_str());
	}
	if(!export_file_stream.is_open())
	{
		return false;
	}

	write_graphviz(export_file_stream, cfg, make_label_writer(get(startaddrstring_t(), cfg)), make_label_writer(get(edgename_t(),cfg)));
 
	return true;

}

bool GraphVizExport::exportGraph(FunctionCallGraph fcg)
{

	if(!export_file_stream.is_open())
	{
		export_file_stream.open(export_filename.c_str());
	}
	if(!export_file_stream.is_open())
	{
		return false;
	}

	write_graphviz(export_file_stream, fcg, make_label_writer(get(function_name_t(), fcg)), make_label_writer(get(caller_points_t(),fcg)));
 
	return true;
}

bool GraphVizExport::exportGraph(MemoryStateGraph msg)
{

	if(!export_file_stream.is_open())
	{
		export_file_stream.open(export_filename.c_str());
	}
	if(!export_file_stream.is_open())
	{
		return false;
	}

	write_graphviz(export_file_stream, msg, make_label_writer(get(cfg_vertex_t(), msg)), make_label_writer(get(edge_id_t(),msg)));
 
	return true;
}

bool GraphVizExport::exportGraph(AbsStackMemGraph asmg)
{

	if(!export_file_stream.is_open())
	{
		export_file_stream.open(export_filename.c_str());
	}
	if(!export_file_stream.is_open())
	{
		return false;
	}

	write_graphviz(export_file_stream, asmg, make_label_writer(get(name_t(), asmg)), make_label_writer(get(asm_id_t(),asmg)));
 
	return true;
}
