#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <windows.h>
#include <algorithm>
#include <cstdlib>
#include <cmath>

#define INT_MAX 999999

using namespace std;

//klasa licząca czas w milisekundach
class HighResTimer
{
  LARGE_INTEGER start;
  LARGE_INTEGER stop;
  double frequency;
public:
  HighResTimer()
  {
    LARGE_INTEGER freq;
    QueryPerformanceFrequency(&freq);
    frequency = (double)freq.QuadPart;
  }
  void StartTimer()//rozpoczęcie liczenia czasu
  {
    QueryPerformanceCounter(&start);
  }
  double StopTimer()//skończenie liczenia czasu
  {
    QueryPerformanceCounter(&stop);
    return ((LONGLONG)(stop.QuadPart - start.QuadPart)*1000.0/frequency); //wyliczenie wyniku w ms
  }
};

//funkcja wyliczająca koszt przebycia drogi
int getSumWeight(vector<int> &cycle, vector<vector<int>> &graph){
    int sum=0;

    for (int i=0;i<cycle.size()-1;i++){

        sum += graph[cycle[i]][cycle[i+1]];
    }
    sum += graph[cycle[cycle.size()-1]][cycle[0]];
    return sum;
}

//funkcja generująca randomowy cykl
vector<int> generateRandomCycle(vector<vector<int>>&graph, int &n){
    int tab[n];
    for(int i =0; i<n;i++){
        tab[i] = i;
    }
    next_permutation(tab,tab+n);
    vector<int> route (tab,tab+n);
    return route;

}

//funkcja sprawdzająca czy w wektorze znajduje się już dany wierzchołek
bool checkIfExsist(vector<int> route, int i){
    for(int temp : route){
        if (temp == i ){
            return true;
        }
    }
    return false;
}

//generuje cykl metodą NearNeighbour
vector<int> generateRandomCycleNN(vector<vector<int>> &graph, int& n){
    vector<int> route;
    int j = 0, max, next = 0, temp = 0;
    while(j<n){
        max = INT_MAX;
        next = temp;
        route.push_back(next);
        for(int i = 0 ; i<n ; i++ ){
            if(!checkIfExsist(route,i) && i!=next){
                if (graph[next][i] < max) {
                    temp = i;
                    max = graph[next][i];
                }
            }
        }
        j++;
    }
    return route;
}

//funkcja obliczająca temperaturę początkową
double calculateStartTemp(vector<vector<int>>&graph, int &n){

    int cost = 0, first,second, previous_cost, actual_cost;
    vector<int> route = generateRandomCycle(graph,n);
    previous_cost = getSumWeight(route,graph);
    for ( int i = 0; i<n ;i++){
        do{
            first = rand()%n; // liczby z zakresu od 0 do n-1
            second = rand()%n;
        }while (first == second);
        iter_swap(route.begin()+first, route.begin()+second);
        actual_cost = getSumWeight(route,graph);
        cost += fabs(actual_cost - previous_cost);
    }
    cost = cost / n;
    return fabs(cost/log(0.92));
}

//oblicz prawdopodobieństwo
double calculateProbability(double &difference, double &actualTemperature){
    return exp((difference/actualTemperature));
}

//funkcja zamieniająca kolejność w łuku
void swapBows(vector<int>&route, int first, int second){
  if(first > second ){
    int temp = second;
    second = first;
    first = temp;
  }
  while (first <second){
    int temp = route[second];
    route[second] = route[first];
    route[first] = temp;
    second--;
    first++;
  }
}
//algorytm stromy (steepest)
void simulatedAnnealingSteepest(vector<vector<int>>&graph, int &n, vector<int>&solution,int &cost,int cooling, int startSolution, int startTemp) {
  double randNumber, probability, difference, actualTemperature, startTemperature, freezing = 0.999, expectedTime = 240000, expectedTemp;
  HighResTimer timer;
  vector<int> route, additionalRoute, tempR, tests, tempRoute;
  int first, second, result, temp, final, newCost, iterations = 0;
  if(n <= 15 ){ // w zależności od wielkości n wyznaczany jest czas trwania algorytmu stały
    expectedTime = 1000;
  } else if (n > 15 && n <50 ) {
    expectedTime = 120000;
  }
  if(startSolution == 1){ // różne możliwości wyboru rozwiązania początkowego
    tempRoute = generateRandomCycle(graph,n);
    tempR = tempRoute;
  } else {
    tempRoute = generateRandomCycleNN(graph,n);
    tempR = tempRoute;
  }
  if(startTemp == 1){ // różne możliwości wyboru temperatury początkowej
    actualTemperature = calculateStartTemp(graph,n);
  } else {
    actualTemperature = 1000000000;
  }
  if(cooling ==1 ){ // różne wersje chłodzenia
    expectedTemp = 0.1;
  } else {
    expectedTemp = actualTemperature/2;
  }
  cost =  getSumWeight(tempRoute, graph);
  final = cost;
  temp = cost;
  timer.StartTimer(); // początek liczenia czasu
    while (actualTemperature >= expectedTemp) {
      result = getSumWeight(tempRoute, graph);
      additionalRoute = tempRoute;
      iterations ++;
      for (int i = 0; i < 10 * n; i++) { // 10*n to liczba iteracji w epoce
        tests = additionalRoute;
        do {
          first = rand() % n; // liczby z zakresu od 0 do n-1
          second = rand() % n;
        } while (first == second);
        //iter_swap(tests.begin() + first,tests.begin() + second); // 2-k zamiana
        swapBows(tests,first,second); // zamiana łuków
        newCost = getSumWeight(tests, graph);
        difference = result - newCost;
        if (difference > 0) { // jeśli nowa ścieżka jest lepsza od poprzedniej
          if(newCost < temp){
            temp = newCost;
            tempR = tests;
          }
        }
      }
      if (temp < final){
          final = temp;
          solution = tempR;
          tempRoute = tempR;
      } else { // nie znaleźliśmy lepszego rozwiązania więc
        randNumber = ((double)rand() / RAND_MAX);
        difference = result - temp; // wuliczenie różnicy
        probability = calculateProbability(difference, actualTemperature); // obliczenie prawdopodobieństwa
        if (probability > randNumber) {
          tempRoute = tests;
        }
      }
      if(cooling == 1 ) { // chłodzenie geometryczne
        static_cast<double>(actualTemperature = actualTemperature*freezing);
      } else if (cooling == 2) { // chłodzenie boltzmana
        static_cast<double>(actualTemperature = actualTemperature/(1+log10( iterations)));
      }
      double t2 = timer.StopTimer();
      if (t2 > expectedTime) {
        break;
      }
    }
    cost = final;
}

//algorytm greedy
void simulatedAnnealingGreedy(vector<vector<int>>&graph, int &n, vector<int>&solution,int &cost, int cooling, int startSolution, int startTemp){
  double randNumber, probability, difference, actualTemperature, startTemperature, expectedTime  = 360000, freezing, expectedTemp;
  vector<int> tempRoute;
  vector<int> route,additionalRoute, tests;
  HighResTimer timer;
  int first,second, result, iterations, noChange = 0, newCost, final;
  if(n <= 15 ){
    expectedTime = 1000;
  } else if (n > 15 && n <50 ) {
    expectedTime = 120000;
  }
  if(startSolution == 1){
    tempRoute = generateRandomCycle(graph,n);
  } else {
    tempRoute = generateRandomCycleNN(graph,n);
  }
  if(startTemp == 1){
    startTemperature = calculateStartTemp(graph,n);
    freezing = 0.999;
  } else {
    startTemperature = 100000000;
    freezing = 0.999;
  }
  if(cooling ==1 ){
    expectedTemp = 0.1;
  } else {
    expectedTemp = startTemperature/2;
  }
  cost = getSumWeight(tempRoute, graph);
  final = cost;
  iterations = 0;
  actualTemperature = startTemperature;
  timer.StartTimer();
  while(actualTemperature >= expectedTemp) {
      result = getSumWeight(tempRoute, graph);
      iterations ++;
      additionalRoute = tempRoute;
      for(int i = 0 ; i<10*n; i++){ // 5 to liczba iteracji w epoce
        tests = additionalRoute;
        do{
          first = rand()%n; // liczby z zakresu od 0 do n-1
          second = rand()%n;
        }while (first == second);
        iter_swap(tests.begin()+first, tests.begin()+second); // 2-k zamiana
        newCost = getSumWeight(tests,graph);
        difference = result - newCost;
        if(difference > 0){ // jeśli nowa ścieżka jest lepsza od poprzedniej
          noChange = 0;
          if(newCost<final){
            final = newCost;
            solution = tests;
            //cout<<final<<" ";
          }
          tempRoute = tests;
          break;
        } else {
          noChange ++;
          randNumber = ((double)rand() / RAND_MAX);
          probability = calculateProbability(difference,actualTemperature);
          if (probability > randNumber){
            tempRoute = tests;
            break;
          }
        }
      }
      if(cooling == 1 ) { // chłodzenie geometryczne
      static_cast<double>(actualTemperature = actualTemperature*freezing);
      } else if (cooling == 2) { // chłodzenie boltzmana
        static_cast<double>(actualTemperature = actualTemperature/(1+log10( iterations)));
      }
      double t2 = timer.StopTimer();
      if(t2 > expectedTime){
        break;
      }
  }
  cost = final;
}

//czytanie z pliku
void readFromFile(string s, vector<vector<int>> &graph, int &n){

    int weight;
    ifstream myFile;
    myFile.open(s);
    if(myFile.is_open()){
        myFile>>n;
        for(int i=0;i<n;i++){
            graph.push_back(vector<int>());
            for(int j=0; j < n; j++){
                myFile>>weight;
                graph[i].push_back(weight);
            }
        }
    }
    else{
        cout<<"File is not read properly";
    }
    myFile.close();

}

//funkcja konwertująca wektor intów na string
string printSolution(vector<int> &solution){
    stringstream ss;
    for(int i=0;i<solution.size();i++){

        if(i != 0){
            ss << " ";
        }
        ss << solution[i];
    }
    return ss.str();

}

//funkcja czyszcząca wektory
void clearVectors(vector<vector<int>> &graph ,vector<int> &solution){

    for (int i=0;i<graph.size();i++){
        graph[i].clear();
    }
    graph.clear();
    solution.clear();
}



int main( ) {

    string line, csvName, dataFile;
    char bracket;
    int repeat, optimumCost, n, temp2;
    vector<int> checked;
    vector<int> solution;
    vector<int> route;
    vector<vector<int>> graph;
    ofstream csvFile;
    HighResTimer timer;

    ifstream myInitFile;
    myInitFile.open("initialiaze.INI");

    // wyłuskanie z ostatniej linii pliku .INI nazwy pliku wyjściowego
    while (getline(myInitFile, line)) {
        csvName = line;
    }

    myInitFile.close();
    myInitFile.open("initialiaze.INI");

    // ponowne otwarcie pliku
    if (myInitFile.is_open()) {
        while (getline(myInitFile, line)) {             // wczytanie kolejnej lini
            if (line.find(".csv") != std::string::npos) { // sprawdzenie czy linia zawiera rozszerzenie .csv
                break;
            }
            istringstream stream(line);
            stream >> dataFile >> repeat >> optimumCost >>bracket; // wczytanie nazwy instancji, ilosci powtórzeń, kosztu optymalnego
            readFromFile(dataFile,graph,n); // wypełnienie zmiennych graph oraz fullgraph danymi
            for (int i = 0; i <= n; i++) { // wczytanie ścieżki z nawiasu kwadratowego
                stream >> temp2;
                solution.push_back(temp2);
            }

            for (int j = 0; j < repeat; j++) {
                int finalCost = 0;
                vector<int> route;
                timer.StartTimer();
                //simulatedAnnealingGreedy(graph,n,solution,finalCost,1,2,1);
                simulatedAnnealingSteepest(graph,n,solution,finalCost,2,2,1);
                int t2 = timer.StopTimer(); //skończenie liczenia czasu
                if(j == 0){
                  csvFile.open(csvName,  std::ios::out |  std::ios::app);
                  csvFile<<dataFile<<","<<repeat<<","<<optimumCost<<",\n";
                  csvFile<<finalCost<<","<<t2<<"\n";
                  csvFile.close();
                }else{
                  csvFile.open(csvName,  std::ios::out |  std::ios::app);
                  csvFile<<finalCost<<","<<t2<<"\n";
                  csvFile.close();
                }
            }
            clearVectors(graph,solution);
        }
    }
    return 0;
}

