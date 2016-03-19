pkgconfdir=$(sysconfdir)/$(PACKAGE)
yamldir = $(pkgconfdir)/yaml
pythondir = $(pkglibdir)

AM_CFLAGS = -fPIC -Wall -Wextra -I$(top_srcdir) -I$(top_srcdir)/util \
	    -I$(top_srcdir)/include -DPKG_DATA_DIR=\"$(pkgdatadir)\" \
	    -DPKG_CONFIG_DIR=\"$(pkgconfdir)\" -pg -g -ggdb -O0

AM_LDFLAGS=-rdynamic

