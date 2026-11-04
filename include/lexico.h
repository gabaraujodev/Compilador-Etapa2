#ifndef LEXICO_H
#define LEXICO_H

#include "tabela_simbolos.h"

typedef struct {
    FILE *arquivo;
    int atual, seguinte, anterior_cr;
    size_t linha, coluna;
    TabelaSimbolos *ts;
} Lexico;

void lexico_iniciar(Lexico *lexico, FILE *arquivo, TabelaSimbolos *ts);
Token lexico_proximo(Lexico *lexico);

#endif