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
#ifndef _VIVUG_CREATOR_HPP_
#define _VIVUG_CREATOR_HPP_


// by activating this define the VivuGraphCreator uses the implementation for loops and paths from CFGLoopHelper.
#define VIVUG_CREATOR_USES_CFG_LOOP_HELPER

#include "global.h"
#include "constants.h"
#include "graph_structure.h"
#include "fcgobj.hpp"
#include "cfgobj.hpp"
#include "context_stack.hpp"
#include "cfgloophelper.hpp"

#include <map>
#include <stack>

#define FLOW_JOIN_NODE "Meta Flow Join Node"

/**
 * \brief Pair that binds the memory state graph to the corresponding control flow graph.
 * The control flow graph hold all necessary basic block and edge informations, whereas the
 * memory state graph contains the abstract memory states (created by VIVU approach) only.
 */
typedef pair<ControlFlowGraph, MemoryStateGraph> CFGMSGPair;
/**
 * \brief Binds a vertex of the memory state graph to its corresponding basic block (or other cfg node).
 * This represents the memory state when the cfg vertex is inducted.
 */
typedef pair<CFGVertex, MSGVertex> CFGMSGVPair;


/**
 * \brief Map to combine node control flow graph and in memory state graph.
 */
typedef map<CFGVertex, MSGVertex> CfgMsgMap;

typedef map<uint32_t, uint32_t> ContextMap;

typedef map<CFGVertex, bool> CfgvBoolMap;

///**
// * \brief Structure that binds a node to its context (the place where the function to which the node belongs is called from).
// * Needed to allow context sensive path searching.
// */
//struct node_context_t 
//{
//	CFGVertex node; /// The node.
//	ContextStack context; /// It's calling context.
//};

/**
 * \brief Structure to assign the precipitant loop exit nodes in the msg to the loop head in the reference cfg.
 * A percipitant loop exit node is e.g. a continue statement in a loop or any other interim jump back to the loop head.
 * This structure assings the percipitant loop exit nodes to the loop head in the reference cfg to create forward step unroll edges from this percipitant loop exits of the first unrolled iteration to the loop head of the rest of the iterations.
 * The loop head is represented as a cfg node, because the msg node of the loop edge of the rest of the iterations is not yet created when building the msg for a loop.
 * Notice: Only the percipitant loop exit nodes are attached in this structure, not the final (last) loop exit node. 
 */
struct loop_nodes_t 
{
	CFGVertex loop_head;  // Head of a loop as cfg vertrex
	vector<MSGVertex> loop_exits; // Vector of percipitant loop exit nodes of the loop defined by the loop head.
};

/**
 * \brief Class that builds an memory state graph by using the VIVU approach.
 * The memory state graph extends the control flow graph by the abstract memory state for each
 * node (e.g. basic block) of the control flow graph. 
 * Notice that the memory state graph inlines functions (to differ between serveral call contexts) and
 * separates (partly unrolls) the first loop execution and all further following.
 * The VIVU graph creator also sets the correct flow (loop bound) values for the memory state graph by altering the loop constrains of the (unrolled) reference control flow graph.
 */
#ifdef VIVUG_CREATOR_USES_CFG_LOOP_HELPER
class VivuGraphCreator : CFGLoopHelper {
#else
class VivuGraphCreator {
#endif
	public:
		/**
		 * \brief Constructor for class.
		 * \param cfgraph The control flow graph for which the memory state graph will be created. Using an scfg without inducting edge is recommended.
		 * \param entry Entry node of the control flow graph that is used for memory state graph creation.
		 * \param exit Exit node of the control flow graph that is used for memory state graph creation. Notice that the inducting edge from exit to entry will be not handled as back egde and will not be created for the memory state graph.
		 */
		VivuGraphCreator(ControlFlowGraph cfgraph, CFGVertex entry, CFGVertex exit);
		/**
		 * \brief Default destructor-
		 */
		virtual ~VivuGraphCreator();
		/**
		 * \brief Builds up the memory state graph and returns it.
		 * To bind the reference control flow graph to the memory state graph, whicht referes to its nodes, both will be bundled.
		 * \returns The created memory state graph and the control flow graph as pair.
		 */
		CFGMSGPair createVivuGraph(void);
		/**
		 * \brief Returns the entry nodes for the memory state graph and it's reference control flow graph.
		 * \returns Pair of entry nodes of the memory state graph and the reference control flow graph.
		 */
		CFGMSGVPair getEntry(void);
		/**
		 * \brief Returns the exit nodes for the memory state graph and it's reference control flow graph.
		 * \returns Pair of exit nodes of the memory state graph and the reference control flow graph.
		 */
		CFGMSGVPair getExit(void);
	private:
		/**
		 * \brief Creates a part of the memory state graph, if a back edge is found.
		 * Instead of creating back edges in the memory state gra[h
		 * the code between back edge target and source will be virtually unrolled (added for a second time in the memory state graph).
		 * This is to separate the first loop interation by all others, to allow a first miss always hit clasification.
		 * Notice that the first iteration of the loop, introduced by the back edge, is already presented in the memory state graph by
		 * prior processing (before finding the back edge).
		 * 
		 * The function is called for a sequential code part like a loop body or the whole programm. Loops within the sequential code are unrolled by stripping of the first loop iteration and then calling this function for the loop body recursively and after it's return connecting the resulting loop body with the back edge. 
		 * \param start Starting vertex of the part of code that should be unrolled. To read the structure from the reference cfg and add the nodes to the memory state graph, the starting node for both graphes is used.
		 * \param end Control flow graph vertex that defines the end of the code part to unroll. This cfg vertex is the one that has the back edge to the start vertex.
		 * \returns The memory state graph vertex that corresponds to the end vertex provided as parameter.
		 */
		MSGVertex createVivuGraphForSequentialCode(CFGMSGVPair start, CFGVertex end);
		/**
		 * \brief Adds a vertex to the memory state graph and connects it. 
		 * If the vertex if is not found in the map it will be created, otherwise the found vertex is used. This allows merging control flows in the memory state graph within one linear code section, handled by the createVivuGraphForSequentialCode(). Iff the target node is the cfg_exit the node will never be created.
		 * \param source The source vertexes that will be connectet to the new memory state graph vertex.
		 * \param target The target vertex of the reference control flow graph. This is needed as vertex property for the new memory state graph vertex.
		 * \param map Map with msg and cfg nodes, that specifies if the target node is already present or should be created. If now map is provides (by NULL) the node is created anyway.
		 * \param original_edge_type The type of the original edge (that is transposed into the msg representation). The type of the edge is adopted.
		 * \param correspondingEdge The edge that corresponds to the edge in the msg, that is created. For ForwardStepUnroll edges the loop injecting edge is the one that correspondes to it.
		 * \returns The created and connected memory state graph vertex.
		 */
		MSGVertex addMSGVertex(CFGMSGVPair source, CFGVertex target, CfgMsgMap *map, edge_type_t original_edge_type, CFGEdge correspondingEdge);

#ifndef VIVUG_CREATOR_USES_CFG_LOOP_HELPER
		/*!
		 * \brief Obtains the loop bound for a loop with the basic_block as loop head.
		 * \param basic_block The loop head node, identified by the BackwardJump at the end of the loop.
		 * \returns The maximum loop bound stored in the circ_t Property. (Note is only available if CONF_USE_FLOWFACT_FILE is set and the loop was found in the flowfact file).
		 * This method only works for loop heads with only _one_ forward/meta edges in there input edges set. Otherwise the method fails with an assert().
		 */
		uint32_t getLoopBoundForLoopHead(CFGVertex basic_block, CFGEdge loop_causing_back_edge);

		/**
		 * \brief Returns the loop injecting edge for a loop with the basic_block as loop head.
		 * \param basic_block The loop head node, identified by the BackwardJump at the end of the loop.
		 * \returns The injecting edge (either a forward or a meta edge) of the loop.
		 * This method only works for loop heads with only _one_ forward/meta edges in there input edges set. Otherwise the method fails with an assert().
		 */
		CFGEdge getLoopInjectingEdgeForLoopHead(CFGVertex loop_head, CFGEdge loop_causing_back_edge);
#endif

		/**
		 * \brief Checks if all but one return edges of a loop were registered in the loop_nodes_t structure.
		 * \param loop_nodes A structure of all loop heads including the assigned percipitant loop exit nodes.
		 * \param loop_head The head of the loop of which it should be checked if only one unhandled loop exit edge is not holdin the loop_nodes_t structure.
		 * \returns True if there is only one loop back edge not registered in the loop_nodes_t structure, else false.
		 */
		inline bool isLastUnhandledLoopEdge(vector<loop_nodes_t> *loop_nodes, CFGVertex loop_head);

		/**
		 * \brief Returns the number of back edges that targets the given node.
		 * \param node A cfg node to get the number of back edges that targets that node. 
		 * \returns The number of back edges that targets the given node.
		 */
		inline uint32_t getBackEdgeInDegree(CFGVertex node);

		/**
		 * \brief Returns the number of registered back edges of a loop head.
		 * \param loop_nodes A structure of all loop heads including the assigned percipitant loop exit nodes.
		 * \param loop_head The head of the loop of which the number of registered back edges should be returned.
		 * \returns The number of registered back edges of a loop head.
		 */
		inline uint32_t getNumberOfHandledBackEdges(vector<loop_nodes_t> *loop_nodes, CFGVertex loop_head);

		/**
		 * \brief Registers a percipitant loop exit node to a loop (represented by the loop head) in the loop_nodes_t structure.
		 * \param loop_nodes A structure of all loop heads including the assigned percipitant loop exit nodes, to which the exit node is added.
		 * \param loop_exit The msg node that exits a loop body and jumps back to the loop head.
		 * \param loop_head The head of the loop for which an exit node should be registered.		 
		 * Note: Only percipitant loop exit nodes will be registered, because only this nodes will be connected by addForwardStepUnrollEdgesForUnrolledLoopHead(). The last loop exit node will be connected by the addMSGVertex() method.
		 */
		inline void registerLoopExitNodeToLoop(vector<loop_nodes_t> *loop_nodes, MSGVertex loop_exit, CFGVertex loop_head);

		/**
		 * \brief Adds the ForwardStepUnroll edges of all registered loop exit nodes of the unrolled first iteration of a loop to a defined node.
		 * \param loop_nodes A structure of all loop heads including the assigned percipitant loop exit nodes for which the edges should be added.
		 * \param loop_head The head of the loop  for which all loop exit nodes should be connected to the loop_flow_join_node.
		 * \param loop_injecting_edge The edge that causes the loop.
		 * \param loop_flow_join_node The node that concentrates all edges that leave the unrolled first loop iteration to join their flows. This node is added and connected to the head of the rest of iterations loop head by addMSGVertex().
		 */
		inline void addForwardStepUnrollEdgesForUnrolledLoopHead(vector<loop_nodes_t> *loop_nodes, CFGVertex loop_head, CFGEdge loop_injecting_edge, MSGVertex loop_flow_join_node);

		/**!
		 * \brief Obtains for the head of the rest of iterations loop (the loop head with the stripped of first iteration) the upstream FLOW_JOIN_NODE.
		 * \param rest_loop_head The head of the rest of iterations loop, for which the FLOW_JOIN_NODE should be returned.
		 * \returns The FLOW_JOIN_NODE, that collects all back edges of the first unrolled iteration of a loop, of the rest of iterations loop defined by rest_loop_head.
		 */
		inline MSGVertex getFlowJoinNodeFromRestOfLoopHead(MSGVertex rest_loop_head);

#ifndef VIVUG_CREATOR_USES_CFG_LOOP_HELPER
		/**
		 * \brief Checks if the given edge is a loop causing edge (a back edge directed to a loop head).
		 * Either this edge targets a loop edge with a forward step/jump or meta edge (only one is allowed) or it has only the back edge as in edge, then it is assumed that that there is a decision at loop tail.
		 * TODO: Crosscheck the decision if it is a loop causig back edge with entries in FlowFactReader.
		 * FIXME: This implementation is doubtful, thus the decision may be incorrect. With tested benchmarks no error was found, but the checking is very fragile and some unknown cases may cause errorous decision.
		 * \param back_edge A back edge that shold be tested if it is a loop causing edge or not.
		 * \returns True if the back_edge belongs to a loop.
		 */
		bool isLoopCausingBackEdge(CFGEdge back_edge);

		/**
		 * \brief Checks if there is a path from a begin and to an end node with a given node on it.
		 * This method is used to check if a given node is in a connected component for which a part of the vivu graph is created see createVivuGraphForSequentialCode().
		 * \param node The node for which it is checked if there is a path from begin to end with the node on it.
		 * \param path_begin Start node of the path to find.
		 * \param path_end End node of the path to find.
		 * \param same_stack_level If true, all three nodes have to be on the same stack level. (None of them can be in another function, but functions can be between these nodes.)
		 * \returns True if a path was found from begin to end with the given node on it was found, else false.
		 */
		bool isNodeOnPath(CFGVertex node, CFGVertex path_begin, CFGVertex path_end, bool same_stack_level);

		/**
		 * \brief Checks if a given node has a given predecessor.
		 * Note: the method calls the isPredecessorNode(CFGVertex, CFGVertex, CFGVertex) using the cfg_exit as barrier node, that cannot be a predecessor of any node. So the barrier node will not hinder any node to find its predecessor.
		 * \param predecessor Possible predecessor node of the given node.
		 * \param node The node for which should be determined if it has a given predecessor.
		 * \param consider_context If set the method is sensitive to calling contexts on searching for a path from predecessor to node. This dissallows the traversing of impossible paths.
		 * \param same_stack_level If set the predecessor and the node have to be in the same function.
		 * \returns True if the given node has the given predecessor node as predecessor, else false.
		 */
		bool isPredecessorNode(CFGVertex predecessor, CFGVertex node, bool consider_context, bool same_stack_level);

		/**
		 * \brief Checks if a given node has a given predecessor, but when searching do not cross a given barrier node.
		 * \param predecessor Possible predecessor node of the given node.
		 * \param node The node for which should be determined if it has a given predecessor.
		 * \param barrier_node Beyond the barrier node no further nodes will be inspected, to prevent detecting false predecessors in circulations build by non inlining function calls.
		 * \param consider_context If set the method is sensitive to calling contexts on searching for a path from predecessor to node. This dissallows the traversing of impossible paths.
		 * \param same_stack_level If set the predecessor and the node have to be in the same function.
		 * \returns True if the given node has the given predecessor node as predecessor, else false.
		 */
		bool isPredecessorNode(CFGVertex predecessor, CFGVertex node, CFGVertex barrier_node, bool consider_context, bool same_stack_level);

		/**
		 * \brief Checks if a given node has a given successor.
		 * Note: the method calls the isSuccessorNode(CFGVertex, CFGVertex, CFGVertex) using the cfg_entry as barrier node, that cannot be a successor of any node. So the barrier node will not hinder any node to find its successor.
		 * \param successor Possible successor node of the given node.
		 * \param node The node for which should be determined if it has a given successor.
		 * \param consider_context If set the method is sensitive to calling contexts on searching for a path from node to successor. This dissallows the traversing of impossible paths.
		 * \param same_stack_level If set the node and the successor have to be in the same function.
		 * \returns True if the given node has the given successor node as successor, else false.
		 */

		bool isSuccessorNode(CFGVertex successor, CFGVertex node, bool consider_context, bool same_stack_level);

		/**
		 * \brief Checks if a given node has a given successor, but when searching do not cross a given barrier node.
		 * \param successor Possible successor node of the given node.
		 * \param node The node for which should be determined if it has a given successor.
		 * \param barrier_node Beyond the barrier node no further nodes will be inspected, to prevent detecting false successors in circulations build by non inlining function calls.
		 * \param consider_context If set the method is sensitive to calling contexts on searching for a path from node to successor. This dissallows the traversing of impossible paths.
		 * \param same_stack_level If set the node and the successor have to be in the same function.
		 * \returns True if the given node has the given successor node as successor, else false.
		 */
		bool isSuccessorNode(CFGVertex successor, CFGVertex node, CFGVertex barrier_node, bool consider_context, bool same_stack_level);

		/**
		 * \brief Checks if two nodes are connected by a path.
		 * Therefore the method calls the getPath() method.
		 * \param start Start node for the path.
		 * \param end End node of the path.
		 * \returns True if there is a path between the two nodes, otherwise false.
		 */
		bool isConnectedViaPath(CFGVertex start, CFGVertex end);

		/**
		 * \brief Finds a path from the start node to the end node, by using only ForwardStep, ForwardJump and Meta edges.
		 * The path search is context sensitive. Such that invalid paths build by traversing call / return points that do not math are not taken.
		 * The difference to the isNodeOnPath() function is that here no non loop causing back edges are used to check connection.
		 * \param start Start node for the path (e.g. the loop head that is target of the loop causing back edge).
		 * \param end End node of the path (e.g. the loop bottom that is source of the loop causing back edge).
		 * \returns One possible path from start to end, including start and end node. If start and end nodes are equal the path contains only one node. If the path is empty no path exists.
		 */
		vector<CFGVertex> getPath(CFGVertex start, CFGVertex end);

		/**
		 * \brief Gets the injecting edge for a loop body path.
		 * The method searches for an in-edge on the path that is not connected to another node on the path, furthermore the source node of this in-edge has not to be connected to the start and end node of the path (which are representing the loop head and the loop bottom node). The connection test is done by isNodeOnPath() and is needed since only one path of the loop body is represented in the path parameter not the whole loop body (that may contain different paths).
		 * \param path A path from loop head to loop bottom (determined by "following" the back_edge in getPath()).
		 * \param injecting_edge The pointer where the found edge should be written to, if one is found.
		 * \returns True if an injecting edge was found, else false.
		 */
		bool getInjectingEdgeForPath(vector<CFGVertex> path, CFGEdge *injecting_edge);
#endif 

#ifndef VIVUG_CREATOR_USES_CFG_LOOP_HELPER
		/**
		 * \brief The reference control flow graph for the creation of the memory state graph
		 */
		ControlFlowGraph cfg;
		/**
		 * \brief Entry point of the reference control flow graph.
		 */
		CFGVertex cfg_entry;
		/**
		 * \brief Exit point of the reference control flow graph.
		 */
		CFGVertex cfg_exit;
#endif

		// TODO document graph properties
#ifndef VIVUG_CREATOR_USES_CFG_LOOP_HELPER
		property_map<ControlFlowGraph, nodetype_t>::type nodeTypeNProp;
		property_map<ControlFlowGraph, startaddr_t>::type startAddrNProp;
		property_map<ControlFlowGraph, startaddrstring_t>::type startAddrStringNProp;
		property_map<ControlFlowGraph, endaddr_t>::type endAddrNProp;
#endif
		property_map<ControlFlowGraph, calllabel_t>::type callLabelNProp;
		property_map<ControlFlowGraph, bbcode_t>::type bbCodeNProp;
		property_map<ControlFlowGraph, bbsize_t>::type bbSizeNProp;
		property_map<ControlFlowGraph, bbinstrs_t>::type bbInstrsNProp;

#ifndef VIVUG_CREATOR_USES_CFG_LOOP_HELPER
		property_map<ControlFlowGraph, edgetype_t>::type edgeTypeEProp;
		property_map<ControlFlowGraph, circ_t>::type circEProp;
#endif
		property_map<ControlFlowGraph, cost_t>::type costEProp;
		property_map<ControlFlowGraph, cost_onchip_t>::type costOnChipEProp;
		property_map<ControlFlowGraph, cost_offchip_t>::type costOffChipEProp;
		property_map<ControlFlowGraph, capacityl_t>::type capacitylEProp;
		property_map<ControlFlowGraph, capacityh_t>::type capacityhEProp;
		property_map<ControlFlowGraph, activation_t>::type actEProp;
		property_map<ControlFlowGraph, edgename_t>::type edgeNameEProp;

		/**
		 * \brief The memory state graph that is created according to the reference control flow graph. The memory state graph is intended to represent the abstract cache/disp state for data flow analysis. It is created using the VIVU approach.
		 * The nodes of the graph correspondes to the nodes of a (s)cfg graph, such that the abstract memory state
		 * is attached to the (s)cfg. The memory state node represents the memory state of the memory, when the control 
		 * flow enteres the corresponding basic block or node. Thus the memory state before executing a certain basic block is 
		 * attached to the basic block.
		 */
		MemoryStateGraph msg;
		/**
		 * \brief Entry point of the memory state graph.
		 */
		MSGVertex msg_entry;
		/**
		 * \brief Exit point of the memory state graph.
		 */
		MSGVertex msg_exit;

		// TODO document graph properties
		property_map<MemoryStateGraph, mem_state_t>::type memStateNProp;
		property_map<MemoryStateGraph, mem_state_valid_t>::type memStateValNProp;
		property_map<MemoryStateGraph, msg_context_id_t>::type contextIDNProp;
		property_map<MemoryStateGraph, cfg_vertex_t>::type mappedCFGVertexNProp;
		property_map<MemoryStateGraph, msg_edgetype_t>::type msgEdgeTypeEProp;
		property_map<MemoryStateGraph, msg_circ_t>::type msgCircEprop;
//		property_map<MemoryStateGraph, msg_static_flow_t>::type msgSflowEprop;


		// TODO documentation needed
		ContextMap contextMap;

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
