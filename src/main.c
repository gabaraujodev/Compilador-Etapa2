#include "sintatico.h"
#include <stdio.h>

/* A estrutura de leitura e o lexico sao os da etapa 1.
   A etapa 2 substitui a impressao de tokens pela analise sintatica. */
int main(int argc, char **argv) {
    FILE *arquivo;
    TabelaSimbolos ts;
    Lexico lexico;
    Sintatico parser;
    int ok;

    if (argc != 2) {
        fprintf(stderr, "Uso: %s arquivo_fonte.txt\n", argv[0]);
        return 2;
    }
    arquivo = fopen(argv[1], "rb");
    if (!arquivo) {
        perror(argv[1]);
        return 2;
    }

    ts_iniciar(&ts);
    lexico_iniciar(&lexico, arquivo, &ts);
    sintatico_iniciar(&parser, &lexico);
    ok = sintatico_analisar(&parser);
    sintatico_finalizar(&parser);

    if (ferror(arquivo)) {
        fprintf(stderr, "ERRO DE LEITURA: nao foi possivel ler o arquivo.\n");
        ok = 0;
    }
    ts_liberar(&ts);
    fclose(arquivo);

    if (!ok) return 1;
    puts("SUCESSO: analise lexica e sintatica concluidas sem erros.");
    return 0;
}
