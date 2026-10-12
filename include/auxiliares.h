#ifndef AUXILIARES_H
#define AUXILIARES_H

#include <stdbool.h>
#include <stddef.h>

typedef struct {
    int **segmentos;
    size_t *longitudes;
    size_t cantidad;
} Cuadricula;

bool cuadricula_crear(const int *arreglo,
                      size_t longitud,
                      size_t cantidad_segmentos,
                      Cuadricula *salida);
void cuadricula_destruir(Cuadricula *cuadricula);

bool k_way_merge(const Cuadricula *cuadricula,
                 int **resultado,
                 size_t *longitud_resultado);

void ordenar_insercion(int *arreglo, size_t longitud);
bool busqueda_lineal(const int *arreglo, size_t longitud, int elemento);
int combinar_suma(int acumulado, int valor);

#endif
