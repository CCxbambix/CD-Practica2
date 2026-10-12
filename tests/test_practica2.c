#include "auxiliares.h"
#include "busqueda.h"
#include "convergecast.h"
#include "nodo.h"
#include "ordenamiento.h"
#include "simulador.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define TIEMPO_DE_EJECUCION 10U
#define NUM_ESTRELLA 8

#define COMPROBAR(condicion, ...)                         \
    do {                                                  \
        if (!(condicion)) {                               \
            fprintf(stderr, "    FALLO: ");              \
            fprintf(stderr, __VA_ARGS__);                 \
            fprintf(stderr, "\n");                       \
            correcta = false;                             \
        }                                                 \
    } while (0)

static int comparar_enteros(const void *a, const void *b) {
    int x = *(const int *)a;
    int y = *(const int *)b;
    return (x > y) - (x < y);
}

static void destruir_nodos(Nodo **nodos, size_t cantidad) {
    for (size_t i = 0; i < cantidad; ++i) {
        nodo_destruir(nodos[i]);
    }
}

static bool test_auxiliares(void) {
    bool correcta = true;
    const int datos[] = {9, 2, 7, 1, 8, 3, 6, 5, 4, 0};
    Cuadricula cuadricula = {0};

    COMPROBAR(cuadricula_crear(datos, 10, 3, &cuadricula),
              "cuadricula_crear regreso false");
    if (correcta) {
        COMPROBAR(cuadricula.longitudes[0] == 4 &&
                      cuadricula.longitudes[1] == 3 &&
                      cuadricula.longitudes[2] == 3,
                  "la division 10/3 no produjo segmentos 4,3,3");

        for (size_t i = 0; i < cuadricula.cantidad; ++i) {
            ordenar_insercion(cuadricula.segmentos[i],
                              cuadricula.longitudes[i]);
        }

        int *fusionado = NULL;
        size_t longitud = 0;
        COMPROBAR(k_way_merge(&cuadricula, &fusionado, &longitud),
                  "k_way_merge regreso false");
        COMPROBAR(longitud == 10, "k_way_merge perdio elementos");
        for (size_t i = 0; i < longitud; ++i) {
            COMPROBAR(fusionado[i] == (int)i,
                      "resultado incorrecto en la posicion %zu", i);
        }
        free(fusionado);
    }
    cuadricula_destruir(&cuadricula);

    const int pocos[] = {10, 20, 30};
    COMPROBAR(cuadricula_crear(pocos, 3, 5, &cuadricula),
              "fallo cuando hay mas trabajadores que elementos");
    if (correcta) {
        const size_t esperadas[] = {1, 1, 1, 0, 0};
        for (size_t i = 0; i < 5; ++i) {
            COMPROBAR(cuadricula.longitudes[i] == esperadas[i],
                      "longitud incorrecta para el segmento %zu", i);
        }
    }
    cuadricula_destruir(&cuadricula);
    return correcta;
}

static bool probar_arbol(const int *const *hijos,
                         const size_t *cantidades,
                         size_t cantidad_nodos) {
    bool correcta = true;
    Simulador simulador;
    Nodo **nodos = calloc(cantidad_nodos, sizeof(*nodos));
    int *padres = malloc(cantidad_nodos * sizeof(*padres));
    if (nodos == NULL || padres == NULL) {
        free(nodos);
        free(padres);
        return false;
    }

    for (size_t i = 0; i < cantidad_nodos; ++i) {
        padres[i] = -1;
    }
    padres[0] = 0;
    for (size_t padre = 0; padre < cantidad_nodos; ++padre) {
        for (size_t j = 0; j < cantidades[padre]; ++j) {
            int hijo = hijos[padre][j];
            if (hijo < 0 || (size_t)hijo >= cantidad_nodos ||
                padres[hijo] != -1) {
                correcta = false;
            } else {
                padres[hijo] = (int)padre;
            }
        }
    }

    COMPROBAR(simulador_inicializar(&simulador),
              "no se pudo inicializar el simulador");
    if (correcta) {
        for (size_t i = 0; i < cantidad_nodos; ++i) {
            COMPROBAR(padres[i] != -1, "el nodo %zu no tiene padre", i);
            nodos[i] = nodo_convergecast_crear((int)i,
                                                padres[i],
                                                hijos[i],
                                                cantidades[i],
                                                (int)i,
                                                combinar_suma);
            COMPROBAR(nodos[i] != NULL, "no se pudo crear el nodo %zu", i);
            if (nodos[i] != NULL) {
                COMPROBAR(simulador_registrar_nodo(&simulador, nodos[i]),
                          "no se pudo registrar el nodo %zu", i);
            }
        }
    }

    if (correcta) {
        for (size_t i = 0; i < cantidad_nodos; ++i) {
            COMPROBAR(nodo_convergecast_iniciar(nodos[i], &simulador),
                      "no se pudo iniciar el nodo %zu", i);
        }
        COMPROBAR(simulador_ejecutar_hasta(&simulador, TIEMPO_DE_EJECUCION),
                  "fallo durante la simulacion");

        int suma_esperada = (int)(cantidad_nodos * (cantidad_nodos - 1) / 2);
        COMPROBAR(nodo_convergecast_finalizo(nodos[0]),
                  "la raiz no finalizo");
        COMPROBAR(nodo_convergecast_resultado(nodos[0]) == suma_esperada,
                  "la raiz obtuvo %d y se esperaba %d",
                  nodo_convergecast_resultado(nodos[0]),
                  suma_esperada);
        COMPROBAR(simulador.mensajes_entregados == cantidad_nodos - 1,
                  "convergecast debe entregar n-1 mensajes");
        COMPROBAR(!simulador_tiene_eventos(&simulador),
                  "quedaron eventos pendientes");
    }

    simulador_destruir(&simulador);
    destruir_nodos(nodos, cantidad_nodos);
    free(nodos);
    free(padres);
    return correcta;
}

static bool test_convergecast(void) {
    static const int A1_0[] = {1, 2};
    static const int A1_1[] = {3};
    static const int A1_2[] = {5};
    static const int A1_3[] = {4};
    static const int *const ARBOL_1[] = {
        A1_0, A1_1, A1_2, A1_3, NULL, NULL
    };
    static const size_t G1[] = {2, 1, 1, 1, 0, 0};

    static const int A2_0[] = {1, 2};
    static const int A2_1[] = {5};
    static const int A2_2[] = {3};
    static const int A2_3[] = {4};
    static const int A2_5[] = {6, 7};
    static const int *const ARBOL_2[] = {
        A2_0, A2_1, A2_2, A2_3, NULL, A2_5, NULL, NULL
    };
    static const size_t G2[] = {2, 1, 1, 1, 0, 2, 0, 0};

    static const int A3_0[] = {1, 2, 3};
    static const int A3_1[] = {4, 5};
    static const int A3_2[] = {6};
    static const int A3_3[] = {7, 8, 9};
    static const int A3_6[] = {10};
    static const int *const ARBOL_3[] = {
        A3_0, A3_1, A3_2, A3_3, NULL, NULL,
        A3_6, NULL, NULL, NULL, NULL
    };
    static const size_t G3[] = {3, 2, 1, 3, 0, 0, 1, 0, 0, 0, 0};

    return probar_arbol(ARBOL_1, G1, 6) &&
           probar_arbol(ARBOL_2, G2, 8) &&
           probar_arbol(ARBOL_3, G3, 11);
}

static bool crear_estrella_ordenamiento(Simulador *simulador,
                                        Nodo **nodos) {
    static const int trabajadores[] = {1, 2, 3, 4, 5, 6, 7};
    static const int coordinador[] = {0};

    if (!simulador_inicializar(simulador)) {
        return false;
    }
    nodos[0] = nodo_ordenamiento_crear(0, trabajadores, 7);
    for (int i = 1; i < NUM_ESTRELLA; ++i) {
        nodos[i] = nodo_ordenamiento_crear(i, coordinador, 1);
    }
    for (int i = 0; i < NUM_ESTRELLA; ++i) {
        if (nodos[i] == NULL || !simulador_registrar_nodo(simulador, nodos[i])) {
            return false;
        }
    }
    return true;
}

static bool test_ordenamiento(void) {
    bool correcta = true;
    const int arreglo[] = {31, 7, 100, 4, 18, 2, 71, 6, 9, 1, 50, 12};
    const size_t longitud = sizeof(arreglo) / sizeof(arreglo[0]);
    int esperado[sizeof(arreglo) / sizeof(arreglo[0])];
    for (size_t i = 0; i < longitud; ++i) {
        esperado[i] = arreglo[i];
    }
    qsort(esperado, longitud, sizeof(*esperado), comparar_enteros);

    Simulador simulador;
    Nodo *nodos[NUM_ESTRELLA] = {0};
    COMPROBAR(crear_estrella_ordenamiento(&simulador, nodos),
              "no se pudo crear la estrella");

    if (correcta) {
        for (int i = 0; i < NUM_ESTRELLA; ++i) {
            COMPROBAR(nodo_ordenamiento_iniciar(nodos[i],
                                                &simulador,
                                                arreglo,
                                                longitud),
                      "no se pudo iniciar el nodo %d", i);
        }
        COMPROBAR(simulador_ejecutar_hasta(&simulador, TIEMPO_DE_EJECUCION),
                  "fallo durante la simulacion");
        COMPROBAR(nodo_ordenamiento_finalizo(nodos[0]),
                  "el coordinador no finalizo");

        size_t longitud_resultado = 0;
        const int *resultado = nodo_ordenamiento_resultado(nodos[0],
                                                           &longitud_resultado);
        COMPROBAR(longitud_resultado == longitud,
                  "el resultado tiene longitud incorrecta");
        for (size_t i = 0; i < longitud && resultado != NULL; ++i) {
            COMPROBAR(resultado[i] == esperado[i],
                      "valor incorrecto en la posicion %zu", i);
        }
        COMPROBAR(simulador.mensajes_entregados == 14,
                  "se esperaban 7 trabajos y 7 respuestas");
    }

    simulador_destruir(&simulador);
    destruir_nodos(nodos, NUM_ESTRELLA);
    return correcta;
}

static bool crear_estrella_busqueda(Simulador *simulador, Nodo **nodos) {
    static const int trabajadores[] = {1, 2, 3, 4, 5, 6, 7};
    static const int coordinador[] = {0};

    if (!simulador_inicializar(simulador)) {
        return false;
    }
    nodos[0] = nodo_busqueda_crear(0, trabajadores, 7);
    for (int i = 1; i < NUM_ESTRELLA; ++i) {
        nodos[i] = nodo_busqueda_crear(i, coordinador, 1);
    }
    for (int i = 0; i < NUM_ESTRELLA; ++i) {
        if (nodos[i] == NULL || !simulador_registrar_nodo(simulador, nodos[i])) {
            return false;
        }
    }
    return true;
}

static bool probar_busqueda(int elemento, bool esperado) {
    bool correcta = true;
    const int arreglo[] = {31, 7, 100, 4, 18, 2, 71, 6, 9, 1, 50, 12};
    const size_t longitud = sizeof(arreglo) / sizeof(arreglo[0]);
    Simulador simulador;
    Nodo *nodos[NUM_ESTRELLA] = {0};

    COMPROBAR(crear_estrella_busqueda(&simulador, nodos),
              "no se pudo crear la estrella");
    if (correcta) {
        for (int i = 0; i < NUM_ESTRELLA; ++i) {
            COMPROBAR(nodo_busqueda_iniciar(nodos[i],
                                            &simulador,
                                            arreglo,
                                            longitud,
                                            elemento),
                      "no se pudo iniciar el nodo %d", i);
        }
        COMPROBAR(simulador_ejecutar_hasta(&simulador, TIEMPO_DE_EJECUCION),
                  "fallo durante la simulacion");
        COMPROBAR(nodo_busqueda_finalizo(nodos[0]),
                  "el coordinador no recibio todas las respuestas");
        COMPROBAR(nodo_busqueda_resultado(nodos[0]) == esperado,
                  "resultado incorrecto al buscar %d", elemento);
        COMPROBAR(simulador.mensajes_entregados == 14,
                  "se esperaban 7 trabajos y 7 respuestas");
    }

    simulador_destruir(&simulador);
    destruir_nodos(nodos, NUM_ESTRELLA);
    return correcta;
}

static bool test_busqueda(void) {
    return probar_busqueda(100, true) && probar_busqueda(-1000, false);
}

typedef bool (*FuncionPrueba)(void);

static int ejecutar_prueba(const char *nombre, FuncionPrueba prueba) {
    printf("[PRUEBA] %s\n", nombre);
    if (prueba()) {
        printf("  OK\n");
        return 0;
    }
    printf("  FALLO\n");
    return 1;
}

int main(void) {
    int fallos = 0;
    fallos += ejecutar_prueba("Funciones auxiliares", test_auxiliares);
    fallos += ejecutar_prueba("Convergecast en tres arboles", test_convergecast);
    fallos += ejecutar_prueba("Ordenamiento distribuido", test_ordenamiento);
    fallos += ejecutar_prueba("Busqueda distribuida", test_busqueda);

    if (fallos == 0) {
        printf("\nTodas las pruebas pasaron.\n");
        return 0;
    }
    fprintf(stderr, "\nFallaron %d prueba(s).\n", fallos);
    return 1;
}
