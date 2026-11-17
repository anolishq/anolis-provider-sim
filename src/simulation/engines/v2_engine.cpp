#include "v2_engine.hpp"

#include <format>
#include <fstream>
#include <sstream>
#include <stdexcept>

#include "fluxgraph/v2/load.hpp"
#include "logging/logger.hpp"

namespace sim_engine {

namespace fg = fluxgraph::v2;

void V2Engine::initialize(const std::string &config_path) {
    std::ifstream file(config_path);
    if (!file) {
        throw std::runtime_error(std::format("graph file '{}' cannot be opened", config_path));
    }
    std::ostringstream text;
    text << file.rdbuf();
    auto loaded = fg::load_yaml(text.str());
    if (!loaded) {
        throw std::runtime_error(std::format("graph file '{}': {}", config_path, loaded.error().message));
    }
    auto compiled = fg::Engine::compile(loaded.value());
    if (!compiled) {
        throw std::runtime_error(std::format("graph file '{}' refused: {}", config_path, compiled.error().message));
    }
    engine_ = std::make_unique<fg::Engine>(std::move(compiled.value()));

    // The ticker calls once a period of its own. A graph of another period would run at another speed than
    // the clock on the wall, so it is refused.
    const auto ticker_ns = static_cast<fg::Time>(1e9 / tick_rate_hz_);
    if (engine_->period() != ticker_ns) {
        throw std::runtime_error(std::format("graph file '{}': its period is {} ns and the ticker's {} ns", config_path,
                                             engine_->period(), ticker_ns));
    }

    const fg::Setting *declared = loaded.value().find("inputs");
    if (declared != nullptr && declared->is(fg::Setting::Kind::map)) {
        for (const auto &name : declared->as_map().keys) {
            inputs_.push_back({name, engine_->input(name).value()});
        }
    }
    PSIM_LOG_INFO("V2Engine", "graph " << config_path << " compiled, period " << engine_->period() / 1'000'000
                                       << " ms, " << inputs_.size() << " inputs, " << engine_->outputs().size()
                                       << " outputs");
}

TickResult V2Engine::tick(const std::map<std::string, double> &actuators) {
    for (const auto &input : inputs_) {
        const auto found = actuators.find(input.name);
        if (found == actuators.end()) {
            continue;  // never written: the graph has it `not_ready`, and says so in what it gives
        }
        if (auto written = engine_->write(input.handle, fg::State::good, found->second, now_); !written) {
            PSIM_LOG_WARN("V2Engine", "write of " << input.name << " refused: " << written.error().message);
        }
    }
    auto ran = engine_->tick(now_);
    if (!ran) {
        PSIM_LOG_ERROR("V2Engine", "tick refused: " << ran.error().message);
        return TickResult{false, {}, {}};
    }
    now_ += engine_->period();

    TickResult result{true, {}, {}};
    for (const auto &output : engine_->outputs()) {
        if (fg::usable(output.value.state) && output.value.type == fg::Type::number) {
            result.sensors.emplace(std::string(output.name), output.value.number);
        }
    }
    return result;
}

std::vector<std::string> V2Engine::list_signals() {
    std::vector<std::string> names;
    for (const auto &output : engine_->outputs()) {
        names.emplace_back(output.name);
    }
    return names;
}

}  // namespace sim_engine
