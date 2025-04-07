#ifndef ROTAS_H
#define ROTAS_H

#include "lista_ligada.h"

typedef struct {
    char *nome_localidade;
    double distancia_parcial;
    double distancia_acumulada;
} PontoRota;

typedef struct {
    int id;
    ListaLigada *pontos;
    double distancia_total;
} Rota;

void carregar_rotas(const char *nome_arquivo, ListaLigada *lista_rotas, ListaLigada *lista_localidades);
void processar_rota(Rota *rota, ListaLigada *lista_localidades);
void processar_todas_rotas(ListaLigada *lista_rotas, ListaLigada *lista_localidades);
void salvar_rotas(const char *nome_arquivo, ListaLigada *lista_rotas);
int comparador_rotas(const void *a, const void *b);

#endif
