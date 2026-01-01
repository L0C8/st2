#include "core/calc.hpp"

#include <numeric>

namespace st2 {

std::optional<double> relative_strength_index(const std::vector<double>& closes, std::size_t period) {
    if (period == 0 || closes.size() < period + 1) {
        return std::nullopt;
    }

    const std::size_t diff_count = closes.size() - 1;
    std::vector<double> gains(diff_count);
    std::vector<double> losses(diff_count);

    for (std::size_t i = 1; i < closes.size(); ++i) {
        const double delta = closes[i] - closes[i - 1];
        if (delta > 0) {
            gains[i - 1] = delta;
            losses[i - 1] = 0.0;
        } else {
            gains[i - 1] = 0.0;
            losses[i - 1] = -delta;
        }
    }

    auto sum_range = [](const std::vector<double>& values, std::size_t count) {
        return std::accumulate(values.begin(),
                               values.begin() + static_cast<std::ptrdiff_t>(count),
                               0.0);
    };

    double avg_gain = sum_range(gains, period) / static_cast<double>(period);
    double avg_loss = sum_range(losses, period) / static_cast<double>(period);

    for (std::size_t i = period; i < gains.size(); ++i) {
        avg_gain = ((avg_gain * (period - 1)) + gains[i]) / static_cast<double>(period);
        avg_loss = ((avg_loss * (period - 1)) + losses[i]) / static_cast<double>(period);
    }

    if (avg_loss == 0.0) {
        return 100.0;
    }

    const double rs = avg_gain / avg_loss;
    return 100.0 - (100.0 / (1.0 + rs));
}

}  // namespace st2
