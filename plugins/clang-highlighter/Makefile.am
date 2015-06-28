plugindir = $(libdir)/gnome-builder/plugins
plugin_LTLIBRARIES = libgnome-builder-plugin-clang-highlighter.la
plugin_DATA = clang-highlighter.plugin

libgnome_builder_plugin_clang_highlighter_la_SOURCES = \
	ide-clang-highlighter.c \
	ide-clang-highlighter.h \
	$(NULL)

libgnome_builder_plugin_clang_highlighter_la_CFLAGS = \
	-I$(top_srcdir)/libide \
	-I$(top_srcdir)/contrib/egg \
	-I$(top_srcdir)/plugins/clang \
	$(BUILDER_CFLAGS) \
	$(DEBUG_CFLAGS) \
	$(OPTIMIZE_CFLAGS) \
	$(NULL)

libgnome_builder_plugin_clang_highlighter_la_LIBADD = \
	$(BUILDER_LIBS) \
	$(top_builddir)/plugins/clang/libgnome-builder-plugin-clang.la \
	$(NULL)

libgnome_builder_plugin_clang_highlighter_la_LDFLAGS = \
	$(OPTIMIZE_LDFLAGS) \
	-avoid-version \
	-module \
	$(NULL)

EXTRA_DIST = $(plugin_DATA)

-include $(top_srcdir)/git.mk
