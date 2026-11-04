#include "tabela_simbolos.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static void sem_memoria(void) {
    fprintf(stderr, "Falha interna: memoria insuficiente\n");
    exit(2);
}

size_t ts_instalar(TabelaSimbolos *ts, const char *lexema, TipoToken tipo) {
    char *copia;
    size_t i;

    /* Normalizacao ASCII: a linguagem nao diferencia maiusculas de minusculas. */
    size_t n = strlen(lexema);
    char *canonico = malloc(n + 1);
    if (!canonico) sem_memoria();
    for (i = 0; i < n; i++)
        canonico[i] = (char)tolower((unsigned char)lexema[i]);
    canonico[n] = '\0';
    for (i = 0; i < ts->quantidade; i++) {
        if (strcmp(ts->itens[i].lexema, canonico) == 0) {
            free(canonico);
            return i;
        }
    }

    if (ts->quantidade == ts->capacidade) {
        size_t capacidade = ts->capacidade ? ts->capacidade * 2 : 32;
        Simbolo *novos = realloc(ts->itens, capacidade * sizeof *novos);
        if (!novos) sem_memoria();
        ts->itens = novos;
        ts->capacidade = capacidade;
    }

    copia = canonico;
    i = ts->quantidade++;
    ts->itens[i].lexema = copia;
    ts->itens[i].token = tipo;

    return i;
}

void ts_iniciar(TabelaSimbolos *ts) {
    static const struct { const char *nome; TipoToken tipo; } reservadas[] = {
        {"program", TK_PROGRAM}, {"begin", TK_BEGIN}, {"end", TK_END},
        {"int", TK_INT}, {"float", TK_FLOAT}, {"char", TK_CHAR},
        {"if", TK_IF}, {"then", TK_THEN}, {"else", TK_ELSE},
        {"while", TK_WHILE}, {"do", TK_DO}, {"repeat", TK_REPEAT},
        {"until", TK_UNTIL}, {"in", TK_IN}, {"out", TK_OUT}
    };
    size_t i;
    ts->itens = NULL;
    ts->quantidade = ts->capacidade = 0;

    for (i = 0; i < sizeof reservadas / sizeof reservadas[0]; i++)
        ts_instalar(ts, reservadas[i].nome, reservadas[i].tipo);
}

void ts_imprimir(const TabelaSimbolos *ts, FILE *saida) {
    size_t i;
    fprintf(saida, "\nTABELA DE SIMBOLOS\nIndice | Token | Lexema\n");

    for (i = 0; i < ts->quantidade; i++)
        fprintf(saida, "%zu | %s | %s\n", i, token_nome(ts->itens[i].token), ts->itens[i].lexema);
}

void ts_liberar(TabelaSimbolos *ts) {
    size_t i;

    for (i = 0; i < ts->quantidade; i++) 
        free(ts->itens[i].lexema);
        
    free(ts->itens);
    ts->itens = NULL;
    ts->quantidade = ts->capacidade = 0;
}
