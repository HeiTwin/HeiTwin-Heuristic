#ifndef GREEDY_RED_DEG_LIMIT_HH
#define GREEDY_RED_DEG_LIMIT_HH

#include "../config/config.hh"
#include "../data_structures/graph.hh"
#include "../reductions/red_deg_limit.hh"
#include "../reductions/red_deg_limit_random.hh"
#include "../utils.hh"

class GreedyRedDegLimit {
public:
  static std::string name() { return "GreedyRedDegLimit"; }

  static void solve(graph& g, Config& config, unsigned int initial_tww = 1) {
    int depth = config.search_depth;
    if (!depth) depth = g.n;

    // Do one contraction to find initial tww
    Config c;
    c.search_depth = 1;
    config.random = false;
    run_solver(g, c, Solver::GREEDY_STRONG);
    initial_tww = std::max<unsigned int>(initial_tww, g.twin_width);
    LOG("Initial tww: " << initial_tww);

    int contracted_last_time = 10;

    for (int i = initial_tww; g.n > 1 && depth > 0; i += get_increase(g, contracted_last_time)) {
      LOG("Increased by " << get_increase(g, contracted_last_time));
      // TODO: depth limit not respected by reductions
      unsigned int old_n = g.n;
      if (config.reduction_frequency && i % config.reduction_frequency == 0) {
        // reduce(g, config);
      }
      depth -= old_n - g.n;
      old_n = g.n;
      if (config.random) {
        reduce<RedDegLimitRandomReduction>(g, config.reductions.reduce_exhaustively, i, depth);
      } else {
        reduce<RedDegLimitReduction>(g, config.reductions.reduce_exhaustively, i, depth);
      }
      contracted_last_time = old_n - g.n;
      depth -= contracted_last_time;
    }
  }

  static inline int get_increase(graph& g, int contracted_last_time) {
    double fraction_contracted = (double)(contracted_last_time + 1) / (g.n + contracted_last_time);
    double graph_size_factor = 0.1 + g.edges.size() / 750000. * 0.9;
    double val = graph_size_factor / fraction_contracted * 0.8;
    return std::min<int>(g.n / 50., std::max<int>(val, 1));
  }
};

#endif /* end of include guard: GREEDY_RED_DEG_LIMIT_HH */
