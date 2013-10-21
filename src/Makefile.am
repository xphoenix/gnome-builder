-include $(top_srcdir)/Makefile.introspection


bin_PROGRAMS =
noinst_LTLIBRARIES =


AM_CPPFLAGS = \
	$(DEVHELP_CFLAGS) \
	$(GIR_CFLAGS) \
	$(GJS_CFLAGS) \
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
	-I$(srcdir)/command \
	-I$(srcdir)/core \
	-I$(srcdir)/dialogs \
	-I$(srcdir)/editor \
	-I$(srcdir)/git \
	-I$(srcdir)/language \
	-I$(srcdir)/language/c \
	-I$(srcdir)/log \
	-I$(srcdir)/nautilus \
	-I$(srcdir)/project \
	-I$(srcdir)/resources \
	-I$(srcdir)/snippets \
	-I$(srcdir)/util \
	-I$(srcdir)/workspace


AM_LDFLAGS = \
	-lclang \
	-lm \
	-L/usr/lib64/llvm \
	$(DEVHELP_LIBS) \
	$(GIR_LIBS) \
	$(GJS_LIBS) \
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
	animation/gb-animation.c \
	animation/gb-animation.h \
	animation/gb-frame-source.c \
	animation/gb-frame-source.h \
	command/gb-command.c \
	command/gb-command-dbus.c \
	command/gb-command-dbus.h \
	command/gb-command.h \
	command/gb-command-stack.c \
	command/gb-command-stack.h \
	core/gb-application.c \
	core/gb-application.h \
	core/gb-service.c \
	core/gb-service.h \
	core/gb-subprocess-manager.c \
	core/gb-subprocess-manager.h \
	dialogs/gb-create-project-dialog.c \
	dialogs/gb-create-project-dialog.h \
	dialogs/gb-prefs-dialog.c \
	dialogs/gb-prefs-dialog.h \
	editor/gb-source-document.c \
	editor/gb-source-document.h \
	editor/gb-source-view.c \
	editor/gb-source-view.h \
	git/gb-git-service.c \
	git/gb-git-service.h \
	log/gb-log.c \
	log/gb-log.h \
	nautilus/nautilus-floating-bar.c \
	nautilus/nautilus-floating-bar.h \
	project/gb-project.c \
	project/gb-project.h \
	project/gb-project-info.c \
	project/gb-project-info.h \
	project/gb-project-service.c \
	project/gb-project-service.h \
	snippets/gb-source-snippet.c \
	snippets/gb-source-snippet-chunk.c \
	snippets/gb-source-snippet-chunk.h \
	snippets/gb-source-snippet-context.c \
	snippets/gb-source-snippet-context.h \
	snippets/gb-source-snippet.h \
	snippets/gb-source-snippet-parser.c \
	snippets/gb-source-snippet-parser.h \
	snippets/gb-source-snippet-private.h \
	snippets/gb-source-snippets.c \
	snippets/gb-source-snippets.h \
	snippets/gb-source-snippets-manager.c \
	snippets/gb-source-snippets-manager.h \
	util/fuzzy.c \
	util/fuzzy.h \
	util/gb-buffer-tasks.c \
	util/gb-buffer-tasks.h \
	util/gb-gtk.c \
	util/gb-gtk.h \
	util/gb-tab-label.c \
	util/gb-tab-label.h \
	util/trie.c \
	util/trie.h \
	workspace/gb-back-forward-list.c \
	workspace/gb-back-forward-list.h \
	workspace/gb-document.c \
	workspace/gb-document.h \
	workspace/gb-workspace-actions.c \
	workspace/gb-workspace-actions.h \
	workspace/gb-workspace.c \
	workspace/gb-workspace-container.c \
	workspace/gb-workspace-container.h \
	workspace/gb-workspace-container-private.h \
	workspace/gb-workspace-docs.c \
	workspace/gb-workspace-docs.h \
	workspace/gb-workspace-editor.c \
	workspace/gb-workspace-editor.h \
	workspace/gb-workspace-greeter.c \
	workspace/gb-workspace-greeter.h \
	workspace/gb-workspace.h \
	workspace/gb-workspace-section.c \
	workspace/gb-workspace-section.h


RESOURCE_FILES = \
	resources/gnome-builder.gresource.xml \
	resources/ui/gb-application-menu.ui \
	resources/ui/gb-prefs-dialog.ui \
	resources/ui/gb-workspace-container.ui


built_sources = \
	$(builddir)/resources/gb-resources.c \
	$(builddir)/resources/gb-resources.h \
	$(builddir)/git/gb-git-service-dbus.c \
	$(builddir)/git/gb-git-service-dbus.h


GLIB_COMPILE_RESOURCES = glib-compile-resources
GDBUS_CODEGEN = gdbus-codegen


resources/gb-resources.%: $(RESOURCE_FILES)
	$(AM_V_GEN)$(GLIB_COMPILE_RESOURCES) \
		--sourcedir=resources \
		--target=resources/gb-resources.c \
		--generate-source \
		--c-name gb \
		resources/gnome-builder.gresource.xml
	$(AM_V_GEN)$(GLIB_COMPILE_RESOURCES) \
		--sourcedir=resources \
		--target=resources/gb-resources.h \
		--generate-header \
		--c-name gb \
		resources/gnome-builder.gresource.xml


git/gb-git-service-dbus.%: git/org.gnome.Builder.Services.Git.xml
	$(AM_V_GEN)$(GDBUS_CODEGEN) \
		--xml-files git/org.gnome.Builder.Services.Git.xml \
		--interface-prefix org.gnome.Builder \
		--c-namespace GbGitServiceDBus \
		--c-generate-object-manager \
		--generate-c-code=git/gb-git-service-dbus


gsettings_SCHEMAS = \
	editor/org.gnome.Builder.Editor.DocumentSettings.gschema.xml
.PRECIOUS: $(gsettings_SCHEMAS)


@GSETTINGS_RULES@


CLEANFILES = $(built_sources)
