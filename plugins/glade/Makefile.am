DISTCLEANFILES =
BUILT_SOURCES =
CLEANFILES =
EXTRA_DIST = $(plugin_DATA)

plugindir = $(libdir)/gnome-builder/plugins
plugin_LTLIBRARIES = libglade.la
plugin_DATA = glade.plugin

libglade_la_SOURCES = \
	gb-glade-addin.c \
	gb-glade-addin.h \
	gb-glade-panel.c \
	gb-glade-panel.h \
	$(NULL)

nodist_libglade_la_SOURCES = \
	gb-glade-resources.c \
	gb-glade-resources.h

# XXX: temporary, since we need to extract lots of src/ into plugins
libglade_la_CFLAGS = \
	$(BUILDER_CFLAGS) \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/src/documents \
	-I$(top_srcdir)/src/editor \
	-I$(top_srcdir)/src/views \
	-I$(top_srcdir)/src/workbench \
	-I$(top_srcdir)/src/workspace \
	-I$(top_srcdir)/libide \
	-I$(top_srcdir)/contrib/egg \
	$(NULL)

libglade_la_LDFLAGS = \
	$(OPTIMIZE_LDFLAGS) \
	-avoid-version \
	-module \
	$(NULL)

glib_resources_c = gb-glade-resources.c
glib_resources_h = gb-glade-resources.h
glib_resources_xml = gb-glade.gresource.xml
glib_resources_namespace = gb_glade
include $(top_srcdir)/build/autotools/Makefile.am.gresources


-include $(top_srcdir)/git.mk
