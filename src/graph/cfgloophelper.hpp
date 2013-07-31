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
#ifndef _CFGLoopHelper_HPP_
#define _CFGLoopHelper_HPP_


#include "global.h"
#include "constants.h"
#include "graph_structure.h"
#include "context_stack.hpp"
#include <iostream>
#include <fstream>
#include <stack>


/**
 * \brief Structure that binds a node to its context (the place where the function to which the node belongs is called from).
 * Needed to allow context sensive path searching.
 */
struct node_context_t
{
	CFGVertex node; /// The node.
	ContextStack context; /// It's calling context.
};

/*!
 * \brief Helper class for handling loops and paths in a cfg.
 * Provides methods to get an injecting edge or its loop bound for a loop head, and obtaining successor or predecessor nodes, and checking the connection of two nodes.
 * The class is only needed due to the code is needed in VivuGraphCreator and WCPathExporter.
 */
class CFGLoopHelper {
	protected:
		/*!
		 * \brief Constructor
		 * \param cfggraph The graph under observation.
		 * \param cfgentry The entry node of the graph.
		 * \param cfgexit The exit node of the graph.
		 */
		CFGLoopHelper(ControlFlowGraph cfggraph, CFGVertex cfgentry, CFGVertex cfgexit);
		/*!
		 * \brief Default destructor.
		 */
		virtual ~CFGLoopHelper();

		/*!
		 * \brief Obtains the loop bound for a loop with the basic_block as loop head.
		 * \param basic_block The loop head node, identified by the BackwardJump at the end of the loop.
		 * \param loop_causing_back_edge The edge that causes the loop.
		 * \returns The maximum loop bound stored in the circ_t Property. (Note is only available if CONF_USE_FLOWFACT_FILE is set and the loop was found in the flowfact file).
		 * This method only works for loop heads with only _one_ forward/meta edges in there input edges set. Otherwise the method fails with an assert().
		 */
		int32_t getLoopBoundForLoopHead(CFGVertex basic_block, CFGEdge loop_causing_back_edge);
		/**
		 * \brief Returns the loop injecting edge for a loop with the basic_block as loop head.
		 * \param loop_head The loop head node, identified by the BackwardJump at the end of the loop.
		 * \param loop_causing_back_edge The edge that causes the loop.
		 * \returns The injecting edge (either a forward or a meta edge) of the loop.
		 * This method only works for loop heads with only _one_ forward/meta edges in there input edges set. Otherwise the method fails with an assert().
		 */
		CFGEdge getLoopInjectingEdgeForLoopHead(CFGVertex loop_head, CFGEdge loop_causing_back_edge);
		/**
		 * \brief Checks if the given edge is a loop causing edge (a back edge directed to a loop head).
		 * Either this edge targets a loop edge with a forward step/jump or meta edge (only one is allowed) or it has only the back edge as in edge, then it is assumed that that there is a decision at loop tail.
		 * TODO: Crosscheck the decision if it is a loop causig back edge with entries in FlowFactReader.
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
		 * Wrapper for isConnectedViaPath(CFGVertex start, CFGVertex end, bool same_stack_level), calls isConnectedViaPath(start, end, true).
		 * \param start Start node for the path.
		 * \param end End node of the path.
		 * \returns True if there is a path between the two nodes, otherwise false.
		 */
		bool isConnectedViaPath(CFGVertex start, CFGVertex end);
		/**
		 * \brief Checks if two nodes are connected by a path.
		 * Therefore the method calls the getPath() method.
		 * \param start Start node for the path.
		 * \param end End node of the path.
		 * \param same_stack_level If set the start node and end node of the path should be in the same function.
		 * \returns True if there is a path between the two nodes, otherwise false.
		 */
		bool isConnectedViaPath(CFGVertex start, CFGVertex end, bool same_stack_level);
		/**
		 * \brief Finds a path from the start node to the end node, by using only ForwardStep, ForwardJump and Meta edges.
		 * The path search is context sensitive. Such that invalid paths build by traversing call / return points that do not math are not taken.
		 * The difference to the isNodeOnPath() function is that here no non loop causing back edges are used to check connection.
		 * Wrapper for getPath(CFGVertex start, CFGVertex end, bool same_stack_level), calls getPath(start, end, true)
		 * \param start Start node for the path (e.g. the loop head that is target of the loop causing back edge).
		 * \param end End node of the path (e.g. the loop bottom that is source of the loop causing back edge).
		 * \returns One possible path from start to end, including start and end node. If start and end nodes are equal the path contains only one node. If the path is empty no path exists.
		 */
		vector<CFGVertex> getPath(CFGVertex start, CFGVertex end);
		/**
		 * \brief Finds a path from the start node to the end node, by using only ForwardStep, ForwardJump and Meta edges.
		 * The path search is context sensitive. Such that invalid paths build by traversing call / return points that do not math are not taken.
		 * The difference to the isNodeOnPath() function is that here no non loop causing back edges are used to check connection.
		 * \param start Start node for the path (e.g. the loop head that is target of the loop causing back edge).
		 * \param end End node of the path (e.g. the loop bottom that is source of the loop causing back edge).
		 * \param same_stack_level If set the start node and end node of the path should be in the same function.
		 * \returns One possible path from start to end, including start and end node. If start and end nodes are equal the path contains only one node. If the path is empty no path exists.
		 */
		vector<CFGVertex> getPath(CFGVertex start, CFGVertex end, bool same_stack_level);
		/**
		 * \brief Gets the injecting edge for a loop body path.
		 * The method searches for an in-edge on the path that is not connected to another node on the path, furthermore the source node of this in-edge has not to be connected to the start and end node of the path (which are representing the loop head and the loop bottom node). The connection test is done by isNodeOnPath() and is needed since only one path of the loop body is represented in the path parameter not the whole loop body (that may contain different paths).
		 * \param path A path from loop head to loop bottom (determined by "following" the back_edge in getPath()).
		 * \param injecting_edge The pointer where the found edge should be written to, if one is found.
		 * \returns True if an injecting edge was found, else false.
		 */
		bool getInjectingEdgeForPath(vector<CFGVertex> path, CFGEdge* injecting_edge);

	
		CFGVertex cfg_entry;
		CFGVertex cfg_exit;
		ControlFlowGraph cfg;

		property_map<ControlFlowGraph, edgetype_t>::type edgeTypeEProp;
		property_map<ControlFlowGraph, circ_t>::type circEProp;
		property_map<ControlFlowGraph, startaddr_t>::type startAddrNProp;
		property_map<ControlFlowGraph, startaddrstring_t>::type startAddrStringNProp;
		property_map<ControlFlowGraph, nodetype_t>::type nodeTypeNProp;
		property_map<ControlFlowGraph, endaddr_t>::type endAddrNProp;

	private:

		static LoggerPtr logger;
};

#endif
