all: hotpotato

hotpotato: hotpotato.c
	mpiCC -o hotpotato hotpotato.c

clean:
	rm -f hotpotato