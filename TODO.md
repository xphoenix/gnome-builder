# TODO

## Application

 * When calling `gnome-builder' again from the command line, we need to
   make sure that we only bring the current window forward, rather than
   spawning a new instance of the window.
   Alternatively, we can make sure we support multiple projects open within
   one process space.
 * How should we handle opening a file at startup before a project is
   selected? Should we default to an unnamed, new project?

## Multi-process

 * For the resiliency of the application and to make it easier to test
   for leaks, much work should be moved to sub-processes.

   It is important that all of the various subprocesses are easy to write
   and maintain and share the mutliprocess infrastructure.

   A set of subprocesses might include:

   - autocomplection for GIR

     These need to load in various libraries potentially (if using typelib
     instead of .gir files. It would be nice to keep them safe inside their
     own daemon. Especially if they crash.

   - autocompletion with libclang

     Clang can crash and bring things down with it. Better to keep it
     out of process.

   - Type-highlighting can be pushed off to something out of process.

   - Search is difficult and pushing that off into another process can
     simplify the testing of it and each of the providers.


## Multi-process Design

This design also helps by allowing us to build everything as self-contained
services that may be reused.

 * Process 0: User Interface
   - Autocompletion happens in process, with Trie of data that has
     been parsed out of process. This is meant to keep it fast. It would
     be nice if we could moved to mmap()'d version.
   - The code that highlights sourceview lives in process, but the
     part that tokenizes code is out of process. It returns type information
     and positions.
 * Process 1: Search Process
   - Recycled occasionally. These types of programs really fragment memory.
 * Process 2: Build Process (Automake)
   - Will need concept of "tasks" with progress.
 * Process 3: Execution Process
 * Process 4: Debugger Process
 * Process 5: C Tokenizer, Clang Services
   - This process can try to tune how long it stays around. Useful to keep
     things hot but recycle occasionally.

## Snippets

 * Each new line should insert with the same spacing as the previous line.
 * Tabs should be written using the spaces as set in the editor.
 * Add a bunch of default snippets for C.
 * Install default snippets file to user directory on startup if they
   do not exist.

## Search

 * Global search providers.
 * They will be used by the uber-search. (ctrl+period).
 * Uber-search will expand from search icon in top bar.
 * Searches source code, widgets, commits, docs, functions, help, etc.
 * A new-toplevel window will be displayed, will attach to search button.
   Will need proper connection sides set.
 * Activation will navigate to item within Builder.
 * Port relevance.py for fuzzy matching of strings and highlight text.
 * CIndex can locate types that match for us quickly.

## Projects

 * Build automake for GbProject.
 * Dialog to re(configure) a project.
 * Creating a new project should initialize a git repository.
 * Configure GNOME Software information.

## Workspace

 * Try disabling chrome to do fullscreen rather than a second window.
 * Make toplevel notebook a GtkStack and fade between elements.
 * Make the top-level buttons a StackSwitcher.
 * Should build be a toggle button that is down while it is active?
 * Implement Save As for source pane.
   The filename should change to follow that file.
 * Restore the last files opened for a project.
   Probably can store this in .user.builder within the project.
   The .gitignore will ignore it.

## Source

 * Saving should be done asynchronously with progress monitoring.
 * Always try to keep a few lines above the bottom of the screen.
   For example, if i want to have at least 3 lines below, push the
   editor up so that it fits.
 * Control+J should place cursor right after the current scope or
   token. Inside of comments, it could go directly after */
   Inside of a block, it could go to after }.
 * Build a Trie from all Gir files.
 * Lookup documentation based on symbol.
 * Allow changing highlight mode from context menu.
 * Allow changing tabs-v-spaces from context menu.
 * Allow changing number of spaces in a tab from context menu.
 * Use symbol provider to "rename local".
 * Highlight types based on compiler result.
 * Jump-to-definition with Control+Click on type.
 * Relayout text (gq equivalent).
 * Reformat text using clang-format.
   We need to first add a GNU Format Style to clang-format.

## Debugger

 * Everything.
 * Wrap GDB protocal into a series of commands.
 * Use streams to communicate, allows for local/remote.
 * Add/Remove breakpoints.
 * Will need hooks for UI to show debugin instead of run icon.
 * Needs to reuse source editor as much as possible.

## Profiler

 * Everything.
 * Determine what basic UI hooks will be.
 * Wrap strace for system call tracing.
 * Wrap ltrace for poor mans profiler.

## Browser

 * There needs to be a way to view all the files within the project.
   This needs design.
 * Renaming a file should be tracked in git.
 * The sidebar browser may be unavoidable. It is just too convenient for
   opening files despite how much space it usually takes. An alternative
   might be a pane.

## Docs

 * Generate docs from gir file.
 * Needs to be per-language.
 * C and JS first.

## Git

 * Everything.
 * Try to use as much from gitg as we can.

## UI

 * Everything.
 * See what all will be used from Glade.
 * Still needs designs.

## Symbols

 * Hook up symbol browsing for a file using clang.
 * Provide a combo box that can be embedded?
 * Provide a search provider to be used in global search.
