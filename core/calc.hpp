#pragma once

#include <cstddef>
#include <optional>
#include <vector>

namespace st2 {

// Compute the Relative Strength Index (RSI) over the provided closing prices.
// Returns std::nullopt when there is not enough data to compute the metric.
std::optional<double> relative_strength_index(const std::vector<double>& closes, std::size_t period = 14);

}  // namespace st2
