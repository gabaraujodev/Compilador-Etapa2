#include "sintatico.h"
#include <stdio.h>

/* Um token de lookahead; parada imediata no primeiro erro (sem recovery). */
static int falhar(Sintatico *s, const char *esperado) {
    if (s->erro) return 0;
    s->erro = 1;
    fprintf(stderr, "ERRO SINTATICO - linha %zu, coluna %zu: esperado %s; encontrado ",
            s->atual.linha, s->atual.coluna, esperado);
    if (s->atual.tipo == TK_EOF) fputs("fim de arquivo", stderr);
    else {
        texto_imprimir(s->atual.lexema, s->atual.tamanho, stderr);
        fprintf(stderr, " (%s)", token_nome(s->atual.tipo));
    }
    fputs(".\n", stderr);
    return 0;
}

static int avancar(Sintatico *s) {
    token_liberar(&s->atual);
    s->atual = lexico_proximo(s->lexico);
    if (s->atual.tipo == TK_ERRO) {
        fprintf(stderr, "ERRO LEXICO - linha %zu, coluna %zu: %s; trecho ",
                s->atual.linha, s->atual.coluna,
                s->atual.mensagem ? s->atual.mensagem : "token invalido");
        texto_imprimir(s->atual.lexema, s->atual.tamanho, stderr);
        fputc('\n', stderr);
        s->erro = 1;
        return 0;
    }
    return !s->erro;
}

static int esperar(Sintatico *s, TipoToken tipo, const char *descricao) {
    if (s->erro) return 0;
    if (s->atual.tipo != tipo) return falhar(s, descricao);
    return avancar(s);
}

static int eh_tipo(TipoToken t) { return t == TK_INT || t == TK_FLOAT || t == TK_CHAR; }
static int eh_relop(TipoToken t) {
    return t == TK_IGUAL || t == TK_MAIOR || t == TK_MAIOR_IGUAL ||
           t == TK_MENOR || t == TK_MENOR_IGUAL || t == TK_DIFERENTE;
}
static int eh_addop(TipoToken t) { return t == TK_MAIS || t == TK_MENOS || t == TK_OU; }
static int eh_mulop(TipoToken t) { return t == TK_MULT || t == TK_DIV || t == TK_E; }

static int expressao(Sintatico *s);
static int expressao_simples(Sintatico *s);
static int termo(Sintatico *s);
static int fator_a(Sintatico *s);
static int fator(Sintatico *s);
static int comando(Sintatico *s);

static int declaracao_correta(Sintatico *s) {
    if (!eh_tipo(s->atual.tipo)) return falhar(s, "tipo de declaracao");
    if (!avancar(s) || !esperar(s, TK_DOIS_PONTOS, "':' apos o tipo")) return 0;
    if (!esperar(s, TK_ID, "identificador na declaracao")) return 0;
    while (!s->erro && s->atual.tipo == TK_VIRGULA) {
        if (!avancar(s) || !esperar(s, TK_ID, "identificador depois de ','")) return 0;
    }
    return esperar(s, TK_PONTO_VIRGULA, "';' ao final da declaracao");
}

static int declaracoes(Sintatico *s) {
    while (!s->erro && eh_tipo(s->atual.tipo)) {
        if (!declaracao_correta(s)) return 0;
    }
    return !s->erro;
}

static int fator(Sintatico *s) {
    if (s->erro) return 0;
    switch (s->atual.tipo) {
        case TK_ID: case TK_INTEIRO: case TK_REAL: case TK_CARACTERE:
            return avancar(s);
        case TK_ABRE_PAR:
            return avancar(s) && expressao(s) &&
                   esperar(s, TK_FECHA_PAR, "')' para fechar a expressao");
        default:
            return falhar(s, "identificador, constante ou '(' no inicio da expressao");
    }
}
static int fator_a(Sintatico *s) {
    if (s->erro) return 0;
    if (s->atual.tipo == TK_NAO || s->atual.tipo == TK_MENOS) {
        if (!avancar(s)) return 0;
    }
    return fator(s);
}
static int termo(Sintatico *s) {
    if (!fator_a(s)) return 0;
    while (!s->erro && eh_mulop(s->atual.tipo)) {
        if (!avancar(s) || !fator_a(s)) return 0;
    }
    return 1;
}
static int expressao_simples(Sintatico *s) {
    if (!termo(s)) return 0;
    while (!s->erro && eh_addop(s->atual.tipo)) {
        if (!avancar(s) || !termo(s)) return 0;
    }
    return 1;
}
static int expressao(Sintatico *s) {
    if (!expressao_simples(s)) return 0;
    if (!s->erro && eh_relop(s->atual.tipo)) {
        if (!avancar(s) || !expressao_simples(s)) return 0;
    }
    return 1;
}

static int eh_fim_bloco(TipoToken t, int admite_else, int admite_until) {
    return t == TK_END || (admite_else && t == TK_ELSE) ||
           (admite_until && t == TK_UNTIL);
}

static int comandos(Sintatico *s, int admite_else, int admite_until) {
    if (!comando(s)) return 0;
    while (!s->erro && s->atual.tipo == TK_PONTO_VIRGULA) {
        if (!avancar(s)) return 0;
        if (eh_fim_bloco(s->atual.tipo, admite_else, admite_until))
            return falhar(s, "comando depois de ';' (remova o ';' final do bloco)");
        if (!comando(s)) return 0;
    }
    if (s->erro) return 0;
    if (!eh_fim_bloco(s->atual.tipo, admite_else, admite_until))
        return falhar(s, "';' entre comandos ou encerramento do bloco");
    return 1;
}

static int comando(Sintatico *s) {
    if (s->erro) return 0;
    switch (s->atual.tipo) {
        case TK_ID: /* atribuicao */
            return avancar(s) && esperar(s, TK_ATRIB, "'=' na atribuicao") &&
                   expressao_simples(s);
        case TK_IN:
            return avancar(s) && esperar(s, TK_ABRE_PAR, "'(' apos in") &&
                   esperar(s, TK_ID, "identificador como argumento de in") &&
                   esperar(s, TK_FECHA_PAR, "')' apos in");
        case TK_OUT:
            if (!avancar(s) || !esperar(s, TK_ABRE_PAR, "'(' apos out")) return 0;
            if (s->atual.tipo == TK_LITERAL) {
                if (!avancar(s)) return 0;
            } else if (!expressao_simples(s)) return 0;
            return esperar(s, TK_FECHA_PAR, "')' apos out");
        case TK_IF:
            if (!avancar(s) || !expressao(s) ||
                !esperar(s, TK_THEN, "'then' depois da condicao do if") ||
                !declaracoes(s) || !comandos(s, 1, 0)) return 0;
            if (s->atual.tipo == TK_ELSE) {
                if (!avancar(s) || !declaracoes(s) || !comandos(s, 0, 0)) return 0;
            }
            return esperar(s, TK_END, "'end' para fechar if");
        case TK_WHILE:
            return avancar(s) && expressao(s) &&
                   esperar(s, TK_DO, "'do' depois da condicao do while") &&
                   declaracoes(s) && comandos(s, 0, 0) &&
                   esperar(s, TK_END, "'end' para fechar while");
        case TK_REPEAT:
            return avancar(s) && declaracoes(s) && comandos(s, 0, 1) &&
                   esperar(s, TK_UNTIL, "'until' para fechar repeat") && expressao(s);
        default:
            return falhar(s, "comando (atribuicao, if, while, repeat, in ou out)");
    }
}

void sintatico_iniciar(Sintatico *s, Lexico *l) {
    s->lexico = l;
    s->erro = 0;
    s->atual = (Token){TK_EOF, NULL, 0, 1, 1, SEM_SIMBOLO, NULL};
    avancar(s);
}
int sintatico_analisar(Sintatico *s) {
    if (s->erro) return 0;
    return esperar(s, TK_PROGRAM, "'program' no inicio do arquivo") &&
           declaracoes(s) && esperar(s, TK_BEGIN, "'begin' apos as declaracoes") &&
           comandos(s, 0, 0) && esperar(s, TK_END, "'end' do programa") &&
           esperar(s, TK_EOF, "fim de arquivo apos 'end'");
}
void sintatico_finalizar(Sintatico *s) { token_liberar(&s->atual); }
