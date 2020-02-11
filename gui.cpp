#include "gui.h"
#include <algorithm>
#include <bitset>
#include <iostream>

namespace YAChip8
{
static inline std::bitset<8> bitsetReverse(const std::bitset<8> &bitset)
{
    auto str = bitset.to_string();
    std::reverse(str.begin(), str.end());
    return std::bitset<8>(str);
}

GUI::GUI(const int x, const int y, const std::string &title) : vram(texWidth * texHeight, 0)
{
    if (SDL_Init(SDL_INIT_VIDEO))
    {
        throw std::runtime_error("Unable to initialize SDL video subsystem: " + std::string(SDL_GetError()));
    }

    window = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, x, y, SDL_WINDOW_RESIZABLE | SDL_WINDOW_INPUT_FOCUS);
    if (window == nullptr)
    {
        throw std::runtime_error("Unable to create an SDL window: " + std::string(SDL_GetError()));
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == nullptr)
    {
        throw std::runtime_error("Unable to create an SDL renderer: " + std::string(SDL_GetError()));
    }

    if (SDL_RenderSetLogicalSize(renderer, texWidth, texHeight))
    {
        throw std::runtime_error("Unable to set logical renderer size: " + std::string(SDL_GetError()));
    }

    if (SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0))
    {
        throw std::runtime_error("Unable to set render draw color: " + std::string(SDL_GetError()));
    }

    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB332, SDL_TEXTUREACCESS_STREAMING, texWidth, texHeight);
    if (texture == nullptr)
    {
        throw std::runtime_error("Unable to create an SDL texture: " + std::string(SDL_GetError()));
    }
}

GUI::~GUI()
{
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void GUI::setWindowTitle(const std::string &title)
{
    SDL_SetWindowTitle(window, title.c_str());
}

uint8_t GUI::display(const uint8_t *const sprite, const uint8_t x, const uint8_t y, const uint8_t n)
{
    auto result = 0;

    for (uint8_t line = 0; line < n; ++line)
    {
        auto bits = bitsetReverse(std::bitset<8>(*(sprite + line)));
        for (size_t bit = 0; bit < bits.size(); ++bit)
        {
            size_t pixelIndex = (texWidth * ((y + line) % texHeight)) + ((x + bit) % texWidth);
            if (bits[bit] == 1)
            {
                if (vram[pixelIndex] != 0)
                {
                    vram[pixelIndex] = 0;
                    result = 1;
                }
                else
                {
                    vram[pixelIndex] = 255;
                }
            }
        }
    }

    SDL_UpdateTexture(texture, nullptr, vram.data(), texWidth);

    return result;
}

void GUI::clearDisplay()
{
    std::fill(vram.begin(), vram.end(), 0);
    SDL_UpdateTexture(texture, nullptr, vram.data(), texWidth);
}

bool GUI::exitRequested() const
{
    SDL_Event event;
    SDL_PollEvent(&event);
    return event.type == SDL_QUIT;
}

uint8_t GUI::waitKeyPress() const
{
    SDL_Event event;
    for (;;)
    {
        SDL_WaitEvent(&event);
        if (event.type == SDL_KEYDOWN)
        {
            switch (event.key.keysym.sym)
            {
            case SDL_SCANCODE_KP_0:
            case SDL_SCANCODE_0:
                return 0x0;
            case SDL_SCANCODE_KP_1:
            case SDL_SCANCODE_1:
                return 0x1;
            case SDL_SCANCODE_KP_2:
            case SDL_SCANCODE_2:
                return 0x2;
            case SDL_SCANCODE_KP_3:
            case SDL_SCANCODE_3:
                return 0x3;
            case SDL_SCANCODE_KP_4:
            case SDL_SCANCODE_4:
                return 0x4;
            case SDL_SCANCODE_KP_5:
            case SDL_SCANCODE_5:
                return 0x5;
            case SDL_SCANCODE_KP_6:
            case SDL_SCANCODE_6:
                return 0x6;
            case SDL_SCANCODE_KP_7:
            case SDL_SCANCODE_7:
                return 0x7;
            case SDL_SCANCODE_KP_8:
            case SDL_SCANCODE_8:
                return 0x8;
            case SDL_SCANCODE_KP_9:
            case SDL_SCANCODE_9:
                return 0x9;
            case SDL_SCANCODE_A:
                return 0xA;
            case SDL_SCANCODE_B:
                return 0xB;
            case SDL_SCANCODE_C:
                return 0xC;
            case SDL_SCANCODE_D:
                return 0xD;
            case SDL_SCANCODE_E:
                return 0xE;
            case SDL_SCANCODE_F:
                return 0xF;
            default:
                continue;
            }
        }
        else if (event.type == SDL_QUIT)
        {
            SDL_PushEvent(&event);
            return 0;
        }
    }
}

bool GUI::checkKeyPress(const uint8_t key) const
{
    static const Uint8 *const keyboardState = SDL_GetKeyboardState(nullptr);
    SDL_PumpEvents();

    switch (key)
    {
    case 0x0:
        return (keyboardState[SDL_SCANCODE_KP_0] || keyboardState[SDL_SCANCODE_0]);
    case 0x1:
        return (keyboardState[SDL_SCANCODE_KP_1] || keyboardState[SDL_SCANCODE_1]);
    case 0x2:
        return (keyboardState[SDL_SCANCODE_KP_2] || keyboardState[SDL_SCANCODE_2]);
    case 0x3:
        return (keyboardState[SDL_SCANCODE_KP_3] || keyboardState[SDL_SCANCODE_3]);
    case 0x4:
        return (keyboardState[SDL_SCANCODE_KP_4] || keyboardState[SDL_SCANCODE_4]);
    case 0x5:
        return (keyboardState[SDL_SCANCODE_KP_5] || keyboardState[SDL_SCANCODE_5]);
    case 0x6:
        return (keyboardState[SDL_SCANCODE_KP_6] || keyboardState[SDL_SCANCODE_6]);
    case 0x7:
        return (keyboardState[SDL_SCANCODE_KP_7] || keyboardState[SDL_SCANCODE_7]);
    case 0x8:
        return (keyboardState[SDL_SCANCODE_KP_8] || keyboardState[SDL_SCANCODE_8]);
    case 0x9:
        return (keyboardState[SDL_SCANCODE_KP_9] || keyboardState[SDL_SCANCODE_9]);
    case 0xA:
        return keyboardState[SDL_SCANCODE_A];
    case 0xB:
        return keyboardState[SDL_SCANCODE_B];
    case 0xC:
        return keyboardState[SDL_SCANCODE_C];
    case 0xD:
        return keyboardState[SDL_SCANCODE_D];
    case 0xE:
        return keyboardState[SDL_SCANCODE_E];
    case 0xF:
        return keyboardState[SDL_SCANCODE_F];
    default:
        return false;
    }
}

void GUI::render()
{
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, nullptr, nullptr);
    SDL_RenderPresent(renderer);
}
} // namespace YAChip8