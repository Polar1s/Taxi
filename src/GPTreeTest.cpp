#include "GPTree.h"
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <sys/time.h>

using namespace std;

#define TIME_TICK_START gettimeofday( &tv, NULL ); ts = tv.tv_sec * 1000000 + tv.tv_usec;
#define TIME_TICK_END gettimeofday( &tv, NULL ); te = tv.tv_sec * 1000000 + tv.tv_usec;
#define TIME_TICK_PRINT(T) printf("%s RESULT: %.3lf (S)\r\n", (#T), (te - ts) * 1e-6);

struct timeval tv;
long long ts, te;

int main() {
    TIME_TICK_START
    gptree_build();
    TIME_TICK_END
    TIME_TICK_PRINT("build")

    // manual check
    while (true) {
        int x, y;
        scanf("%d%d", &x, &y);
        if (x == -1 || y == -1)
            break;
        else {
            printf("%d\n", gptree_search(x, y));
            vector<int> order;
            gptree_find_path(x, y, order);
            for (int node : order)
                printf("%d ", node);
            printf("\n");
        }
    }

    // random check
    int n = gptree_getsize();
    {
        TIME_TICK_START
        for(int i=0;i<10000;i++)
        {
            int S=rand()%n;
            int T=rand()%n;
            int dis=gptree_search(S,T);
        }
        TIME_TICK_END
        TIME_TICK_PRINT("p2p-SEARCH:")
    }
}
