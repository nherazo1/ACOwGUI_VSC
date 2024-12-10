#include <iostream>
#include <ctime>
#include "Functions.h"
#include <random>
#include <chrono>
#include <cmath>
#include "pybind11/pybind11.h"
#include "pybind11/embed.h"
#include <stdio.h>
#include <pybind11/stl.h>
#include <vector>

std::string results = "These are your results: \n\n";
std::string *resultsPanel = &results;

struct Vector2D {
    std::vector<std::vector<int>> bestRoute_f;
};
Vector2D f_solution {};

void ShowResults(const std::string &newText) {
    std::string *currentText = resultsPanel;
    *currentText += newText; // Append new text with a newline
    resultsPanel  = currentText;

}

long ConsoleInput(){
    long number; // Declare a variable of type long  

    std::cout << "Please enter a number: "; // Prompt the user for input  
    std::cin >> number; // Read input from the console into the variable  

    // Check if the input was successful  
    if (std::cin.fail()) {  
        std::cout << "Invalid input. Please enter a valid long integer." << std::endl;  
    } else {  
        std::cout << "You entered: " << number << std::endl; // Display the entered number  
    }  

    return number; 
}

std::string ACOfunct() {
    /*
    This is the acoModule ACO algorithm
    */

    /* pybind11::scoped_interpreter guard{};  */

    auto data_entry_module = pybind11::module::import("data_entry_");
    /* auto ACO_GUI_module = pybind11::module::import("ACO_GUI");
    auto p_results_function = ACO_GUI_module.attr("m.print_results"); */
    long getVehFleet = data_entry_module.attr("launch_app")().cast<long>();

    ////-------------------------------------------------------------------
    //  Load data
    ////-------------------------------------------------------------------

    // Parameters and variables initialization
    int Iter = 0, maxIterations = 1000;
    double RunTime;
    int MaxRunTime = 1200;
    std::time_t TimeI = std::time(nullptr);
    long double ObjDistGlobal = 20000000000;
    const long Arg = 10;
    long NumVeh;
    //std::cout << "Please enter the number of vehicles as an integer below: \n";
    //NumVeh = ConsoleInput();
    NumVeh = getVehFleet;

    //Pheromone parameters
    const double T0 = 0.85, B = 2.3, q0 = 0.85, a = 0.1;
    std::vector<std::vector<double>> InstanceData;
    InstanceData = csvIN();
    //----------------------------------------------------------------
    // Initialize pheromones
    //----------------------------------------------------------------

    long NumCustomers = InstanceData.size();

/*    for (int item = 0; item < *InstanceData[0]; item++){
        NumCustomers++;
    }*/

    std::vector<std::vector<double>> Pherom(NumCustomers, std::vector<double>(NumCustomers)), ArgMax(NumCustomers, std::vector<double>(NumCustomers));

    for (int i = 0; i < NumCustomers; i++){
        for (int j = 0; j < NumCustomers; j++){
            if (i!=j)
            {
                Pherom[i][j] = T0;
            }
            else
            {
                Pherom[i][j] = 0;
            }
        }
    }

    //----------------------------------------------------------------
    // Create and populate the distance data structure
    //----------------------------------------------------------------

    std::vector<std::vector<double>>  Dist(NumCustomers, std::vector<double>(NumCustomers));


    for (int i = 0; i < NumCustomers; i++)
    {
        for (int j = 0; j < NumCustomers; j++)
        {
            Dist[i][j] = get_distance(InstanceData[i][0], InstanceData[i][1], InstanceData[j][0], InstanceData[j][1]);
        }
    }

    for (int i = 0; i < NumCustomers; i++){
        for (int j = 0; j < NumCustomers; j++){
            if (Dist[i][j] == 0)
            {
                ArgMax[i][j] = 0;
            }
            else
            {
                ArgMax[i][j] = Arg * Pherom[i][j] * pow((1 / Dist[i][j]), B);
            }
        }
    }

    // Get customers' demands
    std::vector<double> CapN(NumCustomers);
    //memset(CapN, 0, NumCustomers*sizeof(double));
    for (int i = 0; i < NumCustomers; i++)
    {
        CapN[i] = InstanceData[i][2];
    }

    ////----------------------------------------------------------------
    //// Start ACO iterations
    ////----------------------------------------------------------------
    int nodeN, currentRoutePos, nodeC, excessVehicle;
    int currentVehicle, currentNumCustomers, numVehAssig, VehsUsed, h, s;
    std::vector<int> PosY(NumVeh), bestPosY(NumVeh);
    double best, CapF, ExcessCapacity;
    std::vector<std::vector<int>> bestRoute(NumVeh, std::vector<int> (NumCustomers + 1));
    bool CapOverflowSW = false;

    std::vector<long> Cap(NumVeh, 0);
    std::vector<long> CapCopy(NumVeh, 0);
    std::string CapList;

    for (long x = 0; x < Cap.size(); x++) {
        /*std::cout << "Insert the capacity of Vehicle " + std::to_string(x + 1) + " here\n";
        std::cin >> Cap[x]; //getVehFleet*/
        auto cap_data_entry_module = pybind11::module::import("data_entry");
        long getVehCap = cap_data_entry_module.attr("launch_app2")(x + 1).cast<long>();
        long VehCapacity = getVehCap;
        Cap[x] = VehCapacity;
        CapList += "V" + std::to_string(x + 1) + " - " + std::to_string(Cap[x]) + ", ";
    }

    CapCopy = Cap;

    do {
        // Iteration parameters
        currentVehicle = 0, currentNumCustomers = 0;
        std::vector<std::vector<int>> route(NumVeh, std::vector<int>(NumCustomers + 1));
        bool CapOverflow = false;
        //Create a list to track unallocated nodes
        std::vector<int> UnAllocNodes(NumCustomers - 1);
        std::vector<double> UnAllocCaps(NumCustomers - 1);
        double RemCap;
        double minCap;
        int start = 1;
        std::iota(UnAllocNodes.begin(), UnAllocNodes.end(), start);

        Cap = CapCopy;
        
        // Rank vehicle capacities
        std::sort(Cap.begin(), Cap.end(), std::greater<long>()); //std::greater()
        // End vehicles ordering

        // Assign customers to vehicle routes until all costumers assigned
        do
        {
            //timerPause(TimeIq, 120, PRINTER(TimeIq)); // this was called to stop process if is taking too long
            route[currentVehicle][0] = 0;
            nodeN = route[currentVehicle][0];
            currentRoutePos = 1;
            CapF = 0;
            int capOptCount = 0;
            int capOptLimit = 15;
            // std::time_t TimeIcap = std::time(nullptr);
            ///--------------------------------------------------------
            /// Random number generator
            std::mt19937_64 rng1;
            // initialize the random number generator with time-dependent seed
            uint64_t timeSeed1 = std::chrono::high_resolution_clock::now().time_since_epoch().count();
            std::seed_seq ss1{uint32_t(timeSeed1 & 0xffffffff), uint32_t(timeSeed1>>32)};
            rng1.seed(ss1);
            // initialize a uniform distribution between 0 and 1
            std::uniform_real_distribution<double> unif(0, 1);
            ///--------------------------------------------------------

            // Add customers until vehicle capacity is exhausted
            do
            {
                // Evaluate random parameter and add customer to vehicle route
                // either randomly or highest ArgMax (highest feromone)
                //timerPause(TimeIcap, 120, PRINTER(TimeIcap)); // this was called to stop process if is taking too long
                double currentRandomNumber = unif(rng1);
                if (currentRandomNumber < q0 || currentNumCustomers > NumCustomers - 2)
                {
                    best = ArgMax[nodeN][nodeN];
                    for (int k = 0; k < NumCustomers; k++) // look for the best next node for route
                    {
                        if (best < ArgMax[nodeN][k]) // if current node is better than current best
                        {
                            int cont = 0;

                            for (const auto& row : route)
                            {
                                // Check if current node is not repeated in the route
                                auto it = std::find(row.begin(), row.end(),k);

                                // If element is found, flag a node found
                                if (it != row.end()) {
                                    cont = 1;
                                }

                            }

                            if (cont == 0) // Select current best if it is not repeated in the route
                            {
                                best = ArgMax[nodeN][k];
                                nodeC = k;
                            }


                        }

                    }

                    nodeN = nodeC;
                    CapF += CapN[nodeC];

                    if (currentNumCustomers < NumCustomers - 1) // If there are customers left to assign
                    {
                        if (CapF < Cap[currentVehicle]) // Add selected to route if vehicle capacities not exhausted yet
                        {
                            route[currentVehicle][currentRoutePos] = nodeC;
                            currentNumCustomers++;
                            if (currentRoutePos < NumCustomers - 1 && CapF < Cap[currentVehicle])
                            {
                                currentRoutePos++;
                            }
                            // Remove nodeC from UnAllocatedList using erase function and iterators
                            auto it = std::find(UnAllocNodes.begin(), UnAllocNodes.end(),
                                                nodeC);

                            // If element is found, erase it
                            if (it != UnAllocNodes.end()) {
                                UnAllocNodes.erase(it);
                            }

                        }else if (currentVehicle == NumVeh - 1) // Still customers left but not enough capacity in vehicles
                        {
                            Cap[currentVehicle] = 20000000;
                            route[currentVehicle][currentRoutePos] = nodeC;
                            currentNumCustomers++;
                            if (currentRoutePos < NumCustomers - 1 && CapF < Cap[currentVehicle])
                            {
                                currentRoutePos++;
                            }
                            // Remove nodeC from UnAllocatedList using erase function and iterators
                            auto it = std::find(UnAllocNodes.begin(), UnAllocNodes.end(),
                                                nodeC);

                            // If element is found, erase it
                            if (it != UnAllocNodes.end()) {
                                UnAllocNodes.erase(it);
                            }
                            CapOverflow = true;
                        }else if (capOptCount < capOptLimit){
                            CapF -= CapN[nodeC];
                            capOptCount++;
                        }
                    }else
                    {
                        CapF = Cap[currentVehicle] * 1000;
                    }


                }else
                {

                    h = 0;
                    //std::time_t TimeIrand = std::time(nullptr);
                    ///--------------------------------------------------------
                    /// Random number generator
                    std::mt19937_64 rng2;
                    // initialize the random number generator with time-dependent seed
                    uint64_t timeSeed2 = std::chrono::high_resolution_clock::now().time_since_epoch().count();
                    std::seed_seq ss2{uint32_t(timeSeed2 & 0xffffffff), uint32_t(timeSeed2>>32)};
                    rng2.seed(ss2);
                    // initialize a uniform distribution between 1 and NumCustomers
                    std::uniform_real_distribution<double> unif(0, NumCustomers);
                    ///--------------------------------------------------------

                    do
                    {
                        //timerPause(TimeIrand, 120, PRINTER(TimeIrand)); // this was called to stop process if is taking too long
                        int currentRandomNumber = unif(rng2);
                        s = currentRandomNumber;
                        int cont = 0;

                        // Check the node is not repeated in the route
                        for (const auto& row : route)
                        {
                            // Check if current node is not repeated in the route
                            auto it = std::find(row.begin(), row.end(),s);

                            // If element is found, flag a node found
                            if (it != row.end()) {
                                cont = 1;
                            }

                        }
                        if (currentNumCustomers < NumCustomers - 1)
                        {
                            if (cont == 0)
                            {
                                nodeC = s;
                                h = 1;
                                nodeN = nodeC;
                                CapF += CapN[nodeC];
                                if (currentNumCustomers < NumCustomers - 1)
                                {
                                    if (CapF < Cap[currentVehicle])
                                    {
                                        route[currentVehicle][currentRoutePos] = nodeC;
                                        currentNumCustomers++;
                                        if (currentRoutePos < NumCustomers - 1 && CapF < Cap[currentVehicle])
                                        {
                                            currentRoutePos++;
                                        }
                                        // Remove nodeC from UnAllocatedList using erase function and iterators
                                        auto it = std::find(UnAllocNodes.begin(), UnAllocNodes.end(),
                                                            nodeC);

                                        // If element is found, erase it
                                        if (it != UnAllocNodes.end()) {
                                            UnAllocNodes.erase(it);
                                        }

                                    }else if (currentVehicle == NumVeh - 1) // Still customers left but not enough capacity in vehicles
                                    {
                                        Cap[currentVehicle] = 20000000;
                                        route[currentVehicle][currentRoutePos] = nodeC;
                                        currentNumCustomers++;
                                        if (currentRoutePos < NumCustomers - 1 && CapF < Cap[currentVehicle])
                                        {
                                            currentRoutePos++;
                                        }
                                        // Remove nodeC from UnAllocatedList using erase function and iterators
                                        auto it = std::find(UnAllocNodes.begin(), UnAllocNodes.end(),
                                                            nodeC);

                                        // If element is found, erase it
                                        if (it != UnAllocNodes.end()) {
                                            UnAllocNodes.erase(it);
                                        }
                                        CapOverflow = true;
                                    }else if (capOptCount < capOptLimit){
                                        CapF -= CapN[nodeC];
                                        capOptCount++;
                                    }
                                }else
                                {
                                    CapF = Cap[currentVehicle] * 1000;
                                }

                            }

                        }else
                        {
                            CapF = Cap[currentVehicle] * 1000;
                            h = 1;
                        }
                    } while (h < 1);
                }

                UnAllocCaps.clear();
                double assignedCap = 0;
                for (int pos: UnAllocNodes) {
                    UnAllocCaps.push_back(CapN[pos]);
                }
                for (int item: route[currentVehicle]) {
                    assignedCap += CapN[item];
                }
                if (capOptCount < capOptLimit && !CapOverflow){
                    RemCap = Cap[currentVehicle] - assignedCap;
                }else{
                    RemCap = 0;
                }

                if (UnAllocCaps.empty()){
                    minCap = RemCap * 1000;
                }else{
                    minCap = *std::min_element(UnAllocCaps.begin(), UnAllocCaps.end());
                }

            } while (CapF < Cap[currentVehicle] |  RemCap > minCap); // Review subtracting CapF!!!!

            route[currentVehicle][currentRoutePos] = route[currentVehicle][0];
            PosY[currentVehicle] = currentRoutePos;
            numVehAssig = currentVehicle;
            if (currentVehicle < NumVeh - 1)
            {
                currentVehicle = currentVehicle + 1;
            }


        } while (currentNumCustomers < NumCustomers - 1);

        ////----------------------------------------------------------------
        //// Start Local Search Algorithm
        ////----------------------------------------------------------------

        // Initialize parameters
        std::vector<double> objective(NumVeh);
        std::vector<std::vector<int>> routeCopy(NumVeh, std::vector<int>(NumCustomers + 1, 0));

        // Compute current route distance & copy route
        for (int i = 0; i < numVehAssig + 1; i++)
        {
            for (int j = 0; j < PosY[i]; j++)
            {
                objective[i] += Dist[route[i][j]][route[i][j+1]];
            }
            //routeCopy[i][PosY[i]] = 0;

        }
        routeCopy = route;

        // DO until all vehicles are optimized
        int vehIterator = 0;
        do
        {
            // Start iterations
            int maxLSiterations{};
            long double LS_Iter{};

            if (PosY[vehIterator] < 10)
            {
                maxLSiterations = permFunct(PosY[vehIterator], PosY[vehIterator]);
            }else
            {
                maxLSiterations = PosY[vehIterator] * 3;
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
            vehIterator += 1;

        } while (vehIterator < numVehAssig + 1);

        ////----------------------------------------------------------------
        //// End of local search procedure
        ////----------------------------------------------------------------


        ////---------------------------------------------------------------------------------------
        //// Update pheromone matrix, then update pheromone trail if global objective is improved
        ////---------------------------------------------------------------------------------------

        // Update pheromone matrix
        for (int i = 0; i < NumCustomers; i++){
            for (int j = 0; j < NumCustomers; j++){
                if (i!=j)
                {
                    Pherom[i][j] = (((1 - a) * (Pherom[i][j])) + a * T0);
                }
                else
                {
                    Pherom[i][j] = 0;
                }
            }
        }

        // compute current and global distance and compare to evaluate improvement
        double currentDist{};
        //std::time_t TimeIdist = std::time(nullptr);

        for (int i = 0; i < numVehAssig + 1; i++)
        {
            for (int j = 0; j < PosY[i]; j++)
            {
                currentDist += Dist[route[i][j]][route[i][j+1]];
                //timerPause(TimeIdist, 120, PRINTER(TimeIdist)); // this was called to stop process if is taking too long
            }

        }

        //std::time_t TimeIferom = std::time(nullptr);
        // Update pheromone trail of current routes
        for (int i = 0; i < numVehAssig + 1; i++){
            for (int j = 0; j < PosY[i]; j++){
                Pherom[route[i][j]][route[i][j + 1]] = (1 - a) * (Pherom[route[i][j]][route[i][j + 1]]) + a * pow(currentDist, -1);
                //timerPause(TimeIferom, 120, PRINTER(TimeIferom)); // this was called to stop process if is taking too long
            }
        }

        // Save improved route plan if improved
        if (currentDist < ObjDistGlobal)
        {
            CapOverflowSW = false;
            ObjDistGlobal = currentDist;
            //std::time_t TimeIbroute = std::time(nullptr);
            // Resetting the entire 2D vector to zero  
            bestRoute.assign(bestRoute.size(), std::vector<int>(bestRoute[0].size(), 0)); // Reassigns each row to a vector of zeros  

            for (int i = 0; i < numVehAssig + 1; i++)
            {
                for (int j = 0; j < PosY[i] + 1; j++)
                {
                    bestRoute[i][j] = route[i][j];
                    //timerPause(TimeIbroute, 120, PRINTER(TimeIbroute)); // this was called to stop process if is taking too long
                }
                bestPosY[i] = PosY[i];
            }

            VehsUsed = numVehAssig;
            if (CapOverflow)
            {
                CapOverflowSW = true;
                ExcessCapacity = 0;
                excessVehicle = currentVehicle;
                //std::time_t TimeIexcess = std::time(nullptr);
                for (int i = 0; i < PosY[currentVehicle]; i++)
                {
                    ExcessCapacity += CapN[route[currentVehicle][i + 1]];
                    //timerPause(TimeIexcess, 120, PRINTER(TimeIexcess)); // this was called to stop process if is taking too long
                }
            }

            // Update pheromone trail
            //std::time_t TimeIferom2 = std::time(nullptr);
            for (int i = 0; i < numVehAssig + 1; i++){
                for (int j = 0; j < PosY[i]; j++){
                    Pherom[route[i][j]][route[i][j + 1]] = (1 - a) * (Pherom[route[i][j]][route[i][j + 1]])
                            + a * pow(ObjDistGlobal, -1);
                    //timerPause(TimeIferom2, 120, PRINTER(TimeIferom2)); // this was called to stop process if is taking too long
                }
            }

        }

        // show results...
        if (Iter < 7 || Iter >= maxIterations - 5)
        {
            if (Iter == 6)
            {
                ShowResults("*\n");
                ShowResults("*\n");
                ShowResults("*\n");
                //std::cout << *resultsPanel;

            }else
            {
                ShowResults("- Current distance: " + std::to_string(currentDist)  + " Global distance: "
                + std::to_string(ObjDistGlobal) + "\n");
                //std::cout << *resultsPanel;
            }
            if (Iter == maxIterations)
            {
                for (int i = 0; i < VehsUsed + 1; i++)
                {
                    ShowResults("\nRoute " + std::to_string(i + 1) + " Capacity: " + std::to_string(Cap[i]) + "\n");
                    for (int j = 0; j < bestPosY[i] + 1; j++)
                    {
                        if (j%10 == 0 | bestRoute[i][j + 1] == 0){
                            ShowResults(std::to_string(bestRoute[i][j]) + " - \n");
                        }else{
                            ShowResults(std::to_string(bestRoute[i][j])+ " - ");
                        }

                    }
                    std::cout << std::endl;
                }
                ShowResults("\n\nNumber of vehicles used: " + std::to_string(VehsUsed + 1) + "\n\n");
                if (CapOverflowSW)
                {
                    ShowResults("But you need more capacity for vehicle "
                    + std::to_string(excessVehicle + 1) + "\n");
                    ShowResults("The capacity required for this vehicle is: "
                    + std::to_string(ExcessCapacity) + "\n");
                }
                //std::cout << *resultsPanel;
            }


        }

        // Update ArgMax
        for (int i = 0; i < NumCustomers; i++){
            for (int j = 0; j < NumCustomers; j++){
                if (Dist[i][j] == 0)
                {
                    ArgMax[i][j] = 0;
                }
                else
                {
                    ArgMax[i][j] = Arg * Pherom[i][j] * pow((1 / Dist[i][j]), B);
                }
            }
        }

        std::time_t TimeL = std::time(nullptr);
        RunTime = TimeL - TimeI;

        if (RunTime > MaxRunTime){
            ShowResults("Process stopped at Iteration: " + std::to_string(Iter) + "\n");
            ShowResults("- Current distance: " + std::to_string(currentDist)
            + " Global distance: " + std::to_string(ObjDistGlobal) + "\n");
            for (int i = 0; i < VehsUsed + 1; i++)
            {
                ShowResults("\nRoute " + std::to_string(i + 1) + " Capacity: " + std::to_string(Cap[i]) + "\n");
                for (int j = 0; j < bestPosY[i] + 1; j++)
                {
                    if (j%10 == 0 | bestRoute[i][j + 1] == 0){
                        ShowResults(std::to_string(bestRoute[i][j]) + " - \n");
                    }else{
                        ShowResults(std::to_string(bestRoute[i][j])+ " - ");
                    }

                }
                std::cout << std::endl;
            }
            ShowResults("\n\nNumber of vehicles used: " + std::to_string(VehsUsed + 1) + "\n\n");
            //std::cout << *resultsPanel;
            break;
        }

        Iter++;

    } while (Iter <= maxIterations);

    ////----------------------------------------------------------------
    //// Compute and show solution time
    ////----------------------------------------------------------------
    std::time_t TimeF = std::time(nullptr);
    RunTime = TimeF - TimeI;

    ShowResults("\nThe Run Time is: " + std::to_string(RunTime) + "\n");
    f_solution.bestRoute_f = bestRoute;

    return *resultsPanel;
}

std::vector<std::vector<int>> GetRoutes(){
    return f_solution.bestRoute_f;
}

PYBIND11_MODULE(module_name, module){ // arguments are name we give to the module and a module identifier (module)
    module.doc() = "Pybind11Module";
    module.def("ACOfunct", &ACOfunct); // define C++ function in the python module with a name and a function pointer
    module.def("GetRoutes", &GetRoutes);
    pybind11::class_<Vector2D>(module, "Vector2D")
    .def_readwrite ("bestRoute_f", &Vector2D::bestRoute_f);

}

