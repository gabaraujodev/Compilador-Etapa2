#ifndef TABELA_SIMBOLOS_H
#define TABELA_SIMBOLOS_H

#include "token.h"

typedef struct {
    char *lexema;
    TipoToken token;
} Simbolo;

typedef struct {
    Simbolo *itens;
    size_t quantidade, capacidade;
} TabelaSimbolos;

void ts_iniciar(TabelaSimbolos *ts);
size_t ts_instalar(TabelaSimbolos *ts, const char *lexema, TipoToken tipo);
void ts_imprimir(const TabelaSimbolos *ts, FILE *saida);
void ts_liberar(TabelaSimbolos *ts);

#endif
