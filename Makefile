CC = gcc
CFLAGS ?= -std=c11 -Wall -Wextra -Wpedantic -O2
CPPFLAGS ?= -Iinclude
SRC := src/main.c src/lexico.c src/token.c src/tabela_simbolos.c src/sintatico.c
OUT := compilador
ifeq ($(OS),Windows_NT)
OUT := compilador.exe
endif

.PHONY: all test clean
all: $(OUT)
$(OUT): $(SRC) include/lexico.h include/token.h include/tabela_simbolos.h include/sintatico.h
	$(CC) $(CPPFLAGS) $(CFLAGS) $(SRC) -o $(OUT)

test: $(OUT)
	python3 scripts/testar.py ./$(OUT)

clean:
	rm -f compilador compilador.exe
