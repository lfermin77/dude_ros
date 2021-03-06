
#-----------------------------------------------------------------------------
# On Linux,    this should be 'Linux'
# On Mac OS X, this should be 'MAC_OS_X'
#-----------------------------------------------------------------------------

PLATFORM := $(shell uname -s)
MACHINE := $(shell uname -m)

ifeq ($(PLATFORM),Linux)
  ifeq ($(MACHINE),x86_64)
    PLATFORM := LINUX64
  else
    PLATFORM := LINUX
  endif
else
  ifeq ($(PLATFORM),Darwin)
    PLATFORM := MAC_OS_X
  else
    PLATFORM := OTHER
  endif
endif


#-----------------------------------------------------------------------------
# Choose a compiler & its options
#--------------------------------------------------------------------------

CXX      = g++
LINK     = g++

OPTS = -ansi -O3 -MMD -W -frounding-math -Wno-deprecated  
#OPTS = -ansi -g -MMD -W -frounding-math -Wno-deprecated  

ifeq ($(PLATFORM),MAC_OS_X)
 OPTS += -DMACOS
endif


#--------------------------------------------------------------------
# Triangulation
#--------------------------------------------------------------------
TRI_DIR = ./FIST
TRI_INCL= -I${TRI_DIR}

ifeq ($(PLATFORM),MAC_OS_X)
  TRI_LIB = -L${TRI_DIR} -lFIST.osx
else 
  ifeq ($(PLATFORM),LINUX64)
    TRI_LIB = -L${TRI_DIR} -lFIST.linux64
  else
    TRI_LIB = -L${TRI_DIR} -lFIST.linux
  endif
endif


#--------------------------------------------------------------------
# MPFR
#--------------------------------------------------------------------

ifeq ($(PLATFORM),MAC_OS_X)
	MPFR_DIR = /opt/local
	MPFR_INCL= -I${MPFR_DIR}/include
	MPFR_LIB = -L${MPFR_DIR}/lib -lmpfr -lgmp
else 
	MPFR_LIB = -lmpfr -lgmp
endif


#--------------------------------------------------------------------
#CGAL
#--------------------------------------------------------------------

ifeq ($(PLATFORM),MAC_OS_X)
	CGAL_DIR = /opt/local
	CGAL_INCL = -I$(CGAL_DIR)/include/CGAL
	CGAL_LIB = -L$(CGAL_DIR)/lib -lCGAL -lpthread
else
	CGAL_DIR = /usr/local
	CGAL_INCL = -I$(CGAL_DIR)/include/CGAL
	CGAL_LIB = -L$(CGAL_DIR)/lib -lCGAL -lpthread
endif


#--------------------------------------------------------------------
# Put all together
#--------------------------------------------------------------------
TARGET  = dude2d_nogl
SUBDIRS_GL = . src mathtool mpfrc++ $(CGAL_INCL) src/skeleton src/draw
INCLUDE_GL = $(addprefix -I,$(SUBDIRS_GL)) $(TRI_INCL) $(MPFR_INCL)
SRCS_GL=$(wildcard $(addsuffix /*.cpp,$(SUBDIRS_GL)))
OBJS_GL=${SRCS_GL:.cpp=.o}
LIB_GL = $(TRI_LIB) $(MK_LIB) $(MPFR_LIB) $(CGAL_LIB)

CFLAGS_GL   = -DOPENGL_RENDER $(OPTS) $(INCLUDE_GL) 
CXXFLAGS_GL = $(CFLAGS_GL) 

.PHONY: all
all :  $(TARGET)


#--------------------------------------------------------------------
$(TARGET): $(OBJS_GL)
	${CXX} ${CXXFLAGS_GL} -o $@ $(OBJS_GL) $(LIB_GL)

clean:
	-rm -f $(OBJS_GL) $(TARGET) Dependencies

#--------------------------------------------------------------------
.SUFFIXES: .cpp

.cpp.o:
	${CXX} ${CXXFLAGS_GL} -c $< -o $@
	cat $*.d >> Dependencies
	rm -f $*.d 

Dependencies:
	touch Dependencies

include Dependencies


