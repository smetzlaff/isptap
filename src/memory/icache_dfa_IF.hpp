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
#ifndef _ICACHE_DFA_IF_HPP_
#define _ICACHE_DFA_IF_HPP_

#include "global.h"
#include "constants.h"
#include "configuration.hpp"
#include "graph_structure.h"
#include "vivug_creator.hpp"
#include "arch_cfg.hpp"
#include "memory_params.hpp"

#include <iostream>
#include <iomanip>

/*!
 * \brief Class to analyse the data flow of a fully accociative instruction cache.
 * Analyzes the instruction cache hit/miss rate of a given memory state graph / control flow graph pair.
 */
class ICacheDataFlowAnalyzer_IF {
	public:
		/*!
		 * \brief Destructor.
		 */
		virtual ~ICacheDataFlowAnalyzer_IF();
		/*!
		 * \brief Analyzes the cache for the given parameters (or obtained by global configuration object).
		 */
		virtual void analyzeCache(void)=0;

		/*!
		 * \brief Counts all Hits, Misses and NC and the number of unique cache lines to print a log line.
		 */
		virtual void categorizeCacheAccesses(void)=0;
		/*!
		 * \brief Returns the pair of graphs (memory state graph and the corresponding control flow graph) that under analysis.
		 * After calling analyzeCache() the dynamicMemPenaltyEProp is added to the MSG.
		 * \returns The pair of graphs (memory state graph and the corresponding control flow graph) that under analysis.
		 */
		CFGMSGPair getGraph(void);

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
		/**
		 * \brief Returns the memory size that is used for the representation of the cache memory states in the data flow analysis.
		 * Depending on the analysis type either the abstract states or the concrete states are taken into account.
		 * \returns The memory size that is used for the representation of the cache memory states in the data flow analysis.
		 */
		virtual uint64_t getUsedMemSize(void) = 0;
		/*!
		 * \brief Returns the number of maintained memory references (cache lines or function addresses) that is needed by the memory states used by the data flow analysis.
		 * Depending on the analysis type either the abstract states or the concrete states are taken into account.
		 * \returns The number of maintained memory references that is needed by the memory states used by the data flow analysis.
		 */
		virtual uint64_t getUsedMemReferences(void) = 0;
		/**
		 * \brief Returns the number of concrete cache memory states needed for the data flow analysis.
		 * Depending on the analysis type either the abstract states or the concrete states are taken into account.
		 * \returns The number of concrete cache memory states needed for the data flow analysis.
		 */
		virtual uint64_t getRepresentationStateCount(void) = 0;
		/*!
		 * \brief Returns the different memory states that are distinguished by the data flow analysis.
		 * \returns The different memory states that are distinguished by the data flow analysis.
		 */
		virtual uint64_t getMemoryStateCount(void) = 0;
	protected:
		/*!
		 * \brief Sets the cache parameters.
		 * This function is called on construction using the parameters of the global configuration object.
		 */
		virtual void setCacheParameters(void)=0;
		/*!
		 * \brief The pair of graphs containing the memory state graph and it's reference control flow graph under analysis.
		 */
		CFGMSGPair graph;
		/*!
		 * \brief Entry node of the graph pair.
		 */
		CFGMSGVPair entry;
		/*!
		 * \brief Exit node of the graph pair.
		 */
		CFGMSGVPair exit;
		/*!
		 * \brief The memory parameters of the cache under analysis
		 */
		cache_params_t cache_parameters;
		/**
		 * \brief Pointer to the global architecutre configuration object.
		 */
		ArchConfig *arch_cfg;
//		/**
//		 * \brief Pointer to the LOGCXX logger object.
//		 */
//		static LoggerPtr logger;
};

#endif
