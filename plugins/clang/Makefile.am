plugindir = $(libdir)/gnome-builder/plugins
plugin_LTLIBRARIES = libgnome-builder-plugin-clang.la
plugin_DATA = clang.plugin

libgnome_builder_plugin_clang_la_SOURCES = \
	gb-clang-plugin.c \
	ide-clang-completion-item.c \
	ide-clang-completion-item.h \
	ide-clang-private.h \
	ide-clang-service.c \
	ide-clang-service.h \
	ide-clang-symbol-node.c \
	ide-clang-symbol-node.h \
	ide-clang-symbol-tree.c \
	ide-clang-symbol-tree.h \
	ide-clang-translation-unit.c \
	ide-clang-translation-unit.h \
	$(NULL)

libgnome_builder_plugin_clang_la_CFLAGS = \
	-I$(top_srcdir)/libide \
	-I$(top_srcdir)/contrib/egg \
	$(BUILDER_CFLAGS) \
	$(DEBUG_CFLAGS) \
	$(OPTIMIZE_CFLAGS) \
	$(NULL)

libgnome_builder_plugin_clang_la_LIBADD = \
	$(BUILDER_LIBS) \
	-lclang \
	$(NULL)

libgnome_builder_plugin_clang_la_LDFLAGS = \
	$(CLANG_LDFLAGS) \
	$(OPTIMIZE_LDFLAGS) \
	-avoid-version \
	-module \
	$(NULL)

EXTRA_DIST = $(plugin_DATA)

-include $(top_srcdir)/git.mk
