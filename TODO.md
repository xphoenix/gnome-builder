# TODO

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
