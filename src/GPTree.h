#ifndef GP_TREE_H
#define GP_TREE_H

#include <vector>

#define INF 0x3fffffff

void gptree_build();
int gptree_getsize();
int gptree_search(int S, int T);
int gptree_search_catch(int S, int T, int bound = INF);
int gptree_find_path(int S, int T, std::vector<int> &order);

std::vector<int> gptree_KNN(int S, int K, const std::vector<int> &T);
std::vector<int> gptree_KNN_bound(int S, int K, const std::vector<int> &T, int bound = INF);
std::vector<int> gptree_Range(int S, int R, const std::vector<int> &T);

#endif
