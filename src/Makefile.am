bin_PROGRAMS = deinvert
deinvert_CPPFLAGS = -std=c++11 -Wall -Wextra -Wstrict-overflow -Wshadow \
									$(MACPORTS_CF) \
									-Wuninitialized -pedantic $(RFLAGS)
deinvert_LDADD = $(MACPORTS_LD) -lc $(LIQUID) $(SNDFILE)
deinvert_SOURCES = deinvert.cc liquid_wrappers.cc wdsp.cc
