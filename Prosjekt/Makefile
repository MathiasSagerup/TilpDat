# Set the default goal to 'taco'
.DEFAULT_GOAL := taco

# Define compiler and flags (non-recursive assignment)
CC := gcc
CFLAGS := -O0 -g3

# Define object files
OBJ := drikke.o grønnsaker.o main.o protein.o taco_krydder.o taco_lefse.o taco_saus.o

taco: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -f $(OBJ) taco
