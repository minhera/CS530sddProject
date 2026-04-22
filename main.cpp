#include "limited_assembler.h"
#include <iostream>
using namespace std;

int main(int argc, char* argv[]){
    // Check if there are any source file passed on
    if (argc < 2){
        cerr << "Have to pass at least one source file to run the program\n";
        return 1;
    }

    // Iterate through the passed arguemtns (excluding the first becauase thats the program)
    for (int i = 1; i < argc; i++){
        string filename = argv[i];
        LimitedAssembler assembler = LimitedAssembler(filename);
        assembler.assemble(argv[i]);  
    }

    return 0;
}
