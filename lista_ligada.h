#ifndef LISTA_LIGADA_H
#define LISTA_LIGADA_H

typedef struct No {
    void *dados;
    struct No *proximo;
} No;

typedef struct {
    No *cabeca;
    int (*comparador)(const void *, const void *);
} ListaLigada;

ListaLigada *criar_lista(int (*comparador)(const void *, const void *));
void destruir_lista(ListaLigada *lista, void (*liberar_dados)(void *));
void inserir_ordenado(ListaLigada *lista, void *dados);
void *remover(ListaLigada *lista, const void *chave, int (*comparador_chave)(const void *, const void *));
void *buscar(ListaLigada *lista, const void *chave, int (*comparador_chave)(const void *, const void *));
void iterar_lista(ListaLigada *lista, void (*funcao)(void *));

#endif
