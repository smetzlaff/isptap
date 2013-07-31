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
#include "console_string_printer.hpp"

ConsoleStringPrinter::ConsoleStringPrinter(void)
{
	line_length = LINE_LENGTH;
}

void ConsoleStringPrinter::setLineLength(size_t lineLength)
{
	line_length = lineLength;
}

string ConsoleStringPrinter::chopToLineLength(const string p)
{
	size_t next_break = line_length;
	string s = p;
	while(s.length() > next_break)
	{
		size_t break_pos = s.find(" ", next_break);
		if(break_pos >= s.length())
		{
			// no whitespace found
			break;
		}
		s.insert(break_pos, "\n");
		// remove blank at break_pos
		next_break = break_pos + LINE_LENGTH;
	}
	return s;
}

string ConsoleStringPrinter::chopToLineLength(const string p, const string delimiter)
{
	size_t next_break = line_length;
	string s = p;
	while(s.length() > next_break)
	{
		size_t break_pos = s.find(" ", next_break);
		if(break_pos >= s.length())
		{
			// no whitespace found
			break;
		}
		s.insert(break_pos, delimiter);
		s.insert(break_pos+delimiter.length(), "\n");
		next_break = break_pos + LINE_LENGTH;
	}
	return s;
}
