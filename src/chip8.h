#ifndef CHIP8_H
#define CHIP8_H

#include <vector>
#include <array>
#include <stack>
#include <cinttypes>
#include <thread>

#include "gui.h"

namespace YAChip8
{
class Chip8
{
private:
    std::array<uint8_t, 16> V;
    uint16_t I;
    uint16_t PC;
    uint8_t DT;
    uint8_t ST;

    GUI gui;

    std::vector<uint8_t> memory;
    std::stack<uint16_t> stack;

    std::thread timerThread;

    uint16_t fetch();
    void timer();
    void decode_and_execute(const uint16_t opcode);

public:
    Chip8(const std::pair<const int, const int> &logical_size, const int scale_factor, const std::string &file_name);
    ~Chip8();
    Chip8(const Chip8 &) = delete;
    Chip8 &operator=(const Chip8 &) = delete;

    void load_program(const std::string &file_name);
    void run();
};
} // namespace YAChip8
#endif // CHIP8_H
