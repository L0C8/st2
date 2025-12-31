#include "core/runtime.hpp"

#include <filesystem>
#include <iostream>

namespace st2 {

Runtime::Runtime() : m_data_dir("data") {}

Runtime::Runtime(std::filesystem::path data_dir) : m_data_dir(std::move(data_dir)) {}

bool Runtime::ensure_data_dir() const {
    std::error_code ec;
    if (std::filesystem::exists(m_data_dir, ec)) {
        return true;
    }
    std::filesystem::create_directories(m_data_dir, ec);
    if (ec) {
        std::cerr << "Failed to create data directory at " << m_data_dir << ": " << ec.message() << "\n";
        return false;
    }
    return true;
}

}  // namespace st2
