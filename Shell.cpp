#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <unistd.h>
#include <sys/wait.h>
#include <cstring>
#include <chrono> 
#include <sys/resource.h>
#include <sys/time.h>
#include <fcntl.h> 
#include <signal.h> 
#include <fstream> 

//Función que se encarga de parsear el prompt del usuario para
//poder recibir multiples argumentos

std::vector<char*> parsear_input(const std::string &prompt){
    std::istringstream iss(prompt); //Tokeniza la entrada
    std::string token; //Almacena los token por palabras
    std::vector<char*> args; //Vector que alamacenará cada token

    //Se encarga de parsear la entrada por espacios en el prompt
    while (iss >> token){
        char *arg = new char[token.size() + 1];
        std::strcpy(arg, token.c_str());
        args.push_back(arg);
    }
    args.push_back(nullptr); //execvp necesita NULL como último argumento
    return args;
}
//Función que se encarga de dividir todo el prompt por '|'

std::vector<std::string> dividir_comandos(const std::string &prompt){
    std::vector<std::string> comandos;
    std::istringstream iss(prompt);
    std::string cmd;

    while (std::getline(iss, cmd, '|')){
        //Quitamos espacios al inicio
        while (!cmd.empty() && cmd.front() == ' ') cmd.erase(cmd.begin());
        //Quitamos espacios al final
        while (!cmd.empty() && cmd.back() == ' ') cmd.pop_back();
        comandos.push_back(cmd);
    }
    return comandos;
}

// función para ejecutar miprof, 
void ejecutar_miprof(const std::vector<std::string>& args_miprof) { 
    auto argumentos=parsear_input(args_miprof[0]); 

    //variables para guardar tiempo real
    std::chrono::high_resolution_clock::time_point inicio_real, fin_real;
    struct rusage usage_before, usage_after;
    getrusage(RUSAGE_CHILDREN, &usage_before); // guardamos el estado antes del hijo
    
    std::string nombre_archivo; // archivo para guardar resultados
    std::string comando; // almacenará ejec, ejecsave, etc
    int maxtiempo=0;

    //para los escenarios al usar miprof (ejecsave y ejecutar)
    if (argumentos[1]!=nullptr){
        comando=argumentos[1]; // ejec, ejecsave, ejecutar

        // ejecsave: tiene un argumento extra para el nombre de archivo
        if (comando == "ejecsave") {
            if (argumentos[2] != nullptr) {
                nombre_archivo = argumentos[2];
            } else {
                std::cerr << "Error! ejecsave requiere un nombre de archivo" << std::endl;
                for (auto arg : argumentos) delete[] arg;
                return;
            }
        } 
        else if (comando == "ejecutar") {
            // ejecutar: argumento extra, el cual es numérico
            if (argumentos[2] != nullptr && std::strcmp(argumentos[2], "maxtiempo") == 0) {
                if (argumentos[2] != nullptr) {
                    maxtiempo = std::atoi(argumentos[2]); // tiempo máximo para usarlo más adelante
                } else {
                    std::cerr << "Error: maxtiempo requiere valor numérico" << std::endl;
                    for (auto arg : argumentos) delete[] arg;
                    return;
                }
            }
        }
        //en el caso que no sea ninguna de las 3 opciones
        else if (comando != "ejec") {
            std::cerr << "Error: modo no reconocido: " << comando << std::endl;
            for (auto arg : argumentos) delete[] arg;
            return;
        }
    }
    

    inicio_real=std::chrono::high_resolution_clock::now();
    pid_t pid = fork();
    if(pid==0){
        if (comando == "ejec") {
            execvp(argumentos[2], argumentos.data()+2);
        } else if (comando == "ejecsave") {
            execvp(argumentos[3], argumentos.data()+3);
        } else if (comando == "ejecutar") {
            execvp(argumentos[4], argumentos.data()+4);
        }
        perror("execvp");
        exit(1);
    } else if (pid>0){
        int estado_hijo;
        
        // implementar maxtiempo

        //esperar hasta que el proceso hijo termine
        waitpid(pid, &estado_hijo, 0);

        //medicion de tiempos
        fin_real=std::chrono::high_resolution_clock::now();
        //getusage
        getrusage(RUSAGE_CHILDREN, &usage_after);
 
        auto tiempo_real = std::chrono::duration_cast<std::chrono::milliseconds>(fin_real - inicio_real);
        // restamos los tiempos para obtener solo lo del comando
        double tiempo_usuario = ((usage_after.ru_utime.tv_sec - usage_before.ru_utime.tv_sec) + (usage_after.ru_utime.tv_usec - usage_before.ru_utime.tv_usec)/1e6) * 1000;     //(tv_sec = parte entera de los segundos) (tv_usec = parte fraccionaria en microsegundos)
        double tiempo_sistema = ((usage_after.ru_stime.tv_sec - usage_before.ru_stime.tv_sec) + (usage_after.ru_stime.tv_usec - usage_before.ru_stime.tv_usec)/1e6) * 1000;     // se divide y multiplica para ajustar la unidad de medida a milisegundos

        //maximum resident set
        long max_resident_set = usage_after.ru_maxrss;

        //Se imprimen los resultados
        std::cout << "\n===== Resultados de miprof ===== \n" << std::endl;
        std::cout << "Comando ejecutado: ";
        if (comando == "ejecsave") {
            for (int i = 3; argumentos[i] != nullptr; i++) {
                std::cout << argumentos[i] << " ";
            }
        } else if (comando == "ejec") {
            for (int i = 2; argumentos[i] != nullptr; i++) {
                std::cout << argumentos[i] << " ";
            }
        }
        std::cout << std::endl;

        if(!nombre_archivo.empty()) std::cout << "Archivo: " << nombre_archivo << std::endl;
        std::cout << "-Tiempo real: "<< tiempo_real.count() << " ms" << std::endl;
        std::cout << "-Tiempo usuario: " << tiempo_usuario << " ms" << std::endl;
        std::cout << "-Tiempo sistema: " << tiempo_sistema << " ms" << std::endl;
        std::cout << "-Max resident set: " << max_resident_set << " KB" << std::endl;

        // En el caso de que se ejecute "ejecsave" se deben guardar los resultados en un archivo
        if(comando == "ejecsave" && !nombre_archivo.empty()){
            std::ofstream out(nombre_archivo, std::ios::app); // crea si no existe, agrega si existe
            if(out){
                out << "\n\n===== Resultados de miprof =====\n";
                out << "Comando: ";
                for(int i = 1; argumentos[i] != nullptr; ++i)
                    out << argumentos[i] << " ";
                out << "\n";
                out << "-Tiempo real: " << tiempo_real.count() << " ms\n";
                out << "-Tiempo usuario: " << tiempo_usuario << " ms\n";
                out << "-Tiempo sistema: " << tiempo_sistema << " ms\n";
                out << "-Max resident set: " << max_resident_set << " KB\n";
                out << "===============================\n";
            } else {
                std::cerr << "Error al abrir el archivo " << nombre_archivo << std::endl;
            }
        }

    } else {
        perror("fork");
    }
    for (auto args:argumentos) delete[] args;
}

int main()
{
    std::string prompt;

    while(true){
        std::cout <<"mishell:$ " << std::flush;
        if (!std::getline(std::cin, prompt)) break;
        if (prompt == "exit") break;
        if (prompt.empty()) continue;
        //Se identifica los comandos a usar en el prompt y cuantos son
        auto comandos = dividir_comandos(prompt);
        int n = comandos.size();
        //Identificamos comando myprof, si n=1 y argumentos posibles :
        // "miprof", "ejec" o "ejecsave"+"archivo", "comando" y "args"
        if (n==1) {
            auto argumentos=parsear_input(comandos[0]);
            //vemos que el token sea "myprof"
            if(argumentos[0]!=nullptr && std::strcmp(argumentos[0],"miprof")==0){
                ejecutar_miprof(comandos); 
                for (auto args:argumentos) delete[] args; // liberamos memoria del parseo
                continue;
            }
            // si no, podemos liberar memoria y continuar
            for (auto args:argumentos) delete[] args;
        }

        //Creación de pipes

        std::vector<int[2]> pipes (n-1);
        for (int i = 0 ; i < n-1; i++){
            if (pipe(pipes[i]) == -1){
                perror("pipe");
                exit(1);
            }
        }
        //Creación de procesos conectados por pipes
        for (int i = 0; i < n; i++){
            auto args = parsear_input(comandos[i]);
            pid_t pid = fork();
            if (pid == 0){
                //Proceso Hijo
                //si no es el primero, lee desde pipe anterior
                if (i > 0){
                    dup2(pipes[i-1][0], STDIN_FILENO);
                }
                //si no es el ultimo, escribe en pipe actual
                if (i < n-1){
                    dup2(pipes[i][1], STDOUT_FILENO);
                }

                //hijos cierran todas las pipes
                for (int j = 0; j < n-1; j++){
                    close(pipes[j][0]);
                    close(pipes[j][1]);
                }
                execvp(args[0], args.data());
                perror("execvp");
                exit(1);
            }
            //Padre libera memoria de tokens
            for (auto arg: args) delete[] arg;
        }
        //Padre cierra todos los pipes
        for (int i = 0; i < n-1; i++){
            close(pipes[i][0]);
            close(pipes[i][1]);
        }
        //Esperar a todos los hijos
        for (int i = 0; i < n; i++){
            wait(nullptr);
        }
    }
    
    return 0;
}
