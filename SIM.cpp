#include <stdio.h> // Standard Input/Output functions
#include <stdlib.h> // Standard Library functions
#include <string.h> // String functions
#include <iostream> // Input/Output stream
#include <fstream> // File stream
#include <vector> // Vector container
#include <cmath> // Math functions
#include <map> // Map container
#include <climits> // Constants for limits of integral types
#include <queue> // Queue container for FIFO queue
using namespace std;

int NUM_ADDRESSES = 1024; // Number of memory addresses
unsigned long TimeOfAccess = 1; // Variable to track time of memory access, initialized to 1 and incremented
int CurPageNum = 0; // Current page number, initialized to 0 and incremented

map<int, int> psizes; // Map to store process IDs and their respective sizes
queue<int> fifoQueue; // FIFO queue to implement FIFO page replacement algorithm

struct page {
    int pageNum; // Page number
    int validBit; // Valid bit indicating whether the page is valid or not
    unsigned long lastTimeAccessed; // Last time the page was accessed
};

// Function to look up memory location for a given process ID, location, and page size
int lookupMemoryLoc(int pid, int loc, int pagesize) {
    if (loc < 0 || loc > psizes[pid]) { // Check if the memory location is within bounds for the process
        throw std::invalid_argument("Error: Received out-of-bounds memory location for specified process");
    }
    int result = (int)ceil((float)((float)loc / (float)pagesize)) - 1; // Calculate page index based on location and page size
    return result;
}

// Optimal page replacement algorithm
int findOptimalIndex(page* mainMemory[], vector<page**>& table, int numPages, int pid, int pageSize, int ptraceSize) {
    int indexToReplace = -1; // Index of the page to be replaced
    int maxDistance = INT_MIN; // Maximum distance to the next access of a page

    // Iterate through each page in mainMemory
    for (int i = 0; i < numPages; ++i) {
        bool pageFound = false;
        
        // Check future accesses of pages in the process trace
        for (int j = ptraceSize; j < psizes[pid]; ++j) {
            int pageIndex = lookupMemoryLoc(pid, j, pageSize); // Get the index of the page in the process's address space
            if (table[pid][pageIndex]->pageNum == mainMemory[i]->pageNum) { // If page is found in process address space
                pageFound = true;
                if (j > maxDistance) { // Update maxDistance if next access is farther
                    maxDistance = j;
                    indexToReplace = i; // Update indexToReplace
                }
                break;
            }
        }
        if (!pageFound) { // If the page won't be used again in the future accesses
            indexToReplace = i; // Set the indexToReplace to the current page
            break;
        }
    }
    return indexToReplace; // Return the index of the page to be replaced
}

int main(int argc, char** argv) {
    // Check if the correct number of command line arguments are provided
    if (argc != 6) {
        printf("Invalid number of command line arguments\n");
        return (-1);
    }

    int pageSize = atoi(argv[3]); // Convert page size from string to integer
    int algCase = -1; // Initialize algorithm case

    string algo = argv[4]; // Extract algorithm name from command line arguments
    // Determine the algorithm case based on the provided algorithm name
    if (algo.compare("FIFO") == 0) {
        algCase = 0; // Set algorithm case to FIFO
    }
    else if (algo.compare("LRU") == 0) {
        algCase = 1; // Set algorithm case to LRU
    }
    else if (algo.compare("Clock") == 0) {
        algCase = 2; // Set algorithm case to Clock
    }
    else if (algo.compare("Optimal") == 0) { 
        algCase = 3; // Set algorithm case to Optimal
    }
    else {
        printf("Invalid parameter value for page replacement algorithm\n");
        exit(-1);
    }

    string flag = argv[5]; // Extract pre-paging flag from command line arguments
    bool prePaging;
    bool notYetPrepaged = true;
    // Determine pre-paging based on the provided flag
    if (flag == "+")
        prePaging = true; // Set pre-paging to true
    else if (flag == "-")
        prePaging = false; // Set pre-paging to false
    else {
        printf("Invalid parameter value for pre-paging [+/-]\n");
        exit(-1);
    }

    vector<page**> table; // Vector to store tables for each process
    ifstream plist(argv[1], ifstream::in); // Open process list file
    int plistLines = 0; // Counter for number of lines in process list
    char c;
    char lastChar = '\n';
    bool readPID = false;
    string currPID = "";
    int pid = 0;
    string currSize = "";
    int size = 0;
    int numPages; // Number of pages per process

    while (plist.good()) {
        c = plist.get(); // Read characters from the process list file
        if (c == '\n' && lastChar != '\n') { // Check for end of line and avoid counting empty lines
            size = atoi(currSize.c_str()); // Convert process size from string to integer
            pid = atoi(currPID.c_str()); // Convert process ID from string to integer
            psizes[pid] = size; // Store process size in the psizes map
            numPages = (int)(ceil(((float)size / (float)pageSize))); // Calculate the number of pages for the process
            // Allocate memory for the pages of the current process
            page** anythingreally = new page * [numPages];
            // Initialize the pages of the current process
            for (int i = 0; i < numPages; i++) {
                anythingreally[i] = new page;
                anythingreally[i]->pageNum = CurPageNum;
                CurPageNum++;
                anythingreally[i]->validBit = 0;
                anythingreally[i]->lastTimeAccessed = 0;
            }
            table.push_back(anythingreally); // Store the pages of the current process in the table
            plistLines++; // Increment the number of lines processed from the process list
            currSize = ""; // Reset the current process size
            currPID = ""; // Reset the current process ID
            readPID = false; // Reset the flag for reading process ID
        }
        else if (!readPID && c != ' ') { // Accumulate characters for process ID until space character is encountered
            currPID = currPID + c;
        }
        else if (c == ' ') { // Set flag for reading process ID after encountering space character
            readPID = true;
        }
        else { // Accumulate characters for process size
            currSize = currSize + c;
        }
        lastChar = c; // Store the last character read
    }
    plist.close(); // Close the process list file

    numPages = NUM_ADDRESSES / pageSize; // Calculate the total number of pages
    int pagesPerProgram = numPages / plistLines; // Calculate the number of pages per program
    printf("Pages per program = %d \n", pagesPerProgram);

    // Declare and initialize mainMemory
    page** mainMemory = new page*[numPages];
    
    // Initializing main memory and FIFO queue
    for (int i = 0; i < plistLines; ++i) { // Iterate over each process
        for (int j = 0; j < pagesPerProgram; ++j) { // Iterate over each page in the process
            // Initialize the main memory with the pages of the current process
            mainMemory[j + pagesPerProgram * i] = new page;
            mainMemory[j + pagesPerProgram * i] = table[i][j];
            table[i][j]->validBit = 1;
            table[i][j]->lastTimeAccessed = TimeOfAccess;
            TimeOfAccess++;
            fifoQueue.push(j + pagesPerProgram * i); // Push the initial pages into FIFO queue
        }
    }
    numPages = pagesPerProgram * plistLines; // Update the total number of pages
    printf("Main memory number of pages = %d \n", numPages);

    ifstream ptrace(argv[2], ifstream::in); // Open the page trace file for reading
    lastChar = '\n'; // Initialize the last character read to newline
    readPID = false; // Reset the flag for reading process ID
    currPID = ""; // Reset the current process ID
    pid = 0; // Reset the process ID
    currSize = ""; // Reset the current process size
    size = 0; // Reset the process size
    int clockIndex = 0; // Initialize the clock index for Clock algorithm
    int ptraceLines = 0; // Counter for the number of lines processed from the page trace
    printf("Finished Plist, now ptracing\n");
    int faultCounter = 0; // Counter for page faults
    int indexToSwap = 0; // Index of the page to swap in main memory
    int tempsize; // Temporary variable for storing the size of a page
    int numMemoryAccesses = 0; // Counter for the number of memory accesses

    while (ptrace.good()) {
        c = ptrace.get(); // Read characters from the page trace file
        if (c == '\n' && lastChar != '\n') { // Check for end of line and avoid counting empty lines
            size = atoi(currSize.c_str()); // Convert page size from string to integer
            pid = atoi(currPID.c_str()); // Convert process ID from string to integer

            if (table[pid][lookupMemoryLoc(pid, size, pageSize)]->validBit == 0) { // Check if the page is not in memory
                faultCounter++; // Increment page fault counter

                // Determine the page replacement algorithm to use
                prepagecase:
                if (algCase == 0) { // FIFO algorithm
                    indexToSwap = fifoQueue.front();
                    fifoQueue.pop();
                    fifoQueue.push(indexToSwap);
                }
                else if (algCase == 1) { // LRU algorithm
                    unsigned long minTime = TimeOfAccess;
                    int minIndex = 0;
                    for (int i = 0; i < numPages; ++i) {
                        if (minTime > mainMemory[i]->lastTimeAccessed) {
                            minIndex = i;
                            minTime = mainMemory[i]->lastTimeAccessed;
                        }
                    }
                    indexToSwap = minIndex;
                }
                else if (algCase == 2) { // Clock algorithm
                    while (mainMemory[clockIndex]->validBit == 1) {
                        mainMemory[clockIndex]->validBit = 2;
                        clockIndex = (clockIndex + 1) % numPages;
                    }
                    indexToSwap = clockIndex;
                    clockIndex = (clockIndex + 1) % numPages;
                }
                else if (algCase == 3) { // Optimal algorithm
                    indexToSwap = findOptimalIndex(mainMemory, table, numPages, pid, pageSize, size);
                }

                // Invalidate the page being replaced
                mainMemory[indexToSwap]->validBit = 0;
                int checkPageNum = mainMemory[indexToSwap]->pageNum;

                // Update main memory with the new page
                if (notYetPrepaged) {
                    mainMemory[indexToSwap] = table[pid][lookupMemoryLoc(pid, size, pageSize)];
                    mainMemory[indexToSwap]->validBit = 1;
                    mainMemory[indexToSwap]->lastTimeAccessed = TimeOfAccess;
                }
                else {
                    mainMemory[indexToSwap] = table[pid][lookupMemoryLoc(pid, tempsize, pageSize)];
                    mainMemory[indexToSwap]->validBit = 1;
                    mainMemory[indexToSwap]->lastTimeAccessed = TimeOfAccess;
                }
                TimeOfAccess++;

                // Perform pre-paging if enabled
                if (prePaging && notYetPrepaged) {
                    notYetPrepaged = false;
                    tempsize = (size + pageSize) % psizes[pid];
                    if (tempsize > size && table[pid][lookupMemoryLoc(pid, tempsize, pageSize)]->pageNum == checkPageNum) {
                        tempsize = (tempsize + pageSize) % psizes[pid];
                    }
                    while (tempsize > size && table[pid][lookupMemoryLoc(pid, tempsize, pageSize)]->validBit == 1) {
                        tempsize = (tempsize + pageSize) % psizes[pid];
                        if (tempsize > size && table[pid][lookupMemoryLoc(pid, tempsize, pageSize)]->pageNum == checkPageNum) {
                            tempsize = (tempsize + pageSize) % psizes[pid];
                        }
                    }
                    if (tempsize > size) {
                        goto prepagecase; // Jump to pre-page case
                    }
                }
            }
            else if (algCase == 1) { // Update last access time for LRU algorithm
                table[pid][lookupMemoryLoc(pid, size, pageSize)]->lastTimeAccessed = TimeOfAccess;
                TimeOfAccess++;
            }
            else if (table[pid][lookupMemoryLoc(pid, size, pageSize)]->validBit == 2) { // Update valid bit for Clock algorithm
                table[pid][lookupMemoryLoc(pid, size, pageSize)]->validBit = 1;
            }

            notYetPrepaged = true; // Reset pre-paging flag
            ptraceLines++; // Increment the number of lines processed from the page trace
            currSize = ""; // Reset the current page size
            currPID = ""; // Reset the current process ID
            readPID = false; // Reset the flag for reading process ID
            numMemoryAccesses++; // Increment the number of memory accesses
        }
        else if (!readPID && c != ' ') { // Accumulate characters for process ID until space character is encountered
            currPID = currPID + c;
        }
        else if (c == ' ') { // Set flag for reading process ID after encountering space character
            readPID = true;
        }
        else { // Accumulate characters for page size
            currSize = currSize + c;
        }
        lastChar = c; // Store the last character read
    }

    ptrace.close(); // Close the page trace file

    // Calculate performance metrics
    float pageFaultRate = (float)faultCounter / (float)numMemoryAccesses;
    float averageAccessTime = (float)TimeOfAccess / (float)numMemoryAccesses;

    // Print performance metrics
    printf("Number of memory accesses: %d\n", numMemoryAccesses);
    printf("Number of page faults: %d\n", faultCounter);
    printf("Page Fault Rate: %.2f\n", pageFaultRate);
    printf("Average Access Time: %.2f\n", averageAccessTime);

    return 0; // Exit the program
}
