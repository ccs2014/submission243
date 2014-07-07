OBJ = pump_simulator.o
EXE = simulate

SRC = pump_simulator.c
H = 

CFLAGS=-I ../core -I./ -g -lz -std=gnu99 -MMD -MP -pedantic -Wall
CFLAGS+=-O2 -mtune=native

$(EXE): $(OBJ) Makefile
	gcc $(CFLAGS) -lm $(OBJ) -o $(EXE)

pump_simulator.o: pump_simulator.c $(H) $(SRC) Makefile
	gcc $(CFLAGS) -c $*.c


-include pump_simulator.d

junk += *.o *.d simulate*

.PHONY: assemble

clean:
	rm -rf $(junk)
