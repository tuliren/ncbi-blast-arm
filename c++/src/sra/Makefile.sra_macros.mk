# $Id: Makefile.sra_macros.mk 504904 2016-06-20 18:10:47Z ucko $
# Historical names for VDB macros.

SRA_INCLUDE     = $(VDB_INCLUDE)
SRA_SDK_SYSLIBS = $(VDB_LIBS)
SRAXF_LIBS      = $(SRA_SDK_LIBS)
SRA_LIBS        = $(SRA_SDK_LIBS)
BAM_LIBS        = $(SRA_SDK_LIBS)
SRAREAD_LDEP    = $(SRA_SDK_LIBS)
SRAREAD_LIBS    = $(sraread) $(SRA_SDK_LIBS)
