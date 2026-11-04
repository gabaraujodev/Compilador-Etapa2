#ifndef SINTATICO_H
#define SINTATICO_H

#include "lexico.h"

typedef struct {
    Lexico *lexico;
    Token atual;
    int erro;
} Sintatico;

/* O analisador consome tokens gerados pelo lexico da etapa 1.
   Retorna 1 se o programa e sintaticamente valido, 0 caso contrario. */
void sintatico_iniciar(Sintatico *s, Lexico *l);
int sintatico_analisar(Sintatico *s);
void sintatico_finalizar(Sintatico *s);

#endif
