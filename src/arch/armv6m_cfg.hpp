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
#ifndef _ARMV6M_CONFIG_HPP_
#define _ARMV6M_CONFIG_HPP_

#include "arch_cfg.hpp"

#define ARMV6M_ARITHMETIC_OP_LATENCY "arithmetic"
#define ARMV6M_ARITHMETIC_OP_LATENCY_WITH_PC "arithmetic_with_pc"
#define ARMV6M_LOGIC_OP_LATENCY "logic"
#define ARMV6M_MULTIPLY_LATENCY "multiply"

#define ARMV6M_MRS_LATENCY "read_special_reg"
#define ARMV6M_MSR_LATENCY "write_special_reg"

#define ARMV6M_BC_TAKEN_LATENCY "branch_conditional_taken_latency"
#define ARMV6M_BC_NOTTAKEN_LATENCY "branch_conditional_nottaken_latency"
#define ARMV6M_B_LATENCY "branch_unconditional_latency"
#define ARMV6M_BL_LATENCY "branch_and_link_latency"
#define ARMV6M_BX_LATENCY "branch_and_exchange_latency"
#define ARMV6M_BLX_LATENCY "branch_link_and_exchange_latency"
#define ARMV6M_POP_AND_RETURN_EXTRA_LATENCY "pop_and_return_extra_latency"

#define ARMV6M_DSB_LATENCY "data_sync_barrier"
#define ARMV6M_DMB_LATENCY "data_mem_barrier"
#define ARMV6M_ISB_LATENCY "instr_sync_barrier"


class Armv6mConfig : public ArchConfig {
	public:
		static Armv6mConfig *getInstance(void);
		static Armv6mConfig* getInstanceWithHelpMessages(void);

		uint32_t getArithmeticOpLatency(bool PcAsTarget);
		uint32_t getLogicOpLatency(void);
		uint32_t getMultLatency(void);
		uint32_t getMRSLatency(void);
		uint32_t getMSRLatency(void);
		uint32_t getUncondBranchLatency(void);
		uint32_t getCondBranchLatency(bool taken);
		uint32_t getBLLatency(void);
		uint32_t getBXLatency(void);
		uint32_t getBLXLatency(void);
		uint32_t getPopAndReturnExtraLatency(void);
		uint32_t getDSBLatency(void);
		uint32_t getDMBLatency(void);
		uint32_t getISBLatency(void);

		virtual uint32_t getCallLatency(bool onchip);
		virtual uint32_t getReturnLatency(bool onchip);
		virtual bool isFetchMemIndependent(void);

		virtual string getHelpMessage(void);
	private:
		// function called by public members: getInstance(void) and getInstanceWithHelpMessages()
		static Armv6mConfig* getInstance(bool initialise_help_messages);
		Armv6mConfig(bool initialise_help_messages);
		virtual ~Armv6mConfig();

		static Armv6mConfig *singleton;

		/**
		 * \brief Pointer to the LOGCXX logger object.
		 */
		static LoggerPtr logger;
};


#endif 
