#include "auxiliares.h"

#include <stdlib.h>
#include <string.h>

bool cuadricula_crear(const int *arreglo,
                      size_t longitud,
                      size_t cantidad_segmentos,
                      Cuadricula *salida) {
    if (salida == NULL || cantidad_segmentos == 0 ||
        (longitud > 0 && arreglo == NULL)) {
        return false;
    }

    salida->segmentos = calloc(cantidad_segmentos, sizeof(*salida->segmentos));
    salida->longitudes = calloc(cantidad_segmentos, sizeof(*salida->longitudes));
    salida->cantidad = cantidad_segmentos;
    if (salida->segmentos == NULL || salida->longitudes == NULL) {
        cuadricula_destruir(salida);
        return false;
    }

    size_t base = longitud / cantidad_segmentos;
    size_t residuo = longitud % cantidad_segmentos;
    size_t inicio = 0;

    for (size_t i = 0; i < cantidad_segmentos; ++i) {
        size_t tamano = base + (i < residuo ? 1U : 0U);
        salida->longitudes[i] = tamano;
        if (tamano > 0) {
            salida->segmentos[i] = malloc(tamano * sizeof(*salida->segmentos[i]));
            if (salida->segmentos[i] == NULL) {
                cuadricula_destruir(salida);
                return false;
            }
            memcpy(salida->segmentos[i],
                   arreglo + inicio,
                   tamano * sizeof(*arreglo));
        }
        inicio += tamano;
    }
    return true;
}

void cuadricula_destruir(Cuadricula *cuadricula) {
    if (cuadricula == NULL) {
        return;
    }
    if (cuadricula->segmentos != NULL) {
        for (size_t i = 0; i < cuadricula->cantidad; ++i) {
            free(cuadricula->segmentos[i]);
        }
    }
    free(cuadricula->segmentos);
    free(cuadricula->longitudes);
    cuadricula->segmentos = NULL;
    cuadricula->longitudes = NULL;
    cuadricula->cantidad = 0;
}

bool k_way_merge(const Cuadricula *cuadricula,
                 int **resultado,
                 size_t *longitud_resultado) {
    if (cuadricula == NULL || resultado == NULL || longitud_resultado == NULL) {
        return false;
    }

    size_t total = 0;
    for (size_t i = 0; i < cuadricula->cantidad; ++i) {
        total += cuadricula->longitudes[i];
    }

    int *salida = total == 0 ? NULL : malloc(total * sizeof(*salida));
    size_t *posiciones = calloc(cuadricula->cantidad, sizeof(*posiciones));
    if ((total > 0 && salida == NULL) ||
        (cuadricula->cantidad > 0 && posiciones == NULL)) {
        free(salida);
        free(posiciones);
        return false;
    }

    for (size_t k = 0; k < total; ++k) {
        size_t elegido = cuadricula->cantidad;
        for (size_t i = 0; i < cuadricula->cantidad; ++i) {
            if (posiciones[i] < cuadricula->longitudes[i] &&
                (elegido == cuadricula->cantidad ||
                 cuadricula->segmentos[i][posiciones[i]] <
                     cuadricula->segmentos[elegido][posiciones[elegido]])) {
                elegido = i;
            }
        }
        salida[k] = cuadricula->segmentos[elegido][posiciones[elegido]++];
    }

    free(posiciones);
    *resultado = salida;
    *longitud_resultado = total;
    return true;
}

void ordenar_insercion(int *arreglo, size_t longitud) {
    (void)arreglo;
    (void)longitud;

    /*
     * TODO:
     * Implementar un algoritmo simple de ordenamiento.
     */
}

bool busqueda_lineal(const int *arreglo, size_t longitud, int elemento) {
    (void)arreglo;
    (void)longitud;
    (void)elemento;

    /*
     * TODO:
     * Buscar el elemento recorriendo el arreglo.
     */

    for(size_t i = 0; i<longitud;i++){
        if(arreglo[i]== elemento){
            return true;
        }
    }
    return false;
}

int combinar_suma(int acumulado, int valor) {
    return acumulado + valor;
}
