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
#ifndef _BBSISP_OPTIMIZER_JP_HPP_
#define _BBSISP_OPTIMIZER_JP_HPP_

#include "bbsisp_optimizer.hpp"
#include "arch_cfg.hpp"
#include "isa.hpp"
#include "dumpline_parser.hpp"

//#define ENABLE_IW_FLUSH_PENALTY


/**
 * \brief A structure to store consecutive basic blocks
 */
struct bb_connect_t {
	/**
	 * \brief The id of the source bb in the bb_data
	 */
	uint32_t src_id;
	/**
	 * \brief The id of the target bb in the bb_data
	 */
	uint32_t tgt_id;
	/**
	 * \brief The number of activations of the connective edge on the WCET-critical path
	 */
	uint32_t activation_count;
	/**
	 * \brief The connection type of both consecutive blocks.
	 */
	connection_type_t type;
};


/**
 * \brief Structure for all necessary basic block informations.
 * Needed by BBSISPOptimizer to store all informations for each basic block.
 */
struct bb_record_t {
	/**
	 * \brief Basic block ID used for the ILP generation as valiable identifier.
	 */
	uint32_t id;
	/**
	 * \brief Address of the basic block.
	 */
	uint32_t address;
	/**
	 * \brief Size of the basic block in bytes.
	 */
	uint32_t size;
	/**
	 * \brief The benefit of the basic block, if the function is contained in the scratchpad.
	 * The benefit depends on the used optimization metric (CONF_USE_RATIO_FILES, CONF_USE_DYNAMIC_INSTRUCTION_COUNT_AS_METRIC, CONF_USE_WCET_PATH_LENGTH_AS_METRIC).
	 */
	uint32_t benefit;
};



/**
 * \brief Assigns the id of the basic block (that is used in the LP formulation to identify the basic block) to its vertex in the control flow graph.
 */
typedef map<CFGVertex, uint32_t> BbidCfgMap;
/**
 * \brief Assigns the type of the displacement from the opcode of the last instruction to the address of the basic block to which the instruction belongs.
 */
typedef map<uint32_t, displacement_type_t> DisplacementMap;

/**
 * \brief Optimization for a static instruction scratchpad, with basic blocks as granularity.
 * Note: the cost for jumping into the scratchpad and jumping back is not considered yet.
 * The optimization depends on the used metric (CONF_USE_RATIO_FILES, CONF_USE_DYNAMIC_INSTRUCTION_COUNT_AS_METRIC, CONF_USE_WCET_PATH_LENGTH_AS_METRIC).
 */
class BBSISPOptimizerJP : public BBSISPOptimizer {
	public:
		/**
		 * \brief Default constructor.
		 */
		BBSISPOptimizerJP(ControlFlowGraph cfgraph, CFGVertex entry, CFGVertex exit);
		/**
		 * \brief Default destructor.
		 */
		virtual ~BBSISPOptimizerJP();
		/**
		 * \brief Determines the block assignment. The implementation of the function depends on the type of optimisation.
		 */
		virtual void calculateBlockAssignment(void);
		/**
		 * \brief Returns the jump penalty that was determined during solving the BBSISP assignment ILP.
		 * \returns The jump penalty that was determined during solving the BBSISP assignmend ILP.
		 */
		uint32_t getEstimatedJumpPenalty(void);

		virtual sisp_result_t getResults(void);
	private:
		/**
		 * \brief Sets the assignment found by the LpSolver.
		 * \param lpresult Vector with the basic block variable and its assignment value (1 for in scratchpad, else 0)
		 */
		virtual void setAssignment(vector<lp_result_set> lpresult);
		/**
		 * \brief Gather all necessary data for each basic block and store it in the bb_data vector.
		 */
		void generateBBRecords(void);
		/**
		 * \brief Gather all necessary data for connected bbs and store it in the bb_connect vector.
		 */
		void generateBBConnections(void);
		/**
		 * \brief Delivers all id of the bb_connect vector that have the given src_id as source.
		 * \param scr_id The source block to find all connections for.
		 * \returns All ids of the bb_connect vector that have the same src_id as the parameter.
		 */
		vector<uint32_t> getBlockConnectionIds(uint32_t src_id);
		/**
		 * \brief Clears all memory that was dynamically aquired by the class during runtime.
		 */
		virtual void clear(void);
		/**
		 * \brief Generates the ILP for the Knapsack formulation of the optimization. 
		 * The ILP objective function and its constraints are stored in ilp_knapsack_formulation
		 */
		void generateKnapsackILPFormulation(void);
		/**
		 * \brief Registers the node of the CFG to its assigned node id in the bbidMap. 
		 * \param v The id of the vertex in the CFG.
		 * \param node_id The id used for the flow variables.
		 */
		virtual void registerNodeIdMapping(CFGVertex v, uint32_t node_id);
		/**
		 * \brief Returns the type of the displacement of a jump for a given basic block id.
		 * For looking up the displacement type the classes dispMap is used. This map is created during generateBBRecords() by registerBlockLastInstrDisplacement().
		 * \param id The basic block id of the basic block, stored in id field of the bb_data vector.
		 * \returns The displacement type of the instruction that terminates the basic block. If the instruction is no control flow changing instruction, NoDisplacement is returned. On any error UnknownDisplacementType is returned.
		 */
		displacement_type_t getDisplacementTypeOfLastBBInstruction(uint32_t id);
		/**
		 * \brief Registers for a basic block the displacement type of the last instruction.
		 * If the last instruction is a jump or call, the opcode is analyzed and the type of the displacement part is returned.
		 * For the case that a basic block is not terminated by a jump instruction the displacement type NoDisplacement is used.
		 * To store the displacement type information all registered basic blocks the dispMap is used. As key for accessing the displacement type the basic block address is used.
		 * \param bb_addr The address of the basic block of whicht the displacement type for the last instruction is to be registered.
		 * \param bb_code The code of the basic block.
		 */
		void registerBlockLastInstrDisplacement(uint32_t bb_addr, string bb_code);
		/**
		 * \brief Generates the size constraint of a basic block including the penalty due to additional flow conserving effort.
		 * \param id The index of a basic block in the bb_data vector.
		 * \returns The formulation of the dynamic basic block size (including any penalties).
		 * A dynamic size contraint is for example as follows:
		 * sp3 = (16 + 4 + 2) a3 - 4 a4 - 2 a5; 
		 * The bb has a size of 16 but if it is located in the scratchpad and a4 and a2 not a size penalty of 6 is to be considered.
		 */
		string getBBSizeWithPenaltyConstraint(uint32_t id);
		/**
		 * \brief Generates the XOR constraints of a registered basic block connection.
		 * \param id The id determines the number of the basic block connection as in the bb_connect vector.
		 * \returns The formulation of the assignment of XOR between two consecutive basic blocks.
		 * An example is:
		 * xor3004 <= a3 + a4;
		 * xor3004 >= a3 - a4;
		 * xor3004 >= - a3 + a4;
		 * xor3004 <= 2 - a3 - a4;
		 * Thus a3 XOR a4 can be modelled in the LP.
		 */
		string getXorConstraint(uint32_t id);

		/**
		 * \brief Generates the ANDN constraints of a registered basic block connection.
		 * \param id The id determines the number of the basic block connection as in the bb_connect vector.
		 * \returns The formulation of the assignment of ANDN between two consecutive basic blocks.
		 * An example is:
		 * xor3004 <= a3;
		 * xor3004 <= a4;
		 * xor3004 + 1 >= a3 + 1 - a4;
		 * Thus a3 AND !a4 can be modelled in the LP.
		 */
		string getAndNConstraint(uint32_t id);
		/**
		 * \brief Small inline function that pushes a string into the binary constraint vector, if the string is not empty.
		 * \param bin_constraint String representing an XOR or an ANDN constraint to be pushed into the binary_constraints vector, but only if it is not empty.
		 */
		void bin_constraint_push_back(string bin_constraint);
		/**
		 * \brief Sets variables that is calculated during solving the BB assingment ILP
		 * \param lp_result The variable vector from the lp_solver, that contains global variables e.g. "wentry", "sp" and "jp"
		 */
		virtual void setVariables(vector<lp_result_set> lp_result);

#ifdef ENABLE_IW_FLUSH_PENALTY
		bool isNodeEnteredOnlyByContinuousAdressing(uint32_t node);
#endif
		/**
		 * \brief All necessary data for each basic block, see structure for details.
		 */
		vector<bb_record_t> bb_data;
		/**
		 * \brief The source, target and type of directly connected basic blocks.
		 */
		vector<bb_connect_t> bb_connect;

		/**
		 * \brief Vector containing all xor and andn constraints of the ILP
		 */
		vector<string> binary_constraints;
		/**
		 * \brief Map that registers every cfg bb node to its internal id
		 */
		BbidCfgMap bbidMap;
		/**
		 * \brief Map that registers for every basic block the type of the displacement of its last instruction.
		 * If the last instruction is a control flow changing instruction the displacement_type_t of this instruction is
		 * registered to the address of the basic block (can be disp4, disp8, ... indirect). If the last instruction is no
		 * jump or call the type NoDisplacement is assigned to this basic block.
		 */
		DisplacementMap dispMap;
		/**
		 * \brief Contains the calculated jump penalty obtained from the BBSISP assignment ILP.
		 */
		uint32_t ilp_solution_jump_penalty;

		/**
		 * \brief Graph property that contains the code of the basic blocks.
		 */
		property_map<ControlFlowGraph, bbcode_t>::type bbCodeNProp;
		/**
		 * \brief Pointer to object for parsing the lines of the dump files.
		 */
		DumpLineParser *dlp;
		/**
		 * \brief Pointer to object for decoding and encoding instructions for the used ISA.
		 */
		ISA *isa;
		/**
		 * \brief Pointer to the global architectural configuration object.
		 */
		ArchConfig *arch_cfg;
};

#endif
