if ENABLE_CMAKE_PLUGIN

plugindir = $(libdir)/gnome-builder/plugins
plugin_LTLIBRARIES = libcmake-plugin.la
dist_plugin_DATA = cmake.plugin

libcmake_plugin_la_SOURCES = \
	cmake-plugin.c \
	ide-cmake-builder.c \
	ide-cmake-builder.h \
	ide-cmake-build-task.c \
	ide-cmake-build-task.h \
	ide-cmake-build-system.c \
	ide-cmake-build-system.h \
	$(NULL)

libcmake_plugin_la_CFLAGS = $(PLUGIN_CFLAGS)
libcmake_plugin_la_LDFLAGS = $(PLUGIN_LDFLAGS)

include $(top_srcdir)/plugins/Makefile.plugin

endif

-include $(top_srcdir)/git.mk
