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
#ifndef _HELP_MESSAGE_TYPES_HPP_
#define _HELP_MESSAGE_TYPES_HPP_

struct help_message_t {
	string property;
	string help_message;
	string default_value;
};

#define PREPARE_OPTION_STRING(map, prop, dval, prepare_help_message) map.insert(pair<string, string>(prop, dval)); \
	if(prepare_help_message){\
	help_message_t tmp; \
	tmp.property = prop; \
	tmp.help_message = HELP_ ## prop; \
	tmp.default_value = dval; \
	config_properties.push_back(tmp); \
	}

#define PREPARE_OPTION_UINT(map, prop, dval, prepare_help_message) map.insert(pair<string, uint32_t>(prop, dval)); \
	if(prepare_help_message){\
	help_message_t tmp; \
	tmp.property = prop; \
	tmp.help_message = HELP_ ## prop; \
	{ \
		stringstream s; \
		s << dval; \
		tmp.default_value = s.str(); \
	} \
	config_properties.push_back(tmp); \
	}

#define ADD_HELP_MESSAGE(prop, prepare_help_message) if(prepare_help_message){\
	help_message_t tmp; \
	tmp.property = prop; \
	tmp.help_message = HELP_ ## prop; \
	config_properties.push_back(tmp); \
	}

#endif
