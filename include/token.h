#ifndef TOKEN_H
#define TOKEN_H

#include <stddef.h>
#include <stdio.h>

typedef enum {
    TK_EOF, TK_ERRO, TK_ID, TK_INTEIRO, TK_REAL, TK_CARACTERE, TK_LITERAL,
    TK_PROGRAM, TK_BEGIN, TK_END, TK_INT, TK_FLOAT, TK_CHAR,
    TK_IF, TK_THEN, TK_ELSE, TK_WHILE, TK_DO, TK_REPEAT, TK_UNTIL,
    TK_READ, TK_WRITE, TK_ATRIB, TK_IGUAL, TK_MAIOR, TK_MAIOR_IGUAL,
    TK_MENOR, TK_MENOR_IGUAL, TK_DIFERENTE, TK_MAIS, TK_MENOS,
    TK_OU, TK_MULT, TK_DIV, TK_MOD, TK_E, TK_NAO,
    TK_ABRE_PAR, TK_FECHA_PAR, TK_VIRGULA, TK_PONTO_VIRGULA, TK_PONTO,
    TK_IN, TK_OUT, TK_DOIS_PONTOS
} TipoToken;

typedef struct {
    TipoToken tipo;
    char *lexema;
    size_t tamanho, linha, coluna, indice_ts;
    const char *mensagem;
} Token;

#define SEM_SIMBOLO ((size_t)-1)
const char *token_nome(TipoToken tipo);
void token_imprimir(const Token *token, FILE *saida);
void token_liberar(Token *token);
void texto_imprimir(const char *texto, size_t tamanho, FILE *saida);

#endif
