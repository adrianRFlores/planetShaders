# Planet Shaders

# Sun Shader

Una Capa

https://github.com/adrianRFlores/planetShaders/assets/84111818/88bbe2ca-b2e1-4b62-adee-95dd354e171b

# Rocky Planet with Moon

Cuatro capas:
1. Mar, con gradiente de un tono azul oscuro a uno claro.
2. Tierra, con gradiente entre amarillo claro para la arena, hacia un tono café para tierra. El gradiente está pesado hacia la tierra.
3. Nubes
4. Luces nocturnas: Al ser un planeta civilizado (similar al nuestro), los habitantes encienden las luces de sus casas de noche. Estas luces son otra capa, y solo se muestra de noche, sobre terreno rocoso.

La luna cuenta con tres capas:
1. Terreno rocoso, con gradiente de un café oscuro hacia un tono más claro.
2. Lagunas de metano (similar a Titán).
3. Volcanes, representados por pequeñas lagunas rojas.

https://github.com/adrianRFlores/planetShaders/assets/84111818/45081ee0-0cfa-4b25-a300-4bd356fa9da9

# Cold, Rocky Planet

Este planeta está basado en Eve, de Kerbal Space Program.

Dos capas:
1. Terreno rocoso, con gradiente de un morado oscuro hacia un tono más claro.
2. Lagunas congeladas

https://github.com/adrianRFlores/planetShaders/assets/84111818/8cbd3ecd-f6b8-489d-b4bc-551aaa7c8570

# Gas Giant

Una Capa, con gradiente de verde claro hacia un tono más oscuro.

https://github.com/adrianRFlores/planetShaders/assets/84111818/dfdc2b92-5d3c-4352-a7c4-4cffc293aaec

# Gas Giant with Rings

Una capa, utiliza un shader simple. No utiliza texturas ni materiales.

https://github.com/adrianRFlores/planetShaders/assets/84111818/38f2d792-269f-489c-9dfe-b1c32464ffa2

# Compilar en Windows
Utilizar MinGW con ``mingw32-make.exe`` dentro del directorio del Makefile. SDL y GLM vienen empacados dentro de la carpeta src.
``mingw32-pthreads-w32`` debe estar instalado en MinGW para poder utilizar threading.
