CC = g++
CFLAGS += -Wall
CFLAGS += --std=c++11

RM = rm -f

ODIR = ../bin

LIB_CFG1 = ../lib/config1.a
# LIB_CFG2 = ../lib/config2.a

SRC_FILES = $(wildcard *.cc)

BINS_CFG1 = $(patsubst %.cc,$(ODIR)/%-config1,$(SRC_FILES))
# BINS_CFG2 = $(patsubst %.cc,$(ODIR)/%-config2,$(SRC_FILES))

.PHONY: all clean
all: $(BINS_CFG1) # $(BINS_CFG2)


$(ODIR)/%-config1: %.cc
	$(CC) $(CFLAGS) -o $@ $^ $(LIB_CFG1)

# $(ODIR)/%-config2: %.cc
# 	$(CC) $(CFLAGS) -o $@ $^ $(LIB_CFG2)

clean:
	-@$(RM) $(BINS_CFG1)
# 	-@$(RM) $(BINS_CFG2)
