#include "busqueda.h"

#include "auxiliares.h"
#include "mensaje.h"

#include <stdlib.h>

typedef struct {
    bool coordinador;
    bool iniciado;
    bool finalizo;
    bool encontrado;
    size_t esperados;
    size_t recibidos;
    bool *respondio;
} EstadoBusqueda;

static void destruir_estado_busqueda(void *estado_generico) {
    EstadoBusqueda *estado = estado_generico;
    if (estado == NULL) {
        return;
    }
    free(estado->respondio);
    free(estado);
}

static size_t indice_trabajador(const Nodo *nodo, int id) {
    for (size_t i = 0; i < nodo->cantidad_vecinos; ++i) {
        if (nodo->vecinos[i] == id) {
            return i;
        }
    }
    return nodo->cantidad_vecinos;
}
static void procesar_trabajo_busqueda(
    Nodo *nodo,
    const Mensaje *mensaje,
    Simulador *simulador) {

    /*
     * TODO:
     * 1. Buscar mensaje->valor en mensaje->datos.
     * 2. Crear MSG_BUSCAR_RESULTADO.
     * 3. Guardar verdadero o falso en respuesta.valor.
     * 4. Enviar la respuesta al coordinador.
     */
    //Buscamos el valor del punto 1
    int valorDato = mensaje->datos[mensaje->valor];
    //Creamos una variable para nuestro mensaje
    Mensaje respuesta;
    if(!mensaje_crear(&respuesta,MSG_BUSCAR_RESULTADO,nodo->id,nodo->id,valorDato,NULL,0)){
        simulador->error_memoria=true;
        return;
    }
    if(!simulador_enviar(simulador,0,&respuesta,TICK_BUSQUEDA)){
        simulador->error_memoria = true;
        return;
    }
}

static void procesar_resultado_busqueda(Nodo *nodo,
                                        const Mensaje *mensaje) {
    EstadoBusqueda *estado = nodo->estado;
    size_t indice = indice_trabajador(nodo, mensaje->origen);
    if (indice == nodo->cantidad_vecinos || estado->respondio[indice]) {
        return;
    }

    estado->respondio[indice] = true;
    ++estado->recibidos;
    estado->encontrado = estado->encontrado || mensaje->valor != 0;
    estado->finalizo = estado->recibidos == estado->esperados;
}

static void recibir_busqueda(Nodo *nodo,
                             const Mensaje *mensaje,
                             Simulador *simulador) {
    EstadoBusqueda *estado = nodo->estado;
    if (!estado->coordinador && mensaje->tipo == MSG_BUSCAR_TRABAJO) {
        procesar_trabajo_busqueda(nodo, mensaje, simulador);
    } else if (estado->coordinador &&
               mensaje->tipo == MSG_BUSCAR_RESULTADO) {
        procesar_resultado_busqueda(nodo, mensaje);
    }
}

Nodo *nodo_busqueda_crear(int id,
                          const int *vecinos,
                          size_t cantidad_vecinos) {
    EstadoBusqueda *estado = calloc(1, sizeof(*estado));
    if (estado == NULL) {
        return NULL;
    }
    estado->coordinador = id == 0;
    estado->esperados = estado->coordinador ? cantidad_vecinos : 0;

    if (estado->coordinador && cantidad_vecinos > 0) {
        estado->respondio = calloc(cantidad_vecinos, sizeof(*estado->respondio));
        if (estado->respondio == NULL) {
            destruir_estado_busqueda(estado);
            return NULL;
        }
    }

    Nodo *nodo = nodo_crear(id,
                            vecinos,
                            cantidad_vecinos,
                            recibir_busqueda,
                            estado,
                            destruir_estado_busqueda);
    if (nodo == NULL) {
        destruir_estado_busqueda(estado);
    }
    return nodo;
}

bool nodo_busqueda_iniciar(Nodo *nodo,
                           Simulador *simulador,
                           const int *arreglo,
                           size_t longitud,
                           int elemento) {
    (void)nodo;
    (void)simulador;
    (void)arreglo;
    (void)longitud;
    (void)elemento;

    /*
     * TODO para el coordinador:
     * 1. Dividir el arreglo con cuadricula_crear.
     * 2. Crear MSG_BUSCAR_TRABAJO.
     * 3. Colocar el elemento buscado en mensaje.valor.
     * 4. Enviar un segmento a cada trabajador.
     */
    EstadoBusqueda *estado = nodo->estado;
    if(!estado->coordinador){
        return true;
    }
    if(estado->iniciado){
        return true;
    }
    estado->iniciado=true;
    if(estado->esperados==0&&longitud>0){
        return false;
    }
    Cuadricula secondArray;
    if(!cuadricula_crear(arreglo,longitud,estado->esperados,&secondArray)){
        simulador->error_memoria = true;
        return false;
    }
    for(size_t i = 0; i<=estado->esperados-1; i++){
        Mensaje buscarTrabajo;
        if(!mensaje_crear(&buscarTrabajo,MSG_BUSCAR_TRABAJO,nodo->id,nodo->id,elemento,secondArray.segmentos[i],secondArray.longitudes[i])){
        simulador->error_memoria = true;
        return false;
        }
        if(!simulador_enviar(simulador,nodo->vecinos[i],&buscarTrabajo,TICK_BUSQUEDA)){
            simulador->error_memoria = true;
            return false;
        }
    }
    cuadricula_destruir(&secondArray);
    return true;
}

bool nodo_busqueda_finalizo(const Nodo *nodo) {
    if (nodo == NULL || nodo->estado == NULL) {
        return false;
    }
    const EstadoBusqueda *estado = nodo->estado;
    return estado->finalizo;
}

bool nodo_busqueda_resultado(const Nodo *nodo) {
    if (nodo == NULL || nodo->estado == NULL) {
        return false;
    }
    const EstadoBusqueda *estado = nodo->estado;
    return estado->encontrado;
}
