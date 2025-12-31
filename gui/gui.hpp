#pragma once

#include <SDL.h>
#include <SDL_ttf.h>
#include <string>
#include <vector>

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

    // Dropdown state
    bool m_dropdown_open{false};
    int m_selected_option{0};
    std::vector<std::string> m_options;
    std::vector<SDL_Color> m_option_colors;

    // Tabs state
    int m_active_tab{0};  // 0, 1, 2

    // Rich text panel state
    std::vector<std::string> m_logs;

    void render_frame(bool button_hovered, int mouse_x, int mouse_y);
    void render_button(bool hovered);
    void render_dropdown(int mouse_x, int mouse_y);
    void render_tabs(int mouse_x, int mouse_y);
    void render_tab_content(int mouse_x, int mouse_y);
    void render_rich_text();
    SDL_Texture* render_text(const std::string& text, SDL_Color color);
    bool point_in_rect(int x, int y, const SDL_Rect& r) const;
};

}  // namespace st2
