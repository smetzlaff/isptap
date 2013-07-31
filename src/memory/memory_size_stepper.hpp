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
#ifndef _MEMORY_SIZE_STEPPER_HPP_
#define _MEMORY_SIZE_STEPPER_HPP_

#include "global.h"
#include "constants.h"
#include "configuration.hpp"
#include "carcore_cfg.hpp"
#include "memory_params.hpp"


class MemorySizeStepper {

	public:
		MemorySizeStepper();
		virtual ~MemorySizeStepper();

		uint32_t getMemorySize(void);

		cache_params_t getCacheParameters(void);
		disp_params_t getDispParameters(void);
		sisp_params_t getSispParameters(void);
		nomem_params_t getNoMemParameters(void);

		bool isValidSize(void);
		void increaseSize(void);

	private:
		MemoryParameters mp;
		uint32_t actual_size;
		uint32_t min_size;
		uint32_t max_size;
		uint32_t step_size;

		/**
		 * \brief Pointer to the global configuration object.
		 */
		Configuration *conf;
		/**
		 * \brief Pointer to the global architecutre configuration object.
		 */
		CarCoreConfig *cccfg;


};

#endif
