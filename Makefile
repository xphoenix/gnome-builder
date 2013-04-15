TARGET := gnome-builder

all: $(TARGET)

OBJECTS :=
OBJECTS += egg/egg-list-box.o
OBJECTS += gd/gd-header-bar.o
OBJECTS += gd/gd-revealer.o
OBJECTS += nautilus/nautilus-floating-bar.o
OBJECTS += src/gb-animation.o
OBJECTS += src/gb-application.o
OBJECTS += src/gb-compiler.o
OBJECTS += src/gb-compiler-clang.o
OBJECTS += src/gb-compiler-js.o
OBJECTS += src/gb-compiler-result.o
OBJECTS += src/gb-debugger.o
OBJECTS += src/gb-debugger-gdb.o
OBJECTS += src/gb-docs-pane.o
OBJECTS += src/gb-docs-provider.o
OBJECTS += src/gb-docs-provider-devhelp.o
OBJECTS += src/gb-docs-provider-gir.o
OBJECTS += src/gb-file-filters.o
OBJECTS += src/gb-frame-source.o
OBJECTS += src/gb-language.o
OBJECTS += src/gb-language-c.o
OBJECTS += src/gb-language-js.o
OBJECTS += src/gb-language-formatter.o
OBJECTS += src/gb-language-formatter-c.o
OBJECTS += src/gb-language-formatter-js.o
OBJECTS += src/gb-profiler.o
OBJECTS += src/gb-profiler-ltrace.o
OBJECTS += src/gb-profiler-strace.o
OBJECTS += src/gb-profiler-valgrind.o
OBJECTS += src/gb-project.o
OBJECTS += src/gb-project-item.o
OBJECTS += src/gb-project-file.o
OBJECTS += src/gb-runner.o
OBJECTS += src/gb-runner-program.o
OBJECTS += src/gb-search-provider.o
OBJECTS += src/gb-source-diff.o
OBJECTS += src/gb-source-gutter-renderer-compiler.o
OBJECTS += src/gb-source-gutter-renderer-diff.o
OBJECTS += src/gb-source-overlay.o
OBJECTS += src/gb-source-pane.o
OBJECTS += src/gb-source-snippet.o
OBJECTS += src/gb-source-snippet-chunk.o
OBJECTS += src/gb-source-snippets.o
OBJECTS += src/gb-source-state.o
OBJECTS += src/gb-source-state-insert.o
OBJECTS += src/gb-source-state-snippet.o
OBJECTS += src/gb-source-view.o
OBJECTS += src/gb-symbol.o
OBJECTS += src/gb-symbol-combo-box.o
OBJECTS += src/gb-symbol-pane.o
OBJECTS += src/gb-symbol-provider.o
OBJECTS += src/gb-symbol-provider-c.o
OBJECTS += src/gb-symbol-provider-js.o
OBJECTS += src/gb-terminal-pane.o
OBJECTS += src/gb-workspace.o
OBJECTS += src/gb-workspace-layout.o
OBJECTS += src/gb-workspace-layout-edit.o
OBJECTS += src/gb-workspace-layout-splash.o
OBJECTS += src/gb-workspace-layout-switcher.o
OBJECTS += src/gb-workspace-pane.o
OBJECTS += src/gb-workspace-pane-group.o
OBJECTS += src/main.o

PKGS :=
PKGS += gtk+-3.0
PKGS += gtksourceview-3.0
PKGS += vte-2.90

DEBUG :=
DEBUG += -g

WARNINGS :=
WARNINGS += -Wall
WARNINGS += -Werror

CFLAGS :=
CFLAGS += $(shell pkg-config --cflags $(PKGS))
CFLAGS += -I.

LIBS :=
LIBS += $(shell pkg-config --libs $(PKGS))
LIBS += -L/usr/lib64/llvm -lclang
LIBS += -lm

%.o: %.c %.h
	$(CC) -c -o $@ $(DEBUG) $(WARNINGS) $(CFLAGS) $*.c

main.o: main.c
	$(CC) -c -o $@ $(DEBUG) $(WARNINGS) $(CFLAGS) $<

$(TARGET): $(OBJECTS)
	$(CC) -o $@ $(DEBUG) $(OBJECTS) $(LIBS)

clean:
	rm -f $(OBJECTS) $(TARGET)
