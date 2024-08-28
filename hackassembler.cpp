#include "Assembler.h"

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        cerr << "Usage: " << argv[0] << "<input.asm> <output.hack>";
        return 1;
    }

    HackAssembler assembler(argv[1], argv[2]);
    assembler.assemble();
    return 0;
}