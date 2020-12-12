#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <windows.h>
#include <algorithm>
#include <cstdlib>
#include <math.h>
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

vector<int> generateRandomCycle(vector<vector<int>>&graph, int &n){
    int tab[n];
    for(int i =0; i<n;i++){
        tab[i] = i;
    }
    next_permutation(tab,tab+n);
    vector<int> route (tab,tab+n);
    return route;

}

bool checkIfExsist(vector<int> route, int i){
    for(int temp : route){
        if (temp == i ){
            return true;
        }
    }
    return false;
}

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
double calculateProbability(double &difference, double &actualTemperature){
    return exp((difference/actualTemperature));
}

vector<int> choseBestRoute(vector<vector<int>> &routes, vector<vector<int>> &graph){
  int minimumcost = INT_MAX, cost;
  vector<int> bestRoute;
  for(int i=0 ; i<routes.size(); i++){
    cost = getSumWeight(routes[i],graph);
    if (cost < minimumcost) {
      minimumcost = cost;
      bestRoute = routes[i];
    }
  }
  return bestRoute;
}
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
//algorytm stromy, chłodzenie geometryczny
void simulatedAnnealingSteepest(vector<vector<int>>&graph, int &n, vector<int>&solution,int &cost, int startTemp) {
  double randNumber, probability, difference, actualTemperature, startTemperature, freezing, expectedTime = 360000, expectedTemp = 0.1;
  HighResTimer timer;
  int first, second, result, temp;
  if(n <= 15 ){
    expectedTime = 1000;
  } else if (n > 15 && n <50 ) {
    expectedTime = 120000;
  }
  vector<int> route, additionalRoute, tempR, tests;
  int newCost;
  vector<int> tempRoute = generateRandomCycleNN(graph, n);
  cost =  getSumWeight(tempRoute, graph);
  temp = cost;
  if(startTemp == 1){
    startTemperature = calculateStartTemp(graph, n);
    freezing = 0.999;
  } else {
    startTemperature = 1000000000;
    freezing = 0.98;
  }
  timer.StartTimer();
  actualTemperature = startTemperature;
    while (actualTemperature >= expectedTemp) {
      result = getSumWeight(tempRoute, graph);
      additionalRoute = tempRoute;
      for (int i = 0; i < 10 * n; i++) { // 5 to liczba iteracji w epoce
        tests = additionalRoute;
        do {
          first = rand() % n; // liczby z zakresu od 0 do n-1
          second = rand() % n;
        } while (first == second);
        //iter_swap(additionalRoute.begin() + first,additionalRoute.begin() + second);
        swapBows(tests,first,second);
        newCost = getSumWeight(tests, graph);
        difference = result - newCost;
        if (difference > 0) { // nowa ścieżka jest lepsza od poprzedniej
          if(newCost < temp){
            temp = newCost;
            tempR = tests;
            solution = tests;
          }
        }
      }
      if (temp < result){
        tempRoute = tempR;
        solution = tempR;
        cost = temp;
      } else {
        randNumber = ((double)rand() / RAND_MAX);
        difference = result - temp;
        probability = calculateProbability(difference, actualTemperature);
        if (probability > randNumber) {
          tempRoute = tempR;
          cost = temp;
        }
      }
      static_cast<double>(actualTemperature = actualTemperature * freezing);
      double t2 = timer.StopTimer();
      if (t2 > expectedTime) {
        break;
      }
    }
}
void simulatedAnnealingGreedy(vector<vector<int>>&graph, int &n, vector<int>&solution,int &cost, int cooling, int startSolution, int startTemp){
  double randNumber, probability, difference, actualTemperature, startTemperature, expectedTime  = 360000, freezing;
  vector<int> tempRoute;
  vector<int> route,additionalRoute, tests;
  HighResTimer timer;
  int first,second, result, iterations, expectedTemp, noChange = 0 ;
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
    freezing = 0.99;
  } else {
    startTemperature = 100000000;
    freezing = 0.95;
  }
  if(cooling ==1 ){
    expectedTemp = 0.1;
  } else {
    expectedTemp = startTemperature/2;
  }
  cost = getSumWeight(tempRoute, graph);
  result = cost;
  iterations = 0;
  actualTemperature = startTemperature;
  timer.StartTimer();
  while(actualTemperature >= expectedTemp) {
      //result = getSumWeight(tempRoute, graph);
      iterations ++;
      additionalRoute = tempRoute;
      for(int i = 0 ; i<4*n; i++){ // 5 to liczba iteracji w epoce
        tests = additionalRoute;
        do{
          first = rand()%n; // liczby z zakresu od 0 do n-1
          second = rand()%n;
        }while (first == second);
        iter_swap(tests.begin()+first, tests.begin()+second);
        difference = result - getSumWeight(tests,graph);
        if(difference > 0){ //nowa ścieżka jest lepsza od poprzedniej
          tempRoute = tests;
          cost = getSumWeight(tests,graph);
          cout<<cost<<endl;
          result = cost;
          solution = tests;
          break;
        } else {
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
        static_cast<double>(actualTemperature = actualTemperature/(log10( iterations)));
      }
      double t2 = timer.StopTimer();
      if(t2 > expectedTime){
        break;
      }
  }
}

//wymiana łuków jako sposób wyboru rozwiązania w sąsiedztwie z chłodzeniem geo
void simulatedAnnealingSwappingBows(vector<vector<int>>&graph, int &n, vector<int>&solution,int &cost, int startSolution){
  double randNumber, probability, difference, actualTemperature;
  double expectedTime = 360000;
  int first,second, result;
  vector<int> route,additionalRoute, tempRoute;
  boolean time = false;
  HighResTimer timer;
  if(n <= 15 ){
    expectedTime = 1000;
  } else if (n > 15 && n <50 ) {
    expectedTime = 120000;
  }
  if(startSolution == 1){
    tempRoute = generateRandomCycle(graph,n);
    result = getSumWeight(tempRoute,graph);
  } else {
    tempRoute = generateRandomCycleNN(graph,n);
    result = getSumWeight(tempRoute,graph);
    cost = result;
  }
  timer.StartTimer();
  while(time == false){
    actualTemperature = calculateStartTemp(graph,n);
    //actualTemperature = 1000000000;
      while(actualTemperature >= 0.1) {
        additionalRoute = tempRoute;
        for(int i = 0 ; i<3*n; i++){ // 5 to liczba iteracji w epoce
          do{
            first = rand()%n; // liczby z zakresu od 0 do n-1
            second = rand()%n;
          }while (first == second);
          swapBows(additionalRoute, first, second);
          difference = result - getSumWeight(additionalRoute,graph);
          if(difference > 0){ //nowa ścieżka jest lepsza od poprzedniej
            tempRoute = additionalRoute;
            cost = getSumWeight(additionalRoute,graph);
            cout<<cost<<endl;
            result = cost;
            solution = additionalRoute;
            break;
          } else {
            randNumber = ((double)rand() / RAND_MAX);
            probability = calculateProbability(difference,actualTemperature);
            if (probability > randNumber){
              tempRoute = additionalRoute;
              break;
            }
          }
        }
        static_cast<double>(actualTemperature = actualTemperature*0.99);
        double t2 = timer.StopTimer();
        if(t2 > expectedTime){
          time = true;
          break;
        }
      }
      time = true;
    }
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
                //timer.StartTimer();
                //simulatedAnnealingGreedy(graph, n, solution,finalCost, 1, 2, 1 );
                //simulatedAnnealingGreedy(graph,n,solution,finalCost,1,2,2);
               simulatedAnnealingSteepest(graph,n,solution,finalCost,1);
                //simulatedAnnealingSwappingBows(graph,n,solution,finalCost,2);
                cout<<"final wynik"<<finalCost;
                //double t2 = timer.StopTimer(); //skończenie liczenia czasu
                /*if(j == 0){
                  csvFile.open(csvName,  std::ios::out |  std::ios::app);
                  csvFile<<dataFile<<","<<repeat<<","<<t2<<","<<printSolution(route)<<",\n";
                  csvFile<<finalCost<<"\n";
                  csvFile.close();
                }else{
                  csvFile.open(csvName,  std::ios::out |  std::ios::app);
                  csvFile<<finalCost<<"\n";
                  csvFile.close();
                }*/
            }
            clearVectors(graph,solution);
        }
    }
    return 0;
}

