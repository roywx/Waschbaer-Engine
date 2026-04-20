#include "Engine.h"
#include "Timer.h"

int main(int argc, char* argv[]){
    SCOPED_TIMER("program");
    Engine engine;
    engine.GameLoop();

    return 0;
}


