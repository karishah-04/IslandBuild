/*
Name: Karina Shah
Date: March 31st, 2023
Description: Use randomizers to create an island on a 2D Array
Usage: <exe> -s <int>
*/

#include <iostream>
#include <cmath>
#include <stdlib.h>
#include <stdio.h>
#include <iomanip>
#include <string.h>
#include <fstream>
using namespace std;

// Function Parameters
int** makeParticleMap(int width, int height, int windowX, int windowY, int radius, int numParticle, int maxLife);
int** moveExists(int** map, int width, int height, int x, int y);
int findMax(int** map, int width, int height);
void normalizeMap(int** map, int width, int height, float maxVal);
char** polishMap(int** map, int width, int height, int waterLine);
void printMap(int** map, int width, int height);
void printMapChar(char** map, int width, int height);
void writeToFile(int** map, int width, int height, ofstream& file);
void writeToFileChar(char** map, int width, int height, ofstream& file);
double frand();

int main(int argc, char** argv){
    if(argc == 2 && strcmp(argv[0], "-s")){ // Checks if seed is given along with -s
        srand(atoi(argv[1])); // Assigns given seed
    }
    else{
        srand (time(0)); // If no seed is given, them defaults to 0.
    }
    // Variables needed for input
    int width = 0;
    int height = 0;
    int winX = 0;
    int winY = 0;
    int rad = 0;
    int numPar = 0;
    int maxLife = 0;
	int waterLine = 0;
    int** map = 0;
    // Gets input from user and stores it in respective variables
    cout << "Welcome to Karina's CSE240 island generator!" << endl;
    cout << "Enter grid width: ";
    cin >> width;
    cout << "Enter grid height: ";
    cin >> height;
    cout << "Enter drop-zone x-coordinate (0 - " << width << "): ";
    cin >> winX;
    cout << "Enter drop-zone y-coordinate (0 - " << height << "): ";
	cin >> winY;
	cout << "Enter drop-zone radius (minimum 2): ";
	cin >> rad;
	cout << "Enter number of particles to drop: ";
	cin >> numPar;
	cout << "Enter max life of particles: ";
	cin >> maxLife;
	cout << "Enter value for waterline (40-200): ";
	cin >> waterLine;
    // Creates and opens file to write into
    ofstream file;
    file.open("island.txt");
    // Creates new particle map
    map = makeParticleMap(width, height, winX, winY, rad, numPar, maxLife);
    // Prints out the raw map and writes it to file
    cout << "Raw Map" << endl;
    printMap(map, width, height);
    writeToFile(map, width, height, file);
    // Generates the max value in the map
    float max = (float)findMax(map, width, height);
    // Normalizes the map
    normalizeMap(map, width, height, max);
    // Prints out the normalized map and writes it to file
    cout << "Normal Map " << endl;
    printMap(map, width, height);
    writeToFile(map, width, height, file);
    // Creates the polished map
    char** polishedMap = polishMap(map, width, height, waterLine);
    // Prints out the polished map and writes it to file
    cout << "Polished Map " << endl;
    printMapChar(polishedMap, width, height);
    writeToFileChar(polishedMap, width, height, file);
    // Deletes the normalized map (raw and normalized in the same array) and the polished map from heap
    for(int i = 0; i < height; i++){
        delete[] map[i];
    }
    delete[] map;
    for(int i = 0; i < height; i++){
        delete[] polishedMap[i];
    }
    delete[] polishedMap;
    // Closes the file
    file.close();
}

// Takes in parameters and makes a whole new map from a 2D array. Generates the drop point and simulates the "rolling" process and returns a raw map
int** makeParticleMap(int width, int height, int windowX, int windowY, int radius, int numParticle, int maxLife){
    // Creates the new map using width and height
    int** map = new int*[height];
    for(int i = 0; i < height; i++){
        map[i] = new int[width];
    }
    // Initializes all the elements to 0.
    for(int i = 0; i < height; i++){
        for(int j = 0; j < width; j++){
            map[i][j] = 0;
        }
    }
    // Initializes x and y out of bounds to enter the while loop
    int x = -1, y = -1;
    // Variables used in generation of drop zone within given radius of given points
    double r, theta;
    for(int i = 0; i < numParticle; i++){
        // Resets x and y out of bounds to enter while loop again for second drop and on
        x = -1;
        y = -1;
        // Checks to make sure drop coordinates are in the map
        while((x < 0) || (y < 0) || (x >= height) || (y >= width)){
            // If coordinates are out of bounds of drop zone, puts them back in
            if(x < radius){
                x = radius;
            }
            if(y < radius){
                y = radius;
            }
            // Calculates the proper drop zone coordinates and sets it to x and y
            r = radius * sqrt(frand());
            theta = frand() * 2 * M_PI;
            x = (int)(windowX + r * cos(theta));
            y = (int)(windowY + r * sin(theta));
        }
        // Increments the element at drop
        map[x][y]++;
        // Goes through the rolling process starting with second set of coordinates
        for(int j = 0; j < maxLife -1; j++){
            // Creates a 2D array of current valid moves at specific location
            int** move = moveExists(map, width, height, x, y);
            // If there are valid moves
            if(move[8][0] != 0){
                // Choose one at random from list
                int choice = rand() % move[8][0];
                // Set x and y to that coordinate
                x = move[choice][0];
                y = move[choice][1];
                // Increment element at coordinate
                map[x][y]++;
                // Free 2D array from heap
                for(int i = 0; i < 9; i++){
                    delete[] move[i];
                }
                delete[] move;
            }
            // If there are no valid moves
            else{
                // Just free 2D array from heap and break out of for loop to start a new drop
                for(int i = 0; i < 9; i++){
                    delete[] move[i];
                }
                delete[] move;
                break;
            }
        }
    }
    // Return map
    return map;
}

int** moveExists(int** map, int width, int height, int x, int y){
    // Create 2D array to store valid moves
    int** rv = new int*[9];
    // Set each element in rv to 0
    for(int i = 0; i < 9; i++){
        rv[i] = new int[2];
        rv[i][0] = 0;
        rv[i][1] = 0;
    }
    // Increments each time theres a new valid array
    int a = 0;
    // Top left of current coordinate
    if(x - 1 > -1 && y - 1 > -1){
        if(map[x][y] >= map[x-1][y-1]){
            // Adds valid move to array
            rv[a][0] = x-1;
            rv[a][1] = y-1;
            rv[8][0]++;
            a++;
        }
    }
    // Bottom left of current coordinate
    if(x - 1 > -1 && y + 1 < width){
        if(map[x][y] >= map[x-1][y+1]){
            // Adds valid move to array
            rv[a][0] = x-1;
            rv[a][1] = y+1;
            rv[8][0]++;
            a++;
        }
    }
    // Top right of current coordinate
    if(x + 1 < height && y - 1 > -1){
        if(map[x][y] >= map[x+1][y-1]){
            // Adds valid move to array
            rv[a][0] = x+1;
            rv[a][1] = y-1;
            rv[8][0]++;
            a++;
        }
    }
    // Bottom right of current coordinate
    if(x + 1 < height && y + 1 < width){
        if(map[x][y] >= map[x+1][y+1]){
            // Adds valid move to array
            rv[a][0] = x+1;
            rv[a][1] = y+1;
            rv[8][0]++;
            a++;
        }
    }
    // Top of current coordinate
    if(x - 1 > -1){
        if(map[x][y] >= map[x-1][y]){
            // Adds valid move to array
            rv[a][0] = x-1;
            rv[a][1] = y;
            rv[8][0]++;
            a++;
        }
    }
    // Bottom of current coordinate
    if(x + 1 < height){
        if(map[x][y] >= map[x+1][y]){
            // Adds valid move to arra
            rv[a][0] = x+1;
            rv[a][1] = y;
            rv[8][0]++;
            a++;
        }
    }
    // Left of current coordinate
    if(y - 1 > -1){
        if(map[x][y] >= map[x][y-1]){
            // Adds valid move to array
            rv[a][0] = x;
            rv[a][1] = y-1;
            rv[8][0]++;
            a++;
        }
    }
    // Right of current coordinate
    if(y + 1 < width){
        if(map[x][y] >= map[x][y+1]){
            // Adds valid move to array
            rv[a][0] = x;
            rv[a][1] = y+1;
            rv[8][0]++;
            a++;
        }
    }
    return rv;
}

// Finds max in 2D int array
int findMax(int** map, int width, int height){
    // Sets max to 0 to start with
    int max = 0;
    // Iterates through every element
    for(int i = 0; i < height; i++){
        for(int j = 0; j < width; j++){
            // Compares current max with element to see if it's bigger. If bigger, then set it to the max to make it the new max
            if(map[i][j] > max){
                max = map[i][j];
            }
        }
    }
    // Returns the final max
    return max;
}

// Goes through map and normalizes it
void normalizeMap(int** map, int width, int height, float maxVal){
    // Iterates through every element
    for(int i = 0; i < height; i++){
        for(int j = 0; j < width; j++){
            // Sets each value by using calculation to normalize it to 255
            map[i][j] = (int)((map[i][j] / maxVal) * 255);
        }
    }
}

// Creates a 2D char array and uses the normalized map to create polished map
char** polishMap(int** map, int width, int height, int waterLine){
    // Creates the land zone
    int landZone = 255-waterLine;
    // Creates new 2D char array
    char** polished = new char*[height];
    for(int i = 0; i < height; i++){
        polished[i] = new char[width];
    }
    // Iterates through every element
    for(int i = 0; i < height; i++){
        for(int j = 0; j < width; j++){
            // If current element is under the waterLine, checks if it's shallow or deep water
            if(map[i][j] <= waterLine){
                if(map[i][j] < (int)(waterLine * 0.5)){
                    polished[i][j] = '#';
                }
                else{
                    polished[i][j] = '~';
                }
            }
            // If current element is above the water, checks if its coast/beach, plains/grass, forests, or mountains
            else{
                if(map[i][j] < (int)(waterLine + 0.15 * landZone)){
                    polished[i][j] = '.';
                }
                else if(map[i][j] >= (int)(waterLine + 0.15 * landZone) && map[i][j] < (int)(waterLine + 0.4 * landZone)){
                    polished[i][j] = '-';
                }
                else if(map[i][j] >= (int)(waterLine + 0.4 * landZone) &&  map[i][j] < (waterLine + 0.8 * landZone)){
                    polished[i][j] = '*';
                }
                else{
                    polished[i][j] = '^';
                }
            }
        }
    }
    // Return polished map
    return polished;
}

// Goes through each element of an int array and prints it out padded
void printMap(int** map, int width, int height){
    // Iterates through every element
    for(int i = 0; i < height; i++){
        for(int j = 0; j < width; j++){
            // Prints out element padded
            cout << setw(4) << map[i][j] << " ";
        }
        // New line for end of the line
        cout << endl;
    }
}

// Goes through each element of an char array and prints it out padded
void printMapChar(char** map, int width, int height){
    // Iterates through every element
    for(int i = 0; i < height; i++){
        for(int j = 0; j < width; j++){
            // Prints out element
            cout << map[i][j] << " ";
        }
        // New line for end of the line
        cout << endl;
    }
}

// Goes through each element of an int array and writes it to a file padded
void writeToFile(int** map, int width, int height, ofstream& file){
    // Iterates through every element
    for(int i = 0; i < height; i++){
        for(int j = 0; j < width; j++){
            // Writes it to a file padded
            file << setw(4) << map[i][j] << " ";
        }
        // New line for the end of the line
        file << endl;
    }
    // New line for the end of the file
    file << endl;
}

// Goes through each element of an char array and writes it to a file padded
void writeToFileChar(char** map, int width, int height, ofstream& file){
    // Iterates through every element
    for(int i = 0; i < height; i++){
        for(int j = 0; j < width; j++){
            // Writes it to a file
            file << map[i][j] << " ";
        }
        // New line for the end of the line
        file << endl;
    }
    // New line for the end of the file
    file << endl;
}

// Generates a random number from 0 to 1
double frand(){
    return (double)rand() / ((double)RAND_MAX+1);
}

