#include <iostream>
#include "../include/Engine.hpp"

int main(int argc, char** argv){
    
    //Test de productor consumidor (3 thread productores, 3 consumidores) con buffer acotado (9x9)
    // Cual es la secuencia de disparo. 
    // Secuencia de disparo hardcodeada. Mostrar la red productor y consumidor
    //Documentando con diagrama de secuencia
    std::cout<<"Starting program...\n";
    auto engine = new Engine();
    engine->run();
    delete engine;
}