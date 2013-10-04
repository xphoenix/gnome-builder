-include $(top_srcdir)/Makefile.introspection

bin_PROGRAMS =
noinst_LTLIBRARIES =


AM_CPPFLAGS = \
	$(DEVHELP_CFLAGS) \
	$(GIR_CFLAGS) \
	$(GJS_CFLAGS) \
	$(GOBJECT_CFLAGS) \
	$(GTK3_CFLAGS) \
	$(GTKSOURCEVIEW_CFLAGS) \
	$(JSON_CFLAGS) \
	$(VTE_CFLAGS) \
	-DG_LOG_DOMAIN=\"Builder\" \
	-I$(top_srcdir)/src \
	-I$(top_builddir)/src \
	-I$(top_srcdir)/src/animation \
	-I$(top_srcdir)/src/app \
	-I$(top_srcdir)/src/compiler \
	-I$(top_srcdir)/src/dbus \
	-I$(top_srcdir)/src/debugger \
	-I$(top_srcdir)/src/docs \
	-I$(top_srcdir)/src/language \
	-I$(top_srcdir)/src/log \
	-I$(top_srcdir)/src/multiprocess \
	-I$(top_srcdir)/src/nautilus \
	-I$(top_srcdir)/src/profiler \
	-I$(top_srcdir)/src/project \
	-I$(top_srcdir)/src/runner \
	-I$(top_srcdir)/src/search \
	-I$(top_srcdir)/src/source \
	-I$(top_srcdir)/src/symbol \
	-I$(top_srcdir)/src/terminal \
	-I$(top_srcdir)/src/trie \
	-I$(top_srcdir)/src/worker \
	-I$(top_srcdir)/src/workspace


AM_LDFLAGS = \
	-lclang \
	-lm \
	-L/usr/lib64/llvm \
	$(DEVHELP_LIBS) \
	$(GIR_LIBS) \
	$(GJS_LIBS) \
	$(GOBJECT_LIBS) \
	$(GTK3_LIBS) \
	$(GTKSOURCEVIEW_LIBS) \
	$(JSON_LIBS) \
	$(VTE_LIBS)


noinst_LTLIBRARIES += libanimation.la
libanimation_la_SOURCES = \
	animation/gb-animation.c \
	animation/gb-animation.h \
	animation/gb-frame-source.c \
	animation/gb-frame-source.h


noinst_LTLIBRARIES += libapp.la
libapp_la_DEPS = \
	$(GB_APPLICATION_RESOURCE_C) \
	$(GB_APPLICATION_RESOURCE_H)
libapp_la_LIBADD = \
	libanimation.la \
	libmultiprocess.la
libapp_la_SOURCES = \
	$(GB_APPLICATION_RESOURCE_C) \
	$(GB_APPLICATION_RESOURCE_H) \
	app/gb-application.c \
	app/gb-application.h \
	app/gb-file-filters.c \
	app/gb-file-filters.h \
	app/gb-util.c \
	app/gb-util.h


noinst_LTLIBRARIES += libcompiler.la
libcompiler_la_SOURCES = \
	compiler/gb-compiler.c \
	compiler/gb-compiler.h \
	compiler/gb-compiler-clang.c \
	compiler/gb-compiler-clang.h \
	compiler/gb-compiler-js.c \
	compiler/gb-compiler-js.h \
	compiler/gb-compiler-result.c \
	compiler/gb-compiler-result.h


noinst_LTLIBRARIES += libdbus.la
libdbus_la_SOURCES = \
	dbus/gb-dbus-typelib.c \
	dbus/gb-dbus-typelib.h


noinst_LTLIBRARIES += libdebugger.la
libdebugger_la_SOURCES = \
	debugger/gb-debugger.c \
	debugger/gb-debugger.h \
	debugger/gb-debugger-gdb.c \
	debugger/gb-debugger-gdb.h


noinst_LTLIBRARIES += libdocs.la
libdocs_la_SOURCES = \
	docs/gb-docs-pane.c \
	docs/gb-docs-pane.h \
	docs/gb-docs-provider.c \
	docs/gb-docs-provider.h \
	docs/gb-docs-provider-devhelp.c \
	docs/gb-docs-provider-devhelp.h \
	docs/gb-docs-provider-gir.c \
	docs/gb-docs-provider-gir.h


noinst_LTLIBRARIES += liblanguage.la
liblanguage_LIBADD = \
	libtrie.la
liblanguage_la_SOURCES = \
	language/gb-language.c \
	language/gb-language.h \
	language/gb-language-c.c \
	language/gb-language-c.h \
	language/gb-language-c-completion-provider.c \
	language/gb-language-c-completion-provider.h \
	language/gb-language-formatter.c \
	language/gb-language-formatter.h \
	language/gb-language-formatter-c.c \
	language/gb-language-formatter-c.h \
	language/gb-language-formatter-js.c \
	language/gb-language-formatter-js.h \
	language/gb-language-js.c \
	language/gb-language-js.h


noinst_LTLIBRARIES += liblog.la
liblog_la_SOURCES = \
	log/gb-log.c \
	log/gb-log.h


noinst_LTLIBRARIES += libmultiprocess.la
libmultiprocess_la_SOURCES = \
	multiprocess/gb-multiprocess-manager.c \
	multiprocess/gb-multiprocess-manager.h


noinst_LTLIBRARIES += libprofiler.la
libprofiler_la_SOURCES = \
	profiler/gb-profiler.c \
	profiler/gb-profiler.h \
	profiler/gb-profiler-ltrace.c \
	profiler/gb-profiler-ltrace.h \
	profiler/gb-profiler-strace.c \
	profiler/gb-profiler-strace.h \
	profiler/gb-profiler-valgrind.c \
	profiler/gb-profiler-valgrind.h


noinst_LTLIBRARIES += libproject.la
libproject_la_SOURCES = \
	project/gb-project.c \
	project/gb-project.h \
	project/gb-project-file.c \
	project/gb-project-file.h \
	project/gb-project-item.c \
	project/gb-project-item.h


noinst_LTLIBRARIES += librunner.la
librunner_la_SOURCES = \
	runner/gb-runner.c \
	runner/gb-runner.h \
	runner/gb-runner-program.c \
	runner/gb-runner-program.h


noinst_LTLIBRARIES += libsearch.la
libsearch_la_SOURCES = \
	search/gb-search-completion.c \
	search/gb-search-completion.h \
	search/gb-search-provider.c \
	search/gb-search-provider.h \
	search/highlight.c \
	search/highlight.h \
	search/fuzzy.c \
	search/fuzzy.h


noinst_LTLIBRARIES += libsource.la
libsource_la_LIBADD = \
	libapp.la \
	libcompiler.la \
	libdbus.la \
	liblanguage.la
libsource_la_SOURCES = \
	source/gb-source-diff.c \
	source/gb-source-diff.h \
	source/gb-source-fullscreen-container.c \
	source/gb-source-fullscreen-container.h \
	source/gb-source-gutter-renderer-compiler.c \
	source/gb-source-gutter-renderer-compiler.h \
	source/gb-source-gutter-renderer-diff.c \
	source/gb-source-gutter-renderer-diff.h \
	source/gb-source-overlay.c \
	source/gb-source-overlay.h \
	source/gb-source-pane.c \
	source/gb-source-pane.h \
	source/gb-source-snippet.c \
	source/gb-source-snippet.h \
	source/gb-source-snippet-private.h \
	source/gb-source-snippet-chunk.c \
	source/gb-source-snippet-chunk.h \
	source/gb-source-snippet-completion-item.c \
	source/gb-source-snippet-completion-item.h \
	source/gb-source-snippet-completion-provider.c \
	source/gb-source-snippet-completion-provider.h \
	source/gb-source-snippet-context.c \
	source/gb-source-snippet-context.h \
	source/gb-source-snippet-parser.c \
	source/gb-source-snippet-parser.h \
	source/gb-source-snippets.c \
	source/gb-source-snippets.h \
	source/gb-source-snippets-manager.c \
	source/gb-source-snippets-manager.h \
	source/gb-source-typelib-completion-item.c \
	source/gb-source-typelib-completion-item.h \
	source/gb-source-typelib-completion-provider.c \
	source/gb-source-typelib-completion-provider.h \
	source/gb-source-view.c \
	source/gb-source-view.h


noinst_LTLIBRARIES += libsymbol.la
libsymbol_la_SOURCES = \
	symbol/gb-symbol-combo-box.c \
	symbol/gb-symbol-combo-box.h \
	symbol/gb-symbol.c \
	symbol/gb-symbol.h \
	symbol/gb-symbol-pane.c \
	symbol/gb-symbol-pane.h \
	symbol/gb-symbol-provider.c \
	symbol/gb-symbol-provider.h \
	symbol/gb-symbol-provider-c.c \
	symbol/gb-symbol-provider-c.h \
	symbol/gb-symbol-provider-js.c \
	symbol/gb-symbol-provider-js.h


noinst_LTLIBRARIES += libterminal.la
libterminal_la_SOURCES = \
	terminal/gb-terminal-pane.c \
	terminal/gb-terminal-pane.h


noinst_LTLIBRARIES += libtrie.la
libtrie_la_SOURCES = \
	trie/trie.c \
	trie/trie.h


noinst_LTLIBRARIES += libworkspace.la
libworkspace_la_LIBADD = \
	libanimation.la \
	libproject.la \
	libsearch.la \
	libterminal.la
libworkspace_la_SOURCES = \
	workspace/gb-workspace.c \
	workspace/gb-workspace.h \
	workspace/gb-workspace-layout-edit.c \
	workspace/gb-workspace-layout-edit.h \
	workspace/gb-workspace-layout.c \
	workspace/gb-workspace-layout.h \
	workspace/gb-workspace-layout-splash.c \
	workspace/gb-workspace-layout-splash.h \
	workspace/gb-workspace-layout-switcher.c \
	workspace/gb-workspace-layout-switcher.h \
	workspace/gb-workspace-pane-group.c \
	workspace/gb-workspace-pane-group.h \
	workspace/gb-workspace-pane.c \
	workspace/gb-workspace-pane.h \
	workspace/gb-workspace-search-provider.c \
	workspace/gb-workspace-search-provider.h \
	workspace/gb-zoomable.c \
	workspace/gb-zoomable.h \
	nautilus/nautilus-floating-bar.c \
	nautilus/nautilus-floating-bar.h


bin_PROGRAMS += gnome-builder
gnome_builder_SOURCES = gnome-builder.c
gnome_builder_LDADD = \
	libapp.la \
	libcompiler.la \
	libdebugger.la \
	libdocs.la \
	liblanguage.la \
	liblog.la \
	libmultiprocess.la \
	libprofiler.la \
	libproject.la \
	librunner.la \
	libsearch.la \
	libsource.la \
	libsymbol.la \
	libterminal.la \
	libtrie.la \
	libworkspace.la


bin_PROGRAMS += gnome-builder-worker
gnome_builder_worker_LDADD = \
	libdbus.la \
	libsearch.la \
	libtrie.la
gnome_builder_worker_SOURCES = \
	gnome-builder-worker.c \
	worker/gb-worker-typelib.c


GLIB_COMPILE_RESOURCE = glib-compile-resources


RESOURCE_FILES = \
	data/gb-application.gresource.xml \
	data/ui/gb-application-menu.ui \
	data/style-schemes/solarized-dark.xml \
	data/style-schemes/solarized-light.xml \
	data/icons/class-16x.png \
	data/icons/deprecated-16x.png \
	data/icons/enum-16x.png \
	data/icons/enum-value-16x.png \
	data/icons/keyword-16x.png \
	data/icons/method-16x.png \
	data/icons/static-16x.png \
	data/icons/template-16x.png


GB_APPLICATION_RESOURCE_C = $(builddir)/app/gb-application-resource.c
GB_APPLICATION_RESOURCE_H = $(builddir)/app/gb-application-resource.h


$(GB_APPLICATION_RESOURCE_C) $(GB_APPLICATION_RESOURCE_H): $(RESOURCE_FILES)
	$(AM_V_GEN)$(GLIB_COMPILE_RESOURCE) \
		--target=$(GB_APPLICATION_RESOURCE_C).tmp \
		--generate-source --c-name=gb_application \
		data/gb-application.gresource.xml
	@$(GLIB_COMPILE_RESOURCE) \
		--target=$(GB_APPLICATION_RESOURCE_H).tmp \
		--generate-header --c-name=gb_application \
		data/gb-application.gresource.xml
	@mv $(GB_APPLICATION_RESOURCE_C).tmp $(GB_APPLICATION_RESOURCE_C)
	@mv $(GB_APPLICATION_RESOURCE_H).tmp $(GB_APPLICATION_RESOURCE_H)


GB_DBUS_TYPELIB_C = $(builddir)/dbus/gb-dbus-typelib.c
GB_DBUS_TYPELIB_H = $(builddir)/dbus/gb-dbus-typelib.h


$(GB_DBUS_TYPELIB_C) $(GB_DBUS_TYPELIB_H): $(srcdir)/dbus/typelib-worker.xml
	$(AM_V_GEN)gdbus-codegen \
		--interface-prefix org.gnome.Builder \
		--c-namespace GbDBus \
		--generate-c-code $(builddir)/dbus/gb-dbus-typelib \
		$(srcdir)/dbus/typelib-worker.xml


EXTRA_DIST = $(RESOURCE_FILES)


CLEANFILES = \
	$(GB_APPLICATION_RESOURCE_C) \
	$(GB_APPLICATION_RESOURCE_H)
