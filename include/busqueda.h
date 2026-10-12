#ifndef BUSQUEDA_H
#define BUSQUEDA_H

#include "nodo.h"
#include "simulador.h"

#include <stdbool.h>
#include <stddef.h>

#define TICK_BUSQUEDA 1U

Nodo *nodo_busqueda_crear(int id,
                          const int *vecinos,
                          size_t cantidad_vecinos);
bool nodo_busqueda_iniciar(Nodo *nodo,
                           Simulador *simulador,
                           const int *arreglo,
                           size_t longitud,
                           int elemento);
bool nodo_busqueda_finalizo(const Nodo *nodo);
bool nodo_busqueda_resultado(const Nodo *nodo);

#endif
