// simulator_schedule_example.cpp
#include <algorithm>
#include <iostream>
#include <mutex>
#include <string>
#include <vector>
#include "thread_pool.h"
#include "registrars.hpp"          // your registrars and registration bridge
#include "common/AbstractGameManager.h"
#include "common/GameResult.h"
#include "common/SatelliteView.h"

// a trivial snapshot map just for example
class FlatMap : public SatelliteView {
public:
    FlatMap(std::size_t w, std::size_t h): w_(w), h_(h) {}
    char getObjectAt(std::size_t x, std::size_t y) const override {
        return (x < w_ && y < h_) ? '.' : '#';
    }
private:
    std::size_t w_, h_;
};

struct ResultRow {
    std::string gm_name;
    std::string a1_name;
    std::string a2_name;
    std::string map_name;
    int winner;
    std::size_t rounds;
};

static void run_one_game(
    const GameManagerFactory& gm_factory,
    const std::string& gm_name,
    const AlgorithmRegistrar::AlgorithmAndPlayerFactories& A,
    const AlgorithmRegistrar::AlgorithmAndPlayerFactories& B,
    const SatelliteView& map,
    const std::string& map_name,
    bool verbose,
    std::vector<ResultRow>& out_rows,
    std::mutex& out_mtx)
{
    auto gm = gm_factory(verbose);

    const std::size_t W = 32, H = 20, MAX_STEPS = 400, NUM_SHELLS = 20;
    auto p1 = A.createPlayer(1, 1, 1, MAX_STEPS, NUM_SHELLS);
    auto p2 = B.createPlayer(2, W - 2, H - 2, MAX_STEPS, NUM_SHELLS);

    GameResult res = gm->run(
        W, H,
        map, map_name,
        MAX_STEPS, NUM_SHELLS,
        *p1, A.name(), *p2, B.name(),
        A.tankFactory(), B.tankFactory()
    );

    std::lock_guard<std::mutex> lk(out_mtx);
    out_rows.push_back(ResultRow{
        gm_name, A.name(), B.name(), map_name, res.winner, res.rounds
    });
}

// schedule a bunch of games using a thread pool that respects the assignment rules
void schedule_and_run_games(
    const std::vector<GameManagerFactory>& gm_factories,
    const std::vector<std::string>& gm_names,
    const std::vector<AlgorithmRegistrar::AlgorithmAndPlayerFactories>& algos,
    const std::vector<std::string>& map_names,
    int num_threads,        // from command line
    bool verbose)
{
    // build all jobs
    struct Job {
        GameManagerFactory gm_factory;
        std::string gm_name;
        const AlgorithmRegistrar::AlgorithmAndPlayerFactories* A;
        const AlgorithmRegistrar::AlgorithmAndPlayerFactories* B;
        std::string map_name;
    };
    std::vector<Job> jobs;

    // example: comparative run of all game managers for a single map
    FlatMap the_map(32, 20);
    for (std::size_t gi = 0; gi < gm_factories.size(); ++gi) {
        for (std::size_t ai = 0; ai < algos.size(); ++ai) {
            for (std::size_t bi = 0; bi < algos.size(); ++bi) {
                if (ai > bi) continue; // avoid duplicates in the demo
                jobs.push_back(Job{
                    gm_factories[gi], gm_names[gi],
                    &algos[ai], &algos[bi],
                    map_names.empty() ? std::string("demo_map") : map_names.front()
                });
            }
        }
    }

    std::vector<ResultRow> rows;
    std::mutex rows_mtx;

    // decide worker count
    std::size_t worker_count = 0;
    if (num_threads >= 2) {
        // it is allowed to use fewer threads than requested
        worker_count = std::min<std::size_t>(num_threads, jobs.size());
    }

    if (worker_count == 0) {
        // single thread mode
        for (const auto& j : jobs) {
            run_one_game(
                j.gm_factory, j.gm_name, *j.A, *j.B,
                the_map, j.map_name, verbose, rows, rows_mtx
            );
        }
    } else {
        ThreadPool pool(worker_count); // workers in addition to the main thread
        for (const auto& j : jobs) {
            pool.submit([&, j] {
                run_one_game(
                    j.gm_factory, j.gm_name, *j.A, *j.B,
                    the_map, j.map_name, verbose, rows, rows_mtx
                );
            });
        }
        pool.wait_idle(); // main waits for all simulation jobs
        pool.shutdown();  // optional, destructor also does it
    }

    // print or write output file here
    for (const auto& r : rows) {
        std::cout << r.gm_name << "  " << r.a1_name
                  << " vs " << r.a2_name
                  << " on " << r.map_name
                  << " winner " << r.winner
                  << " rounds " << r.rounds << "\n";
    }
}





// inside your Simulator after loading .so files
std::vector<GameManagerFactory> gm_factories;
std::vector<std::string> gm_names;
for (const auto& e : GameManagerRegistrar::get().entries()) {
    gm_factories.push_back(e.factory);
    gm_names.push_back(e.so_name);
}

std::vector<AlgorithmRegistrar::AlgorithmAndPlayerFactories> algos;
for (const auto& a : AlgorithmRegistrar::get()) {
    algos.push_back(a); // requires copyable holder, or store pointers
}

std::vector<std::string> map_names = {"map01"}; // fill from folder in your code

int num_threads = /* parsed from command line or default */;
bool verbose = /* parsed */;

schedule_and_run_games(gm_factories, gm_names, algos, map_names, num_threads, verbose);
