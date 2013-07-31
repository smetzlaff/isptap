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
#include "cache_state_maintainer.hpp"

LoggerPtr CacheStateMaintainer::logger(Logger::getLogger("CacheStateMaintainer"));


CacheStateMaintainer::CacheStateMaintainer(cache_params_t params)
{
	cache_parameters = params;
	// TODO: Refactor parameters! Use ways for LRU/FIFO to prepare associative cache analysis.
	assert((cache_parameters.size != 0) && (cache_parameters.line_size != 0) && (cache_parameters.line_size_bit != 0));
}
