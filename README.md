# Shell_Creation
Creación de una Shell en Linux requerido como la primera tarea del ramo Sistemas Operativos.

A continuación veremos una interfaz de shell comun y corriente que empieza con mishell$: 

## Compilación y ejecución
Para poder compilar se requiere de:
- Un compilador para C++ (g++)
- Sistema Linux/Unix
- Tener las bibliotecas estándar de C++ 

```bash
g++ Shell.cpp -o Shell  
./Shell
```
![Visualización de cómo usar mishell:$](Imagenes_README/Compilar_Ejecutar.png)

## Sintaxis para uso de mishell:$ 
```mishell:$ COMANDO (argumentos) | COMANDO (argumento) | ...```

## Sintaxis comando miprof 
```mishell:$ miprof OPCIÓN (argumentos) COMANDO (argumentos)```

Información que entrega:
  - Tiempo real: tiempo total de ejecución 
  - Tiempo de usuario: tiempo CPU en modo usuario 
  - Tiempo sistema: tiempo CPU en modo kernel 
  - Max Resident Set: memoria RAM máxima usada en KB 

## Ejemplo
### Comando sin y con pipes 

```mishell:$ ls -la```

![Uso de comando básico](Imagenes_README/mishell_sin_pipes.png)

```mishell:$ ps -aux | sort -nr -k 4 | head -20```

![Uso de comando con pipes](Imagenes_README/mishell_con_pipes.png)

### Comandos de miprof
1. ejec: 
```mishell:$ miprof ejec sort archivo.txt```
![Uso de miprof con ejec](Imagenes_README/miprof_ejec.png)
2. ejecsave: 
```mishell:$ miprof ejecsave resultados.txt sort archivo.txt```
![Uso de miprof con ejecsave para guardar resultados](Imagenes_README/miprof_ejecsave.png)
3. ejecución con máximo de tiempo: 
```mishell:$ miprof ejecutar maxtiempo 2 sleep 4```
![Uso de miprof y comando con tiempo máximo de ejecución](Imagenes_README/miprof_maxtiempo.png)




