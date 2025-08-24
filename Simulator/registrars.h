// registrars.hpp  (Simulator project)
#pragma once
#include <functional>
#include <memory>
#include <string>
#include <vector>
#include <cassert>
#include "../common/ActionRequest.h"
#include "../common/BattleInfo.h"
#include "../common/SatelliteView.h"
#include "../common/TankAlgorithm.h"
#include "../common/Player.h"
#include "../common/GameResult.h"
#include "../common/AbstractGameManager.h"
#include <stdexcept>

// ----- Algorithm registrar: couples PlayerFactory + TankAlgorithmFactory for one .so -----
class AlgorithmRegistrar {
    public:
        class AlgorithmAndPlayerFactories {
            std::string so_name_;
            TankAlgorithmFactory tankFactory_;
            PlayerFactory playerFactory_;
        public:
            explicit AlgorithmAndPlayerFactories(std::string name): so_name_(std::move(name)) {}
            const std::string& name() const { return so_name_; }
            void setTankFactory(TankAlgorithmFactory&& f) { assert(!tankFactory_); tankFactory_ = std::move(f); }
            void setPlayerFactory(PlayerFactory&& f)      { assert(!playerFactory_); playerFactory_ = std::move(f); }
            bool hasTank()   const { return (bool)tankFactory_; }
            bool hasPlayer() const { return (bool)playerFactory_; }
            std::unique_ptr<Player> createPlayer(int pi, size_t x, size_t y, size_t ms, size_t ns) const {
                return playerFactory_(pi, x, y, ms, ns);
            }
            std::unique_ptr<TankAlgorithm> createTank(int pi, int ti) const {
                return tankFactory_(pi, ti);
            }
            const TankAlgorithmFactory& tankFactory() const { return tankFactory_; }
            const PlayerFactory& playerFactory() const { return playerFactory_; }
        };
    private:
        std::vector<AlgorithmAndPlayerFactories> algos_;
        static AlgorithmRegistrar* self_;
        AlgorithmRegistrar() = default;
public:
    struct BadRegistration {
        std::string name; bool hasName, hasPlayer, hasTank;
    };

    static AlgorithmRegistrar& get() {
        static AlgorithmRegistrar inst;
        self_ = &inst;
        return inst;
    }

    // Simulator calls this just before dlopen of an Algorithm .so
    void beginRegistration(const std::string& so_base_name) {
        algos_.emplace_back(so_base_name);
    }
    // Called by PlayerRegistration/TankAlgorithmRegistration constructors inside the .so
    void addPlayerFactory(PlayerFactory&& f) {
        assert(!algos_.empty()); algos_.back().setPlayerFactory(std::move(f));
    }
    void addTankFactory(TankAlgorithmFactory&& f) {
        assert(!algos_.empty()); algos_.back().setTankFactory(std::move(f));
    }
    // After dlopen returns, Simulator should validate
    void validateLast() {
        const auto& last = algos_.back();
        const bool ok = !last.name().empty() && last.hasPlayer() && last.hasTank();
        if(!ok) throw BadRegistration{last.name(), !last.name().empty(), last.hasPlayer(), last.hasTank()};
    }
    void removeLast() { if(!algos_.empty()) algos_.pop_back(); }

    // iteration
    auto begin() const { return algos_.begin(); }
    auto end()   const { return algos_.end(); }
    size_t size() const { return algos_.size(); }
    void clear() { algos_.clear(); }
};

// ----- GameManager registrar: holds one GameManagerFactory per .so -----
class GameManagerRegistrar {
public:
    struct Entry { std::string so_name; GameManagerFactory factory; };
    static GameManagerRegistrar& get() { static GameManagerRegistrar inst; return inst; }

    void beginRegistration(const std::string& so_base_name) {
        gms_.push_back(Entry{so_base_name, {}});
    }
    void setFactoryOnLast(GameManagerFactory&& f) {
        assert(!gms_.empty()); assert(!gms_.back().factory); gms_.back().factory = std::move(f);
    }
    void validateLast() {
        if(gms_.empty() || !gms_.back().factory) { if(!gms_.empty()) gms_.pop_back(); throw std::runtime_error("GameManager .so did not register a factory"); }
    }
    void removeLast() { if(!gms_.empty()) gms_.pop_back(); }

    const std::vector<Entry>& entries() const { return gms_; }
    void clear() { gms_.clear(); }
private:
    std::vector<Entry> gms_;
};

// Note: Registration structs are implemented in separate .cpp files:
// - GameManagerRegistration.cpp
// - PlayerRegistration.cpp  
// - TankAlgorithmRegistration.cpp

// Define the static member variable
AlgorithmRegistrar* AlgorithmRegistrar::self_ = nullptr;