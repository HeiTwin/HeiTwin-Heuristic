#ifndef TREE_CONTRACT_DELUXE_HH
#define TREE_CONTRACT_DELUXE_HH

#include <algorithm>
#include <cstdlib>
#include <random>
#include <tuple>

#include "../data_structures/graph.hh"
#include "../utils.hh"
#include "tree_contract.hh"

class TreeContractDeluxe {
public:
  static std::string name() { return "TreeContractDeluxe"; }

  static void solve(graph &g, int k = 5,
                    int iterations = -1) {
    std::vector<node *> nodes;
    nodes.resize(k);

    std::vector<std::tuple<unsigned int, int, int>> options;
    options.resize((k * k - 1) / 2);

    std::vector<bool> contracted;
    contracted.resize(k, false);
    unsigned int best_red_degree;

    // put nodes into array
    node *current = g.get_first_node();

  end_while:
    while (g.n >= k && iterations != 0) {
      for (int i = 0; i < k; ++i) {
        if (!current) {
          current = g.get_first_node();
          goto end_while;
        }
        nodes[i] = current;
        current = current->next;
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

      --iterations;
    }

    if (iterations != 0)
      TreeContract::solve(g);
  }
};

#endif /* end of include guard: TREE_CONTRACT_DELUXE_HH */
