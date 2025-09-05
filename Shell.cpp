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


int main()
{
    std::string prompt;

    while(true){
        std::cout <<"$ " << std::flush;
        if (!std::getline(std::cin, prompt)) break;

        if (prompt == "exit") break;

        auto args = parsear_input(prompt);

        pid_t pid = fork();
        if (pid == 0){
            //Proceso hijo
            if (execvp(args[0], args.data()) == -1){
                perror("execvp");
            }
            exit(1);
        }
        else if(pid > 0){
            //Proceso padre
            int status;
            waitpid(pid, &status, 0);
        }
        else {
            perror("fork");
        }

        //Liberamos la memoria usada al parsear el argumento
        for (auto arg : args){
            delete[] arg;
        }
    }

    return 0;
}
