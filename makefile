# applications
#CC=/usr/local/share/sdcc/bin/sdcc
CC=sdcc
RM=rm -rf
PROG=sudo picprog
SERIAL=/dev/ttyS0
SIM=gpsim
TABLER=./lookuptable.py

# program specific configuration
FAMILY=pic14
CHIP=16f627a
SRC_DIR=src
SRC=main.c
BUILD=build

# rules
all: program

compile: clean
	( cd $(BUILD) &&  $(CC) --use-non-free -m$(FAMILY) -p$(CHIP) ../$(SRC_DIR)/$(SRC) )

clean:
	$(RM) $(BUILD)/*

program: compile
	$(PROG) -p $(SERIAL) -i $(BUILD)/$(SRC:.c=.hex) -d pic$(CHIP) --erase --burn

simulate: compile
	$(SIM) $(BUILD)/$(SRC:.c=.cod)

table:
	$(TABLER)


.PHONY: all $(SRC:.c=.hex) compile clean program
.SILENT: all $(SRC:.c=.hex) compile clean program table
