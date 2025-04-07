#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>
#include <stdlib.h>  

void *malloc_seguro(size_t tamanho);
char *duplicar_string(const char *origem);
bool validar_coordenadas(double latitude, double longitude);
double calcular_distancia(double lat1, double lon1, double lat2, double lon2);
void liberar_localidade(void *dados);
void liberar_rota(void *dados);

#endif
