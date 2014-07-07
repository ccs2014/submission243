OBJ = pump_simulator.o
EXE = simulate

SRC = pump_simulator.c
H = 

CFLAGS=-I ../core -I./ -g  -std=gnu99 -MMD -MP -pedantic -Wall
CFLAGS+=-O2 -mtune=native
LDFLAGS = -lz

$(EXE): $(OBJ) Makefile
	gcc $(CFLAGS) -lm $(OBJ) -o $(EXE) $(LDFLAGS)

pump_simulator.o: pump_simulator.c $(H) $(SRC) Makefile
	gcc $(CFLAGS) -c $*.c $(LDFLAGS)


-include pump_simulator.d

junk += *.o *.d simulate*

.PHONY: assemble

clean:
	rm -rf $(junk)
