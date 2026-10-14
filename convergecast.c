#include "convergecast.h"

#include "mensaje.h"

#include <stdlib.h>

typedef struct {
    int padre;
    int acumulado;
    size_t respuestas;
    bool *hijo_respondio;
    bool enviado;
    bool finalizo;
    FuncionCombinar combinar;
} EstadoConvergecast;

static void destruir_estado_convergecast(void *estado_generico) {
    EstadoConvergecast *estado = estado_generico;
    if (estado == NULL) {
        return;
    }
    free(estado->hijo_respondio);
    free(estado);
}

static bool enviar_acumulado(Nodo *nodo,
                             EstadoConvergecast *estado,
                             Simulador *simulador) {
    Mensaje mensaje;
    if (!mensaje_crear(&mensaje,
                       MSG_CONVERGECAST,
                       nodo->id,
                       nodo->id,
                       estado->acumulado,
                       NULL,
                       0)) {
        return false;
    }

    bool correcto = simulador_enviar(simulador,
                                      estado->padre,
                                      &mensaje,
                                      TICK_CONVERGECAST);
    mensaje_destruir(&mensaje);
    if (correcto) {
        estado->enviado = true;
    }
    return correcto;
}

static size_t indice_hijo(const Nodo *nodo, int id_hijo) {
    for (size_t i = 0; i < nodo->cantidad_vecinos; ++i) {
        if (nodo->vecinos[i] == id_hijo) {
            return i;
        }
    }
    return nodo->cantidad_vecinos;
}

static void recibir_convergecast(Nodo *nodo,
                                 const Mensaje *mensaje,
                                 Simulador *simulador) {
    if (nodo == NULL || mensaje == NULL || simulador == NULL || nodo->estado == NULL) {
        return;
    }
    if (mensaje->tipo != MSG_CONVERGECAST) {
        return;
    }

    EstadoConvergecast *estado = nodo->estado;

    size_t indice = indice_hijo(nodo, mensaje->origen);

    if (indice >= nodo->cantidad_vecinos) {
        return;
    }
    if (estado->hijo_respondio[indice]) {
        return;
    }

    estado->acumulado = estado->combinar(estado->acumulado, mensaje->valor);
    estado->hijo_respondio[indice] = true;
    estado->respuestas++;

    if (estado->respuestas == nodo->cantidad_vecinos) {
        if (nodo->id == estado->padre) {
            estado->finalizo = true;
        } else {
            enviar_acumulado(nodo, estado, simulador);
        }
    }
}

Nodo *nodo_convergecast_crear(int id,
                              int padre,
                              const int *hijos,
                              size_t cantidad_vecinos,
                              int valor_local,
                              FuncionCombinar combinar) {
    if (id < 0 || padre < 0 || combinar == NULL) {
        return NULL;
    }

    EstadoConvergecast *estado = calloc(1, sizeof(*estado));
    if (estado == NULL) {
        return NULL;
    }
    estado->padre = padre;
    estado->acumulado = valor_local;
    estado->combinar = combinar;

    if (cantidad_vecinos > 0) {
        estado->hijo_respondio = calloc(cantidad_vecinos,
                                        sizeof(*estado->hijo_respondio));
        if (estado->hijo_respondio == NULL) {
            destruir_estado_convergecast(estado);
            return NULL;
        }
    }

    Nodo *nodo = nodo_crear(id,
                            hijos,
                            cantidad_vecinos,
                            recibir_convergecast,
                            estado,
                            destruir_estado_convergecast);
    if (nodo == NULL) {
        destruir_estado_convergecast(estado);
    }
    return nodo;
}

bool nodo_convergecast_iniciar(Nodo *nodo, Simulador *simulador) {
    if (nodo == NULL || simulador == NULL || nodo->estado == NULL){
        return false;
    }
    EstadoConvergecast *estado = nodo->estado;

    if (nodo->cantidad_vecinos == 0) {
        if (nodo->id == estado->padre) {
            estado->finalizo = true;
            return true;
        }
        return enviar_acumulado(nodo, estado, simulador);
    }
    return true;
}

bool nodo_convergecast_finalizo(const Nodo *nodo) {
    if (nodo == NULL || nodo->estado == NULL) {
        return false;
    }
    const EstadoConvergecast *estado = nodo->estado;
    return estado->finalizo;
}

int nodo_convergecast_resultado(const Nodo *nodo) {
    if (nodo == NULL || nodo->estado == NULL) {
        return 0;
    }
    const EstadoConvergecast *estado = nodo->estado;
    return estado->acumulado;
}