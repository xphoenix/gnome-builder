# Snippets

These are trying to be like snipMate in VIM.

The files belong in ~/.config/gnome-builder/snippets with a prefix like:

 * c.snippets for the C language.
 * python.snippets for the Python language.

# Basics

You specify variables with ```${N}``` where ```N``` is the tabstop number.
If you want to place the variable in other places within the snipet, simply
use ```$N``` to specify it.

You can place the cursor with the special number ```$0```.

# Examples

The following is a snippet to insert an include.

```
snippet inc
	#include <${1:stdio.h}>$0
```

The following is a snippet to create a new GtkWindow.

```
snippet Win
	GtkWidget *${1:window} = gtk_window_new (GTK_WINDOW_TOPLEVEL);$0
```

The following is a snippet that lets you specify a variable but reuses it
throughout the snippet.

```
snippet for
	for (${1:i} = 0; $1 < 10; $1++) {
		$0
	}
```
