#include <iostream>
#include <cmath>
#include <fstream>
#include <stdio.h>

#define PRINTER(name) printer(#name, (name))
//using namespace std;

// Mutex for critical sections (Exclude other threads from accessing a function until the first is finnished)
std::mutex mtx;

double get_distance(double x1, double y1, double x2, double y2)
{
    double distance = sqrt(pow((x2 - x1), 2) + pow((y2 - y1), 2));

    return distance;
}

std::string printer(char *name, int value)
{
    return name;
}

int timerPause(double initialTime, int timeInSeconds, std::string name) {

    //// Compute solution time
    
    std::time_t TimeF = std::time(nullptr);
    if (TimeF - initialTime > timeInSeconds)
    {
        std::cout << "The algorithm stopped at: " << name << std::endl;
        std::system("pause");
    }

    return 0;
}

// Input data file into a Matrix
std::vector<std::vector<double>> csvIN(){
    // Variables
    std::ifstream iFile; // input file
    std::vector<std::vector<double>> ndata;
    int i = 0, j = 0, k = 0, column = 0;

    // Open  input file
    iFile.open("data.csv");
    if (!iFile.is_open()){
        std::cout << "Error opening input file!\n";
        return ndata;
    }

    std::string line;
    int lineCount = 0;

    // Read the file line by line
    while (std::getline(iFile, line)) {
        if (!line.empty()) {
            lineCount++; // Increment line count for each line read
        }
    }
    // Reset the file pointer to the beginning
    iFile.clear(); // Clear the end-of-file flag
    iFile.seekg(0, std::ios::beg); // Move the pointer back to the beginning of the file

    std::vector<std::vector<double>> data(lineCount - 1, std::vector<double>(7, 0)); // First line has headers

    // Read from input file
    while (iFile.peek() != EOF)
    {
        k++;
        column++;
        std::string i_fromFile;
        // Write it to an output file
        if (column < 7)
        {
            getline(iFile, i_fromFile, ',');
        }else
        {
            getline(iFile, i_fromFile, '\n');
            column = 0;
        }
                
        if (k>7)
        {
            data[i][j] = stod(i_fromFile);
            j++;
        }
        if (j==7)
        {
            j = 0;
            i++;
        }
        
    }
    
    // Close input and output files
    iFile.close();

    return data;
}


// ----------------------------------------------------------------
// Input data file to Vector (to test with a vector)
// ----------------------------------------------------------------
int csvInVector(){
    // Variables
    std::ifstream iFile; // input file
    std::string data[101][7]{};
    int i = 0, j = 0, k = 0, column = 0;

    // Open  input file
    iFile.open("data.csv");
    if (!iFile.is_open()){
        std::cout << "Error opening input file!\n";
        return 0;
    }

    // Read from input file
    while (iFile.peek() != EOF)
    {
        k++;
        column++;
        std::string i_fromFile;
        // Write it to an output file
        if (column < 7)
        {
            getline(iFile, i_fromFile, ',');
        }else
        {
            getline(iFile, i_fromFile, '\n');
            column = 0;
        }
                
        if (k>7)
        {
            data[i][j] = i_fromFile;
            j++;
        }
        if (j==7)
        {
            j = 0;
            i++;
        }
        
    }

    for (int cont = 0; cont < 7; cont++)
    {
        std::cout << data[57][cont] << " - ";
    }
    
    
    // Close input and output files
    iFile.close();

    return 0;
}


// Output results to a csv file
int csvOUT(){
    // Variables
    std::ofstream oFile; // output file

    // Open  onput file
    oFile.open("out.csv");
    if (!oFile.is_open()){
        std::cout << "Error opening output file!\n";
        return 0;
    }
    
    // Close output files
    oFile.close();

    return 0;
}

// ----------------------------------------------------------------
// Calculate permutations
// ----------------------------------------------------------------
int permFunct(int n, int r)
{
    long long fact=1, numerator, denominator;
    int perm, i=1, sub;

    while(i<=n)
    {
        fact = i*fact;
        i++;
    }
    numerator = fact;                    // n!
    sub = n-r;
    fact = 1;
    i = 1;
    while(i<=sub)
    {
        fact = i*fact;
        i++;
    }
    denominator = fact;                // (n-r)!
    perm = numerator/denominator;

    return perm;
}

void optimizeRoute(int vehIterator, std::vector<int>& PosY, std::vector<double>& objective,
                    std::vector<std::vector<int>>& routeCopy, std::vector<std::vector<int>>& route,
                    std::vector<std::vector<double>>& Dist){
 
    std::lock_guard<std::mutex> lock(mtx);
 
    int maxLSiterations{};
    long double LS_Iter{};
 
    if (PosY[vehIterator] < 15)
    {
        maxLSiterations = permFunct(PosY[vehIterator], PosY[vehIterator]);
    }else
    {
        maxLSiterations = PosY[vehIterator] * 10;
    }
 
    do
    {
        double bestTest = objective[vehIterator];
        int test_pos1{}, test_pos2{};
        bool sw2 = false, sw3 = false;
 
        // Reset the copy for each iteration to make changes on it
        for (int j = 0; j < PosY[vehIterator] + 1; j++)
        {
            routeCopy[vehIterator][j] = route[vehIterator][j];
        }
 
        // Modify the copy of the route & evaluate if improved
        for (int i = 1; i < PosY[vehIterator]; i++)
        {
            int tmp = routeCopy[vehIterator][2];
            routeCopy[vehIterator][2] = routeCopy[vehIterator][i];
            routeCopy[vehIterator][i] = tmp;
 
            for (int j = 1; j < PosY[vehIterator] - 1; j++)
            {
                for (int k = j + 1; k < PosY[vehIterator]; k++)
                {
                    for (int l = j; l < k + 1; l++)
                    {
                        routeCopy[vehIterator][l] = route[vehIterator][k - l + j];
                    }
 
                    double testObjective{};
                    for (int n = 0; n < PosY[vehIterator]; n++)
                    {
                        testObjective += Dist[routeCopy[vehIterator][n]][routeCopy[vehIterator][n+1]];
                    }
 
                    if (testObjective < objective[vehIterator] && testObjective < bestTest)
                    {
                        bestTest = testObjective;
                        test_pos1 = j;
                        test_pos2 = k;
                        sw2 = true;
                        sw3 = true;
                    }
                    for (int m = 0; m < PosY[vehIterator] + 1; m++)
                    {
                        routeCopy[vehIterator][m] = route[vehIterator][m];
                    }
 
                }
 
            }
 
        }
 
        // Update route when improved
 
        if (sw2)
        {
            // Save new distance and make new copy of the improved route
 
            objective[vehIterator] = bestTest;
 
            for (int l = test_pos1; l < test_pos2 + 1; l++)
            {
                routeCopy[vehIterator][l] = route[vehIterator][test_pos2 - l + test_pos1];
            }
 
            for (int l = 0; l < PosY[vehIterator] + 1; l++)
            {
                route[vehIterator][l] = routeCopy[vehIterator][l];
            }
            sw2 = false;
 
        }
        if (sw3)
        {
            LS_Iter++;
            sw3 = false;
        }else
        {
            LS_Iter = 1E+17;
        }
 
    } while (LS_Iter < maxLSiterations);
 
    objective[vehIterator] = 0;
    for (int j = 0; j < PosY[vehIterator]; j++)
    {
        objective[vehIterator] += Dist[route[vehIterator][j]][route[vehIterator][j+1]];
    }
 
}


// ----------------------------------------------------------------