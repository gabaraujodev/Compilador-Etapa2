#include "token.h"
#include <stdlib.h>

const char *token_nome(TipoToken tipo) {
    static const char *nomes[] = {
        "EOF", "ERRO", "ID", "INTEGER_CONST", "FLOAT_CONST", "CHAR_CONST", "LITERAL",
        "PROGRAM", "BEGIN", "END", "INT", "FLOAT", "CHAR", "IF", "THEN", "ELSE",
        "WHILE", "DO", "REPEAT", "UNTIL", "READ", "WRITE", "ATRIB", "IGUAL",
        "MAIOR", "MAIOR_IGUAL", "MENOR", "MENOR_IGUAL", "DIFERENTE", "MAIS",
        "MENOS", "OU", "MULT", "DIV", "MOD", "E", "NAO", "ABRE_PAR",
        "FECHA_PAR", "VIRGULA", "PONTO_VIRGULA", "PONTO", "IN", "OUT", "DOIS_PONTOS"
    };
    return nomes[tipo];
}

void texto_imprimir(const char *texto, size_t tamanho, FILE *saida) {
    size_t i;
    fputc('"', saida);
    for (i = 0; i < tamanho; i++) {
        unsigned char c = (unsigned char)texto[i];
        if (c == '\n') fputs("\\n", saida);
        else if (c == '\r') fputs("\\r", saida);
        else if (c == '\t') fputs("\\t", saida);
        else if (c == '"' || c == '\\') { fputc('\\', saida); fputc(c, saida); }
        else if (c < 32 || c > 126) fprintf(saida, "\\x%02X", c);
        else fputc(c, saida);
    }
    fputc('"', saida);
}

void token_imprimir(const Token *t, FILE *saida) {
    fprintf(saida, "%zu:%zu <%s", t->linha, t->coluna, token_nome(t->tipo));
    if (t->indice_ts != SEM_SIMBOLO) fprintf(saida, ", TS[%zu]", t->indice_ts);
    else if (t->tipo >= TK_INTEIRO && t->tipo <= TK_LITERAL) {
        fputs(", ", saida);
        texto_imprimir(t->lexema, t->tamanho, saida);
    }
    fputs("> lexema=", saida);
    texto_imprimir(t->lexema, t->tamanho, saida);
    fputc('\n', saida);
}

void token_liberar(Token *token) {
    free(token->lexema);
    token->lexema = NULL;
}
