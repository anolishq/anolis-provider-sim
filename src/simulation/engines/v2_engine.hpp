#pragma once

/**
 * @file v2_engine.hpp
 * @brief A spike: the version 2 core of FluxGraph in this process.
 *
 * Throwaway. One engine, one provider: nothing is shared with another
 * provider as the 1.x server's graph is.
 */

#include <memory>
#include <vector>

#include "fluxgraph/v2/engine.hpp"
#include "simulation/simulation_engine.hpp"

namespace sim_engine {

class V2Engine : public SimulationEngine {
public:
    explicit V2Engine(double tick_rate_hz) : tick_rate_hz_(tick_rate_hz) {}

    void initialize(const std::string &config_path) override;
    void register_devices(const std::vector<std::string> &) override {}
    TickResult tick(const std::map<std::string, double> &actuators) override;
    std::vector<std::string> list_signals() override;

private:
    struct Input {
        std::string name;
        fluxgraph::v2::InputHandle handle;
    };

    double tick_rate_hz_;
    std::unique_ptr<fluxgraph::v2::Engine> engine_;
    std::vector<Input> inputs_;
    fluxgraph::v2::Time now_ = 0;  // the time of the engine: a period a tick, whatever the clock on the wall did
};

}  // namespace sim_engine
