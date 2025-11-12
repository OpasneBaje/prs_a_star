#include <iostream>
#include <queue>
#include <vector>
#include <functional>
#include <math.h>
#include <chrono>
#include <random>
//std::numeric_limits<float>::infinity();

class Graf {
private:
  const std::vector<std::vector<float>> &cvorovi;
  const std::vector<float>&cvorovi_v2;
  int broj_redova, broj_kolona;

  static float Heuristika(int current_x, int current_y, int goal_x, int goal_y, float w_min) {
    int dx = std::abs(goal_x - current_x);
    int dy = std::abs(goal_y - current_y);
    return w_min * std::sqrt(dx * dx + dy * dy);
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
  //Bolji hit rate i manje ubacivanje u heap
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
};
int main()
{
  /*
  const int rows = 10;
  const int cols = 10;

  std::vector<std::vector<float>> cvorovi(rows*cols, std::vector<float>(8, 1.0f));

  cvorovi[21][0] = 100.0f;
  cvorovi[21][1] = 150.0f;
  cvorovi[21][2] = 150.0f;
  cvorovi[21][3] = 150.0f;
  cvorovi[21][4] = 150.0f;
  cvorovi[21][5] = 150.0f;
  cvorovi[21][6] = 150.0f;
  cvorovi[21][7] = 150.0f;

  std::pair<int,int> start = {2,1};
  std::pair<int,int> finish = {7,1};

  Graf graf(cvorovi, rows, cols);
  float path_cost = graf.A_star(start, finish);

  if(path_cost >= 0)
      std::cout << "Najkraci put od (" << start.first << "," << start.second << ") do ("
                << finish.first << "," << finish.second << ") ima cijenu: " << path_cost << "\n";

    std::vector<std::vector<float>> cvorovi(rows * cols, std::vector<float>(8, 1.0f));
    std::pair<int,int> start = {0, 0};
    std::pair<int,int> finish = {9999, 9999};
    Graf graf(cvorovi, rows, cols);

    auto t1 = std::chrono::high_resolution_clock::now();
    int path_cost = graf.A_star(start, finish);
    auto t2 = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> trajanje = t2 - t1;

    if (path_cost >= 0)
        std::cout << "Najkraci put ima cijenu: " << path_cost << "\n";
    else
        std::cout << "Put nije pronadjen!\n";

    std::cout << "Vrijeme izvrsavanja: " << trajanje.count() << " sekundi\n";



    std::vector<float> cvorovi_v2(rows * cols * 8, 1.0f); // sve težine 1.0f

    std::pair<int,int> start = {0, 0};
    std::pair<int,int> finish = {rows - 1, cols - 1};

    // Napravimo graf koristeći flattenovani vektor
    Graf graf(cvorovi_v2, rows, cols);

    auto t1 = std::chrono::high_resolution_clock::now();
    float path_cost = graf.A_star_v2(start, finish); // koristi flattenovani vektor
    auto t2 = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> trajanje = t2 - t1;

    if (path_cost >= 0)
        std::cout << "Najkraci put ima cijenu: " << path_cost << "\n";
    else
        std::cout << "Put nije pronadjen!\n";

    std::cout << "Vrijeme izvrsavanja: " << trajanje.count() << " sekundi\n";

    const int rows = 10000;
    const int cols = 10000;
    std::pair<int,int> start = {0, 0};
    std::pair<int,int> finish = {rows - 1, cols - 1};
    {
      std::cout << "Priprema 2D vektora..." << std::endl;
      std::vector<std::vector<float>> cvorovi2d(rows * cols, std::vector<float>(8, 1.0f));

      std::cout << "Test sa 2D vektorom..." << std::endl;
      Graf graf2d(cvorovi2d, rows, cols);
      auto t1 = std::chrono::high_resolution_clock::now();
      float path_cost_2d = graf2d.A_star(start, finish);
      auto t2 = std::chrono::high_resolution_clock::now();
      std::chrono::duration<double> trajanje2d = t2 - t1;
      std::cout << "Vrijeme (2D vector): " << trajanje2d.count() << " sekundi\n";
    }
    {
      std::cout << "Priprema flattenovanog vektora..." << std::endl;
      std::vector<float> cvorovi_flat(rows * cols * 8, 1.0f);

      std::cout << "Test sa flattenovanim vektorom..." << std::endl;
      Graf graf_flat(cvorovi_flat, rows, cols);
      auto t3 = std::chrono::high_resolution_clock::now();
      float path_cost_flat = graf_flat.A_star_v2(start, finish);
      auto t4 = std::chrono::high_resolution_clock::now();
      std::chrono::duration<double> trajanje_flat = t4 - t3;
      std::cout << "Vrijeme (flatten vector): " << trajanje_flat.count() << " sekundi\n";
    }*/

    const int rows = 12000;
    const int cols = 12000;
    std::pair<int,int> start = {0, 0};
    std::pair<int,int> finish = {rows - 1, cols - 1};

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(1.0f, 10.0f); // težine 1–10
    std::uniform_real_distribution<float> wall(0.0f, 1.0f);  // zidovi

    std::cout << "Priprema mreze..." << std::endl;

    // 2D vector
    std::vector<std::vector<float>> cvorovi2d(rows * cols, std::vector<float>(8));
    // flatten vector
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
                cvorovi2d[x * cols + y][i] = value;
                cvorovi_flat[index_flat + i] = value;
            }
        }
    }

    // Test 2D vector
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
    std::cout << "Vrijeme (2D vector): " << trajanje2d.count() << " sekundi\n";

    // Test flatten vector
    std::cout << "Test flatten vector..." << std::endl;
    Graf graf_flat(cvorovi_flat, rows, cols);
    auto t3 = std::chrono::high_resolution_clock::now();
    float path_cost_flat = graf_flat.A_star_v2(start, finish);
    auto t4 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> trajanje_flat = t4 - t3;
    if(path_cost_flat >= 0)
        std::cout << "Put pronadjen, cijena: " << path_cost_flat << "\n";
    else
        std::cout << "Put nije pronaden!\n";
    std::cout << "Vrijeme (flatten vector): " << trajanje_flat.count() << " sekundi\n";
    return 0;
}
