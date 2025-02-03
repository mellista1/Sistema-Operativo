# System programming
Repositorio con el código de un sistema operativo capaz de correr el "pong" y el "snake"

## ¿De qué se trata? 

### Introducción

Durante la cursada de la materia "Arquitectura y Organización del Computador", realicé un trabajo práctico con el objetivo de ingresar al mundo de
System Programming. 

Al arrancar una computadora, hay una serie de tareas que realiza el sistema operativo
que tienen como objetivo crear un entorno controlado y seguro donde
ejecutar programas y arbitrar el acceso a los recursos.

El trabajo fue incremental a lo largo de varias clases prácticas.
La idea fue crear un único software en modo 32 bits con un mismo conjunto
de archivos y código al cual se le van a ir agregando más código y
nuevos archivos.
Clase a clase, se trabajó una perspectiva o parte diferente del
sistema.

### El manual

Intel nos ofrece documentación para que podamos llevar a cabo la tarea
antes descripta. Como recurso utilicé los siguientes manuales:
Intel® 64 and IA-32 Architectures Software Developer's Manual Volume
1: Basic Architecture

Intel® 64 and IA-32 Architectures Software Developer's Manual Volume
2: Instruction Set Reference,
A-Z

Intel® 64 and IA-32 Architectures Software Developer's Manual Volume 3
(3A, 3B, 3C & 3D):System Programming Guide


### QEMU

Se utiliza como entorno de pruebas el programa QEMU. Este nos
permite simular el arranque de una computadora IBM-PC compatible.
Al inicio, una computadora comienza con la ejecución del POST y del BIOS. 
El BIOS se encarga de reconocer el primer dispositivo de booteo. 
En el presente trabajo, se utiliza como
dispositivo un Floppy Disk (el disquete en lugar del disco rígido como
suele ser comúnmente). Para eso, vamos a utilizar una imagen Floppy Disk
virtual en QEMU como dispositivo de booteo. En el primer sector del
floppy, se almacena el boot-sector (sector de arranque). El BIOS se
encarga de copiar a memoria 512 bytes del sector de booteo, y dejarlo a
partir de la dirección 0x7C00. Luego, se comienza a ejecutar el código a
partir de esta dirección. El boot-sector debe encontrar en el floppy el
archivo KERNEL.BIN y copiarlo a memoria. Éste se copia a partir de la
dirección 0x1200, y luego se ejecuta a partir de esa misma dirección.
Es importante tener en cuenta que el código del boot-sector se encarga
exclusivamente de copiar el kernel y dar el control al mismo, es decir,
no cambia el modo del procesador. Este código inicial viene dado en el
taller y el trabajo consistió en, a partir de ahí, construir parte de
ese kernel de modo que a final de cuatrimestre, pueda ejecutar programas
y tareas sencillas.



