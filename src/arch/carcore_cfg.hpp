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
#ifndef _CARCORE_CONFIG_HPP_
#define _CARCORE_CONFIG_HPP_


#include "arch_cfg.hpp"
#include "tricore_instrset.h"
#include "tricore_isa.hpp"

#define CARCORE_BRANCH_LATENCY "branch_latency"
#define CARCORE_CALL_LATENCY "call_latency"
#define CARCORE_RETURN_LATENCY "return_latency"

#define CARCORE_FETCH_INDEPENDENT_IMEM "fetch_independent_imem"
#define CARCORE_FETCH_OPTIMIZATION_BRANCH_AHEAD "use_fetch_optimization_branch_ahead"
#define CARCORE_FETCH_OPTIMIZATION_ENOUGH_INSTRS "use_fetch_optimization_enough_instrs"

// Latency configuration for additional latencies for floating point instructions (Notice that 0 means the instruction needs one cycle to be executed.)
#define FP_ARITHMETIC_LATENCY "fp_latency_arithmetic" // additional latencies for F_ADD, F_SUB, F_CMP
#define FP_DIVIDE_LATENCY "fp_latency_divide" // additional latencies for F_DIV
#define FP_CONVERSION_LATENCY "fp_latency_conversion" // additional latencies for  F_FTOI, F_FTOIZ, F_FTOQ31, F_FTOQ31Z, F_FTOU, F_FTOUZ, F_ITOF, F_Q31TOF, F_UTOF
#define FP_MULTIPLYACCUMULATE_LATENCY "fp_latency_multiplyaccumulate" // additional latencies for F_MADD, F_MSUB
#define FP_MULTIPLY_LATENCY "fp_latency_multiply" // additional latencies for F_MUL
#define FP_SQRTSEED_LATENCY "fp_latency_sqrtseed" // additional latencies for F_SQSEED
#define FP_UPDATEFLAGS_LATENCY "fp_latency_updateflags" // additional latencies for F_UPDFL


/// Type definition for different latency classes of floating point instructions
typedef enum 
{
	ARITHMETIC,
	DIVIDE,
	CONVERSION,
	MULTIPLYACCUMULATE,
	MULTIPLY,
	SQRTSEED,
	UPDATEFLAGS
} fp_op_latency_t;

class CarCoreConfig : public ArchConfig {
	public:
		static CarCoreConfig* getInstance(void);
		static CarCoreConfig* getInstanceWithHelpMessages(void);

		uint32_t getLMSLatency(int32_t mem_width, bool onchip);
		uint32_t getBranchLatency(void);
		uint32_t getCallLatency(bool onchip);
		uint32_t getReturnLatency(bool onchip);
		uint32_t getFPLatency(fp_op_latency_t type);
		uint32_t getMCLatency(alu_func_t micro_code);
		bool isFetchMemIndependent(void);
		bool isFetchOptBranchAhead(void);
		bool isFetchOptEnoughInstr(void);
		
		virtual string getHelpMessage(void);

	private:
		// function called by public members: getInstance(void) and getInstanceWithHelpMessages()
		static CarCoreConfig* getInstance(bool initialise_help_messages);
		CarCoreConfig(bool initialise_help_messages);
		virtual ~CarCoreConfig();

		static CarCoreConfig *singleton;

		/**
		 * \brief Pointer to the LOGCXX logger object.
		 */
		static LoggerPtr logger;

};

#endif
