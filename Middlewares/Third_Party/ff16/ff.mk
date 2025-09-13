FF_PATH ?= ${shell pwd}/Middlewares/Third_Party/ff16/source

CSRCS += $(shell find $(FF_PATH) -type f -name '*.c')

AFLAGS += "-I$(FF_PATH)"
CFLAGS += "-I$(FF_PATH)"
CXXFLAGS += "-I$(FF_PATH)"