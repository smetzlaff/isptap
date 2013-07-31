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
#include "abs_stack_state.hpp"


LoggerPtr AbsStackMemState::logger(Logger::getLogger("AbsStackMemState"));


AbsStackMemState::AbsStackMemState()
{
}


AbsStackMemState::AbsStackMemState(uint32_t memsize, uint32_t blocksize, FunctionCallGraphObject* fcgo, analysis_type_t analysistype)
{
	mem_size = memsize;
	block_size = blocksize;
	functions = fcgo;
	analysis = analysistype;

	
	setGraphProperties();

	empty_graph = true;
}


AbsStackMemState::AbsStackMemState(const AbsStackMemState& copy)
{
	(*this) = copy;

	setGraphProperties();
}

AbsStackMemState::~AbsStackMemState()
{
}

void AbsStackMemState::exportGraph(string node_id)
{
	Configuration *conf = Configuration::getInstance();

	switch((export_graph_format_t)conf->getUint(CONF_EXPORT_FORMAT))
	{
		case GRAPHML:
			{

				string exportfile_graphml("isptap_asm_for_node_");
				exportfile_graphml += node_id;
				exportfile_graphml += ".graphml";

				GraphMLExport graphml_export(exportfile_graphml);

				if(graphml_export.exportGraph(asmg))
				{
					LOG_DEBUG(logger, "Exported graph for node " << node_id << " to " << exportfile_graphml);
				}
				else
				{
					LOG_WARN(logger, "Export of graph for node " << node_id << " to " << exportfile_graphml << " failed.");
				}
				break;
			}

		case GRAPHVIZ:
			{
				string exportfile_graphviz("isptap_asm_for_node_");
				exportfile_graphviz += node_id;
				exportfile_graphviz += ".graphviz";

				GraphVizExport graphviz_export(exportfile_graphviz);

				if(graphviz_export.exportGraph(asmg))
				{
					LOG_DEBUG(logger, "Exported graph for node " << node_id << " ASMG to " << exportfile_graphviz);
				}
				else
				{
					LOG_WARN(logger, "Export of graph for node " << node_id << " ASMG to " << exportfile_graphviz << " failed.");
				}
				break;
			}
		default:
			LOG_ERROR(logger, "No valid export format defined");
	}



}

void AbsStackMemState::insertFunction(uint32_t function_addr, activation_type_t activation, uint32_t previous_function)
{
	if(!empty_graph)
	{
		ASMVertex active = getNodeForFunction(previous_function);

		if(activation == CALL)
		{
			vector<ASMVertex> successors = getSuccessors(active);
			if(successors.size() == 0)
			{
				ASMVertex new_node = addSuccessorNode(active, function_addr);
				while(getSizeOfBwPath(new_node) > mem_size)
				{
					evictStartNodeFromPath(new_node);
				}
			}
			else if(successors.size() == 1)
			{
				bool extended_used_memory;
				ASMVertex new_node;
				tie(new_node, extended_used_memory) = overwriteFwNodes(active, successors.front(), function_addr);

				if(extended_used_memory)
				{
					while(getSizeOfBwPath(new_node) > mem_size)
					{
						evictStartNodeFromPath(new_node);
					}
				}
			}
			else
			{
				// not implemented
				assert(false);
			}
		}
		else if(activation == RETURN)
		{
			vector<ASMVertex> predecessors = getPredecessors(active);
			if(predecessors.size() == 0)
			{
				ASMVertex new_node = addPredecessorNode(active, function_addr);
				while(getSizeOfFwPath(new_node) > mem_size)
				{
					evictEndNodeFromPath(new_node);
				}
			}
			else if(predecessors.size() == 1)
			{
				bool extended_used_memory;
				ASMVertex new_node;
				tie(new_node, extended_used_memory) = overwriteBwNodes(active, predecessors.front(), function_addr);

				if(extended_used_memory)
				{
					while(getSizeOfFwPath(new_node) > mem_size)
					{
						evictEndNodeFromPath(new_node);
					}
				}
			}
			else
			{
				// not implemented
				assert(false);
			}
		}
		else
		{
			assert(false);
		}
	}
	else
	{
		addNode(function_addr, getFunctionMemSize(function_addr));
		empty_graph = false;
	}
}


void AbsStackMemState::activateFunction(uint32_t function_addr, activation_type_t UNUSED_PARAMETER(activation), uint32_t previous_function)
{
	assert(isInState(function_addr) && isInState(previous_function));

	// the activation of a function deletes all possible parallel branches from memory representation!
}

AbsStackMemState AbsStackMemState::operator+(AbsStackMemState const other)
{
	return this->join(other);
}

AbsStackMemState AbsStackMemState::join(AbsStackMemState const other)
{
	if(analysis == other.analysis)
	{
		if(analysis == MUST)
		{
			// DO THE MUST JOIN
		}
		else if(analysis == MAY)
		{
			// DO THE MAY JOIN
		}
		else
		{
			assert(false);
		}
	}
	else
	{
		assert(false);
	}
	return *this;
}



uint32_t AbsStackMemState::getFunctionMemSize(uint32_t address)
{
	uint32_t f_size = functions->getFunctionSize(address);

	// no function can be found for this entry
	if(f_size == 0)
	{
		LOG_ERROR(logger, "No function found for address ... stopping.");
		assert(false);
	}


	uint32_t f_size_in_mem = ((f_size/block_size)*block_size) + ((f_size % block_size == 0)?(0):(block_size));

	return f_size_in_mem;
}


void AbsStackMemState::setGraphProperties(void)
{
	funcAddrNProp = get(function_addr_t(), asmg);
	funcSizeNProp = get(function_size_t(), asmg);
	nameNProp = get(name_t(), asmg);

	edgeIdEProp = get(asm_id_t(), asmg);
}

ASMVertex AbsStackMemState::getNodeForFunction(uint32_t func_addr)
{
	asmgVertexIter vp;
    for (vp = vertices(asmg); vp.first != vp.second; ++vp.first)
	{
		if(get(funcAddrNProp, *vp.first) == func_addr)
		{
			return *vp.first;
		}
	}
	assert(false);
	return ASMVertex();

}


bool AbsStackMemState::isInState(uint32_t func_addr)
{
	bool is_in_state = false;
	asmgVertexIter vp;
    for (vp = vertices(asmg); vp.first != vp.second; ++vp.first)
	{
		if(get(funcAddrNProp, *vp.first) == func_addr)
		{
			is_in_state = true;
		}
	}


	return is_in_state;

}

uint32_t AbsStackMemState::getSizeOfFwPath(ASMVertex start)
{
	ASMVertex node = start;
	uint32_t size = 0;
	size += get(funcSizeNProp, node);
	vector<ASMVertex> successors;
	ASMEdge e;

	bool finished = false;
	while(out_degree(node, asmg) > 0 && !finished)
	{
		successors = getSuccessors(node);
		if(successors.size() == 1)
		{
			size += get(funcSizeNProp, successors.front());
			node = successors.front();
		}
		else
		{
			uint32_t m_path_size;
			if(analysis == MUST)
			{ 
				m_path_size = 0;
			}
			else 
			{
				m_path_size = (uint32_t)-1;
			}
			for(uint32_t i=0; i < successors.size(); i++)
			{
				uint32_t path_size = getSizeOfFwPath(successors[i]);
				if(analysis == MUST && path_size > m_path_size)
				{
					m_path_size = path_size;
				}
				else if(analysis == MAY && path_size < m_path_size)
				{
					m_path_size = path_size;
				}

			}
			size+= m_path_size;
			finished = true;
		}
	}
	return size;
}


uint32_t AbsStackMemState::getSizeOfBwPath(ASMVertex start)
{
	ASMVertex node = start;
	uint32_t size = 0;
	size += get(funcSizeNProp, node);
	vector<ASMVertex> predecessors;
	ASMEdge e;

	bool finished = false;
	while(in_degree(node, asmg) > 0 && !finished)
	{
		predecessors = getPredecessors(node);
		if(predecessors.size() == 1)
		{
			size += get(funcSizeNProp, predecessors.front());
			node = predecessors.front();
		}
		else
		{
			uint32_t m_path_size;
			if(analysis == MUST)
			{ 
				m_path_size = 0;
			}
			else 
			{
				m_path_size = (uint32_t)-1;
			}

			for(uint32_t i = 0; i<predecessors.size(); i++) 
			{
				uint32_t path_size = getSizeOfBwPath(predecessors[i]);
				if(analysis == MUST && path_size > m_path_size)
				{
					m_path_size = path_size;
				}
				else if(analysis == MAY && path_size < m_path_size)
				{
					m_path_size = path_size;
				}

			}
			size+= m_path_size;
			finished = true;
		}
	}
	return size;
}

vector<ASMVertex> inline AbsStackMemState::getSuccessors(ASMVertex node)
{
	vector<ASMVertex> successors;
	ASMEdge e;


	if(out_degree(node, asmg) == 0)
	{
		// empty vector
		return successors;
	}

	for(asmgOutEdgeIter epo = out_edges(node, asmg); (epo.first != epo.second); ++epo.first) 
	{
		e = *epo.first;
		successors.push_back(target(e,asmg));
	}

	return successors;
}

vector<ASMVertex> inline AbsStackMemState::getPredecessors(ASMVertex node)
{
	vector<ASMVertex> predecessors;
	ASMEdge e;

	if(in_degree(node, asmg) == 0)
	{
		// empty vector
		return predecessors;
	}

	for(asmgInEdgeIter epi = in_edges(node, asmg); (epi.first != epi.second); ++epi.first) 
	{
		e = *epi.first;
		predecessors.push_back(source(e,asmg));
	}

	return predecessors;
}


pair<ASMVertex, bool> AbsStackMemState::overwriteFwNodes(ASMVertex active, ASMVertex evictNode, uint32_t function_addr)
{
	uint32_t function_size = getFunctionMemSize(function_addr);

	uint32_t evict_size = 0;
	bool end_of_path = false;

	ASMVertex node = evictNode;
	while(evict_size < function_size && !end_of_path)
	{
		evict_size += get(funcSizeNProp, node);

		vector<ASMVertex> predecessors = getPredecessors(node);

		assert(predecessors.size() <= 1); // currently only one is supported

		deleteNode(node);
		if(predecessors.size() == 0)
		{
			end_of_path = true;
		}
		else if(predecessors.size() == 1)
		{
			node = predecessors.front();
		}
		else
		{
			// do not know what then
			assert(false);
		}

	}

	ASMVertex new_func = addNode(function_addr, function_size);
	connectNodes(active, new_func);


	if((evict_size > function_size) && !end_of_path)
	{
		ASMVertex buffer_node = addNode(0, function_size-evict_size);
	
		// connect buffer to inserted one
		connectNodes(new_func, buffer_node);
		// connect the buffer to the folloing nodes
		connectNodes(buffer_node, node);
	}

	// true if the memory size may exceeded
	return make_pair(new_func, end_of_path);


}


pair<ASMVertex, bool> AbsStackMemState::overwriteBwNodes(ASMVertex active, ASMVertex evictNode, uint32_t function_addr)
{
	uint32_t function_size = getFunctionMemSize(function_addr);

	uint32_t evict_size = 0;
	bool begin_of_path = false;

	ASMVertex node = evictNode;
	while(evict_size < function_size && !begin_of_path)
	{
		evict_size += get(funcSizeNProp, node);

		vector<ASMVertex> successors = getSuccessors(node);

		assert(successors.size() <= 1); // currently only one is supported

		deleteNode(node);
		if(successors.size() == 0)
		{
			begin_of_path = true;
		}
		else if(successors.size() == 1)
		{
			node = successors.front();
		}
		else
		{
			// do not know what then
			assert(false);
		}

	}

	ASMVertex new_func = addNode(function_addr, function_size);
	connectNodes(new_func, active);


	if((evict_size > function_size) && !begin_of_path)
	{
		ASMVertex buffer_node = addNode(0, function_size-evict_size);
	
		// connect buffer to inserted one
		connectNodes(buffer_node, new_func);
		// connect the buffer to the folloing nodes
		connectNodes(node, buffer_node);
	}

	// true if the memory size may exceeded
	return make_pair(new_func, begin_of_path);
}



ASMVertex inline AbsStackMemState::addNode(uint32_t func_addr, uint32_t func_size)
{
	ASMVertex node;
	node = add_vertex(asmg);
	put(funcAddrNProp, node, func_addr);
	put(funcSizeNProp, node, func_size);
	ostringstream s;
	s << "0x" << hex << func_addr << "/" << dec << func_size;
	put(nameNProp, node, s.str());
	return node;
}


ASMEdge inline AbsStackMemState::connectNodes(ASMVertex first, ASMVertex second)
{
	bool ins_edge;
	ASMEdge e;
	tie(e, ins_edge) = add_edge(first, second, asmg);
	assert(ins_edge);

	return e;
}

void inline AbsStackMemState::deleteNode(ASMVertex node)
{
		remove_vertex(node, asmg);
}


ASMVertex AbsStackMemState::addSuccessorNode(ASMVertex active, uint32_t func_addr)
{
	ASMVertex new_node;
	new_node = addNode(func_addr, getFunctionMemSize(func_addr));
	connectNodes(active, new_node);
	return new_node;
}

ASMVertex AbsStackMemState::addPredecessorNode(ASMVertex active, uint32_t func_addr)
{
	ASMVertex new_node;
	new_node = addNode(func_addr, getFunctionMemSize(func_addr));
	connectNodes(new_node, active);
	return new_node;
}

void AbsStackMemState::evictStartNodeFromPath(ASMVertex node)
{
	vector<ASMVertex> next = getPredecessors(node);
	ASMVertex current = node;
	assert(next.size() !=0);

	while(next.size() != 0)
	{
		assert(next.size() == 1);
		current = next.front();
		next = getPredecessors(current);
	}

	deleteNode(current);
}

void AbsStackMemState::evictEndNodeFromPath(ASMVertex node)
{
	vector<ASMVertex> next = getSuccessors(node);
	ASMVertex current = node;
	assert(next.size() !=0);

	while(next.size() != 0)
	{
		assert(next.size() == 1);
		current = next.front();
		next = getSuccessors(current);
	}

	deleteNode(current);
}

void AbsStackMemState::printMemSet(ostringstream *os, string node_id, bool export_graph)
{
	asmgVertexIter vp;

	if((analysis==MUST))
	{
		*os << "MUST set contains: ";
	}
	else
	{
		*os << "MAY set contains: ";
	}

    for (vp = vertices(asmg); vp.first != vp.second; ++vp.first)
	{
		*os << hex << "(0x" << get(funcAddrNProp, *vp.first) << ") ";
	}

	if(export_graph)
	{
		exportGraph(node_id);
	}
}
