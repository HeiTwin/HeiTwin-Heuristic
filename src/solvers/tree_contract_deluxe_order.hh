#ifndef TREE_CONTRACT_DELUXE_ORDER_HH
#define TREE_CONTRACT_DELUXE_ORDER_HH

#include <algorithm>
#include <cstdlib>
#include <random>
#include <tuple>

#include "../data_structures/graph.hh"
#include "../utils.hh"
#include "tree_contract.hh"

class TreeContractDeluxeOrder {
public:
  static std::string name() { return "TreeContractDeluxeOrder"; }

  template <typename collection>
  static void solve(graph &g, collection order, int k = 5,
                    int iterations = -1) {
    std::vector<node *> nodes;
    nodes.resize(k);
    std::vector<typename collection::iterator> node_positions;
    node_positions.resize(k);

    std::vector<bool> contracted;
    contracted.resize(k, false);
    unsigned int best_red_degree;

    typename collection::iterator it = order.begin();

  restart:
    // put nodes into array
    while (g.n >= k && order.size() >= k && iterations != 0) {
      for (int i = 0; i < k; ++i) {
        if (it == order.end()) {
          it = order.begin();
          goto restart;
        }
        nodes[i] = &g.adj_list[*it];
        node_positions[i] = it;
        ++it;
      }

      // compute generated red degree for each pair
      unsigned int best = -1;
      std::pair<unsigned int, unsigned int> best_pair;
      for (int i = 0; i < k; ++i) {
        for (int j = i + 1; j < k; ++j) {
          auto x = g.get_generated_red_deg(nodes[i], nodes[j], best);
          if (x < best) {
            best = x;
            best_pair = {i, j};
          }
        }
      }

      g.contract(nodes[best_pair.first]->id, nodes[best_pair.second]->id);
      if (nodes[best_pair.first]->id < nodes[best_pair.second]->id) {
        order.erase(node_positions[best_pair.second]);
      } else {
        order.erase(node_positions[best_pair.first]);
      }

      --iterations;
    }

    if (iterations != 0)
      TreeContract::solve(g);
  }
};

#endif /* end of include guard: TREE_CONTRACT_DELUXE_ORDER_HH */
