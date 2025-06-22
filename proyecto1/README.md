# Computación Gráfica Proyecto 1

Tecnológico de Costa Rica
Profesor: Ernesto Rivera Alvarado
Estudiante: Cristian Arias Chaves

El proyecto consiste en comparar los siguientes algoritmos vistos en clase:

* Fuerza bruta
* Incremental
* Incremental versión 2
* Bresenham (Punto medio)

## Como ejecutar el programa

1. Compilar el programa utilizando el comando `make`:

2. Ejecutar el programa pasando los argumentos los siguientes argumentos:

* Resolucion: La resolucion con la cual el programa creara una ventana cuadrada donde pintara las lineas.
* Número de líneas: El número de lineas a dibujar en el cuadrado.
* Número de veces: El número de veces que el programa dibujara la misma cantidad de líneas.

Ejemplo: `./proyecto1 800 600 2000`

### Resultados

El programa escribira en consola los tiempos de cada algoritmo para cada versión. Hay tres versiones del programa:

- Versión 1: Solo ejecuta los algoritmos sin imprimir en pantalla ni guardad ninguna imagen.
- Versión 2: Ejecuta los algoritmos y guarda una imagen por cada uno.
- Versión 3: Ejecuta los algoritmos e imprime en pantalla un frame por algoritmo.

Por cada versión se creara un arreglo de lineas aleatorias diferente.

