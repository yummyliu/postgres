MODULES = metaworker
CPPFLAGS := -I$(libpq_srcdir) $(CPPFLAGS)
LDLIBS := $(libpq_pgport) $(LDLIBS) 

ifdef USE_PGXS
	PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
else
	subdir = src/test/modules/metaworker
	top_builddir = ../../../..
	include $(top_builddir)/src/Makefile.global
	include $(top_srcdir)/contrib/contrib-global.mk
endif
