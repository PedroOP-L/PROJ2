//  PROJ2
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "localidades.h"
#include "rotas.h"
#include "lista_ligada.h"
#include "utils.h"

void mostrar_ajuda() {
    printf("Uso: ./processa_rotas [OPÇÕES]\n");
    printf("Opções:\n");
    printf("  -TL              Testar leitura do ficheiro localidades.txt\n");
    printf("  -LO <arquivo>    Especificar arquivo de saída para localidades\n");
    printf("  -ADI <nome> <lat> <long> Adicionar localidade\n");
    printf("  -REM <nome>      Remover localidade\n");
    printf("  -ROTAS [<id>]    Processar rotas (opcional: ID específico)\n");
    printf("  -LR <arquivo>    Especificar arquivo de saída para rotas\n");
    printf("  -h               Mostrar esta ajuda\n");
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        mostrar_ajuda();
        return EXIT_FAILURE;
    }

    char *arquivo_localidades_saida = NULL;
    char *arquivo_rotas_saida = NULL;
    int flag_tl = 0, flag_adi = 0, flag_rem = 0, flag_rotas = 0;
    char *nome_adi = NULL, *nome_rem = NULL;
    double lat_adi = 0, long_adi = 0;
    int id_rota = -1;

    // Processar argumentos
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-TL") == 0) {
            flag_tl = 1;
        } else if (strcmp(argv[i], "-LO") == 0 && i+1 < argc) {
            arquivo_localidades_saida = argv[++i];
        } else if (strcmp(argv[i], "-ADI") == 0 && i+3 < argc) {
            flag_adi = 1;
            nome_adi = argv[++i];
            lat_adi = atof(argv[++i]);
            long_adi = atof(argv[++i]);
        } else if (strcmp(argv[i], "-REM") == 0 && i+1 < argc) {
            flag_rem = 1;
            nome_rem = argv[++i];
        } else if (strcmp(argv[i], "-ROTAS") == 0) {
            flag_rotas = 1;
            if (i+1 < argc && argv[i+1][0] != '-') {
                id_rota = atoi(argv[++i]);
            }
        } else if (strcmp(argv[i], "-LR") == 0 && i+1 < argc) {
            arquivo_rotas_saida = argv[++i];
        } else if (strcmp(argv[i], "-h") == 0) {
            mostrar_ajuda();
            return EXIT_SUCCESS;
        }
    }

    // Criar estruturas de dados
    ListaLigada *lista_localidades = criar_lista(comparador_localidades);
    ListaLigada *lista_rotas = criar_lista(comparador_rotas);

    // Carregar localidades
    carregar_localidades("localidades.txt", lista_localidades);

    // Processar comandos
    if (flag_adi) {
        adicionar_localidade(lista_localidades, nome_adi, lat_adi, long_adi);
    }

    if (flag_rem) {
        remover_localidade(lista_localidades, nome_rem);
    }

    if (flag_tl || flag_adi || flag_rem) {
        if (arquivo_localidades_saida != NULL) {
            salvar_localidades(arquivo_localidades_saida, lista_localidades);
        } else {
            printf("AVISO: Nenhum arquivo de saída especificado para localidades (-LO)\n");
        }
    }

    if (flag_rotas) {
        carregar_rotas("rotas.txt", lista_rotas, lista_localidades);
        
        if (id_rota != -1) {
            // Processar apenas uma rota específica
            Rota busca = {.id = id_rota};
            Rota *rota = buscar(lista_rotas, &busca, 
                (int (*)(const void *, const void *))comparador_rotas);
            if (rota != NULL) {
                processar_rota(rota, lista_localidades);
            }
        } else {
            // Processar todas as rotas
            processar_todas_rotas(lista_rotas, lista_localidades);
        }
        
        if (arquivo_rotas_saida != NULL) {
            salvar_rotas(arquivo_rotas_saida, lista_rotas);
        } else {
            printf("AVISO: Nenhum arquivo de saída especificado para rotas (-LR)\n");
        }
    }

    // Liberar memória
    destruir_lista(lista_localidades, liberar_localidade);
    destruir_lista(lista_rotas, liberar_rota);

    return EXIT_SUCCESS;
}
// utils 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "utils.h"

#define RAIO_TERRA 6371.0

void *malloc_seguro(size_t tamanho) {
    void *ptr = malloc(tamanho);
    if (ptr == NULL) {
        perror("Erro ao alocar memória");
        exit(EXIT_FAILURE);
    }
    return ptr;
}

char *duplicar_string(const char *origem) {
    if (origem == NULL) return NULL;
    char *copia = malloc_seguro(strlen(origem) + 1);
    strcpy(copia, origem);
    return copia;
}

bool validar_coordenadas(double latitude, double longitude) {
    return (latitude >= -90.0 && latitude <= 90.0) && 
           (longitude >= -180.0 && longitude <= 180.0);
}

double calcular_distancia(double lat1, double lon1, double lat2, double lon2) {
    // Converter graus para radianos
    double phi1 = lat1 * M_PI / 180.0;
    double lambda1 = lon1 * M_PI / 180.0;
    double phi2 = lat2 * M_PI / 180.0;
    double lambda2 = lon2 * M_PI / 180.0;
    
    // Diferenças
    double delta_phi = phi2 - phi1;
    double delta_lambda = lambda2 - lambda1;
    
    // Aproximação usando tangente (como especificado no projeto)
    double delta_y = RAIO_TERRA * tan(delta_phi);
    double delta_x = RAIO_TERRA * tan(delta_lambda);
    
    // Distância euclidiana no plano tangente
    return sqrt(delta_x * delta_x + delta_y * delta_y);
}

void liberar_localidade(void *dados) {
    Localidade *loc = (Localidade *)dados;
    free(loc->nome);
    free(loc);
}

void liberar_rota(void *dados) {
    Rota *rota = (Rota *)dados;
    if (rota->pontos != NULL) {
        No *atual = rota->pontos->cabeca;
        while (atual != NULL) {
            No *proximo = atual->proximo;
            PontoRota *ponto = (PontoRota *)atual->dados;
            free(ponto->nome_localidade);
            free(ponto);
            free(atual);
            atual = proximo;
        }
        free(rota->pontos);
    }
    free(rota);
}
// rotas
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "rotas.h"
#include "localidades.h"
#include "utils.h"
#include "lista_ligada.h"

int comparador_rotas(const void *a, const void *b) {
    const Rota *rota_a = (const Rota *)a;
    const Rota *rota_b = (const Rota *)b;
    if (rota_a->distancia_total < rota_b->distancia_total) return -1;
    if (rota_a->distancia_total > rota_b->distancia_total) return 1;
    return 0;
}

void carregar_rotas(const char *nome_arquivo, ListaLigada *lista_rotas, ListaLigada *lista_localidades) {
    FILE *arquivo = fopen(nome_arquivo, "r");
    if (arquivo == NULL) {
        perror("Erro ao abrir arquivo de rotas");
        exit(EXIT_FAILURE);
    }

    char linha[256];
    Rota *rota_atual = NULL;
    
    while (fgets(linha, sizeof(linha), arquivo) != NULL) {
        // Remover newline no final
        linha[strcspn(linha, "\n")] = '\0';
        
        // Ignorar linhas em branco
        if (strspn(linha, " \t") == strlen(linha)) {
            continue;
        }

        // Verificar se é início de nova rota
        if (strstr(linha, "#ROTA") == linha) {
            int id;
            if (sscanf(linha, "#ROTA %d", &id) == 1) {
                rota_atual = malloc_seguro(sizeof(Rota));
                rota_atual->id = id;
                rota_atual->pontos = criar_lista(NULL); // Não precisa ordenar pontos
                rota_atual->distancia_total = 0.0;
                inserir_ordenado(lista_rotas, rota_atual);
            }
        } else if (rota_atual != NULL) {
            // Adicionar ponto à rota atual
            Localidade busca = {.nome = linha};
            Localidade *localidade = buscar(lista_localidades, &busca, 
                (int (*)(const void *, const void *))comparador_localidades);
            
            if (localidade != NULL) {
                PontoRota *ponto = malloc_seguro(sizeof(PontoRota));
                ponto->nome_localidade = duplicar_string(localidade->nome);
                ponto->distancia_parcial = 0.0;
                ponto->distancia_acumulada = 0.0;
                
                No *novo_no = malloc_seguro(sizeof(No));
                novo_no->dados = ponto;
                novo_no->proximo = NULL;
                
                if (rota_atual->pontos->cabeca == NULL) {
                    rota_atual->pontos->cabeca = novo_no;
                } else {
                    No *atual = rota_atual->pontos->cabeca;
                    while (atual->proximo != NULL) {
                        atual = atual->proximo;
                    }
                    atual->proximo = novo_no;
                }
            }
        }
    }

    fclose(arquivo);
}

void processar_rota(Rota *rota, ListaLigada *lista_localidades) {
    if (rota == NULL || rota->pontos->cabeca == NULL) {
        return;
    }

    No *anterior = rota->pontos->cabeca;
    No *atual = anterior->proximo;
    
    // Primeiro ponto tem distância zero
    PontoRota *ponto_anterior = (PontoRota *)anterior->dados;
    ponto_anterior->distancia_parcial = 0.0;
    ponto_anterior->distancia_acumulada = 0.0;
    
    Localidade busca = {.nome = ponto_anterior->nome_localidade};
    Localidade *loc_anterior = buscar(lista_localidades, &busca, 
        (int (*)(const void *, const void *))comparador_localidades);
    
    if (loc_anterior == NULL) {
        return;
    }

    while (atual != NULL) {
        PontoRota *ponto_atual = (PontoRota *)atual->dados;
        
        Localidade busca_atual = {.nome = ponto_atual->nome_localidade};
        Localidade *loc_atual = buscar(lista_localidades, &busca_atual, 
            (int (*)(const void *, const void *))comparador_localidades);
        
        if (loc_atual != NULL) {
            double distancia = calcular_distancia(
                loc_anterior->latitude, loc_anterior->longitude,
                loc_atual->latitude, loc_atual->longitude);
            
            ponto_atual->distancia_parcial = distancia;
            ponto_atual->distancia_acumulada = ((PontoRota *)anterior->dados)->distancia_acumulada + distancia;
            
            loc_anterior = loc_atual;
        }
        
        anterior = atual;
        atual = atual->proximo;
    }
    
    // Atualizar distância total da rota
    if (anterior != NULL) {
        rota->distancia_total = ((PontoRota *)anterior->dados)->distancia_acumulada;
    }
}

void processar_todas_rotas(ListaLigada *lista_rotas, ListaLigada *lista_localidades) {
    No *atual = lista_rotas->cabeca;
    while (atual != NULL) {
        processar_rota((Rota *)atual->dados, lista_localidades);
        atual = atual->proximo;
    }
    
    // Ordenar rotas por distância total
    ListaLigada *lista_ordenada = criar_lista(comparador_rotas);
    
    atual = lista_rotas->cabeca;
    while (atual != NULL) {
        No *proximo = atual->proximo;
        atual->proximo = NULL;
        inserir_ordenado(lista_ordenada, atual->dados);
        atual = proximo;
    }
    
    // Substituir a lista original pela ordenada
    free(lista_rotas);
    *lista_rotas = *lista_ordenada;
    free(lista_ordenada);
}

void salvar_rotas(const char *nome_arquivo, ListaLigada *lista_rotas) {
    FILE *arquivo = fopen(nome_arquivo, "w");
    if (arquivo == NULL) {
        perror("Erro ao abrir arquivo para escrita");
        return;
    }

    No *rota_atual = lista_rotas->cabeca;
    while (rota_atual != NULL) {
        Rota *rota = (Rota *)rota_atual->dados;
        fprintf(arquivo, "#ROTA %d\n", rota->id);
        
        No *ponto_atual = rota->pontos->cabeca;
        while (ponto_atual != NULL) {
            PontoRota *ponto = (PontoRota *)ponto_atual->dados;
            fprintf(arquivo, "%s %.2f km (total: %.2f km)\n", 
                ponto->nome_localidade, 
                ponto->distancia_parcial,
                ponto->distancia_acumulada);
            ponto_atual = ponto_atual->proximo;
        }
        
        fprintf(arquivo, "Distância total: %.2f km\n\n", rota->distancia_total);
        rota_atual = rota_atual->proximo;
    }

    fclose(arquivo);
}
// localidades 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "localidades.h"
#include "utils.h"
#include "lista_ligada.h"

int comparador_localidades(const void *a, const void *b) {
    const Localidade *loc_a = (const Localidade *)a;
    const Localidade *loc_b = (const Localidade *)b;
    return strcmp(loc_a->nome, loc_b->nome);
}

void carregar_localidades(const char *nome_arquivo, ListaLigada *lista) {
    FILE *arquivo = fopen(nome_arquivo, "r");
    if (arquivo == NULL) {
        perror("Erro ao abrir arquivo de localidades");
        exit(EXIT_FAILURE);
    }

    char linha[256];
    int numero_linha = 0;
    
    while (fgets(linha, sizeof(linha), arquivo) != NULL) {
        numero_linha++;
        
        // Ignorar linhas em branco
        if (strspn(linha, " \t\n") == strlen(linha)) {
            continue;
        }

        char nome[100];
        double latitude, longitude;
        int resultado = sscanf(linha, "%99s %lf %lf", nome, &latitude, &longitude);
        
        if (resultado != 3) {
            fprintf(stderr, "Linha %d: Erro no número de parâmetros\n", numero_linha);
            fclose(arquivo);
            exit(EXIT_FAILURE);
        }

        if (!validar_coordenadas(latitude, longitude)) {
            if (latitude < -90.0 || latitude > 90.0) {
                fprintf(stderr, "Linha %d: Erro latitude fora dos limites\n", numero_linha);
            } else {
                fprintf(stderr, "Linha %d: Erro longitude fora dos limites\n", numero_linha);
            }
            fclose(arquivo);
            exit(EXIT_FAILURE);
        }

        Localidade *nova_localidade = malloc_seguro(sizeof(Localidade));
        nova_localidade->nome = duplicar_string(nome);
        nova_localidade->latitude = latitude;
        nova_localidade->longitude = longitude;
        
        inserir_ordenado(lista, nova_localidade);
    }

    fclose(arquivo);
}

void adicionar_localidade(ListaLigada *lista, const char *nome, double latitude, double longitude) {
    if (!validar_coordenadas(latitude, longitude)) {
        fprintf(stderr, "Coordenadas inválidas\n");
        return;
    }

    // Verificar se localidade já existe
    Localidade busca = {.nome = (char *)nome};
    if (buscar(lista, &busca, (int (*)(const void *, const void *))comparador_localidades) != NULL) {
        fprintf(stderr, "Localidade '%s' já existe\n", nome);
        return;
    }

    Localidade *nova_localidade = malloc_seguro(sizeof(Localidade));
    nova_localidade->nome = duplicar_string(nome);
    nova_localidade->latitude = latitude;
    nova_localidade->longitude = longitude;
    
    inserir_ordenado(lista, nova_localidade);
}

void remover_localidade(ListaLigada *lista, const char *nome) {
    Localidade busca = {.nome = (char *)nome};
    Localidade *removida = remover(lista, &busca, (int (*)(const void *, const void *))comparador_localidades);
    if (removida != NULL) {
        liberar_localidade(removida);
    } else {
        fprintf(stderr, "Localidade '%s' não encontrada\n", nome);
    }
}

void salvar_localidades(const char *nome_arquivo, ListaLigada *lista) {
    FILE *arquivo = fopen(nome_arquivo, "w");
    if (arquivo == NULL) {
        perror("Erro ao abrir arquivo para escrita");
        return;
    }

    // Escrever número de localidades
    int contador = 0;
    No *atual = lista->cabeca;
    while (atual != NULL) {
        contador++;
        atual = atual->proximo;
    }
    fprintf(arquivo, "%d\n", contador);

    // Escrever quantidade de memória usada
    size_t memoria = contador * sizeof(Localidade) + contador * sizeof(No);
    fprintf(arquivo, "%zu bytes\n", memoria);

    // Escrever localidades
    atual = lista->cabeca;
    while (atual != NULL) {
        Localidade *loc = (Localidade *)atual->dados;
        fprintf(arquivo, "%s %.4f %.4f\n", loc->nome, loc->latitude, loc->longitude);
        atual = atual->proximo;
    }

    fclose(arquivo);
}
// lista_ligada
#include <stdlib.h>
#include "lista_ligada.h"

ListaLigada *criar_lista(int (*comparador)(const void *, const void *)) {
    ListaLigada *lista = malloc_seguro(sizeof(ListaLigada));
    lista->cabeca = NULL;
    lista->comparador = comparador;
    return lista;
}

void destruir_lista(ListaLigada *lista, void (*liberar_dados)(void *)) {
    No *atual = lista->cabeca;
    while (atual != NULL) {
        No *proximo = atual->proximo;
        if (liberar_dados != NULL) {
            liberar_dados(atual->dados);
        }
        free(atual);
        atual = proximo;
    }
    free(lista);
}

void inserir_ordenado(ListaLigada *lista, void *dados) {
    No *novo_no = malloc_seguro(sizeof(No));
    novo_no->dados = dados;
    novo_no->proximo = NULL;

    if (lista->cabeca == NULL || lista->comparador(dados, lista->cabeca->dados) < 0) {
        novo_no->proximo = lista->cabeca;
        lista->cabeca = novo_no;
    } else {
        No *atual = lista->cabeca;
        while (atual->proximo != NULL && lista->comparador(dados, atual->proximo->dados) > 0) {
            atual = atual->proximo;
        }
        novo_no->proximo = atual->proximo;
        atual->proximo = novo_no;
    }
}

void *remover(ListaLigada *lista, const void *chave, int (*comparador_chave)(const void *, const void *)) {
    No *anterior = NULL;
    No *atual = lista->cabeca;

    while (atual != NULL) {
        if (comparador_chave(atual->dados, chave) == 0) {
            if (anterior == NULL) {
                lista->cabeca = atual->proximo;
            } else {
                anterior->proximo = atual->proximo;
            }
            void *dados = atual->dados;
            free(atual);
            return dados;
        }
        anterior = atual;
        atual = atual->proximo;
    }
    return NULL;
}

void *buscar(ListaLigada *lista, const void *chave, int (*comparador_chave)(const void *, const void *)) {
    No *atual = lista->cabeca;
    while (atual != NULL) {
        if (comparador_chave(atual->dados, chave) == 0) {
            return atual->dados;
        }
        atual = atual->proximo;
    }
    return NULL;
}

void iterar_lista(ListaLigada *lista, void (*funcao)(void *)) {
    No *atual = lista->cabeca;
    while (atual != NULL) {
        funcao(atual->dados);
        atual = atual->proximo;
    }
}
