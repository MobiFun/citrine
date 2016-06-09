#
# Makefile used to build the Layer1 customizable library 
#
#

include ../layer1_env.mak

L1DBG = -g         # L1 debug option or not

TMCUSTCODE	  = ../tm_cust0
AUDIOCUSTCODE = ../audio_cust0

L1_CST_EXT_OBJ = $(OBJ)/ind_os.obj $(OBJ)/l1tm_cust.obj $(OBJ)/l1tm_tpu$(RF).obj

L1_CST_INT_OBJ = $(OBJ)/l1audio_cust.obj $(OBJ)/l1_cust.obj

EXT_LIB = $(L1_CST_EXT_LIB)

INT_LIB = $(L1_CST_INT_LIB)

all: all_dir $(EXT_LIB) $(INT_LIB)

all_dir:
	-mkdir ..$(SLASH)obj
	-mkdir ..$(SLASH)lib

$(EXT_LIB): $(L1_CST_EXT_OBJ)
	-$(REMOVE) $(subst /,$(SLASH),$(EXT_LIB))
	ar470 r $(EXT_LIB) $(L1_CST_EXT_OBJ)

$(INT_LIB): $(L1_CST_INT_OBJ)
	-$(REMOVE) $(subst /,$(SLASH),$(INT_LIB))
	ar470 r $(INT_LIB) $(L1_CST_INT_OBJ)

clean:
	-$(REMOVE) $(subst /,$(SLASH),$(L1_CST_EXT_OBJ))
	-$(REMOVE) $(subst /,$(SLASH),$(L1_CST_INT_OBJ))
	-$(REMOVE) $(subst /,$(SLASH),$(EXT_LIB))
	-$(REMOVE) $(subst /,$(SLASH),$(INT_LIB))

#
# Sources files
#

$(OBJ)/ind_os.obj: ind_os.c $(L1INC) $(TMINC) $(AUDIOINC)
		$(CL470) $(L1FLAGS16) $(L1DBG) $(CDEFS_TI) -I$(IL1) -I$(IL2) -I$(IL3) \
		      -I$(INC_TM) -I$(IN1) -i$(ICOM) -i$(INC_AUDIO) -I$(INC_GTT) -i$(INC_TM) $(INCDRV) ind_os.c                

$(OBJ)/l1audio_cust.obj: $(AUDIOCUSTCODE)/l1audio_cust.c $(L1INC) $(AUDIOINC) $(TMINC)
		$(CL470) $(L1FLAGS16_NO_OPT_WA) $(L1DBG) $(CDEFS_TI) -I$(IL1) -I$(IL2) -I$(IL3) \
       -I$(IN1) -i$(ICOM) -i$(INC_AUDIO) -i$(INC_TM) -I$(INC_GTT) $(INCDRV) $(INCAUDIOENTITY) $(AUDIOCUSTCODE)/l1audio_cust.c

$(OBJ)/l1tm_cust.obj: $(TMCUSTCODE)/l1tm_cust.c $(L1INC) $(TMINC) $(AUDIOINC)
		$(CL470) $(L1FLAGS16) $(L1DBG) $(CDEFS_TI) -I$(IL1) -I$(IL2) -I$(IL3) -I$(IL7) \
	      -I$(INC_TM) -I$(IN1) -i$(ICOM) -i$(INC_TM) -i$(INC_AUDIO) -I$(INC_GTT) $(INCDRV) $(TMCUSTCODE)/l1tm_cust.c

$(OBJ)/l1tm_tpu$(RF).obj: $(TMCUSTCODE)/l1tm_tpu$(RF).c $(L1INC) $(TMINC)
		$(CL470) $(L1FLAGS16) $(L1DBG) $(CDEFS_TI) -I$(IL1) -I$(IL2) -I$(IL3) -I$(IL7) \
	      -I$(INC_TM) -I$(IN1) -i$(ICOM) -i$(INC_TM) -i$(INC_AUDIO) -I$(INC_GTT) $(INCDRV) $(TMCUSTCODE)/l1tm_tpu$(RF).c

$(OBJ)/l1_cust.obj: l1_cust.c $(L1INC) $(TMINC)
		$(CL470) $(L1FLAGS16) $(L1DBG) $(CDEFS_TI) -I$(IL1) -I$(IL2) -I$(IL3) \
	      -I$(IL7) -I$(IN1) -i$(ICOM) -i$(INC_TM) -i$(INC_AUDIO) -I$(INC_GTT) $(INCDRV) $(INCAUDIOENTITY) l1_cust.c

