#pragma once

#include <SDL.h>
#include <SDL_ttf.h>
#include <string>

namespace st2 {

// Simple SDL2-based GUI placeholder sized to 640x480.
class Gui {
public:
    Gui();
    ~Gui();

    // Initialize SDL, create a window, and run the loop.
    bool init();
    void run();

private:
    SDL_Window* m_window{nullptr};
    SDL_Renderer* m_renderer{nullptr};
    TTF_Font* m_font{nullptr};
    bool m_running{true};

    std::string m_input_text;
    bool m_textbox_focused{false};

    void render_frame(bool fetch_hovered, bool opts_hovered, int mouse_x, int mouse_y);
    void render_textbox(bool hovered);
    void render_button(const SDL_Rect& rect, const std::string& label, bool hovered);
    SDL_Texture* render_text(const std::string& text, SDL_Color color);
    bool point_in_rect(int x, int y, const SDL_Rect& r) const;
};

}  // namespace st2
