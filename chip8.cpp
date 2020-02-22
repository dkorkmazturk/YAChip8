#include "chip8.h"
#include <fstream>
#include <random>
#include <mutex>
#include <atomic>
#include <stdexcept>

namespace YAChip8
{

std::pair<int, int> operator *(const std::pair<int, int>& lhs, const int rhs)
{
    return std::pair<int, int>(lhs.first * rhs, lhs.second * rhs);
}

static std::atomic<bool> runTimerThread(true);
static std::mutex timerMutex;

inline uint16_t Chip8::fetch()
{
    uint16_t opcode = memory[PC];
    opcode <<= 8;
    opcode |= memory[PC + 1];
    PC += 2;

    return opcode;
}

void Chip8::timer()
{
    while (runTimerThread)
    {
        std::this_thread::sleep_for(std::chrono::microseconds(16666));
        {
            std::lock_guard<std::mutex> lckgrd(timerMutex);
            if (DT > 0)
                --DT;
            if (ST > 0)
                --ST;
        }
    }
}

void Chip8::decode_and_execute(const uint16_t opcode)
{
    const uint8_t op = (opcode & 0xF000) >> 12;
    const uint8_t x = (opcode & 0x0F00) >> 8;
    const uint8_t y = (opcode & 0x00F0) >> 4;
    const uint8_t n = opcode & 0x000F;
    const uint8_t kk = opcode & 0x00FF;
    const uint16_t nnn = opcode & 0x0FFF;

    switch (op)
    {
    case 0x0:
        if (nnn == 0x0E0)
        {
            gui.clearDisplay();
        }
        else if (nnn == 0x0EE)
        {
            PC = stack.top();
            stack.pop();
        }
        else
        {
            throw std::runtime_error("Illegal Chip8-8 instruction");
        }
        break;
    case 0x2:
        stack.push(PC);
        [[fallthrough]];
    case 0x1:
        PC = nnn;
        break;
    case 0x3:
        if (V[x] == kk)
            PC += 2;
        break;
    case 0x4:
        if (V[x] != kk)
            PC += 2;
        break;
    case 0x5:
        if (V[x] == V[y])
            PC += 2;
        break;
    case 0x6:
        V[x] = kk;
        break;
    case 0x7:
        V[x] += kk;
        break;
    case 0x8:
        switch (n)
        {
        case 0x0:
            V[x] = V[y];
            break;
        case 0x1:
            V[x] |= V[y];
            break;
        case 0x2:
            V[x] &= V[y];
            break;
        case 0x3:
            V[x] ^= V[y];
            break;
        case 0x4:
            V[0xF] = ((255 - V[y]) < V[x]) ? 1 : 0;
            V[x] += V[y];
            break;
        case 0x5:
            V[0xF] = (V[x] > V[y]) ? 1 : 0;
            V[x] -= V[y];
            break;
        case 0x6:
            V[0xF] = V[x] & 1;
            V[x] >>= 1;
            break;
        case 0x7:
            V[0xF] = (V[y] > V[x]) ? 1 : 0;
            V[x] = V[y] - V[x];
            break;
        case 0xE:
            V[0xF] = (V[x] & 0x80) ? 1 : 0;
            V[x] <<= 1;
            break;
        default:
            throw std::runtime_error("Illegal Chip8-8 instruction");
        }
        break;
    case 0x9:
        if (V[x] != V[y])
            PC += 2;
        break;
    case 0xA:
        I = nnn;
        break;
    case 0xB:
        PC = V[0] + nnn;
        break;
    case 0xC:
    {
        static std::random_device rnddev;
        static std::mt19937 gen(rnddev());
        static std::uniform_int_distribution<uint8_t> unidist(0, 255);

        V[x] = unidist(gen) & kk;
    }
    break;
    case 0xD:
        V[0xF] = gui.display(&memory[I], V[x], V[y], n);
        break;
    case 0xE:
    {
        const auto keyPressed = gui.checkKeyPress(V[x]);
        if (kk == 0x9E)
        {
            if (keyPressed)
                PC += 2;
        }
        else if (kk == 0xA1)
        {
            if (!keyPressed)
                PC += 2;
        }
        else
        {
            throw std::runtime_error("Illegal Chip8-8 instruction");
        }
    }
    break;
    case 0xF:
        switch (kk)
        {
        case 0x07:
        {
            std::lock_guard<std::mutex> lckgrd(timerMutex);
            V[x] = DT;
        }
        break;
        case 0x0A:
            V[x] = gui.waitKeyPress();
            break;
        case 0x15:
        {
            std::lock_guard<std::mutex> lckgrd(timerMutex);
            DT = V[x];
        }
        break;
        case 0x18:
        {
            std::lock_guard<std::mutex> lckgrd(timerMutex);
            ST = V[x];
        }
        break;
        case 0x1E:
            I += V[x];
            break;
        case 0x29:
            I = V[x] * 5;
            break;
        case 0x33:
            // TODO: Make error prone following three opcodes (Overflow)
            {
                uint8_t number = V[x];
                for (uint_fast8_t i = 0; i < 3; i++)
                {
                    memory[I + 2 - i] = number % 10;
                    number /= 10;
                }
            }
            break;

        case 0x55:
            for (uint_fast16_t i = 0; i <= x; i++)
            {
                memory[I + i] = V[i];
            }
            break;
        case 0x65:
            for (uint_fast16_t i = 0; i <= x; i++)
            {
                V[i] = memory[I + i];
            }
            break;
        default:
            throw std::runtime_error("Illegal Chip8-8 instruction");
        }
        break;
    }
}

Chip8::Chip8(const std::pair<const int, const int> &logical_size, const int scale_factor, const std::string &file_name)
    : V({0}), I(0), PC(0x200), DT(0), ST(0), gui(logical_size * scale_factor, logical_size)
{
    memory =
        {
            0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
            0x20, 0x60, 0x20, 0x20, 0x70, // 1
            0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
            0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
            0x90, 0x90, 0xF0, 0x10, 0x10, // 4
            0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
            0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
            0xF0, 0x10, 0x20, 0x40, 0x40, // 7
            0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
            0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
            0xF0, 0x90, 0xF0, 0x90, 0x90, // A
            0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
            0xF0, 0x80, 0x80, 0x80, 0xF0, // C
            0xE0, 0x90, 0x90, 0x90, 0xE0, // D
            0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
            0xF0, 0x80, 0xF0, 0x80, 0x80  // F
        };

    memory.resize(4096);

    if (!file_name.empty())
    {
        load_program(file_name);
    }
}

Chip8::~Chip8()
{
    runTimerThread = false;
    if (timerThread.joinable())
    {
        timerThread.join();
    }
}

void Chip8::load_program(const std::string &file_name)
{
    std::ifstream program_file;
    program_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    program_file.open(file_name, std::ifstream::binary | std::ifstream::ate);

    const auto program_size = program_file.tellg();

    if ((memory.size() > 0x200) && (static_cast<std::size_t>(program_size) > (memory.size() - 0x200)))
    {
        throw std::runtime_error("ROM size is larger than reserved memory");
    }

    program_file.seekg(0);
    program_file.read(reinterpret_cast<char *>(&memory[0x200]), program_size);
    gui.setWindowTitle(file_name);
}

void Chip8::run()
{
    timerThread = std::thread(&Chip8::timer, this);
    while (!gui.exitRequested())
    {
        uint_fast16_t opcode = fetch();
        decode_and_execute(opcode);
        gui.render();
    }

    runTimerThread = false;
    timerThread.join();
}
} // namespace YAChip8