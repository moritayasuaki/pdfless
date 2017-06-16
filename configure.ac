#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([pdfless], [0.2], [zousandazou@gmail.com])
AC_CONFIG_SRCDIR([pdfless.cpp])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([foreign])

# Checks for programs.
AC_PROG_CXX

# Checks for libraries.
AC_CHECK_LIB([aa], [aa_init], ,[
    AC_MSG_ERROR([unable to find the aalib])
])

# Checks for header files.
AC_CHECK_HEADER(aalib.h, ,[
    AC_MSG_ERROR([unable to find aalib.h])
])

AC_LANG_PUSH([C++])
AC_CHECK_HEADERS([poppler/cpp/poppler-document.h poppler/cpp/poppler-image.h poppler/cpp/poppler-page-renderer.h poppler/cpp/poppler-page.h], ,[
    AC_MSG_ERROR([unable to find poppler headers])
])
AC_LANG_POP([C++])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL

# Checks for library functions.

AC_CONFIG_FILES([Makefile])
AC_OUTPUT

