#ifndef GUI_H
#define GUI_H

#include <vector>
#include <string>
#include <cinttypes>
#include <SDL2/SDL.h>

namespace YAChip8
{
class GUI
{
private:
    const int texWidth;
    const int texHeight;

    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;

    std::vector<uint8_t> vram;

public:
    GUI(const std::pair<const int, const int> &screen_size, const std::pair<const int, const int> &logical_size, const std::string &title = "");
    ~GUI();
    GUI(const GUI &) = delete;
    GUI &operator=(const GUI &) = delete;

    void setWindowTitle(const std::string &title);
    uint8_t display(const uint8_t *const sprite, const uint8_t x, const uint8_t y, const uint8_t n);
    void clearDisplay();
    bool exitRequested() const;
    uint8_t waitKeyPress() const;
    bool checkKeyPress(const uint8_t key) const;
    void render();
};
} // namespace YAChip8
#endif // GUI_H
