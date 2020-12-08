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
    return fabs(cost/log(0.89));
}
double calculateProbability(double &difference, double &actualTemperature){
    return exp((difference/actualTemperature));
}

double changeTemperature(double &actual){
    return actual*0,99;
}

void simulatedAnnealing(vector<vector<int>>&graph, int &n, vector<int>&solution,int &cost, int cooling){
    double randNumber, probability, difference, actualTemperature;
    double expectedTime = 120000;
    boolean time = false;
    HighResTimer timer;
    int first,second, result;
    vector<int> route,additionalRoute;
    timer.StartTimer();
    while(time == false){
      double t2 = timer.StopTimer();
      if(t2 > expectedTime){
        break;
      } else {
        vector<int> tempRoute = generateRandomCycle(graph,n);
        result = getSumWeight(tempRoute,graph);
        actualTemperature = calculateStartTemp(graph,n);
        while(actualTemperature >= 0.001) {
          additionalRoute = tempRoute;
          for(int i = 0 ; i<100; i++){ // 5 to liczba iteracji w epoce
            do{
              first = rand()%n; // liczby z zakresu od 0 do n-1
              second = rand()%n;
            }while (first == second);

            iter_swap(additionalRoute.begin()+first, additionalRoute.begin()+second);
            difference = result - getSumWeight(additionalRoute,graph);
            if(difference > 0){ //nowa ścieżka jest lepsza od poprzedniej
              tempRoute = additionalRoute;
              cost = getSumWeight(additionalRoute,graph);
              result = cost;
              solution = additionalRoute;
              break;
            } else {
              randNumber = ((double)rand() / RAND_MAX) + 1;
              probability = calculateProbability(difference,actualTemperature);
              if (probability > randNumber){
                tempRoute = additionalRoute;
                break;
              }
            }
          }
          if(cooling == 1 ) { // chłodzenie geometryczne
            static_cast<double>(actualTemperature = actualTemperature*0.99);
          } else if (cooling == 2) { // chłodzenie boltzmana
            static_cast<double>(actualTemperature = actualTemperature/ (1 + log10( 1.38 * pow(10,-23) )));
            cout<<actualTemperature;
          }
        }
      }
    }
    cout <<cost;
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
    cout<<route[second]<<" ";
    route[first] = temp;
    second--;
    first++;
  }cout<<endl<<endl;
  for (int i:route){
    cout<<i<<" ";
  }
}

void simulatedAnnealingSwappingBows(vector<vector<int>>&graph, int &n, vector<int>&solution,int &cost){
  double randNumber, probability, difference, actualTemperature;
  double expectedTime = 120000;
  boolean time = false;
  HighResTimer timer;
  int first,second, result;
  vector<int> route,additionalRoute;
  timer.StartTimer();
  while(time == false){
    double t2 = timer.StopTimer();
    if(t2 > expectedTime){
      break;
    } else {
      vector<int> tempRoute = generateRandomCycle(graph,n);
      result = getSumWeight(tempRoute,graph);
      actualTemperature = calculateStartTemp(graph,n);
      while(actualTemperature >= 0.001) {
        additionalRoute = tempRoute;
        for(int i = 0 ; i<100; i++){ // 5 to liczba iteracji w epoce
          do{
            first = rand()%n; // liczby z zakresu od 0 do n-1
            second = rand()%n;
          }while (first == second);

          swapBows(additionalRoute, first, second);
          difference = result - getSumWeight(additionalRoute,graph);
          if(difference > 0){ //nowa ścieżka jest lepsza od poprzedniej
            tempRoute = additionalRoute;
            cost = getSumWeight(additionalRoute,graph);
            result = cost;
            solution = additionalRoute;
            break;
          } else {
            randNumber = ((double)rand() / RAND_MAX) + 1;
            probability = calculateProbability(difference,actualTemperature);
            if (probability > randNumber){
              tempRoute = additionalRoute;
              break;
            }
          }
        }
          static_cast<double>(actualTemperature = actualTemperature*0.99);
      }
    }
  }
  cout <<cost;
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
                //timer.StartTimer()
                //simulatedAnnealing(graph, n, solution,finalCost,1);
                vector<int> r {1,2,3,4,5,32,1,2};
                swapBows(r,3,7);
                //cout<<"final wynik"<<finalCost;
                //double t2 = timer.StopTimer(); //skończenie liczenia czasu
                /*if(j == 0){
                  csvFile.open(csvName,  std::ios::out |  std::ios::app);
                  csvFile<<dataFile<<","<<repeat<<","<<finalCost<<","<<printSolution(route)<<",\n";
                  csvFile<<t2<<"\n";
                  csvFile.close();
                }else{
                  csvFile.open(csvName,  std::ios::out |  std::ios::app);
                  csvFile<<t2<<"\n";
                  csvFile<<printSolution(route)<<",\n";
                  csvFile.close();
                }*/
            }
            clearVectors(graph,solution);
        }
    }
    return 0;
}

