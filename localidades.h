#ifndef LOCALIDADES_H
#define LOCALIDADES_H

#include "lista_ligada.h"

typedef struct {
    char *nome;
    double latitude;
    double longitude;
} Localidade;

void carregar_localidades(const char *nome_arquivo, ListaLigada *lista);
void adicionar_localidade(ListaLigada *lista, const char *nome, double latitude, double longitude);
void remover_localidade(ListaLigada *lista, const char *nome);
void salvar_localidades(const char *nome_arquivo, ListaLigada *lista);
int comparador_localidades(const void *a, const void *b);

#endif
