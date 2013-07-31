#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([ISPTAP], [1.0-0], [https://github.com/smetzlaff/isptap], [isptap], [https://github.com/smetzlaff/isptap])
AC_COPYRIGHT([Copyright (C) 2013 Stefan Metzlaff, University of Augsburg, Germany
All rights reserved. This program and the accompanying materials
are made available under the terms of the GNU Public License v3.0
which accompanies this distribution, and is available at
http://www.gnu.org/licenses/gpl.html
])
AC_CONFIG_SRCDIR([src/main.cpp])
AC_CONFIG_HEADERS([config.h])

AC_PREFIX_DEFAULT(/opt/isptap)


# Checks for programs.
AC_PROG_AWK
AC_PROG_CXX
AC_PROG_CC

AC_CHECK_TOOL([is_lpsolve],[lp_solve])
AC_CHECK_TOOL([is_makedepend], [makedepend])
AC_CHECK_TOOL([is_doxy], [doxygen])

# Checks for libraries.
AC_CHECK_LIB([boost_graph], [main])
AC_CHECK_LIB([boost_regex], [main])
# FIXME: Replace `main' with a function in `-lboost_program_options':
AC_CHECK_LIB([boost_program_options], [main])
# FIXME: Replace `main' with a function in `-llog4cxx':
AC_CHECK_LIB([log4cxx], [main])
AC_CHECK_LIB([lpsolve55], [main])

# Checks for header files.
AC_CHECK_HEADERS([stdint.h stdlib.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_HEADER_ASSERT
AC_HEADER_TIME
AC_C_INLINE
AC_TYPE_INT32_T
AC_TYPE_SIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_STRTOD
AC_CHECK_FUNCS([strtol strtoul])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
