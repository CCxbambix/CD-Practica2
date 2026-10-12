#ifndef ORDENAMIENTO_H
#define ORDENAMIENTO_H

#include "nodo.h"
#include "simulador.h"

#include <stdbool.h>
#include <stddef.h>

#define TICK_ORDENAMIENTO 1U
#define NODO_COORDINADOR 0

Nodo *nodo_ordenamiento_crear(int id,
                              const int *vecinos,
                              size_t cantidad_vecinos);
bool nodo_ordenamiento_iniciar(Nodo *nodo,
                               Simulador *simulador,
                               const int *arreglo,
                               size_t longitud);
bool nodo_ordenamiento_finalizo(const Nodo *nodo);
const int *nodo_ordenamiento_resultado(const Nodo *nodo,
                                       size_t *longitud);

#endif
