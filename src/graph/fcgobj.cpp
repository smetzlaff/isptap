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
#include "fcgobj.hpp"

LoggerPtr FunctionCallGraphObject::logger(Logger::getLogger("FunctionCallGraphObject"));

FunctionCallGraphObject::FunctionCallGraphObject() : code_size(0), function_count(0)
{
	// get node properties
	funcNameNProp = get(function_name_t(), fg);
	funcAddrNProp = get(function_addr_t(), fg);
	funcSizeNProp = get(function_size_t(), fg);
	// get edge properties
	funcCallerEProp = get(caller_points_t(), fg);
}

FunctionCallGraphObject::~FunctionCallGraphObject()
{
	incomplete_nodes.clear();
	completed_nodes.clear();
}

void FunctionCallGraphObject::addFunc(function_graph_t funct)
{
	FGVertex u,v;
	LOG_DEBUG(logger, "Adding function " << funct.name << " at 0x" << hex << funct.address);


	u = setCaller(funct.address, funct.name, funct.cfg->getCodeSize());

	bool inserted;
	FuncCfgMap::iterator pos;

	tie(pos, inserted) = funcCMap.insert(make_pair(funct.address, funct.cfg));
	assert(inserted);


	vector<addr_label_t> callees = funct.cfg->getCallTargets();

//	vector<uint32_t> already_added;
//	vector<FGEdge> already_added_edge;
//	bool was_added = false;
//	uint32_t was_added_pos = 0;
//
//	for(uint i = 0; i < callees.size(); i++)
//	{
//		was_added = false;
//		for(uint j = 0; j < already_added.size(); j++)
//		{
//			if(already_added[j] == callees[i].address)
//			{
//				was_added = true;
//				was_added_pos = j;
//			}
//		}
//		if(!was_added)
//		{
//			v = getCallee(callees[i].address);
//			FGEdge e;
//			bool ins_edge;
//
//			tie(e, ins_edge) = add_edge(u, v, fg);
//			assert(ins_edge);
//
//			// set call count to 1
//			put(funcCallerEProp, e,1);
//
//			already_added.push_back(callees[i].address);
//			already_added_edge.push_back(e);
//
//		}
//		else
//		{
//			// increase call count by 1
//			FGEdge e = already_added_edge[was_added_pos];
//			put(funcCallerEProp, e,((get(funcCallerEProp, e))+1));
//
//		}
//	}
//
	for(uint i = 0; i < callees.size(); i++)
	{
			v = getCallee(callees[i].address);
			FGEdge e;
			bool ins_edge;

			tie(e, ins_edge) = add_edge(u, v, fg);
			assert(ins_edge);

			// set call count
			LOG_DEBUG(logger, "Call count for: 0x"  << hex << callees[i].address << " is: " << funct.cfg->getCallSiteCount(callees[i].address));
			put(funcCallerEProp, e,funct.cfg->getCallSiteCount(callees[i].address));
	}

}

FGVertex FunctionCallGraphObject::setCaller(uint32_t function_address, string function_name, uint32_t size)
{
	FGVertex u;
	bool inserted;
	FuncVertexMap::iterator pos;
	tie(pos, inserted) = funcVMap.insert(make_pair(function_address, FGVertex()));
	if(inserted)
	{
		// create vertex in graph
		u = add_vertex(fg);
		// store vertex in map
		pos->second = u;
		// add function to complete list
		completed_nodes.push_back(function_address);
	}
	else
	{
		// get vertex from map (because it already exists)
		u = pos->second;

		eraseFunctionFromIncompleteList(function_address);
	}
	
	// TODO set up properties
	put(funcNameNProp, u, function_name);
	put(funcAddrNProp, u, function_address);
	put(funcSizeNProp, u, size);

//	LOG_DEBUG(logger, "increasing code size " << code_size << " by " << size << " bytes");
	code_size += size;
	function_count++;

	return u;
}

FGVertex FunctionCallGraphObject::getCallee(uint32_t function_address)
{
	FGVertex v;
	FuncVertexMap::iterator pos;

	pos = funcVMap.find(function_address);

	if(pos == funcVMap.end())
	{
		FuncVertexMap::iterator ins_pos;
		bool ins_bool;
		// creating function node for callee
		tie(ins_pos, ins_bool) = funcVMap.insert(make_pair(function_address, FGVertex()));
		assert(ins_bool);

		v = add_vertex(fg);
		ins_pos->second = v;

		pushCalleeToIncompleteList(function_address);
	}
	else
	{
		v = pos->second;
	}

	return v;

}

void FunctionCallGraphObject::pushCalleeToIncompleteList(uint32_t addr)
{
	bool found_entry = false;
	if(addr == UNKNOWN_ADDR)
	{
		return;
	}

	for(uint32_t i=0; i < incomplete_nodes.size(); i++)
	{
		if(incomplete_nodes[i] == addr)
		{
			// found incomplete entry
			found_entry = true;
			break;
		}
	}
	if(!found_entry)
	{
		incomplete_nodes.push_back(addr);
		LOG_DEBUG(logger, "added function 0x" << hex << addr << " to incomplete list.");
	}
	else
	{
		LOG_DEBUG(logger, "function 0x" << hex << addr << " already in incomplete list.");
	}
}

void FunctionCallGraphObject::eraseFunctionFromIncompleteList(uint32_t addr)
{
	bool found_entry = false;
	uint32_t entry = 0;
	for(uint32_t i=0; i < incomplete_nodes.size(); i++)
	{
		if(incomplete_nodes[i] == addr)
		{
			// found incomplete entry
			found_entry = true;
			entry = i;
			break;
		}
	}
	if(found_entry)
	{
		completed_nodes.push_back(incomplete_nodes[entry]);
		incomplete_nodes.erase(incomplete_nodes.begin() + entry);
		LOG_DEBUG(logger, "erased function 0x" << hex << addr << " from incomplete list.");
	}
	else
	{
		LOG_WARN(logger, "The function for 0x" << hex << addr << " is not in the incomplete list!");
	}
}

bool FunctionCallGraphObject::isFinished(void)
{
	if(incomplete_nodes.size()==0)
	{
		return true;
	}
	else
	{
		if(logger->isDebugEnabled())
		{
			ostringstream debug;
			debug << "incomplete list contains: ";

			for(uint32_t i=0; i < incomplete_nodes.size(); i++)
				debug << hex << "0x" <<  incomplete_nodes[i] << " ";

			LOG_DEBUG(logger, debug.str()); 
		}
		return false;
	}
}

uint32_t FunctionCallGraphObject::getCodeSize(void)
{
//	LOG_DEBUG(logger, "returning function size of " << code_size << " bytes");
	return code_size;
}

uint32_t FunctionCallGraphObject::getFunctionCount(void)
{
	return function_count;
}


FunctionCallGraph FunctionCallGraphObject::getFCG()
{
	return fg;
}

vector<uint32_t> FunctionCallGraphObject::getFunctions(void)
{
	return completed_nodes;
}

string FunctionCallGraphObject::getFunctionName(uint32_t address)
{
	FuncVertexMap::iterator pos;

	pos = funcVMap.find(address);

	if(pos == funcVMap.end())
	{
		return "";
	}
	else
	{
		return get(funcNameNProp, pos->second);
	}

}

uint32_t FunctionCallGraphObject::getFunctionAddress(string name)
{

	for(FuncVertexMap::iterator pos = funcVMap.begin(); pos != funcVMap.end(); pos++)
	{
//		LOG_DEBUG(logger, "Searching for " << name << " comparing " << get(funcNameNProp, pos->second) << " address is: 0x" << hex << pos->first);
		if(name.compare(get(funcNameNProp, pos->second))==0)
		{
			return pos->first;
		}
	}
	return UNKNOWN_ADDR;
}

ControlFlowGraphObject* FunctionCallGraphObject::getCFGOForStartFunction(void)
{
	return getCFGOForFunction(start_label.address);
}

ControlFlowGraphObject* FunctionCallGraphObject::getCFGOForFunction(uint32_t address)
{
	FuncCfgMap::iterator pos;

	pos = funcCMap.find(address);

	if(pos == funcCMap.end())
	{
		return NULL;
	}
	else
	{
		return pos->second;
	}
}

void FunctionCallGraphObject::setStartLabel(addr_label_t entry_function)
{
	start_label.label = entry_function.label;
	start_label.address = entry_function.address;
	assert(entry_function.address == getFunctionAddress(entry_function.label));

	LOG_DEBUG(logger, "Setting start label for FCG: " << start_label.label << " at 0x" << hex << start_label.address);
}

addr_label_t FunctionCallGraphObject::getStartLabel(void)
{
	return start_label;
}

uint32_t FunctionCallGraphObject::getFunctionSize(uint32_t address)
{
	FuncVertexMap::iterator pos;

	pos = funcVMap.find(address);

	if(pos == funcVMap.end())
	{
		return 0;
	}
	else
	{
		return get(funcSizeNProp, pos->second);
	}
}


