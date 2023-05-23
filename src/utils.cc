#include "utils.hh"

#include <chrono>
#include <stdexcept>
#include <string>

#include "config/definitions.hh"
#include "reductions/degzero.hh"
#include "reductions/red_deg_limit.hh"
#include "reductions/reductions.hh"
#include "solvers/greedy.hh"
#include "solvers/greedy_red_deg_limit.hh"
#include "solvers/greedy_strong.hh"
#include "solvers/tree_contract.hh"
#include "strategies/greedy.hh"

void reduce(graph &g, Config &config) {
  int old_n = g.n, previous_n = 0;
  auto start = std::chrono::high_resolution_clock::now();
  do {
    previous_n = g.n;
    if (config.reductions.deg_zero) {
      reduce<DegreeZeroReduction>(g);
    }
    if (config.reductions.deg_one) {
      reduce<DegreeOneReduction>(g);
    }
    if (config.reductions.fast_twins) {
      reduce<TwinReduction>(g, 0, true);
    }
    if (config.reductions.twins) {
      reduce<TwinReduction>(g, false);
    }
    if (config.reductions.paths) {
      reduce<PathReduction>(g);
    }
    if (config.reductions.red_deg_limit) {
      reduce<RedDegLimitReduction>(g, config.reductions.reduce_exhaustively);
    }
  } while (config.reductions.reduce_exhaustively && g.n != previous_n);
  auto end = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
  if (config.reductions.reduce_exhaustively) {
    LOG("Reductions ran in " << ((double)duration.count()) / 1e6 << "s and contracted " << old_n - g.n << " nodes");
  } else {
    LOG("Reductions ran in " << ((double)duration.count()) / 1e6 << "s and contracted " << old_n - g.n
                               << " nodes (exhaustively)");
  }
  LOG("Reductions;" << old_n << ";" << g.n << ";" << (g.n / (double)old_n));
}

void run_solver(graph &g, Config &config, Solver solver) {
  auto start = std::chrono::high_resolution_clock::now();
  switch (solver) {
    case Solver::GREEDY:
      Greedy::solve(g, config.search_depth);
      break;
    case Solver::RED_DEG_LIMIT:
      GreedyRedDegLimit::solve(g, config);
      break;
    case Solver::ONLY_REDUCTIONS:
      break;
    case Solver::TREE_CONTRACT:
      solve<TreeContract>(g);
      break;
    case Solver::GREEDY_STRONG:
      solve<GreedyStrong>(g, config);
      break;
    default:
      throw std::invalid_argument("Solver not covered");
  }
  auto end = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
  // std::cout << "c Solver " << get_solver_name(solver) << " ran in " << ((double)duration.count()) / 1e6
  //           << "s, n: " << g.n << std::endl;
}

void write_solution(graph &g, Solution &solution) {
  Solution restore = g.build_solution();
  g.uncontract(restore.contractions.size());
  for (int i = 0; i < solution.contractions.size(); ++i) {
    g.write_to_file(".out/graph/" + std::to_string(i) + ".gr");
    g.contract(solution.contractions[i].u, solution.contractions[i].v);
  }
  g.uncontract(solution.contractions.size());
  for (auto &&c : restore.contractions) {
    g.contract(c.u, c.v);
  }
}
