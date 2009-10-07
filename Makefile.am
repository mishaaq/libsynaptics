AUTOMAKE_OPTIONS = foreign

lib_LTLIBRARIES = libsynaptics.la
LIBRARY_VERSION = '0:1:0'
libsynaptics_la_SOURCES = pad.cpp pad.h synaptics.h syndebug.h synshm.h
libsynaptics_la_LDFLAGS = -version-info $(LIBRARY_VERSION)
library_includedir = $(includedir)/synaptics
library_include_HEADERS = pad.h Shm_0_14_4 Shm_0_14_5 Shm_0_14_6 Shm_0_15_0 Shm_0_15_1 \
			 Shm_0_15_2 Shm_0_99_1 Shm_0_99_2 Shm_0_99_3 Shm_1_0_0 Shm_1_1_0 \
			 Shm_1_1_1 Shm_1_1_2 Shm_1_1_3 synaptics.h syndebug.h synparam.h synshm.h

noinst_PROGRAMS = padtest
padtest_SOURCES = padtest.cpp synaptics.h
padtest_LDADD = libsynaptics.la
