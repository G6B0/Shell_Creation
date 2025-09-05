#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <unistd.h>
#include <sys/wait.h>
#include <cstring>

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
        while (!cmd.empty() && cmd.back() == ' ') cmd.erase(cmd.begin());
        comandos.push_back(cmd);
    }
    return comandos;
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
            auto args = parsear_input(prompt);
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
