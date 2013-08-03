dnl ************************************************************************
dnl Versioning Macros
dnl ************************************************************************
m4_define([gb_major_version], [0])
m4_define([gb_minor_version], [1])
m4_define([gb_micro_version], [1])
m4_define([gb_version],
          [gb_major_version.gb_minor_version.gb_micro_version])
m4_define([gb_binary_age], [m4_eval(100 * gb_minor_version +
           gb_micro_version)])
m4_define([lt_current], [m4_eval(100 * gb_minor_version +
           gb_micro_version - gb_interface_age)])
m4_define([lt_revision], [gb_interface_age])
m4_define([lt_age], [m4_eval(gb_binary_age - gb_interface_age)])


dnl ************************************************************************
dnl Initialize Automake
dnl ************************************************************************
AC_PREREQ([2.64])
AC_INIT([gb],[gb_version])

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([-Wno-portability])
AM_MAINTAINER_MODE([enable])

m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES])
AS_AM_REALLY_SILENT

# Check for programs
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_LEX
AC_PROG_YACC
AM_PATH_GLIB_2_0
GLIB_GSETTINGS

# Initialize libtool
LT_PREREQ([2.2])
LT_INIT


dnl ************************************************************************
dnl Versioning Definitions
dnl ************************************************************************
GB_MAJOR_VERSION=gb_major_version
GB_MINOR_VERSION=gb_minor_version
GB_MICRO_VERSION=gb_micro_version
GB_API_VERSION=1.0
GB_VERSION=gb_version
AC_SUBST(GB_MAJOR_VERSION)
AC_SUBST(GB_MINOR_VERSION)
AC_SUBST(GB_MICRO_VERSION)
AC_SUBST(GB_API_VERSION)
AC_SUBST(GB_VERSION)


dnl ************************************************************************
dnl Check for Required Modules
dnl ************************************************************************
PKG_CHECK_MODULES(DEVHELP,       [libdevhelp-3.0 >= 3.2.0])
PKG_CHECK_MODULES(GIR,           [gobject-introspection-1.0 >= 0.10.2])
PKG_CHECK_MODULES(GOBJECT,       [gobject-2.0 >= 2.28])
PKG_CHECK_MODULES(GTK3,          [gtk+-3.0 >= 3.9.11])
PKG_CHECK_MODULES(GTKSOURCEVIEW, [gtksourceview-3.0 >= 3.2.0])
PKG_CHECK_MODULES(JSON,          [json-glib-1.0 >= 0.14.0])
PKG_CHECK_MODULES(VTE,           [vte-2.90 >= 0.30])


dnl ************************************************************************
dnl Check for xmllint
dnl ************************************************************************
AC_CHECK_PROG(XMLLINT, xmllint)
AC_SUBST(XMLLINT)


dnl ************************************************************************
dnl Check for clang headers.
dnl ************************************************************************
AC_CHECK_HEADER([clang-c/Index.h])


dnl ************************************************************************
dnl Enable extra debugging options
dnl ************************************************************************
m4_define([debug_default],
	  [m4_if(m4_eval(gb_minor_version % 2), [1],
	  [yes], [minimum])])
AC_ARG_ENABLE([debug],
	      [AS_HELP_STRING([--enable-debug=@<:@no/minimum/yes@:>@],
	      		      [turn on debugging @<:@default=debug_default@:>@])],
	      		      [],
	      		      [enable_debug=debug_default])
AS_CASE([$enable_debug],
	[yes], [
		CFLAGS="$CFLAGS -Wold-style-definition"
		CFLAGS="$CFLAGS -Wdeclaration-after-statement"
		CFLAGS="$CFLAGS -Wredundant-decls"
		CFLAGS="$CFLAGS -Wmissing-noreturn"
		CFLAGS="$CFLAGS -Wcast-align"
		CFLAGS="$CFLAGS -Wwrite-strings"
		CFLAGS="$CFLAGS -Winline"
		CFLAGS="$CFLAGS -Wformat"
		CFLAGS="$CFLAGS -Wformat-nonliteral"
		CFLAGS="$CFLAGS -Wformat-security"
		CFLAGS="$CFLAGS -Wswitch-enum"
		CFLAGS="$CFLAGS -Wswitch-default"
		CFLAGS="$CFLAGS -Winit-self"
		CFLAGS="$CFLAGS -Wmissing-include-dirs"
		CFLAGS="$CFLAGS -Wundef"
		CFLAGS="$CFLAGS -Waggregate-return"
		CFLAGS="$CFLAGS -Wmissing-format-attribute"
		CFLAGS="$CFLAGS -Wnested-externs"
		CFLAGS="$CFLAGS -Wshadow"
		CFLAGS="$CFLAGS -DGB_ENABLE_TRACE"
	],
	[minimum], [
		CFLAGS="$CFLAGS -DG_DISABLE_CAST_CHECKS"
	],
	[no], [
		CFLAGS="$CFLAGS -DG_DISABLE_ASSERT"
		CFLAGS="$CFLAGS -DG_DISABLE_CHECKS"
		CFLAGS="$CFLAGS -DG_DISABLE_CAST_CHECKS"
	],
	[AC_MSG_ERROR([Unknown argument to --enable-debug])]
)


dnl ************************************************************************
dnl Unit Tests
dnl ************************************************************************
AC_PATH_PROG([GTESTER], [gtester])
AC_PATH_PROG([GTESTER_REPORT], [gtester-report])
AM_CONDITIONAL(ENABLE_GLIB_TEST, test "x$enable_glibtest" = "xyes")


dnl ************************************************************************
dnl Miscellaneous
dnl ************************************************************************
AC_PATH_PROG([XMLLINT], [xmllint])
GTK_DOC_CHECK([1.11])
GOBJECT_INTROSPECTION_CHECK([0.6.7])


dnl ************************************************************************
dnl Output
dnl ************************************************************************
AC_CONFIG_FILES([
	Makefile
	gnome-builder-1.0.pc
])
AC_OUTPUT

echo ""
echo "Gnome-Builder $VERSION"
echo ""
echo "  Prefix.....................: ${prefix}"
echo "  Debug Level................: ${enable_debug}"
echo "  Compiler Flags.............: ${CFLAGS}"
echo "  Enable API Reference.......: ${enable_gtk_doc}"
echo "  Enable Test Suite..........: ${enable_glibtest}"
echo ""
