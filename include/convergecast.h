#ifndef CONVERGECAST_H
#define CONVERGECAST_H

#include "nodo.h"
#include "simulador.h"

#include <stdbool.h>
#include <stddef.h>

#define TICK_CONVERGECAST 1U

typedef int (*FuncionCombinar)(int acumulado, int valor);

Nodo *nodo_convergecast_crear(int id,
                              int padre,
                              const int *hijos,
                              size_t cantidad_hijos,
                              int valor_local,
                              FuncionCombinar combinar);
bool nodo_convergecast_iniciar(Nodo *nodo, Simulador *simulador);
bool nodo_convergecast_finalizo(const Nodo *nodo);
int nodo_convergecast_resultado(const Nodo *nodo);

#endif
