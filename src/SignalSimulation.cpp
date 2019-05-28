#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <fstream>
#include <sstream>
#include <map>
#include <sys/wait.h>
#include <sys/mman.h>
using namespace std;




//Car information
struct carinfo {
    string licensePlate;
    int time;
    char direction;
} carsinfo_array[50];


int total_elements = 0;
char initial_direction;
int max_cars_perDir;
bool *isVisited;
//Function to read data from file
void fillDataFromFile();
//Function to order direction in clockwise
void rearangeCarInfo();
//Function to display car information
void displayCarInfo(int);
//Function to print all cars
void printAllCarsInfo();

int main()
{

    //First step is to read all data from file.
    fillDataFromFile();
    //Here we rearange the order of cars so that cars can move in clockwise fashion.
    rearangeCarInfo();
    isVisited = (bool *)mmap(NULL, sizeof(bool) * total_elements, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    for (int i = 0; i < total_elements; i++)
    {
        //Only displaying information for child process.
        pid_t pid = fork();
        //if its a parent call, then wait till all child are completed
        if (pid > 0)
        {
            wait(NULL);
        }
        else
        {
            //Verify that all childs have same parent
            displayCarInfo(i);
        }
    }
    //Here the parent waits till its childs complete their execution.
    int i = 0;
    for (i = 0; i < total_elements; i++)
    {
        if (i + 1 == total_elements)
            exit(0);
        wait(NULL);
    }

    return 0;
}


void fillDataFromFile()
{
    string line;
    int k = 1, j = 0;
    //Reading data from file
    ifstream input_file("input.txt");
    //Checking for opening of file
    if (input_file.is_open())
    {
        while (getline(input_file, line))
        {
            //It is assumed that in file line greater than 2 will hold cars information.
            if (k > 2 && line != "")
            {

                string current_line;
                //converting string to stream for spliting purposes in our case tokens are based on spaces.
                stringstream ss(line);
                int i = 0;
                //Tokenization.
                while (getline(ss, current_line, ' '))
                {
                    //if i==0 it is assumed that it is license plate.
                    if (i == 0)
                        carsinfo_array[total_elements].licensePlate = current_line;
                    //if i==1 it is assumed that it is direction.
                    if (i == 1)
                        carsinfo_array[total_elements].direction = current_line[0];
                    //if i==2 it is assumed that it is time.
                    if (i == 2)
                        carsinfo_array[total_elements].time = atoi(current_line.c_str());
                    i++;
                }
                //indicates current array size.
                total_elements++;
            }
            else
            {
                //Initial direction
                if (j == 0)
                    initial_direction = line[0];
                //max number of cars that can pass intersection
                if (j == 1)
                    max_cars_perDir = atoi(line.c_str());
                j++;
            }
            k++;
        }
        //closing file
        input_file.close();
    }
    else
        cout << "Unable to open file\n";
}

void rearangeCarInfo()
{
    //Map that holds clockwise direction information.
    map<char, char> directionMap;
    directionMap['N'] = 'E';
    directionMap['E'] = 'S';
    directionMap['S'] = 'W';
    directionMap['W'] = 'N';
    char curr_dir = initial_direction;

    /*Scanning all the cars and arranging their moving sequence clockwise
  i-e if we start with 'W' then next car will be from 'N' than 'E' than 'S' and
  this sequence repeats. S->W->N->E....*/
    for (int i = 0; i < total_elements; i++)
    {
        bool check = false;
        int j;
        char start = curr_dir;
        //Finding next clockwise pattern location
        do
        {

            for (j = i + 1; j < total_elements; j++)
            {
                //Same location will not be swapped.
                if (curr_dir == carsinfo_array[i].direction)
                {
                    check = true;
                    break;
                }
                /*Only direction that come before some direction i-e if direction is 'E'
        first and we have direction 'N' as initial so we will pick 'N' first than 'E' to follow correct clockwise direction pattern.*/
                if (curr_dir == carsinfo_array[j].direction)
                {
                    struct carinfo temp = carsinfo_array[j];
                    carsinfo_array[j] = carsinfo_array[i];
                    carsinfo_array[i] = temp;
                    check = true;
                    break;
                }
            }

            curr_dir = directionMap[curr_dir];
            //Just skip the loop when correct ordering is done for a particular car.
        } while (!check && start != curr_dir);
    }
}
void displayCarInfo(int index)
{

    //Displays a car information.
    if (index > -1 && index < total_elements && isVisited[index] != true)
    {
        map<char, string> directionDiscription;
        directionDiscription['N'] = "Northbound";
        directionDiscription['E'] = "Eastbound";
        directionDiscription['S'] = "Southbound";
        directionDiscription['W'] = "Westbound";
        carinfo car = carsinfo_array[index];
        cout << "Current Direction: " << directionDiscription[car.direction] << endl;
        int k = 0;

        while (index < total_elements && k < max_cars_perDir)
        {
            //cout<<index<<endl;
            if (car.direction == carsinfo_array[index].direction)
            {
                //cout<<car.licensePlate<<" "<<carsinfo_array[index].licensePlate<<endl;
                cout << "Car " << carsinfo_array[index].licensePlate << " is using the intersection for " << carsinfo_array[index].time << " sec(s)." << endl;
                isVisited[index] = true;
                sleep(carsinfo_array[index].time);
                cout.flush();
                k++;
            }

            index++;
        }
    }
}

void printAllCarsInfo()
{
    //Display all cars.
    for (int i = 0; i < total_elements; i++)
    {
        cout << carsinfo_array[i].licensePlate << " " << carsinfo_array[i].direction << " " << carsinfo_array[i].time << endl;
    }
}
