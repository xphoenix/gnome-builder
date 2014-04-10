-include $(top_srcdir)/Makefile.introspection


bin_PROGRAMS =
noinst_LTLIBRARIES =


AM_CPPFLAGS = \
	$(DEVHELP_CFLAGS) \
	$(GIR_CFLAGS) \
	$(GJS_CFLAGS) \
	$(GLADE_CFLAGS) \
	$(GMODULE_CFLAGS) \
	$(GOBJECT_CFLAGS) \
	$(GTK3_CFLAGS) \
	$(GTKSOURCEVIEW_CFLAGS) \
	$(JSON_CFLAGS) \
	$(VTE_CFLAGS) \
	-DG_LOG_DOMAIN=\"Builder\" \
	-I$(srcdir) \
	-I$(builddir) \
	-I$(srcdir)/animation \
	-I$(srcdir)/core \
	-I$(srcdir)/debugger \
	-I$(srcdir)/designer \
	-I$(srcdir)/dialogs \
	-I$(srcdir)/docs \
	-I$(srcdir)/editor \
	-I$(srcdir)/gedit \
	-I$(srcdir)/git \
	-I$(srcdir)/nautilus \
	-I$(srcdir)/project \
	-I$(srcdir)/resources \
	-I$(srcdir)/trie \
	-I$(srcdir)/util \
	-I$(srcdir)/workspace


AM_LDFLAGS = \
	-lclang \
	-lm \
	-L/usr/lib64/llvm \
	$(DEVHELP_LIBS) \
	$(GIR_LIBS) \
	$(GJS_LIBS) \
	$(GLADE_LIBS) \
	$(GMODULE_LIBS) \
	$(GOBJECT_LIBS) \
	$(GTK3_LIBS) \
	$(GTKSOURCEVIEW_LIBS) \
	$(JSON_LIBS) \
	$(VTE_LIBS)


bin_PROGRAMS += gnome-builder
gnome_builder_SOURCES = \
	$(built_sources) \
	gnome-builder.c \
	core/gb-dbus-daemon.c \
	core/gb-log.c \
	core/gb-dbus-daemon.h \
	core/gb-application.c \
	core/gb-service.c \
	core/gb-log.h \
	core/gb-service.h \
	core/gb-application.h \
	docs/gb-docs-tab.c \
	docs/gb-docs-section.h \
	docs/gb-docs-tab.h \
	docs/gb-docs-section.c \
	animation/gb-frame-source.h \
	animation/gb-animation.h \
	animation/gb-animation.c \
	animation/gb-frame-source.c \
	workspace/gb-tab-label.h \
	workspace/gb-workspace-greeter.c \
	workspace/gb-notebook.h \
	workspace/gb-workspace-container.h \
	workspace/gb-workspace-container.c \
	workspace/gb-tab.c \
	workspace/gb-notebook-group.c \
	workspace/gb-tab-label.c \
	workspace/gb-workspace.h \
	workspace/gb-tab.h \
	workspace/gb-workspace-section.c \
	workspace/gb-notebook-group.h \
	workspace/gb-workspace-greeter.h \
	workspace/gb-notebook.c \
	workspace/gb-workspace-section.h \
	workspace/gb-workspace.c \
	project/gb-project-item.h \
	project/gb-project-info.h \
	project/gb-project-service.h \
	project/gb-project-service.c \
	project/gb-project-info.c \
	project/gb-project.c \
	project/gb-project.h \
	dialogs/gb-create-project-dialog.c \
	dialogs/gb-create-project-dialog.h \
	git/gb-git-section.h \
	git/gb-git-section.c \
	trie/trie.c \
	trie/trie.h \
	designer/gb-designer-section.c \
	designer/gb-designer-section.h \
	nautilus/nautilus-floating-bar.c \
	nautilus/nautilus-floating-bar.h \
	editor/gb-source-snippet-completion-provider.h \
	editor/gb-source-snippet.c \
	editor/gb-source-snippet-completion-item.h \
	editor/gb-editor-tab.h \
	editor/gb-editor-section.c \
	editor/gb-source-snippet-chunk.c \
	editor/gb-source-snippet-private.h \
	editor/gb-source-overlay.c \
	editor/gb-editor-document.h \
	editor/gb-source-snippets-manager.c \
	editor/gb-source-snippet-parser.h \
	editor/gb-source-snippet-chunk.h \
	editor/gb-editor-document.c \
	editor/gb-source-snippet.h \
	editor/gb-source-snippets-manager.h \
	editor/gb-source-snippets.h \
	editor/gb-source-snippet-context.c \
	editor/gb-source-snippet-parser.c \
	editor/gb-source-view.c \
	editor/gb-source-view.h \
	editor/gb-source-snippet-completion-item.c \
	editor/gb-source-snippet-context.h \
	editor/gb-editor-tab.c \
	editor/gb-source-snippets.c \
	editor/gb-editor-section.h \
	editor/gb-source-snippet-completion-provider.c \
	editor/gb-source-overlay.h \
	util/gb-gtk.c \
	util/gb-buffer-tasks.c \
	util/gb-buffer-tasks.h \
	util/gb-gtk.h \
	gedit/gedit-close-button.h \
	gedit/gedit-close-button.c \
	debugger/gb-debugger-section.h \
	debugger/gb-debugger-section.c


# TODO: icons and stuff from gresource
RESOURCE_FILES = \
	gnome-builder.gresource.xml


built_sources = \
	resources/gb-resources.c \
	resources/gb-resources.h


GLIB_COMPILE_RESOURCES = glib-compile-resources


resources/gb-resources.%: $(RESOURCE_FILES)
	$(AM_V_GEN)$(GLIB_COMPILE_RESOURCES) \
		--target=resources/gb-resources.c \
		--generate-source \
		--c-name gb \
		gnome-builder.gresource.xml
	$(AM_V_GEN)$(GLIB_COMPILE_RESOURCES) \
		--target=resources/gb-resources.h \
		--generate-header \
		--c-name gb \
		gnome-builder.gresource.xml


gsettings_SCHEMAS = \
	editor/org.gnome.builder.preferences.editor.gschema.xml \
	editor/org.gnome.builder.preferences.editor.language.gschema.xml

@GSETTINGS_RULES@


CLEANFILES = $(built_sources)

run:
	GSETTINGS_SCHEMA_DIR=editor ./gnome-builder
