#include "gui.hpp"

#include <iostream>
#include <vector>

namespace st2 {

namespace {
constexpr int kWidth = 640;
constexpr int kHeight = 480;
// Simple button rectangle.
constexpr SDL_Rect kButtonRect{20, 20, 140, 40};
constexpr SDL_Rect kDropdownRect{180, 20, 160, 40};
constexpr int kDropdownItemHeight = 28;
constexpr SDL_Rect kTabsRect{20, 80, 600, 40};
constexpr SDL_Rect kContentRect{20, 130, 600, 320};
constexpr int kTabWidth = 80;
}  // namespace

Gui::Gui() = default;

Gui::~Gui() {
    if (m_renderer) {
        SDL_DestroyRenderer(m_renderer);
    }
    if (m_window) {
        SDL_DestroyWindow(m_window);
    }
    SDL_Quit();
}

bool Gui::init() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << "\n";
        return false;
    }

    m_window = SDL_CreateWindow(
        "st2 GUI", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, kWidth, kHeight, SDL_WINDOW_SHOWN);
    if (!m_window) {
        std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << "\n";
        return false;
    }

    m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!m_renderer) {
        std::cerr << "SDL_CreateRenderer failed: " << SDL_GetError() << "\n";
        return false;
    }

    // Example dropdown options (use distinct colors as stand-ins for labels).
    m_options = {
        SDL_Color{200, 80, 80, 255},   // red-ish
        SDL_Color{80, 200, 120, 255},  // green-ish
        SDL_Color{80, 120, 220, 255},  // blue-ish
        SDL_Color{230, 200, 90, 255}   // yellow-ish
    };
    return true;
}

bool Gui::point_in_rect(int x, int y, const SDL_Rect& r) const {
    return x >= r.x && x < r.x + r.w && y >= r.y && y < r.y + r.h;
}

void Gui::render_button(bool hovered) {
    SDL_SetRenderDrawColor(m_renderer, hovered ? 90 : 60, 140, 220, 255);
    SDL_RenderFillRect(m_renderer, &kButtonRect);
    SDL_SetRenderDrawColor(m_renderer, 20, 20, 30, 255);
    SDL_RenderDrawRect(m_renderer, &kButtonRect);
}

void Gui::render_dropdown(int mouse_x, int mouse_y) {
    // Draw the closed box with the selected color.
    const SDL_Color sel = m_options.empty() ? SDL_Color{120, 120, 120, 255} : m_options[m_selected_option];
    SDL_SetRenderDrawColor(m_renderer, sel.r, sel.g, sel.b, 255);
    SDL_RenderFillRect(m_renderer, &kDropdownRect);
    SDL_SetRenderDrawColor(m_renderer, 20, 20, 30, 255);
    SDL_RenderDrawRect(m_renderer, &kDropdownRect);

    if (!m_dropdown_open || m_options.empty()) {
        return;
    }

    // Dropdown list
    SDL_Rect item_rect = kDropdownRect;
    item_rect.y += kDropdownRect.h;
    item_rect.h = kDropdownItemHeight;
    for (size_t i = 0; i < m_options.size(); ++i) {
        item_rect.y = kDropdownRect.y + kDropdownRect.h + static_cast<int>(i) * kDropdownItemHeight;
        bool hovered = point_in_rect(mouse_x, mouse_y, item_rect);
        const SDL_Color c = m_options[i];
        SDL_SetRenderDrawColor(m_renderer, c.r, c.g, c.b, 255);
        SDL_RenderFillRect(m_renderer, &item_rect);
        SDL_SetRenderDrawColor(m_renderer, hovered ? 10 : 20, 10, 30, 255);
        SDL_RenderDrawRect(m_renderer, &item_rect);
    }
}

void Gui::render_tabs(int mouse_x, int mouse_y) {
    // Tab bar background
    SDL_SetRenderDrawColor(m_renderer, 25, 25, 35, 255);
    SDL_RenderFillRect(m_renderer, &kTabsRect);
    SDL_SetRenderDrawColor(m_renderer, 45, 45, 60, 255);
    SDL_RenderDrawRect(m_renderer, &kTabsRect);

    for (int i = 0; i < 3; ++i) {
        SDL_Rect tab{kTabsRect.x + i * kTabWidth, kTabsRect.y, kTabWidth, kTabsRect.h};
        bool active = (i == m_active_tab);
        bool hovered = point_in_rect(mouse_x, mouse_y, tab);
        SDL_SetRenderDrawColor(m_renderer,
                               active ? 90 : (hovered ? 70 : 55),
                               active ? 140 : (hovered ? 110 : 85),
                               active ? 210 : (hovered ? 160 : 130),
                               255);
        SDL_RenderFillRect(m_renderer, &tab);
        SDL_SetRenderDrawColor(m_renderer, 20, 20, 30, 255);
        SDL_RenderDrawRect(m_renderer, &tab);
    }
}

void Gui::render_tab_content(int mouse_x, int mouse_y) {
    // Content background
    SDL_SetRenderDrawColor(m_renderer, 18, 18, 24, 255);
    SDL_RenderFillRect(m_renderer, &kContentRect);
    SDL_SetRenderDrawColor(m_renderer, 35, 35, 45, 255);
    SDL_RenderDrawRect(m_renderer, &kContentRect);

    // Single button per tab; position changes slightly by tab index.
    SDL_Rect btn{kContentRect.x + 20, kContentRect.y + 20, 160, 40};
    btn.x += m_active_tab * 30;
    btn.y += m_active_tab * 10;

    bool hovered = point_in_rect(mouse_x, mouse_y, btn);
    SDL_SetRenderDrawColor(m_renderer, hovered ? 100 : 70, 160, 230, 255);
    SDL_RenderFillRect(m_renderer, &btn);
    SDL_SetRenderDrawColor(m_renderer, 20, 20, 30, 255);
    SDL_RenderDrawRect(m_renderer, &btn);
}

void Gui::render_frame(bool button_hovered, int mouse_x, int mouse_y) {
    // Background
    SDL_SetRenderDrawColor(m_renderer, 30, 30, 40, 255);
    SDL_RenderClear(m_renderer);

    render_button(button_hovered);
    render_dropdown(mouse_x, mouse_y);
    render_tabs(mouse_x, mouse_y);
    render_tab_content(mouse_x, mouse_y);

    SDL_RenderPresent(m_renderer);
}

void Gui::run() {
    if (!init()) {
        return;
    }

    SDL_Event event;
    while (m_running) {
        bool button_hovered = false;
        int mouse_x = -1;
        int mouse_y = -1;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                m_running = false;
            } else if (event.type == SDL_MOUSEMOTION) {
                mouse_x = event.motion.x;
                mouse_y = event.motion.y;
                button_hovered = point_in_rect(mouse_x, mouse_y, kButtonRect);
            } else if (event.type == SDL_MOUSEBUTTONDOWN) {
                SDL_Point p{event.button.x, event.button.y};
                mouse_x = p.x;
                mouse_y = p.y;
                if (SDL_PointInRect(&p, &kButtonRect)) {
                    std::cout << "Fetch clicked (hook API call here)\n";
                }
                // Toggle dropdown
                if (point_in_rect(mouse_x, mouse_y, kDropdownRect)) {
                    m_dropdown_open = !m_dropdown_open;
                } else if (m_dropdown_open) {
                    // Check option clicks
                    SDL_Rect item_rect = kDropdownRect;
                    item_rect.y += kDropdownRect.h;
                    item_rect.h = kDropdownItemHeight;
                    for (size_t i = 0; i < m_options.size(); ++i) {
                        item_rect.y = kDropdownRect.y + kDropdownRect.h + static_cast<int>(i) * kDropdownItemHeight;
                        if (point_in_rect(mouse_x, mouse_y, item_rect)) {
                            m_selected_option = static_cast<int>(i);
                            std::cout << "Selected option " << i << "\n";
                        }
                    }
                    m_dropdown_open = false;
                } else {
                    // Tabs click
                    for (int i = 0; i < 3; ++i) {
                        SDL_Rect tab{kTabsRect.x + i * kTabWidth, kTabsRect.y, kTabWidth, kTabsRect.h};
                        if (point_in_rect(mouse_x, mouse_y, tab)) {
                            m_active_tab = i;
                            std::cout << "Switched to tab " << (i + 1) << "\n";
                        }
                    }
                    // Tab button click (changes position per tab)
                    SDL_Rect btn{kContentRect.x + 20 + m_active_tab * 30,
                                 kContentRect.y + 20 + m_active_tab * 10,
                                 160,
                                 40};
                    if (point_in_rect(mouse_x, mouse_y, btn)) {
                        std::cout << "Tab " << (m_active_tab + 1) << " button clicked\n";
                    }
                }
            }
        }

        render_frame(button_hovered, mouse_x, mouse_y);
    }
}

}  // namespace st2
