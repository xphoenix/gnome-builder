plugindir = $(libdir)/gnome-builder/plugins
plugin_LTLIBRARIES = libgnome-builder-plugin-clang-symbol-resolver.la
plugin_DATA = clang-symbol-resolver.plugin

libgnome_builder_plugin_clang_symbol_resolver_la_SOURCES = \
	ide-clang-symbol-resolver.c \
	ide-clang-symbol-resolver.h \
	$(NULL)

libgnome_builder_plugin_clang_symbol_resolver_la_CFLAGS = \
	-I$(top_srcdir)/libide \
	-I$(top_srcdir)/plugins/clang \
	$(BUILDER_CFLAGS) \
	$(DEBUG_CFLAGS) \
	$(OPTIMIZE_CFLAGS) \
	$(NULL)

libgnome_builder_plugin_clang_symbol_resolver_la_LIBADD = \
	$(BUILDER_LIBS) \
	$(NULL)

libgnome_builder_plugin_clang_symbol_resolver_la_LDFLAGS = \
	$(OPTIMIZE_LDFLAGS) \
	-avoid-version \
	-module \
	$(NULL)

EXTRA_DIST = $(plugin_DATA)

-include $(top_srcdir)/git.mk
