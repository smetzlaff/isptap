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
#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#include <stdint.h>
#include <assert.h>

#include <log4cxx/logger.h>
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/propertyconfigurator.h>
#include <log4cxx/helpers/exception.h>

//#define BB_COST_DO_NOT_DEPEND_ON_BB_EXIT

#define LOG_TRACE LOG4CXX_TRACE
#define LOG_DEBUG LOG4CXX_DEBUG
#define LOG_INFO LOG4CXX_INFO
#define LOG_WARN LOG4CXX_WARN
#define LOG_ERROR LOG4CXX_ERROR

#define UNUSED_PARAMETER(param) __attribute__((__unused__)) (param)

using namespace log4cxx;
using namespace log4cxx::helpers;

using namespace std;

// TODO move this to some better place
enum analysis_type_t { MUST = 0, MAY}; 
enum activation_type_t {CALL = 0, RETURN};

// TODO move to a better place
struct cache_hm_stat_t {
	uint64_t hits;
	uint64_t misses;
	uint64_t ncs;
};

#endif
