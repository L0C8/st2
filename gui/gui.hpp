#pragma once

#include <SDL.h>
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
    bool m_running{true};

    // Dropdown state
    bool m_dropdown_open{false};
    int m_selected_option{0};
    std::vector<SDL_Color> m_options;

    // Tabs state
    int m_active_tab{0};  // 0, 1, 2

    void render_frame(bool button_hovered, int mouse_x, int mouse_y);
    void render_button(bool hovered);
    void render_dropdown(int mouse_x, int mouse_y);
    void render_tabs(int mouse_x, int mouse_y);
    void render_tab_content(int mouse_x, int mouse_y);
    bool point_in_rect(int x, int y, const SDL_Rect& r) const;
};

}  // namespace st2
