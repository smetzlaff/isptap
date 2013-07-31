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
#ifndef _GRAPH_STRUCTURE_H_
#define _GRAPH_STRUCTURE_H_

#include "global.h"
#include "constants.h"
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/copy.hpp>


using namespace boost;

enum node_type_t { BasicBlock=0, Entry, Exit, CallSite, CallPoint, ReturnPoint, FlowJoinNode, UnknownJumpTarget};
enum edge_type_t { ForwardStep=0, ForwardStepUnroll, ForwardJump, BackwardJump, Meta, InductingBackEdge};


/// type of a static flow constraint
/// Either exact, max or min flow
enum flowc_type_t {UNKNOWN=0,EXACT, MAX, MIN};

/// a static flow constraint, containing of the bound and the type
struct static_flow_constraint_t {
	/// The bound of the static flow constraint.
	uint32_t flow_bound;
	/// The type of the static flow constraint.
	flowc_type_t flow_type;
	/// The ID of the static flow constraint.
	/// It is used to keep the semantic of the flow constraint, if e.g. loops are unrolled in vivu creation.
	uint16_t id;
};

// TODO write documentation of usage of the properties.

// --- control flow graph

// vertex property definitions
struct nodetype_t { typedef vertex_property_tag kind; };
struct startaddr_t { typedef vertex_property_tag kind; };
struct startaddrstring_t { typedef vertex_property_tag kind; };
struct endaddr_t { typedef vertex_property_tag kind; };
struct context_id_t { typedef vertex_property_tag kind; };
struct calllabel_t { typedef vertex_property_tag kind; };
struct bbcode_t { typedef vertex_property_tag kind; };
struct bbsize_t { typedef vertex_property_tag kind; };
struct bbinstrs_t { typedef vertex_property_tag kind; };
struct cache_hits_t { typedef vertex_property_tag kind; };
struct cache_misses_t { typedef vertex_property_tag kind; };
struct cache_ncs_t { typedef vertex_property_tag kind; };
typedef property<nodetype_t, node_type_t> NTypeProp;
typedef property<startaddr_t, uint32_t, NTypeProp> StartProp;
typedef property<startaddrstring_t, string, StartProp> StartStringProp;
typedef property<endaddr_t, uint32_t, StartStringProp> AddrProp; // end address of the basic block, for call points this address is used as context address
typedef property<context_id_t, uint32_t, AddrProp> CtxProp;
typedef property<calllabel_t, uint32_t, CtxProp> CallProp;
typedef property<bbcode_t, string, CallProp> CodeProp;
typedef property<bbsize_t, uint32_t, CodeProp> SizeProp;
typedef property<cache_hits_t, uint16_t, SizeProp> CacheHitProp;
typedef property<cache_misses_t, uint16_t, CacheHitProp> CacheMissProp;
typedef property<cache_ncs_t, uint16_t, CacheMissProp> CacheNcProp;
typedef property<bbinstrs_t, uint32_t, CacheNcProp> NodeProp;

// edge property definitions
struct edgetype_t { typedef edge_property_tag kind; };
struct cost_t { typedef edge_property_tag kind; };
struct cost_onchip_t { typedef edge_property_tag kind; }; /// intended to be replaced by mem_penalty_t!
struct cost_offchip_t { typedef edge_property_tag kind; }; /// intended to be replaced by mem_penalty_t!
struct mem_penalty_t { typedef edge_property_tag kind; };
struct capacityl_t { typedef edge_property_tag kind; };
struct capacityh_t { typedef edge_property_tag kind; };
/// max circulation of this edge, needed for adding flow facts to back edges
struct circ_t { typedef edge_property_tag kind; };
/// activation count of the edges, added by ilp graph creator for the wcet critical path
struct activation_t { typedef edge_property_tag kind; };
/// property for static flow constraints, added by flow facts (fx =|<=|>= value)
struct static_flow_t { typedef edge_property_tag kind; };
struct edgename_t { typedef edge_property_tag kind; };
typedef property<edgetype_t, edge_type_t> ETypeProp;
typedef property<cost_t, uint32_t, ETypeProp> CostProp;
typedef property<cost_onchip_t, uint32_t, CostProp> CostOnChipProp;
typedef property<cost_offchip_t, uint32_t, CostOnChipProp> CostOffChipProp;
typedef property<mem_penalty_t, uint32_t, CostOffChipProp> MemPenaltyProp;
typedef property<capacityl_t, uint32_t, MemPenaltyProp> CapProp;
typedef property<capacityh_t, uint32_t, CapProp> Cap2Prop;
typedef property<circ_t, int32_t, Cap2Prop> CircProp;
typedef property<activation_t, uint32_t, CircProp> ActProp;
typedef property<static_flow_t, static_flow_constraint_t, ActProp> SFlowProp;
typedef property<edgename_t, string, SFlowProp> EdgeProp;

// create a typedef for the Graph type
typedef adjacency_list<vecS, vecS, bidirectionalS, NodeProp, EdgeProp> ControlFlowGraph;

// types for vertex and edge in CFG
typedef graph_traits<ControlFlowGraph>::vertex_descriptor CFGVertex;
typedef graph_traits<ControlFlowGraph>::edge_descriptor CFGEdge;


// iterators
// iterator for vertices of the ControlFlowGraph
typedef graph_traits<ControlFlowGraph>::vertex_iterator cfgVertexIter_t;
typedef pair<cfgVertexIter_t, cfgVertexIter_t> cfgVertexIter;

// iterator for edges of the ControlFlowGraph
typedef graph_traits<ControlFlowGraph>::edge_iterator cfgEdgeIter_t;
typedef pair<cfgEdgeIter_t, cfgEdgeIter_t> cfgEdgeIter;

// iterator for out_edges of the ControlFlowGraph
typedef graph_traits<ControlFlowGraph>::out_edge_iterator cfgOutEdgeIter_t;
typedef pair<cfgOutEdgeIter_t, cfgOutEdgeIter_t> cfgOutEdgeIter;

// iterator for in_edges of the ControlFlowGraph
typedef graph_traits<ControlFlowGraph>::in_edge_iterator cfgInEdgeIter_t;
typedef pair<cfgInEdgeIter_t, cfgInEdgeIter_t> cfgInEdgeIter;

// ---- function call graph

// vertex property definitions
//struct functioncfg_t { typedef vertex_property_tag kind; };
struct function_name_t { typedef vertex_property_tag kind; };
struct function_addr_t { typedef vertex_property_tag kind; };
struct function_size_t { typedef vertex_property_tag kind; };
typedef property<function_name_t, string> FuncNameProp;
typedef property<function_addr_t, uint32_t, FuncNameProp> FuncAddrProp;
typedef property<function_size_t, uint32_t, FuncAddrProp> FunctProp;

// edge property definitions
struct caller_points_t { typedef edge_property_tag kind; };
typedef property<caller_points_t, uint32_t> CallerPProp;

//
typedef adjacency_list<vecS, vecS, bidirectionalS, FunctProp, CallerPProp> FunctionCallGraph;

// types for vertex and edge in CFG
typedef graph_traits<FunctionCallGraph>::vertex_descriptor FGVertex;
typedef graph_traits<FunctionCallGraph>::edge_descriptor FGEdge;

// iterators
// iterator for vertices of the FunctionCallGraph
typedef graph_traits<FunctionCallGraph>::vertex_iterator fcgVertexIter_t;
typedef pair<fcgVertexIter_t, fcgVertexIter_t> fcgVertexIter;

// iterator for edges of the FunctionCallGraph
typedef graph_traits<FunctionCallGraph>::edge_iterator fcgEdgeIter_t;
typedef pair<fcgEdgeIter_t, fcgEdgeIter_t> fcgEdgeIter;

// iterator for out_edges of the FunctionCallGraph
typedef graph_traits<FunctionCallGraph>::out_edge_iterator fcgOutEdgeIter_t;
typedef pair<fcgOutEdgeIter_t, fcgOutEdgeIter_t> fcgOutEdgeIter;

// iterator for in_edges of the FunctionCallGraph
typedef graph_traits<FunctionCallGraph>::in_edge_iterator fcgInEdgeIter_t;
typedef pair<fcgInEdgeIter_t, fcgInEdgeIter_t> fcgInEdgeIter;


// ---- abstract memory state entry for data flow analysis

struct abs_mem_entry_t {
	uint32_t address;
	union {
		uint32_t position;
		uint32_t age;
//		struct {
//			uint16_t set_no;
//			uint16_t set_pos;
//		}; // needed for k-way associative caches
	};
};

/// structure to represent the abstract memory state
struct abs_mem_set_t {
	/// memory state as a vector (e.g. representing the lines of a cache or the function entries of the DISP) of vectors (representing the current state of the cache line or function entry of the DISP (because for the may analysis multiple states are possible)) of addresses (representing a cache line of function address)
	vector<abs_mem_entry_t> must_set;
	vector<abs_mem_entry_t> may_set;
	uint32_t max_size;
};

// vertex property definitions
struct mem_state_t { typedef vertex_property_tag kind; };
struct mem_state_valid_t { typedef vertex_property_tag kind; };
struct msg_context_id_t { typedef vertex_property_tag kind; };
struct cfg_vertex_t { typedef vertex_property_tag kind; };
struct mem_cache_hits_t { typedef vertex_property_tag kind; };
struct mem_cache_misses_t { typedef vertex_property_tag kind; };
struct mem_cache_ncs_t { typedef vertex_property_tag kind; };
typedef property<mem_state_t, abs_mem_set_t> MemStateProp;
typedef property<mem_state_valid_t, bool, MemStateProp> MemStateValProp;
typedef property<msg_context_id_t, uint32_t, MemStateValProp> CtxIdProp;
typedef property<mem_cache_hits_t, uint16_t, CtxIdProp> CacheHProp;
typedef property<mem_cache_misses_t, uint16_t, CacheHProp> CacheMProp;
typedef property<mem_cache_ncs_t, uint16_t, CacheMProp> CacheNProp;
typedef property<cfg_vertex_t, CFGVertex, CacheNProp> MemStateVertexProp;

// edge property definitions
struct edge_id_t { typedef edge_property_tag kind; };
struct msg_edgetype_t { typedef edge_property_tag kind; };
struct dynamic_mem_penalty_t { typedef edge_property_tag kind; };
struct msg_circ_t { typedef edge_property_tag kind; };
//struct msg_static_flow_t{ typedef edge_property_tag kind; };
struct cfg_edge_t { typedef edge_property_tag kind; };
typedef property<msg_edgetype_t, edge_type_t> MsgEdgeTypeProp;
typedef property<dynamic_mem_penalty_t, uint32_t, MsgEdgeTypeProp> DMemPenaltyProp; /// penalty calculated by cache analysis for the node that is source of this edge
//typedef property<cfg_edge_t, CFGEdge, DMemPenaltyProp> CFGEdgeProp;
//typedef property<msg_circ_t, uint32_t, CFGEdgeProp> MSGCircProp;
typedef property<msg_circ_t, int32_t, DMemPenaltyProp> MSGCircProp;
typedef property<edge_id_t, uint32_t, MSGCircProp> MemStateEdgeProp;

// create a typedef for the Graph type
typedef adjacency_list<vecS, vecS, bidirectionalS, MemStateVertexProp, MemStateEdgeProp> MemoryStateGraph;

// types for vertex and edge in CFG
typedef graph_traits<MemoryStateGraph>::vertex_descriptor MSGVertex;
typedef graph_traits<MemoryStateGraph>::edge_descriptor MSGEdge;


// iterators
// iterator for vertices of the MemoryStateGraph
typedef graph_traits<MemoryStateGraph>::vertex_iterator msgVertexIter_t;
typedef pair<msgVertexIter_t, msgVertexIter_t> msgVertexIter;

// iterator for edges of the MemoryStateGraph
typedef graph_traits<MemoryStateGraph>::edge_iterator msgEdgeIter_t;
typedef pair<msgEdgeIter_t, msgEdgeIter_t> msgEdgeIter;

// iterator for out_edges of the MemoryStateGraph
typedef graph_traits<MemoryStateGraph>::out_edge_iterator msgOutEdgeIter_t;
typedef pair<msgOutEdgeIter_t, msgOutEdgeIter_t> msgOutEdgeIter;

// iterator for in_edges of the MemoryStateGraph
typedef graph_traits<MemoryStateGraph>::in_edge_iterator msgInEdgeIter_t;
typedef pair<msgInEdgeIter_t, msgInEdgeIter_t> msgInEdgeIter;


// ----
// the AbsStackMemGraph
// ----
//
struct name_t { typedef vertex_property_tag kind; };

typedef property<function_addr_t, uint32_t> ASMAddrProp;
typedef property<function_size_t, uint32_t, ASMAddrProp> ASMSizeProp;
typedef property<name_t, string, ASMSizeProp> AbsStackMemNodeProp;

struct asm_id_t {typedef edge_property_tag kind; };
typedef property<asm_id_t, uint32_t> AbsStackMemEdgeProp;

// create a typedef for the Graph type
typedef adjacency_list<vecS, vecS, bidirectionalS, AbsStackMemNodeProp, AbsStackMemEdgeProp> AbsStackMemGraph;

// types for vertex and edge in CFG
typedef graph_traits<AbsStackMemGraph>::vertex_descriptor ASMVertex;
typedef graph_traits<AbsStackMemGraph>::edge_descriptor ASMEdge;

// iterators
// iterator for vertices of the AbsStackMemGraph
typedef graph_traits<AbsStackMemGraph>::vertex_iterator asmgVertexIter_t;
typedef pair<asmgVertexIter_t, asmgVertexIter_t> asmgVertexIter;

// iterator for edges of the AbsStackMemGraph
typedef graph_traits<AbsStackMemGraph>::edge_iterator asmgEdgeIter_t;
typedef pair<asmgEdgeIter_t, asmgEdgeIter_t> asmgEdgeIter;


// iterator for out_edges of the AbsStackMemGraph
typedef graph_traits<AbsStackMemGraph>::out_edge_iterator asmgOutEdgeIter_t;
typedef pair<asmgOutEdgeIter_t, asmgOutEdgeIter_t> asmgOutEdgeIter;

// iterator for in_edges of the AbsStackMemGraph
typedef graph_traits<AbsStackMemGraph>::in_edge_iterator asmgInEdgeIter_t;
typedef pair<asmgInEdgeIter_t, asmgInEdgeIter_t> asmgInEdgeIter;


// ---- some generic structures

// combining address and label
struct addr_label_t {
	uint32_t address;
	string label;
};

// combining address, label and vertex
struct addr_label_vec_t {
	uint32_t address;
	string label;
	CFGVertex vertex;
};

// combining address, label and size of a function
struct addr_name_size_t
{
	uint32_t address;
	std::string name;
	uint32_t size;
};

#endif
