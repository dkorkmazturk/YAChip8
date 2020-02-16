#include "chip8.h"

int main(int argc, char *argv[])
{
    std::string romName;

    if (argc >= 2)
    {
        romName = argv[1];
    }
    else
    {
        return 1;
    }

    YAChip8::Chip8 chip8(romName);
    chip8.run();

    return 0;
}
