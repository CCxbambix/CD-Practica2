# Practica 2 migrada a C

Migracion de la Practica 2 al simulador discreto de eventos de la Practica 1.
No se incluye el algoritmo de arbol generador.

## Compilar y ejecutar

```bash
make test
```

Para compilar con AddressSanitizer y UndefinedBehaviorSanitizer:

```bash
make sanitize
```

## Estructura

```text
include/                 Interfaces publicas
src/                     Implementaciones
tests/test_practica2.c   Pruebas
Makefile                 Compilacion
```

La infraestructura reutilizada de la Practica 1 es:

- `mensaje.h/.c`: tipos de mensajes y copia profunda de payloads;
- `nodo.h/.c`: estructura comun de los nodos;
- `simulador.h/.c`: cola determinista de eventos.

Los modulos nuevos son:

- `auxiliares.h/.c`: cuadricula, k-way merge, insercion y busqueda lineal;
- `convergecast.h/.c`: reduccion de valores desde las hojas hasta la raiz;
- `ordenamiento.h/.c`: reparto, ordenamiento local y fusion de segmentos;
- `busqueda.h/.c`: reparto, busqueda local y combinacion OR de respuestas.

## Simplificaciones de la migracion

### Convergecast

El arbol se entrega ya construido. Cada nodo recibe directamente su padre y su
lista de hijos. Las hojas envian su valor; cada nodo interno combina los valores
recibidos con su valor local y envia un unico resultado parcial. La raiz guarda
el resultado final.

La operacion se recibe como una funcion binaria `FuncionCombinar`. Las pruebas
usan `combinar_suma`, por lo que la raiz obtiene la suma de los identificadores.
Se envia exactamente un mensaje por cada arista del arbol.

### Ordenamiento

El nodo 0 es el coordinador y los demas son trabajadores:

1. `cuadricula_crear` divide el arreglo equitativamente;
2. cada trabajador ordena un segmento mediante insercion;
3. cada trabajador devuelve su segmento;
4. el coordinador aplica `k_way_merge`.

### Busqueda

El nodo 0 divide el arreglo con la misma funcion `cuadricula_crear`. Cada
trabajador ejecuta una busqueda lineal y devuelve un booleano. El coordinador
combina las respuestas mediante OR.

## Mensajes

```text
MSG_CONVERGECAST
MSG_ORDENAR_TRABAJO
MSG_ORDENAR_RESULTADO
MSG_BUSCAR_TRABAJO
MSG_BUSCAR_RESULTADO
```

Un mensaje conserva el emisor en `origen`, un valor simple en `valor` y,
cuando se necesita, un arreglo dinamico en `datos`.

## Pruebas incluidas

- division balanceada, incluso cuando hay mas trabajadores que elementos;
- k-way merge;
- convergecast en los tres arboles del archivo Python original;
- ordenamiento distribuido en una estrella de ocho nodos;
- busqueda de un elemento presente y de uno ausente.

La prueba de convergecast deriva el padre a partir de las listas de hijos. Esta
informacion es necesaria porque el archivo original solo proporciona aristas
dirigidas de padre a hijo y las hojas necesitan conocer al padre para enviar su
resultado.
