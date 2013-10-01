include $(top_srcdir)/Makefile.tests


noinst_PROGRAMS = \
	test-search-fuzzy \
	test-snippet-context


TEST_PROGS += \
	test-search-fuzzy \
	test-snippet-context


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


test_snippet_context_SOURCES = test-snippet-context.c
test_snippet_context_LDADD = $(top_builddir)/src/libsource.la


test_search_fuzzy_SOURCES = test-search-fuzzy.c
test_search_fuzzy_LDADD = $(top_builddir)/src/libsearch.la
