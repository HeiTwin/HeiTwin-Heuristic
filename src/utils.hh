#ifndef UTILS_HH
#define UTILS_HH

#include <chrono>
#include <type_traits>

#include "config/config.hh"
#include "config/definitions.hh"
#include "data_structures/graph.hh"

#ifdef ENABLE_LOG
#define LOG(msg) std::cout << "c " << __FILE__ << "(" << __LINE__ << "): " << msg << std::endl;
#else
#define LOG(msg) ;
#endif

template <typename Solver, typename... Args>
void solve(graph &g, Args &&...args) {
  auto start = std::chrono::high_resolution_clock::now();
  Solver::solve(g, args...);
  auto end = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
  LOG("Solver " << Solver::name() << " ran in " << ((double)duration.count()) / 1e6 << "s, n: " << g.n);
}

template <typename Reduction, typename... Args>
void reduce(graph &g, Args &&...args) {
  int old_n = g.n;
  auto start = std::chrono::high_resolution_clock::now();
  Reduction::reduce(g, args...);
  auto end = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
  LOG(Reduction::name() << " ran in " << ((double)duration.count()) / 1e6 << "s and contracted " << old_n - g.n
                        << " nodes");
}

template <typename Reduction>
void do_reduce_exhausively(graph &g) {
  Reduction::reduce(g);
}

template <typename Reduction, typename... Reductions>
std::enable_if_t<(sizeof...(Reductions) > 0), void> do_reduce_exhausively(graph &g) {
  Reduction::reduce(g);
  do_reduce_exhausively<Reductions...>(g);
}

template <typename... Reductions>
void reduce_exhausively(graph &g) {
  int old_n = g.n, previous_n = 0;
  auto start = std::chrono::high_resolution_clock::now();
  while (g.n != previous_n) {
    previous_n = g.n;
    do_reduce_exhausively<Reductions...>(g);
  }
  auto end = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
  LOG("Exhaustive reductions ran in " << ((double)duration.count()) / 1e6 << "s and contracted " << old_n - g.n
                                      << " nodes");
}

void reduce(graph &g, Config &config);

void run_solver(graph &g, Config &config, Solver solver);

void write_solution(graph &g, Solution &solution);

#endif
