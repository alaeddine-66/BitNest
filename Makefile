.PHONY: debug

CC = gcc
OPTIONS = -Wall -pedantic -g
BIBLIO=-lreadline
DEBUG_OPTIONS =
EXECUTABLE = main
# Ne pas modifier ci-dessous
SOURCE = $(wildcard *.c)
OBJETS = $(SOURCE:.c=.o)


all: $(EXECUTABLE)

debug: DEBUG_OPTIONS = -g
debug: $(EXECUTABLE)


$(EXECUTABLE): $(OBJETS)
	$(CC) $(DEBUG_OPTIONS) $(OPTIONS) $(OBJETS) $(BIBLIO) -o $(EXECUTABLE)

%.o: %.c
	$(CC) $(DEBUG_OPTIONS) $(OPTIONS) -MMD -c $<

clean:
	rm -vf *.o *.d

-include *.d
