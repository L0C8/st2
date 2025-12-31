#include "gui.hpp"

#include <array>
#include <cstdio>
#include <cctype>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <vector>

namespace st2 {

namespace {
constexpr int kWidth = 640;
constexpr int kHeight = 480;
constexpr SDL_Rect kPanelRect{20, 20, 600, 440};
constexpr SDL_Rect kTextboxRect{40, 60, 400, 40};
constexpr SDL_Rect kButtonRect{460, 60, 120, 40};
constexpr SDL_Rect kOptionsButtonRect{460, 120, 120, 40};

constexpr SDL_Color kTextColor{230, 230, 240, 255};
constexpr SDL_Color kTextShadow{30, 30, 40, 100};
constexpr SDL_Color kPanelBg{18, 18, 24, 255};
constexpr SDL_Color kPanelBorder{35, 35, 45, 255};
constexpr SDL_Color kTextboxBg{28, 28, 36, 255};
constexpr SDL_Color kTextboxBorder{60, 60, 80, 255};
constexpr SDL_Color kButtonColor{70, 160, 230, 255};
}  // namespace

namespace {
std::string sanitize_symbol(const std::string& raw) {
    std::string out;
    out.reserve(raw.size());
    for (char c : raw) {
        if (std::isalnum(static_cast<unsigned char>(c)) || c == '.' || c == '-' || c == '_') {
            out.push_back(c);
        }
    }
    return out;
}

std::string run_fetch(const std::string& symbol) {
    if (symbol.empty()) {
        return "No symbol provided";
    }
    std::string cmd = "python3 py/yfclient/fetch.py " + symbol;
    std::array<char, 256> buffer{};
    std::string result;
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) {
        return "Failed to start Python fetch";
    }
    while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe) != nullptr) {
        result += buffer.data();
    }
    int rc = pclose(pipe);
    if (rc != 0 && result.empty()) {
        std::ostringstream oss;
        oss << "Python fetch exited with code " << rc;
        return oss.str();
    }
    return result;
}

std::string run_options_fetch(const std::string& symbol) {
    if (symbol.empty()) {
        return "No symbol provided";
    }
    std::string cmd = "python3 py/options/cli_fetch.py " + symbol;
    std::array<char, 256> buffer{};
    std::string result;
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) {
        return "Failed to start Python options fetch";
    }
    while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe) != nullptr) {
        result += buffer.data();
    }
    int rc = pclose(pipe);
    if (rc != 0 && result.empty()) {
        std::ostringstream oss;
        oss << "Python options fetch exited with code " << rc;
        return oss.str();
    }
    return result;
}
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

    SDL_StartTextInput();
    m_input_text = "";
    return true;
}

bool Gui::point_in_rect(int x, int y, const SDL_Rect& r) const {
    return x >= r.x && x < r.x + r.w && y >= r.y && y < r.y + r.h;
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

void Gui::render_button(const SDL_Rect& rect, const std::string& label, bool hovered) {
    SDL_SetRenderDrawColor(m_renderer,
                           hovered ? 100 : kButtonColor.r,
                           hovered ? 180 : kButtonColor.g,
                           hovered ? 240 : kButtonColor.b,
                           255);
    SDL_RenderFillRect(m_renderer, &rect);
    SDL_SetRenderDrawColor(m_renderer, 20, 20, 30, 255);
    SDL_RenderDrawRect(m_renderer, &rect);

    SDL_Texture* tex = render_text(label, kTextColor);
    if (tex) {
        int w, h;
        SDL_QueryTexture(tex, nullptr, nullptr, &w, &h);
        SDL_Rect dst{rect.x + (rect.w - w) / 2, rect.y + (rect.h - h) / 2, w, h};
        SDL_RenderCopy(m_renderer, tex, nullptr, &dst);
        SDL_DestroyTexture(tex);
    }
}

void Gui::render_textbox(bool hovered) {
    SDL_SetRenderDrawColor(m_renderer, kTextboxBg.r, kTextboxBg.g, kTextboxBg.b, kTextboxBg.a);
    SDL_RenderFillRect(m_renderer, &kTextboxRect);
    SDL_SetRenderDrawColor(m_renderer,
                           hovered || m_textbox_focused ? 120 : kTextboxBorder.r,
                           hovered || m_textbox_focused ? 140 : kTextboxBorder.g,
                           hovered || m_textbox_focused ? 200 : kTextboxBorder.b,
                           255);
    SDL_RenderDrawRect(m_renderer, &kTextboxRect);

    const std::string& text = m_input_text.empty() ? std::string("(enter text)") : m_input_text;
    SDL_Color color = m_input_text.empty() ? SDL_Color{160, 160, 170, 255} : kTextColor;
    SDL_Texture* tex_shadow = render_text(text, kTextShadow);
    SDL_Texture* tex = render_text(text, color);
    if (tex && tex_shadow) {
        int w, h;
        SDL_QueryTexture(tex, nullptr, nullptr, &w, &h);
        SDL_Rect shadow_dst{kTextboxRect.x + 9, kTextboxRect.y + (kTextboxRect.h - h) / 2 + 1, w, h};
        SDL_Rect dst{kTextboxRect.x + 8, kTextboxRect.y + (kTextboxRect.h - h) / 2, w, h};
        SDL_RenderCopy(m_renderer, tex_shadow, nullptr, &shadow_dst);
        SDL_RenderCopy(m_renderer, tex, nullptr, &dst);
    }
    if (tex_shadow) SDL_DestroyTexture(tex_shadow);
    if (tex) SDL_DestroyTexture(tex);
}

void Gui::render_frame(bool fetch_hovered, bool opts_hovered, int mouse_x, int mouse_y) {
    SDL_SetRenderDrawColor(m_renderer, 30, 30, 40, 255);
    SDL_RenderClear(m_renderer);

    SDL_SetRenderDrawColor(m_renderer, kPanelBg.r, kPanelBg.g, kPanelBg.b, kPanelBg.a);
    SDL_RenderFillRect(m_renderer, &kPanelRect);
    SDL_SetRenderDrawColor(m_renderer, kPanelBorder.r, kPanelBorder.g, kPanelBorder.b, kPanelBorder.a);
    SDL_RenderDrawRect(m_renderer, &kPanelRect);

    bool textbox_hovered = point_in_rect(mouse_x, mouse_y, kTextboxRect);
    render_textbox(textbox_hovered);
    render_button(kButtonRect, "Print", fetch_hovered);
    render_button(kOptionsButtonRect, "Options", opts_hovered);

    SDL_RenderPresent(m_renderer);
}

void Gui::run() {
    if (!init()) {
        return;
    }

    SDL_Event event;
    while (m_running) {
        bool fetch_hovered = false;
        bool options_hovered = false;
        int mouse_x = -1;
        int mouse_y = -1;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                m_running = false;
            } else if (event.type == SDL_MOUSEMOTION) {
                mouse_x = event.motion.x;
                mouse_y = event.motion.y;
                fetch_hovered = point_in_rect(mouse_x, mouse_y, kButtonRect);
                options_hovered = point_in_rect(mouse_x, mouse_y, kOptionsButtonRect);
            } else if (event.type == SDL_MOUSEBUTTONDOWN) {
                SDL_Point p{event.button.x, event.button.y};
                mouse_x = p.x;
                mouse_y = p.y;
                if (point_in_rect(mouse_x, mouse_y, kButtonRect)) {
                    const std::string symbol = sanitize_symbol(m_input_text);
                    std::cout << "Textbox content: \"" << m_input_text << "\"\n";
                    std::cout << "Sanitized symbol: \"" << symbol << "\"\n";
                    std::cout << "Python fetch output:\n" << run_fetch(symbol) << std::flush;
                } else if (point_in_rect(mouse_x, mouse_y, kOptionsButtonRect)) {
                    const std::string symbol = sanitize_symbol(m_input_text);
                    std::cout << "Options fetch for: \"" << symbol << "\"\n";
                    std::cout << "Python options output:\n" << run_options_fetch(symbol) << std::flush;
                }
                m_textbox_focused = point_in_rect(mouse_x, mouse_y, kTextboxRect);
            } else if (event.type == SDL_TEXTINPUT) {
                if (m_textbox_focused) {
                    m_input_text += event.text.text;
                }
            } else if (event.type == SDL_KEYDOWN) {
                if (m_textbox_focused && event.key.keysym.sym == SDLK_BACKSPACE) {
                    if (!m_input_text.empty()) {
                        m_input_text.pop_back();
                    }
                }
            }
        }

        render_frame(fetch_hovered, options_hovered, mouse_x, mouse_y);
    }
}

}  // namespace st2
