include $(top_srcdir)/Makefile.tests


noinst_PROGRAMS = \
	test-back-forward-list \
	test-browser \
	test-editor \
	test-greeter \
	test-search-fuzzy \
	test-snippet-context \
	test-snippet-parser \
	test-symbol-browser


TEST_PROGS += \
	test-search-fuzzy \
	test-snippet-context \
	test-snippet-parser


AM_CPPFLAGS = \
	$(DEVHELP_CFLAGS) \
	$(GIR_CFLAGS) \
	$(GJS_CFLAGS) \
	$(GOBJECT_CFLAGS) \
	$(GTK3_CFLAGS) \
	$(GTKSOURCEVIEW_CFLAGS) \
	$(JSON_CFLAGS) \
	$(VTE_CFLAGS) \
	-DGETTEXT_PACKAGE=\""gnome-builder"\" \
	-DG_LOG_DOMAIN=\"Builder\" \
	-I$(top_srcdir)/src \
	-I$(top_builddir)/src \
	-I$(top_srcdir)/src/animation \
	-I$(top_srcdir)/src/app \
	-I$(top_srcdir)/src/browser \
	-I$(top_srcdir)/src/compiler \
	-I$(top_srcdir)/src/debugger \
	-I$(top_srcdir)/src/docs \
	-I$(top_srcdir)/src/language \
	-I$(top_srcdir)/src/nautilus \
	-I$(top_srcdir)/src/profiler \
	-I$(top_srcdir)/src/project \
	-I$(top_srcdir)/src/runner \
	-I$(top_srcdir)/src/search \
	-I$(top_srcdir)/src/source \
	-I$(top_srcdir)/src/symbol \
	-I$(top_srcdir)/src/terminal \
	-I$(top_srcdir)/src/trie \
	-I$(top_srcdir)/src/workspace


test_back_forward_list_SOURCES = test-back-forward-list.c
test_back_forward_list_LDADD = \
	$(top_builddir)/src/libapp.la


test_browser_SOURCES = test-browser.c
test_browser_LDADD = \
	$(top_builddir)/src/libapp.la \
	$(top_builddir)/src/libbrowser.la \
	$(top_builddir)/src/libproject.la \
	$(top_builddir)/src/libsource.la \
	$(top_builddir)/src/libtrie.la \
	$(top_builddir)/src/libworkspace.la


test_symbol_browser_SOURCES = test-symbol-browser.c
test_symbol_browser_LDADD = \
	$(top_builddir)/src/libapp.la \
	$(top_builddir)/src/libbrowser.la \
	$(top_builddir)/src/libproject.la \
	$(top_builddir)/src/libsource.la \
	$(top_builddir)/src/libsymbol.la \
	$(top_builddir)/src/libtrie.la \
	$(top_builddir)/src/libworkspace.la


test_editor_SOURCES = test-editor.c
test_editor_LDADD = \
	$(top_builddir)/src/libsource.la \
	$(top_builddir)/src/libworkspace.la \
	$(top_builddir)/src/libtrie.la \
	$(top_builddir)/src/libapp.la


test_greeter_SOURCES = test-greeter.c
test_greeter_LDADD = \
	$(top_builddir)/src/libapp.la \
	$(top_builddir)/src/libworkspace.la


test_snippet_context_SOURCES = test-snippet-context.c
test_snippet_context_LDADD = $(top_builddir)/src/libsource.la


test_snippet_parser_SOURCES = test-snippet-parser.c
test_snippet_parser_LDADD = $(top_builddir)/src/libsource.la


test_search_fuzzy_SOURCES = test-search-fuzzy.c
test_search_fuzzy_LDADD = $(top_builddir)/src/libsearch.la
