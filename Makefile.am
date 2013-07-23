EXTRA_DIST =
CLEANFILES =
DISTCLEANFILES =

include src/Makefile.include
include tests/Makefile.include

ACLOCAL_AMFLAGS = -I m4 ${ACLOCAL_FLAGS}
DISTCHECK_CONFIGURE_FLAGS = --enable-silent-rules

builderdocdir = ${datadir}/doc/gnome-builder-@VERSION@
builderdoc_DATA =
builderdoc_DATA += COPYING
builderdoc_DATA += NEWS
builderdoc_DATA += README

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA =
pkgconfig_DATA += $(top_builddir)/gnome-builder-1.0.pc

EXTRA_DIST += $(builderdoc_DATA)

dist-hook:
	@if test -d "$(srcdir)/.git"; then                              \
          (cd "$(srcdir)" &&                                            \
           $(top_srcdir)/missing --run git log --stat ) > ChangeLog.tmp \
           && mv -f ChangeLog.tmp $(top_distdir)/ChangeLog              \
           || (rm -f ChangeLog.tmp;                                     \
               echo Failed to generate ChangeLog >&2);                  \
	else                                                            \
	  echo A git checkout is required to generate a ChangeLog >&2;  \
	fi

uninstall-local:
	-rm -r $(builderdocdir)
