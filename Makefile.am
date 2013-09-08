CLEANFILES =
DISTCLEANFILES =
EXTRA_DIST = $(builderdoc_DATA)

SUBDIRS = src tests

ACLOCAL_AMFLAGS = -I m4 ${ACLOCAL_FLAGS}
DISTCHECK_CONFIGURE_FLAGS = --enable-silent-rules

builderdocdir = ${datadir}/doc/gnome-builder-@VERSION@
builderdoc_DATA = \
	COPYING \
	NEWS \
	README

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = \
	$(top_builddir)/gnome-builder-1.0.pc

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
