#pragma once

#include <filesystem>

namespace st2 {

// Lightweight runtime helper for data/log setup.
class Runtime {
public:
    Runtime();
    explicit Runtime(std::filesystem::path data_dir);

    // Ensure the data directory exists; create it if missing.
    // Returns true on success, false if the directory could not be created.
    bool ensure_data_dir() const;

    const std::filesystem::path& data_dir() const { return m_data_dir; }

private:
    std::filesystem::path m_data_dir;
};

}  // namespace st2
