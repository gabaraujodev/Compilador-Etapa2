#include "lexico.h"
#include <stdlib.h>

static int letra(int c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

static int digito(int c) { 
    return c >= '0' && c <= '9'; 
}

static int inicio_id(int c) { 
    return letra(c) || c == '_';
}

static int parte_id(int c) { 
    return inicio_id(c) || digito(c); 
}

static int espaco(int c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f' || c == '\v';
}

static void avancar(Lexico *l) {
    if (l->atual == EOF) 
        return;

    if (l->atual == '\r') { 
        l->linha++; l->coluna = 1; 
    }
    else if (l->atual == '\n') {
        if (!l->anterior_cr) 
            l->linha++;

        l->coluna = 1;
    } else 
        l->coluna++;

    l->anterior_cr = l->atual == '\r';
    l->atual = l->seguinte;
    l->seguinte = fgetc(l->arquivo);
}

static void consumir(Lexico *l, Token *t, size_t *capacidade) {
    if (t->tamanho + 2 > *capacidade) {
        size_t nova = *capacidade ? *capacidade * 2 : 32;
        char *p = realloc(t->lexema, nova);
        if (!p) { 
            fprintf(stderr, "Falha interna: memoria insuficiente\n"); 
            exit(2); 
        }
        t->lexema = p;
        *capacidade = nova;
    }
    t->lexema[t->tamanho++] = (char)l->atual;
    t->lexema[t->tamanho] = '\0';
    avancar(l);
}

static Token erro(Token t, const char *mensagem) {
    t.tipo = TK_ERRO;
    t.mensagem = mensagem;
    return t;
}

void lexico_iniciar(Lexico *l, FILE *arquivo, TabelaSimbolos *ts) {
    l->arquivo = arquivo;
    l->atual = fgetc(arquivo);
    l->seguinte = fgetc(arquivo);
    l->linha = l->coluna = 1;
    l->anterior_cr = 0;
    l->ts = ts;
}

Token lexico_proximo(Lexico *l) {
    int c;
    Token t = {TK_EOF, NULL, 0, l->linha, l->coluna, SEM_SIMBOLO, NULL};
    size_t capacidade = 0;

    /* Ignora espacos e os dois formatos de comentarios do enunciado.
       A linguagem usa { ... } e % ate o fim da linha (nao modulo). */
    for (;;) {
        while (espaco(l->atual))
            avancar(l);
        t.linha = l->linha;
        t.coluna = l->coluna;
        if (l->atual == '%') {
            do { avancar(l); } while (l->atual != EOF && l->atual != '\n' && l->atual != '\r');
            continue;
        }
        if (l->atual == '{') {
            avancar(l);
            while (l->atual != EOF && l->atual != '}')
                avancar(l);
            if (l->atual == EOF)
                return erro(t, "comentario { sem fechamento }");
            avancar(l);
            continue;
        }
        break;
    }
    if (l->atual == EOF) 
        return t;

    if (inicio_id(l->atual)) {
        do { 
            consumir(l, &t, &capacidade); 
        } while (parte_id(l->atual));

        t.indice_ts = ts_instalar(l->ts, t.lexema, TK_ID);
        t.tipo = l->ts->itens[t.indice_ts].token;
        return t;
    }

    if (digito(l->atual)) {
        t.tipo = TK_INTEIRO;
        do { 
            consumir(l, &t, &capacidade); 
        } while (digito(l->atual));

        if (l->atual == '.') {
            consumir(l, &t, &capacidade);
            if (!digito(l->atual)) 
                return erro(t, "float exige digito depois do ponto");

            t.tipo = TK_REAL;
            do { 
                consumir(l, &t, &capacidade); 
            } while (digito(l->atual));
        }
        if (inicio_id(l->atual)) {
            do { 
                consumir(l, &t, &capacidade); 
            } while (parte_id(l->atual));

            return erro(t, "numero seguido de letras ou _: identificador nao pode iniciar com digito");
        }
        return t;
    }

    if (l->atual == '"') {
        consumir(l, &t, &capacidade);
        while (l->atual != '"') {
            if (l->atual == EOF || l->atual == '\n' || l->atual == '\r')
                return erro(t, "literal sem aspas duplas de fechamento na mesma linha");

            if (l->atual > 127) {
                while (l->atual != EOF && l->atual != '"' && l->atual != '\n' && l->atual != '\r')
                    consumir(l, &t, &capacidade);

                if (l->atual == '"') 
                    consumir(l, &t, &capacidade);

                return erro(t, "literal contem caractere fora de ASCII");
            }
            consumir(l, &t, &capacidade);
        }
        consumir(l, &t, &capacidade);
        t.tipo = TK_LITERAL;
        return t;
    }

    if (l->atual == '\'') {
        int ascii;
        consumir(l, &t, &capacidade);

        if (l->atual == EOF)
            return erro(t, "char exige exatamente um caractere ASCII entre apostrofos");
   
        if (l->atual == '\'' && l->seguinte != '\'') {
            consumir(l, &t, &capacidade);
            return erro(t, "char vazio: exige um caractere ASCII");
        }

        ascii = l->atual <= 127;
        consumir(l, &t, &capacidade);

        if (!ascii || l->atual != '\'') {
            while (l->atual != EOF && l->atual != '\'' && !espaco(l->atual) && l->atual != ';' && l->atual != ')' && l->atual != ',')
                consumir(l, &t, &capacidade);

            if (l->atual == '\'') 
                consumir(l, &t, &capacidade);
            
            return erro(t, "char exige exatamente um caractere ASCII entre apostrofos");
        }

        consumir(l, &t, &capacidade);
        t.tipo = TK_CARACTERE;
        return t;
    }

    c = l->atual;
    consumir(l, &t, &capacidade);

    switch (c) {
        case '=': t.tipo = TK_ATRIB; if (l->atual == '=') { consumir(l, &t, &capacidade); t.tipo = TK_IGUAL; } break;
        case '>': t.tipo = TK_MAIOR; if (l->atual == '=') { consumir(l, &t, &capacidade); t.tipo = TK_MAIOR_IGUAL; } break;
        case '<': t.tipo = TK_MENOR; if (l->atual == '=') { consumir(l, &t, &capacidade); t.tipo = TK_MENOR_IGUAL; } break;
        case '!': t.tipo = TK_NAO; if (l->atual == '=') { consumir(l, &t, &capacidade); t.tipo = TK_DIFERENTE; } break;
        case '&':
            if (l->atual != '&') 
                return erro(t, "& isolado: use &&");

            consumir(l, &t, &capacidade); t.tipo = TK_E; break;
        case '|':
            if (l->atual != '|') 
                return erro(t, "| isolado: use ||");

            consumir(l, &t, &capacidade); t.tipo = TK_OU; break;
        case '+': t.tipo = TK_MAIS; break;
        case '-': t.tipo = TK_MENOS; break;
        case '*': t.tipo = TK_MULT; break;
        case '/': t.tipo = TK_DIV; break;
        case '(': t.tipo = TK_ABRE_PAR; break;
        case ')': t.tipo = TK_FECHA_PAR; break;
        case ',': t.tipo = TK_VIRGULA; break;
        case ';': t.tipo = TK_PONTO_VIRGULA; break;
        case '.': t.tipo = TK_PONTO; break;
        case ':': t.tipo = TK_DOIS_PONTOS; break;
        default: return erro(t, "caractere nao pertence aos tokens da linguagem");
    }
    return t;
}