#include "core/runtime.hpp"
#include "gui/gui.hpp"

int main() {
    st2::Runtime runtime;
    if (!runtime.ensure_data_dir()) {
        return 1;
    }

    st2::Gui gui;
    gui.run();
    return 0;
}
