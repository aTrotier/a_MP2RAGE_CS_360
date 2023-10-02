# -------------------------------------------------------------
# 
# Copyright (c) 2020
# Bruker BioSpin MRI GmbH
# D-76275 Ettlingen, Germany
#
# All Rights Reserved
#
# You can freely change this Makefile.
#
# -------------------------------------------------------------

EXECDISKUNIT = /opt/PV-360.3.4

OVERLAY = a_MP2RAGE_CS_360

OBJS = \
	initMeth.o \
	loadMeth.o \
	parsRelations.o \
	BaseLevelRelations.o \
	RecoRelations.o \
	backbone.o \
	adjust.o \
	deriveVisu.o 

PPGS = \
	a_MP2RAGE_CS_360.ppg \
	a_MP2RAGE_CS_360Angio.ppg 

include $(EXECDISKUNIT)/prog/include/make/MethodMRI.makefile

