#include <algorithm>
#include <chrono>
#include <csignal>
#include <iostream>
#include <stdexcept>

#include "config/config.hh"
#include "config/definitions.hh"
#include "data_structures/graph.hh"
#include "data_structures/solution.hh"
#include "reductions/reductions.hh"
#include "solvers/greedy.hh"
#include "solvers/greedy_red_deg_limit.hh"
#include "solvers/tree_contract.hh"
#include "solvers/tree_contract_deluxe.hh"
#include "solvers/tree_contract_deluxe_order.hh"
#include "strategies/greedy.hh"
#include "utils.hh"

int original_n = -1;
Solution current_best_solution;
int current_objective = INT_MAX;
bool printing = false;

/**
 * @brief print the TreeContract solution without using the graph data structure
 */
void print_default_solution(int n) {
  int depth = std::ceil(std::log2(n));
  for (int current_depth = 0; current_depth <= depth; ++current_depth) {
    int pow = std::pow(2, current_depth);
    for (int i = 0; i < n; i += 2 * pow) {
      int other = i + pow;
      if (other < n) {
        std::cout << (i + 1) << " " << (other + 1) << std::endl;
      }
    }
  }
}

void print_solution() {
  if (!printing) {
    std::cout << "c" << std::endl;
    printing = true;
    if (!current_best_solution.contractions.empty()) {
      current_best_solution.print();
    } else {
      LOG("default solution");
      print_default_solution(original_n);
    }
    exit(0);
  }
}

inline void update_objective(graph &g) {
  if (g.twin_width < current_objective) {
    current_best_solution = g.build_solution();
    current_objective = g.twin_width;
  }
}

inline void local_search(graph &g, int k) {
  int uncontractions_till_worst = g.uncontractions_till_worst_tww();
  LOG("Undoing " << uncontractions_till_worst << " + " << (original_n / 10000)
                 << " contractions to worst tww");
  int uncontractions = std::min<int>(
      g.con_seq.size(), uncontractions_till_worst + original_n / 10000);
  g.uncontract(uncontractions);
  LOG("Uncontracted tww: " << g.twin_width);
  TreeContractDeluxe::solve(g, k + 2, 50);
  TreeContractDeluxe::solve(g, k);
  update_objective(g);
  LOG("Twin width: " << g.twin_width);
  update_objective(g);
}

std::vector<unsigned int> bfs_ordering(graph &g, unsigned int start = 0) {
  std::vector<unsigned int> result;
  result.reserve(g.n);
  std::vector<bool> visited;
  visited.resize(g.adj_list.size(), false);
  std::list<node *> queue;
  node *nd = start == 0 ? g.get_first_node() : &g.adj_list[start];
  queue.push_back(nd);
  result.push_back(nd->id);
  visited[nd->id] = true;
  while (!queue.empty()) {
    nd = queue.front();
    queue.pop_front();
    for (auto &&e : *nd) {
      if (!visited[e.target]) {
        visited[e.target] = true;
        queue.push_back(&g.adj_list[e.target]);
        result.push_back(e.target);
      }
    }
  }
  for (int i = 1; i <= g.adj_list.size(); i++) {
    node *nd = &g.adj_list[i];
    if (nd->active && !visited[nd->id]) {
      queue.push_back(nd);
      result.push_back(nd->id);
      visited[nd->id] = true;
      while (!queue.empty()) {
        nd = queue.front();
        queue.pop_front();
        for (auto &&e : *nd) {
          if (!visited[e.target]) {
            visited[e.target] = true;
            queue.push_back(&g.adj_list[e.target]);
            result.push_back(e.target);
          }
        }
      }
    }
  }
  return result;
}

void signal_handler(int signal) {
  if (signal == SIGTERM) {
    print_solution();
  }
}

int main(int argc, char **argv) {
  auto start_time = std::chrono::high_resolution_clock::now();

  std::signal(SIGTERM, signal_handler);
  graph g;
  g.read_from_stdin();
  original_n = g.n;

  reduce<DegreeZeroReduction>(g);
  reduce<DegreeOneReduction>(g);
  // if (g.n < 100000) {
  // LOG("Running fast twins");
  // reduce<TwinReduction>(g, 0, true); // fast twins
  // }
  int old_n = g.n;

  // Red deg limit for small graphs
  if (g.n < 10000 && g.m < 310000 * 2) {
    LOG("Running red deg limit");
    Config config = {
        .strategy = Strategy::SOLVER,
        .solver = Solver::RED_DEG_LIMIT,
        .reductions = ReductionConfig{},
        .reduction_frequency = 1,
        .refinement_iterations = 0,
        .uncontraction_depth = 0,
        .search_depth = 0,
        .random = 0,
        .write_solution = false,
        .write_heuristic_solution = false,
    };
    GreedyRedDegLimit::solve(g, config);
    LOG("RedDegLimit tww: " << g.twin_width);
    update_objective(g);
    g.uncontract(old_n - 1);
  }

  // std::vector<unsigned int> degree_sum_order;
  // for (node *nd = g.get_first_node(); nd; nd = nd->next) {
  //   degree_sum_order.push_back(nd->id);
  // }
  // std::vector<unsigned int> degrees;
  // degrees.resize(g.adj_list.size(), 0);
  // for (node *nd = g.get_first_node(); nd; nd = nd->next) {
  //   for (auto &&e : *nd) {
  //     degrees[nd->id] += e.target;
  //   }
  //   // degrees[nd->id] /= nd->deg; // average
  // }
  // std::sort(degree_sum_order.begin(), degree_sum_order.end(),
  //           [&](const unsigned int &a, const unsigned int &b) -> bool {
  //             return degrees[a] < degrees[b];
  //           });

  std::vector<unsigned int> bfs_ordering_vector = bfs_ordering(g);
  LOG("n: " << g.n << ", order size: " << bfs_ordering_vector.size());
  std::vector<unsigned int> bfs_ordering2_vector =
      bfs_ordering(g, bfs_ordering_vector.back());

  int k = 3;
  if (g.n < 20000)
    k = 4;
  if (g.n < 10000)
    k = 5;
  for (; true; k++) {
    LOG("TreeContractDeluxe k=" << k);
    // Natural order
    TreeContractDeluxe::solve(g, k);
    LOG("Twin width: " << g.twin_width);
    update_objective(g);
    // local_search(g, k);
    g.uncontract(old_n - 1);

    // degree order
    // std::list<unsigned int> order_list;
    // for (unsigned int& x : degree_sum_order) {
    //   order_list.push_back(x);
    // }
    // TreeContractDeluxeOrder::solve(g, order_list, k);
    // LOG("Twin width (degree order): " << g.twin_width);
    // update_objective(g);
    // g.uncontract(old_n - 1);

    std::list<unsigned int> bfs_order;
    // bfs ordering
    for (unsigned int x : bfs_ordering_vector) {
      bfs_order.push_back(x);
    }
    TreeContractDeluxeOrder::solve(g, bfs_order, k);
    LOG("Twin width (bfs order): " << g.twin_width);
    update_objective(g);
    g.uncontract(old_n - 1);

    // bfs ordering 2x
    // bfs_order.clear();
    // for (unsigned int x : bfs_ordering2_vector) {
    //   bfs_order.push_back(x);
    // }
    // TreeContractDeluxeOrder::solve(g, bfs_order, k);
    // LOG("Twin width (bfs order 2x): " << g.twin_width);
    // update_objective(g);
    // g.uncontract(old_n - 1);
  }

  print_solution();
}
