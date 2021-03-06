AC_INIT([osux], [0.1], [lmaugere@ipb.fr, tmijieux@ipb.fr])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AM_SILENT_RULES([yes])

### TOOLS ##

AC_PROG_CC
AM_PROG_AR
PKG_PROG_PKG_CONFIG
AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION([0.19.8])
LT_INIT([win32-dll dlopen disable-static])

### LIBRARIES ##

PKG_CHECK_MODULES([YAML], [yaml-0.1])
PKG_CHECK_MODULES([LZMA], [liblzma])
PKG_CHECK_MODULES([PYTHON2], [python2])
PKG_CHECK_MODULES([CRYPTO], [libcrypto])
PKG_CHECK_MODULES([SQLITE], [sqlite3])
PKG_CHECK_MODULES([GLIB], [glib-2.0])
PKG_CHECK_MODULES([GMODULE], [gmodule-2.0])
PKG_CHECK_MODULES([GIO], [gio-2.0])
PKG_CHECK_MODULES([GOBJECT], [gobject-2.0])
PKG_CHECK_MODULES([GTK3], [gtk+-3.0])
PKG_CHECK_MODULES([GST], [gstreamer-1.0])
PKG_CHECK_MODULES([EPOXY], [epoxy])
PKG_CHECK_MODULES([GTS], gts >= 0.7.2, ,
                       [AC_CHECK_PROG(GTS_CONFIG, gts-config,
                       gts-config, false)
		       if test "x$GTS_CONFIG" = "xfalse" ; then
                          AC_MSG_ERROR([gts-config not in path, aborting])
                       fi
                       GTS_CFLAGS=$(${GTS_CONFIG} --cflags)
                       GTS_LIBS=$(${GTS_CONFIG} --libs)])
PKG_CHECK_MODULES(
        [MYSQL], libmysqlclient, ,
        [AC_CHECK_PROG(MYSQL_CONFIG, mysql_config, mysql_config, false)
            if test "x$MYSQL_CONFIG" = "xfalse" ; then
               MYSQL_CFLAGS="-IC:/msys64/mingw64/include/mysql/ -march=x86-64 -mtune=generic -O2 -pipe -Wunused -Wno-uninitialized"
               MYSQL_LIBS="-LC:/msys64/mingw64/lib/ -lmysqlclient"
            else
                MYSQL_CFLAGS=$(${MYSQL_CONFIG} --cflags)
                MYSQL_LIBS=$(${MYSQL_CONFIG} --libs)
            fi
            ])

### VARIABLES ##

AC_SUBST(GTS_CFLAGS)
AC_SUBST(GTS_LIBS)
AC_SUBST(MYSQL_CFLAGS)
AC_SUBST(MYSQL_LIBS)


### OUTPUT ##
AC_CONFIG_FILES([
                  po/Makefile.in
		  Makefile
		  src/Makefile
		  src/thirdparty/Makefile
		  src/include/Makefile
      		  src/lib/Makefile
		  src/lib/util/Makefile
		  src/lib/beatmap/Makefile
		  src/lib/mod/Makefile
               	  src/lib/database/Makefile
		  src/lib/replay/Makefile
                  src/tools/Makefile
                  src/tools/dbctl/Makefile
                  src/tools/taiko_generator/Makefile
		  src/taikorank/Makefile
		  src/taikorank/yaml/Makefile
		  src/editor/Makefile
		  src/viewer/Makefile
               ])
AC_OUTPUT
