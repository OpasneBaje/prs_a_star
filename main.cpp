
//std::numeric_limits<float>::infinity();
#include <iostream>
#include <queue>
#include <vector>
#include <functional>
#include <math.h>
#include <chrono>
#include <random>
#include <tuple>
#include <limits>
#include <algorithm>
#include <fstream>
#include <omp.h>
#include <immintrin.h>
#include <atomic>

template<int D = 4>
class DaryHeap {
private:
    std::vector<int> indices;
    std::vector<float> f_costs;
    std::vector<float> g_costs;
    std::vector<int> parents;

    void siftUp(int pos) {
        int parent_pos = (pos - 1) / D;

        float my_f = f_costs[pos];
        int my_idx = indices[pos];
        float my_g = g_costs[pos];
        int my_parent = parents[pos];

        while(pos > 0 && my_f < f_costs[parent_pos]) {
            indices[pos] = indices[parent_pos];
            f_costs[pos] = f_costs[parent_pos];
            g_costs[pos] = g_costs[parent_pos];
            parents[pos] = parents[parent_pos];

            pos = parent_pos;
            parent_pos = (pos - 1) / D;
        }

        indices[pos] = my_idx;
        f_costs[pos] = my_f;
        g_costs[pos] = my_g;
        parents[pos] = my_parent;
    }

    void siftDown(int pos) {
        int size = f_costs.size();

        float my_f = f_costs[pos];
        int my_idx = indices[pos];
        float my_g = g_costs[pos];
        int my_parent = parents[pos];

        while(true) {
            int first_child = D * pos + 1;
            if(first_child >= size) break;

            int smallest_child = first_child;
            float smallest_f = f_costs[first_child];

            int last_child = std::min(first_child + D, size);
            for(int child = first_child + 1; child < last_child; child++) {
                if(f_costs[child] < smallest_f) {
                    smallest_child = child;
                    smallest_f = f_costs[child];
                }
            }

            if(my_f <= smallest_f) break;

            indices[pos] = indices[smallest_child];
            f_costs[pos] = f_costs[smallest_child];
            g_costs[pos] = g_costs[smallest_child];
            parents[pos] = parents[smallest_child];

            pos = smallest_child;
        }

        indices[pos] = my_idx;
        f_costs[pos] = my_f;
        g_costs[pos] = my_g;
        parents[pos] = my_parent;
    }

public:
    void reserve(size_t capacity) {
        indices.reserve(capacity);
        f_costs.reserve(capacity);
        g_costs.reserve(capacity);
        parents.reserve(capacity);
    }

    void push(int idx, float f, float g, int parent) {
        indices.push_back(idx);
        f_costs.push_back(f);
        g_costs.push_back(g);
        parents.push_back(parent);
        siftUp(indices.size() - 1);
    }

    std::tuple<int, float, float, int> pop() {
        int result_idx = indices[0];
        float result_f = f_costs[0];
        float result_g = g_costs[0];
        int result_parent = parents[0];

        int last = indices.size() - 1;
        indices[0] = indices[last];
        f_costs[0] = f_costs[last];
        g_costs[0] = g_costs[last];
        parents[0] = parents[last];

        indices.pop_back();
        f_costs.pop_back();
        g_costs.pop_back();
        parents.pop_back();

        if(!indices.empty())
            siftDown(0);

        return {result_idx, result_f, result_g, result_parent};
    }

    bool empty() const { return indices.empty(); }
    size_t size() const { return indices.size(); }
};

class Graf {
private:
  const std::vector<std::vector<float>> &cvorovi;
  const std::vector<float>&cvorovi_v2;
  int broj_redova, broj_kolona;

  static float Heuristika(int current_x, int current_y, int goal_x, int goal_y, float w_min) {
    /*int dx = std::abs(goal_x - current_x);
    int dy = std::abs(goal_y - current_y);
    return w_min * std::sqrt(dx * dx + dy * dy);*/
    int dx = std::abs(goal_x - current_x);
    int dy = std::abs(goal_y - current_y);
    return w_min * (float)std::max(dx, dy);
  }
public:
  Graf(const std::vector<std::vector<float>> &c, int r, int k) : cvorovi(c), cvorovi_v2(std::vector<float>(0)) {
    broj_redova = r;
    broj_kolona = k;
  }
  Graf(const std::vector<float> &c, int r, int k): cvorovi_v2(c), cvorovi(std::vector<std::vector<float>>(0)) {
    broj_redova = r;
    broj_kolona = k;
  }
  //Osnovna sekvencijalna metoda
  float A_star(std::pair<int,int>start, std::pair<int,int>finish) {
  //Koristenje flatten vektora
  //Koristenje 1D
     if(start.first < 0 || start.first >= broj_redova ||
        start.second < 0 || start.second >= broj_kolona ||
        finish.first < 0 || finish.first >= broj_redova ||
        finish.second < 0 || finish.second >= broj_kolona){

          throw std::domain_error("Start ili finis van granica");
          return -1.0f;
    }

    int index_trenutniCvor = start.first*broj_kolona+start.second;
    int index_finishCvor = finish.first*broj_kolona+finish.second;

    const int dx[8] = { 0, -1, 0,  1, -1, -1,  1, 1 };
    const int dy[8] = { 1, 0,  -1, 0, 1,  -1, 1, -1 };
    //Desno, Gore, Lijevo, Dole, G. Desno, G. Lijevo, D. Desno, D. Lijevo

    std::vector<std::tuple<bool, float, int>>/*obradjen, g(n), od kojeg cvora je dobio */ obradjeniCvorovi(broj_redova*broj_kolona, std::make_tuple(false, 0.0f, 0));

    auto cmp = [](const std::tuple<int,float, float,int> &a, const std::tuple<int,float, float,int> &b){
        return std::get<1>(a) > std::get<1>(b);
    };
    std::priority_queue<std::tuple<int,float,float,int>/*cvor, f(n), g(n), od_koga*/,std::vector<std::tuple<int,float,float,int>>,decltype(cmp)> minHeap(cmp);

    minHeap.push({index_trenutniCvor, 0,0,0});

    std::vector<float> heuristika_za_okolne_cvorove(8);
    std::tuple<int,float,float,int> trenutniCvor;
    while(minHeap.size() > 0) {
      trenutniCvor = minHeap.top();
      minHeap.pop();
      if(std::get<0>(trenutniCvor)==index_finishCvor){
        return std::get<2>(trenutniCvor);
      }
      index_trenutniCvor = std::get<0>(trenutniCvor);
      if(std::get<0>(obradjeniCvorovi[index_trenutniCvor])){
         continue;
      }

      int x = index_trenutniCvor / broj_kolona;
      int y = index_trenutniCvor % broj_kolona;

      heuristika_za_okolne_cvorove[0] = Heuristika(x,y+1, finish.first, finish.second,1.);//Desno
      heuristika_za_okolne_cvorove[1] = Heuristika(x-1,y, finish.first, finish.second,1.);//Gore
      heuristika_za_okolne_cvorove[2] = Heuristika(x,y-1, finish.first, finish.second,1.);//Lijevo
      heuristika_za_okolne_cvorove[3] = Heuristika(x+1,y, finish.first, finish.second,1.);//Dole
      heuristika_za_okolne_cvorove[4] = Heuristika(x-1,y+1, finish.first, finish.second,1.);//Gore desno
      heuristika_za_okolne_cvorove[5] = Heuristika(x-1,y-1, finish.first, finish.second,1.);//Gore lijevo
      heuristika_za_okolne_cvorove[6] = Heuristika(x+1,y+1, finish.first, finish.second,1.);//Dole desno
      heuristika_za_okolne_cvorove[7] = Heuristika(x+1,y-1, finish.first, finish.second,1.);//Dole lijevo

      for(int i = 0; i < 8; i++) {
        int nx = x + dx[i];
        int ny = y + dy[i];

        if(nx < 0 || nx >= broj_redova || ny < 0 || ny >= broj_kolona)
            continue;

        if(cvorovi[index_trenutniCvor][i] == std::numeric_limits<float>::infinity())
            continue;

        int index_susjeda = nx * broj_kolona + ny;
        if(std::get<0>(obradjeniCvorovi[index_susjeda]) == false)
          minHeap.push({index_susjeda,heuristika_za_okolne_cvorove[i] + std::get<2>(trenutniCvor)+cvorovi[std::get<0>(trenutniCvor)][i], std::get<2>(trenutniCvor)+cvorovi[std::get<0>(trenutniCvor)][i], std::get<0>(trenutniCvor)});
      }
      obradjeniCvorovi[std::get<0>(trenutniCvor)] = {true, std::get<2>(trenutniCvor), std::get<3>(trenutniCvor)};
    }

    return -1.0f;

  }
  float A_star_v2(std::pair<int,int>start, std::pair<int,int>finish) {
    if(start.first < 0 || start.first >= broj_redova ||
       start.second < 0 || start.second >= broj_kolona ||
       finish.first < 0 || finish.first >= broj_redova ||
       finish.second < 0 || finish.second >= broj_kolona) {
        throw std::domain_error("Start ili finis van granica");
        return -1.0f;
    }

    int index_trenutniCvor = start.first * broj_kolona + start.second;
    int index_finishCvor = finish.first * broj_kolona + finish.second;

    const int dx[8] = { 0, -1, 0, 1, -1, -1, 1, 1 };
    const int dy[8] = { 1, 0, -1, 0, 1, -1, 1, -1 };

    std::vector<std::tuple<bool, float, int>>/*obradjen, g(n) ili f(n) ako niej obradjen, od kojeg cvora je dobio */ obradjeniCvorovi(broj_redova * broj_kolona, std::make_tuple(false, std::numeric_limits<float>::infinity(), 0));

    auto cmp = [](const std::tuple<int,float,float,int> &a, const std::tuple<int,float,float,int> &b){
        return std::get<1>(a) > std::get<1>(b);
    };
    std::priority_queue<std::tuple<int,float,float,int>/*cvor, f(n), g(n), od_koga*/,std::vector<std::tuple<int,float,float,int>>,decltype(cmp)> minHeap(cmp);

    minHeap.push({index_trenutniCvor, 0, 0, 0});

    std::vector<float> heuristika_za_okolne_cvorove(8);
    std::tuple<int,float,float,int> trenutniCvor;

    while(!minHeap.empty()) {
        trenutniCvor = minHeap.top();
        minHeap.pop();

        if(std::get<0>(trenutniCvor) == index_finishCvor)
            return std::get<2>(trenutniCvor);

        index_trenutniCvor = std::get<0>(trenutniCvor);

        if(std::get<0>(obradjeniCvorovi[index_trenutniCvor]))
            continue;

        int x = index_trenutniCvor / broj_kolona;
        int y = index_trenutniCvor % broj_kolona;

        // Heuristika za okolne čvorove
        for(int i = 0; i < 8; i++)
            heuristika_za_okolne_cvorove[i] = Heuristika(x + dx[i], y + dy[i], finish.first, finish.second, 1.0f);

        for(int i = 0; i < 8; i++) {
            int nx = x + dx[i];
            int ny = y + dy[i];
            if(nx < 0 || nx >= broj_redova || ny < 0 || ny >= broj_kolona)
                continue;

            int weight_index = index_trenutniCvor * 8 + i;
            if(cvorovi_v2[weight_index] == std::numeric_limits<float>::infinity())
                continue;

            int index_susjeda = nx * broj_kolona + ny;
            if(!std::get<0>(obradjeniCvorovi[index_susjeda])) {
                float g_novo = std::get<2>(trenutniCvor) + cvorovi_v2[weight_index];
                float f_novo = g_novo + heuristika_za_okolne_cvorove[i];
                if(f_novo < std::get<1>(obradjeniCvorovi[index_susjeda])){
                  minHeap.push({index_susjeda, f_novo, g_novo, index_trenutniCvor});
                  std::get<1>(obradjeniCvorovi[index_susjeda]) = f_novo;
                }
            }
        }

        obradjeniCvorovi[index_trenutniCvor] = {true, std::get<2>(trenutniCvor), std::get<3>(trenutniCvor)};
    }

    return -1.0f;
  }
  //Bolji hit rate i manje ubacivanje u heap i drugi heap
  std::pair<float, std::vector<int>> A_star_v3(std::pair<int,int>start, std::pair<int,int>finish) {
    if(start.first < 0 || start.first >= broj_redova ||
       start.second < 0 || start.second >= broj_kolona ||
       finish.first < 0 || finish.first >= broj_redova ||
       finish.second < 0 || finish.second >= broj_kolona) {
        throw std::domain_error("Start ili finis van granica");
    }

    int index_trenutniCvor = start.first * broj_kolona + start.second;
    int index_finishCvor = finish.first * broj_kolona + finish.second;

    const int dx[8] = { 0, -1, 0, 1, -1, -1, 1, 1 };
    const int dy[8] = { 1, 0, -1, 0, 1, -1, 1, -1 };

    std::vector<std::tuple<bool, float, int>>/*obradjen, g(n) ili f(n) ako niej obradjen, od kojeg cvora je dobio */ obradjeniCvorovi(broj_redova * broj_kolona, std::make_tuple(false, std::numeric_limits<float>::infinity(), 0));
    DaryHeap<4> minHeap;
    minHeap.reserve(broj_redova * broj_kolona / 20);
    minHeap.push(index_trenutniCvor, 0, 0, 0);/*cvor, f(n), g(n), od_koga*/

    std::vector<float> heuristika_za_okolne_cvorove(8);
    std::tuple<int,float,float,int> trenutniCvor;/*cvor, f(n), g(n), od_koga*/

    while(!minHeap.empty()) {
        trenutniCvor = minHeap.pop();

        if(std::get<0>(trenutniCvor) == index_finishCvor){
          std::vector<int> putanja;
          index_trenutniCvor = std::get<0>(trenutniCvor);

          obradjeniCvorovi[index_trenutniCvor] = {true, std::get<2>(trenutniCvor), std::get<3>(trenutniCvor)};

          int trenutni = index_finishCvor;
          int start_index = start.first * broj_kolona + start.second;

          while(trenutni != start_index) {
              putanja.push_back(trenutni);
              trenutni = std::get<2>(obradjeniCvorovi[trenutni]);
          }
          putanja.push_back(start_index);  // dodaj i start

          return {std::get<2>(trenutniCvor), putanja};
        }

        index_trenutniCvor = std::get<0>(trenutniCvor);

        if(std::get<0>(obradjeniCvorovi[index_trenutniCvor]))
            continue;

        int x = index_trenutniCvor / broj_kolona;
        int y = index_trenutniCvor % broj_kolona;
        for(int i = 0; i < 8; i++)
            heuristika_za_okolne_cvorove[i] = Heuristika(x + dx[i], y + dy[i], finish.first, finish.second, 1.0f);
        for(int i = 0; i < 8; i++) {
            int nx = x + dx[i];
            int ny = y + dy[i];
            if(nx < 0 || nx >= broj_redova || ny < 0 || ny >= broj_kolona)
                continue;

            int weight_index = index_trenutniCvor * 8 + i;
            if(cvorovi_v2[weight_index] == std::numeric_limits<float>::infinity())
                continue;

            int index_susjeda = nx * broj_kolona + ny;
            if(!std::get<0>(obradjeniCvorovi[index_susjeda])) {
                float g_novo = std::get<2>(trenutniCvor) + cvorovi_v2[weight_index];
                float f_novo = g_novo + heuristika_za_okolne_cvorove[i];
                if(f_novo < std::get<1>(obradjeniCvorovi[index_susjeda])){
                  minHeap.push(index_susjeda, f_novo, g_novo, index_trenutniCvor);
                  std::get<1>(obradjeniCvorovi[index_susjeda]) = f_novo;
                }
            }
        }

        obradjeniCvorovi[index_trenutniCvor] = {true, std::get<2>(trenutniCvor), std::get<3>(trenutniCvor)};
    }

    return {-1., std::vector<int>(0)};
  }
  //Paralelizacija bidirectional(pokrene se iz dvije tacke pretraga)
  std::pair<float, std::vector<int>> A_star_v4(std::pair<int,int>start, std::pair<int,int>finish) {
    if(start.first < 0 || start.first >= broj_redova ||
       start.second < 0 || start.second >= broj_kolona ||
       finish.first < 0 || finish.first >= broj_redova ||
       finish.second < 0 || finish.second >= broj_kolona) {
      throw std::domain_error("Start ili finis van granica");
    }
    const int N = broj_redova * broj_kolona;
    std::vector<std::atomic<uint8_t>> closedBits(N);
    #pragma omp parallel for schedule(static)
    for (int i = 0; i < N; i++)
        closedBits[i].store(0, std::memory_order_relaxed);

    std::atomic<bool> done{false};
    std::atomic<int> meet{-1};

    std::vector<std::tuple<bool, float, int>>/*obradjen, g(n) ili f(n) ako nije obradjen, od kojeg cvora je dobio */ obradjeniCvoroviA(broj_redova * broj_kolona, std::make_tuple(false, std::numeric_limits<float>::infinity(), 0));
    std::vector<std::tuple<bool, float, int>>/*obradjen, g(n) ili f(n) ako nije obradjen, od kojeg cvora je dobio */ obradjeniCvoroviB(broj_redova * broj_kolona, std::make_tuple(false, std::numeric_limits<float>::infinity(), 0));
    #pragma omp parallel sections num_threads(2) shared(obradjeniCvoroviA, obradjeniCvoroviB, closedBits, done, meet)
    {
      #pragma omp section
      {
        int index_trenutniCvor = start.first * broj_kolona + start.second;
        int index_finishCvor = finish.first * broj_kolona + finish.second;

        const int dx[8] = { 0, -1, 0, 1, -1, -1, 1, 1 };
        const int dy[8] = { 1, 0, -1, 0, 1, -1, 1, -1 };

        DaryHeap<4> minHeap;
        minHeap.reserve(broj_redova * broj_kolona / 20);
        minHeap.push(index_trenutniCvor, 0, 0, 0);/*cvor, f(n), g(n), od_koga*/

        std::vector<float> heuristika_za_okolne_cvorove(8);
        std::tuple<int,float,float,int> trenutniCvor;/*cvor, f(n), g(n), od_koga*/

        while(!minHeap.empty() && !done.load(std::memory_order_relaxed)) {
            trenutniCvor = minHeap.pop();

            if(std::get<0>(trenutniCvor) == index_finishCvor){
              //Gotovo
            }

            index_trenutniCvor = std::get<0>(trenutniCvor);

            if(std::get<0>(obradjeniCvoroviA[index_trenutniCvor]))
                continue;

            int x = index_trenutniCvor / broj_kolona;
            int y = index_trenutniCvor % broj_kolona;

            for(int i = 0; i < 8; i++)
                heuristika_za_okolne_cvorove[i] = Heuristika(x + dx[i], y + dy[i], finish.first, finish.second, 1.0f);
            for(int i = 0; i < 8; i++) {
                int nx = x + dx[i];
                int ny = y + dy[i];

                if(nx < 0 || nx >= broj_redova || ny < 0 || ny >= broj_kolona)
                    continue;

                int weight_index = index_trenutniCvor * 8 + i;
                if(cvorovi_v2[weight_index] == std::numeric_limits<float>::infinity())
                    continue;

                int index_susjeda = nx * broj_kolona + ny;
                if(!std::get<0>(obradjeniCvoroviA[index_susjeda])) {
                    float g_novo = std::get<2>(trenutniCvor) + cvorovi_v2[weight_index];
                    float f_novo = g_novo + heuristika_za_okolne_cvorove[i];
                    if(f_novo < std::get<1>(obradjeniCvoroviA[index_susjeda])){
                      minHeap.push(index_susjeda, f_novo, g_novo, index_trenutniCvor);
                      std::get<1>(obradjeniCvoroviA[index_susjeda]) = f_novo;
                    }
                    if (closedBits[index_susjeda].load(std::memory_order_acquire) & 2) {
                      if (!done.exchange(true, std::memory_order_acq_rel)){
                          meet.store(index_susjeda, std::memory_order_release);
                          obradjeniCvoroviA[index_susjeda] = {true, g_novo, index_trenutniCvor};
                          closedBits[index_susjeda].fetch_or(1, std::memory_order_acq_rel);
                      }
                      break;
                    }
                }
            }

            obradjeniCvoroviA[index_trenutniCvor] = {true, std::get<2>(trenutniCvor), std::get<3>(trenutniCvor)};
            uint8_t prev = closedBits[index_trenutniCvor].fetch_or(1, std::memory_order_acq_rel);
            if (prev & 2) {
                if (!done.exchange(true, std::memory_order_acq_rel))
                    meet.store(index_trenutniCvor, std::memory_order_release);
                break;
            }
        }
      }
      #pragma omp section
      {
        int index_trenutniCvor = finish.first * broj_kolona + finish.second;
        int index_finishCvor = start.first * broj_kolona + start.second;

        const int dx[8] = { 0, -1, 0, 1, -1, -1, 1, 1 };
        const int dy[8] = { 1, 0, -1, 0, 1, -1, 1, -1 };

        DaryHeap<4> minHeap;
        minHeap.reserve(broj_redova * broj_kolona / 20);
        minHeap.push(index_trenutniCvor, 0, 0, 0);

        std::vector<float> heuristika_za_okolne_cvorove(8);
        std::tuple<int,float,float,int> trenutniCvor;

        while(!minHeap.empty() && !done.load(std::memory_order_relaxed)) {
            trenutniCvor = minHeap.pop();

            if(std::get<0>(trenutniCvor) == index_finishCvor){
                //Gotovo
            }

            index_trenutniCvor = std::get<0>(trenutniCvor);

            if(std::get<0>(obradjeniCvoroviB[index_trenutniCvor]))
                continue;

            int x = index_trenutniCvor / broj_kolona;
            int y = index_trenutniCvor % broj_kolona;

            for(int i = 0; i < 8; i++)
                heuristika_za_okolne_cvorove[i] = Heuristika(x + dx[i], y + dy[i], start.first, start.second, 1.0f);
            for(int i = 0; i < 8; i++) {
                int nx = x + dx[i];
                int ny = y + dy[i];

                if(nx < 0 || nx >= broj_redova || ny < 0 || ny >= broj_kolona)
                    continue;
                int weight_index = index_trenutniCvor * 8 + i;
                if(cvorovi_v2[weight_index] == std::numeric_limits<float>::infinity())
                    continue;

                int index_susjeda = nx * broj_kolona + ny;
                if(!std::get<0>(obradjeniCvoroviB[index_susjeda])) {
                    float g_novo = std::get<2>(trenutniCvor) + cvorovi_v2[weight_index];
                    float f_novo = g_novo + heuristika_za_okolne_cvorove[i];
                    if(f_novo < std::get<1>(obradjeniCvoroviB[index_susjeda])){
                      minHeap.push(index_susjeda, f_novo, g_novo, index_trenutniCvor);
                      std::get<1>(obradjeniCvoroviB[index_susjeda]) = f_novo;
                    }
                  if (closedBits[index_susjeda].load(std::memory_order_acquire) & 1) {
                    if (!done.exchange(true, std::memory_order_acq_rel)) {
                        meet.store(index_susjeda, std::memory_order_release);
                        obradjeniCvoroviB[index_susjeda] = {true, g_novo, index_trenutniCvor};
                        closedBits[index_susjeda].fetch_or(2, std::memory_order_acq_rel);
                    }
                    break;
                  }
                }
            }

            obradjeniCvoroviB[index_trenutniCvor] = {true, std::get<2>(trenutniCvor), std::get<3>(trenutniCvor)};
            uint8_t prev = closedBits[index_trenutniCvor].fetch_or(2, std::memory_order_acq_rel);
            if (prev & 1) {
                if (!done.exchange(true, std::memory_order_acq_rel))
                    meet.store(index_trenutniCvor, std::memory_order_release);
                break;
            }
        }
      }
    }
    int m = meet.load(std::memory_order_acquire);
    if (m == -1) return {-1.f, {}};
    else {
      std::vector<int> putanja, putanja1, putanja2;
      #pragma omp parallel sections num_threads(2) shared(putanja1,putanja2)
      {
        #pragma omp section
        {
          int trenutni = m;
          int start_index = start.first * broj_kolona + start.second;

          while(trenutni != start_index) {
              putanja1.push_back(trenutni);
              trenutni = std::get<2>(obradjeniCvoroviA[trenutni]);
          }
          putanja1.push_back(start_index);
        }
        #pragma omp section
        {
          int trenutni = m;
          int finish_index = finish.first * broj_kolona + finish.second;

          while(trenutni != finish_index) {
              putanja2.push_back(trenutni);
              trenutni = std::get<2>(obradjeniCvoroviB[trenutni]);
          }
          putanja2.push_back(finish_index);
        }
      }
      std::reverse(putanja1.begin(), putanja1.end());
      putanja.insert(putanja.end(), putanja1.begin(), putanja1.end());
      if (!putanja2.empty()) {
          putanja.insert(putanja.end(), putanja2.begin() + 1, putanja2.end());
      }
      return {std::get<1>(obradjeniCvoroviA[m])+std::get<1>(obradjeniCvoroviB[m]), putanja};
    }
  }
};
int main()
{
    const int rows = 1000;
    const int cols = 1000;
    std::pair<int,int> start = {0, 0};
    std::pair<int,int> finish = {rows - 1, cols - 1};

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(1.0f, 10.0f); // težine 1–10
    std::uniform_real_distribution<float> wall(0.0f, 1.0f);  // zidovi

    std::cout << "Priprema mreze..." << std::endl;

    // 2D vector
    //std::vector<std::vector<float>> cvorovi2d(rows * cols, std::vector<float>(8));
    // flatten vector
    /*
    std::vector<float> cvorovi_flat(rows * cols * 8);

    for(int x = 0; x < rows; x++){
        for(int y = 0; y < cols; y++){
            int index_flat = (x * cols + y) * 8;
            for(int i = 0; i < 8; i++){
                float value;
                if(wall(gen) < 0.05){
                    value = std::numeric_limits<float>::infinity(); // zid
                } else {
                    value = dis(gen);
                }
                // popunjavamo oba vektora
                //cvorovi2d[x * cols + y][i] = value;
                cvorovi_flat[index_flat + i] = value;
            }
        }
    }*/

    /* Test 2D vector
    std::cout << "Test 2D vector..." << std::endl;
    Graf graf2d(cvorovi2d, rows, cols);
    auto t1 = std::chrono::high_resolution_clock::now();
    float path_cost_2d = graf2d.A_star(start, finish);
    auto t2 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> trajanje2d = t2 - t1;
    if(path_cost_2d >= 0)
        std::cout << "Put pronadjen, cijena: " << path_cost_2d << "\n";
    else
        std::cout << "Put nije pronaden!\n";
    std::cout << "Vrijeme (2D vector): " << trajanje2d.count() << " sekundi\n";*/
    std::vector<float> cvorovi_flat(rows * cols * 8,
                                    std::numeric_limits<float>::infinity());

    const int dx[8]  = { 0, -1, 0, 1, -1, -1, 1, 1 };
    const int dy[8]  = { 1,  0,-1, 0,  1, -1, 1,-1 };
    const int opp[8] = { 2,  3, 0, 1,  7,  6, 5, 4 };

    // Generiši samo "polovinu" ivica pa preslikaj na drugu stranu
    for (int x = 0; x < rows; x++) {
      for (int y = 0; y < cols; y++) {
        int u = x * cols + y;

        for (int i = 0; i < 8; i++) {
          int nx = x + dx[i];
          int ny = y + dy[i];
          if (nx < 0 || nx >= rows || ny < 0 || ny >= cols) continue;

          int v = nx * cols + ny;

          // Da ne dupliramo: kreiraj težinu samo kad je u < v
          if (u < v) {
            float w;
            if (wall(gen) < 0.05f) w = std::numeric_limits<float>::infinity();
            else                   w = dis(gen);

            cvorovi_flat[u * 8 + i]        = w;           // u -> v
            cvorovi_flat[v * 8 + opp[i]]   = w;           // v -> u (suprotni smjer)
          }
        }
      }
    }
    auto ispisiPutanju = [&](const std::vector<int>& p) {
    std::cout << "Duzina putanje: " << p.size() << " cvorova.\n";

    // Ako je putanja preduga, ispisemo samo pocetak i kraj
    if (p.size() > 20) {
        std::cout << "Putanja (prvih 5 -> ... -> zadnjih 5): ";
        for (size_t i = 0; i < 5; i++) {
            int idx = p[i];
            std::cout << "(" << idx / cols << "," << idx % cols << ") -> ";
        }
        std::cout << " ... ";
        for (size_t i = p.size() - 5; i < p.size(); i++) {
            int idx = p[i];
            std::cout << " -> (" << idx / cols << "," << idx % cols << ")";
        }
    } else {
        // Ako je kratka, ispisemo sve
        std::cout << "Putanja: ";
        for (int idx : p) {
            std::cout << "(" << idx / cols << "," << idx % cols << ") -> ";
        }
    }
    std::cout << "CILJ\n";
};
    std::cout << "Test flatten vector1..." << std::endl;
    Graf graf_flat(cvorovi_flat, rows, cols);
    auto t1 = std::chrono::high_resolution_clock::now();
    auto path_cost_flat1 = graf_flat.A_star_v3(start, finish);
    auto t2 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> trajanje_flat1 = t2 - t1;
    if(path_cost_flat1.first >= 0){
        std::cout << "Put pronadjen, cijena: " << path_cost_flat1.first << "\n";
        ispisiPutanju(path_cost_flat1.second);
    }
    else
        std::cout << "Put nije pronaden!\n";
    std::cout << "Vrijeme (flatten vector): " << trajanje_flat1.count() << " sekundi\n";

    // Test flatten vector
    std::cout << "Test flatten vector2..." << std::endl;
    auto t3 = std::chrono::high_resolution_clock::now();
    auto path_cost_flat = graf_flat.A_star_v4(start, finish);
    auto t4 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> trajanje_flat = t4 - t3;
    if(path_cost_flat.first >= 0){
        std::cout << "Put pronadjen, cijena: " << path_cost_flat.first << "\n";
        ispisiPutanju(path_cost_flat.second);
    }
    else
        std::cout << "Put nije pronaden!\n";
    std::cout << "Vrijeme (flatten vector): " << trajanje_flat.count() << " sekundi\n";
    return 0;
}
