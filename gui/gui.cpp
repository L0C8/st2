#include "gui.hpp"

#include <filesystem>
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
constexpr SDL_Rect kRichRect{340, 130, 280, 320};

constexpr SDL_Color kTextColor{230, 230, 240, 255};
constexpr SDL_Color kTextShadow{30, 30, 40, 100};
constexpr SDL_Color kPanelBg{18, 18, 24, 255};
constexpr SDL_Color kPanelBorder{35, 35, 45, 255};
constexpr SDL_Color kTabButtonColor{70, 160, 230, 255};
}  // namespace

Gui::Gui() = default;

Gui::~Gui() {
    if (m_font) {
        TTF_CloseFont(m_font);
    }
    if (m_renderer) {
        SDL_DestroyRenderer(m_renderer);
    }
    if (m_window) {
        SDL_DestroyWindow(m_window);
    }
    TTF_Quit();
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

    if (TTF_Init() != 0) {
        std::cerr << "TTF_Init failed: " << TTF_GetError() << "\n";
        return false;
    }

    // Load a default font; adjust path if your system differs.
    const std::vector<std::filesystem::path> font_candidates = {
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf"};
    for (const auto& path : font_candidates) {
        if (std::filesystem::exists(path)) {
            m_font = TTF_OpenFont(path.c_str(), 16);
            if (m_font) break;
        }
    }
    if (!m_font) {
        std::cerr << "Failed to load a TTF font; checked default candidates.\n";
        return false;
    }

    // Example dropdown options with colors.
    m_options = {"Option A", "Option B", "Option C", "Option D"};
    m_option_colors = {
        SDL_Color{200, 80, 80, 255},   // red-ish
        SDL_Color{80, 200, 120, 255},  // green-ish
        SDL_Color{80, 120, 220, 255},  // blue-ish
        SDL_Color{230, 200, 90, 255}   // yellow-ish
    };

    m_logs = {
        "test test test test test"
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
    const SDL_Color sel = m_option_colors.empty() ? SDL_Color{120, 120, 120, 255} : m_option_colors[m_selected_option];
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
        const SDL_Color c = m_option_colors[i % m_option_colors.size()];
        SDL_SetRenderDrawColor(m_renderer, c.r, c.g, c.b, 255);
        SDL_RenderFillRect(m_renderer, &item_rect);
        SDL_SetRenderDrawColor(m_renderer, hovered ? 10 : 20, 10, 30, 255);
        SDL_RenderDrawRect(m_renderer, &item_rect);

        SDL_Texture* text_tex = render_text(m_options[i], kTextColor);
        if (text_tex) {
            int w, h;
            SDL_QueryTexture(text_tex, nullptr, nullptr, &w, &h);
            SDL_Rect dst{item_rect.x + 8, item_rect.y + (item_rect.h - h) / 2, w, h};
            SDL_RenderCopy(m_renderer, text_tex, nullptr, &dst);
            SDL_DestroyTexture(text_tex);
        }
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

SDL_Texture* Gui::render_text(const std::string& text, SDL_Color color) {
    SDL_Surface* surface = TTF_RenderUTF8_Blended(m_font, text.c_str(), color);
    if (!surface) {
        std::cerr << "TTF_RenderUTF8_Blended failed: " << TTF_GetError() << "\n";
        return nullptr;
    }
    SDL_Texture* tex = SDL_CreateTextureFromSurface(m_renderer, surface);
    SDL_FreeSurface(surface);
    if (!tex) {
        std::cerr << "SDL_CreateTextureFromSurface failed: " << SDL_GetError() << "\n";
    }
    return tex;
}

void Gui::render_rich_text() {
    // Panel background
    SDL_SetRenderDrawColor(m_renderer, kPanelBg.r, kPanelBg.g, kPanelBg.b, kPanelBg.a);
    SDL_RenderFillRect(m_renderer, &kRichRect);
    SDL_SetRenderDrawColor(m_renderer, kPanelBorder.r, kPanelBorder.g, kPanelBorder.b, kPanelBorder.a);
    SDL_RenderDrawRect(m_renderer, &kRichRect);

    int y = kRichRect.y + 8;
    for (const auto& line : m_logs) {
        SDL_Texture* tex_shadow = render_text(line, kTextShadow);
        SDL_Texture* tex = render_text(line, kTextColor);
        if (tex && tex_shadow) {
            int w, h;
            SDL_QueryTexture(tex, nullptr, nullptr, &w, &h);
            SDL_Rect shadow_dst{kRichRect.x + 9, y + 1, w, h};
            SDL_Rect dst{kRichRect.x + 8, y, w, h};
            SDL_RenderCopy(m_renderer, tex_shadow, nullptr, &shadow_dst);
            SDL_RenderCopy(m_renderer, tex, nullptr, &dst);
            SDL_DestroyTexture(tex_shadow);
            SDL_DestroyTexture(tex);
            y += h + 4;
            if (y > kRichRect.y + kRichRect.h - 20) {
                break;  // stop if out of space
            }
        } else {
            if (tex_shadow) SDL_DestroyTexture(tex_shadow);
            if (tex) SDL_DestroyTexture(tex);
        }
    }
}

void Gui::render_frame(bool button_hovered, int mouse_x, int mouse_y) {
    // Background
    SDL_SetRenderDrawColor(m_renderer, 30, 30, 40, 255);
    SDL_RenderClear(m_renderer);

    render_button(button_hovered);
    render_dropdown(mouse_x, mouse_y);
    render_tabs(mouse_x, mouse_y);
    render_tab_content(mouse_x, mouse_y);
    render_rich_text();

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
