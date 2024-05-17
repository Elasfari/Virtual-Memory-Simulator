#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <map>
#include <climits> // for INT_MAX
#include <queue> // for FIFO queue
using namespace std;

int NUM_ADDRESSES = 1024;
unsigned long TimeOfAccess = 1; // set then increment
int CurPageNum = 0; // set then increment

map<int, int> psizes;
queue<int> fifoQueue; // Declare FIFO queue

struct page {
    int pageNum;
    int validBit;
    unsigned long lastTimeAccessed;
};

int lookupMemoryLoc(int pid, int loc, int pagesize) {
    if (loc < 0 || loc > psizes[pid]) { // first check to see if the memory location is valid for this process
        throw std::invalid_argument("Error: Received out-of-bounds memory location for specified process");
    }
    int result = (int)ceil((float)((float)loc / (float)pagesize)) - 1;
    return result;
}

// Optimal algorithm
int findOptimalIndex(page* mainMemory[], vector<page**>& table, int numPages, int pid, int pageSize, int ptraceSize) {
    int indexToReplace = -1;
    int maxDistance = INT_MIN;

    for (int i = 0; i < numPages; ++i) {
        bool pageFound = false;
        for (int j = ptraceSize; j < psizes[pid]; ++j) {
            int pageIndex = lookupMemoryLoc(pid, j, pageSize);
            if (table[pid][pageIndex]->pageNum == mainMemory[i]->pageNum) {
                pageFound = true;
                if (j > maxDistance) {
                    maxDistance = j;
                    indexToReplace = i;
                }
                break;
            }
        }
        if (!pageFound) { // If the page won't be used again
            indexToReplace = i;
            break;
        }
    }
    return indexToReplace;
}

int main(int argc, char** argv) {
    if (argc != 6) {
        printf("Invalid number of command line arguments\n");
        return (-1);
    }

    int pageSize = atoi(argv[3]);
    int algCase = -1;

    string algo = argv[4];
    if (algo.compare("FIFO") == 0) {
        algCase = 0;
    }
    else if (algo.compare("LRU") == 0) {
        algCase = 1;
    }
    else if (algo.compare("Clock") == 0) {
        algCase = 2;
    }
    else if (algo.compare("Optimal") == 0) { 
        algCase = 3;
    }
    else {
        printf("Invalid parameter value for page replacement algorithm\n");
        exit(-1);
    }


    string flag = argv[5]; 
    bool prePaging;
    bool notYetPrepaged = true;
    if (flag == "+")
        prePaging = true;
    else if (flag == "-")
        prePaging = false;
    else {
        printf("Invalid parameter value for pre-paging [+/-]\n");
        exit(-1);
    }

    vector<page**> table; 
    ifstream plist(argv[1], ifstream::in);
    int plistLines = 0;
    char c;
    char lastChar = '\n';
    bool readPID = false;
    string currPID = "";
    int pid = 0;
    string currSize = "";
    int size = 0;
    int numPages;

    while (plist.good()) {
        c = plist.get();
        if (c == '\n' && lastChar != '\n') {
            size = atoi(currSize.c_str());
            pid = atoi(currPID.c_str());
            psizes[pid] = size;
            numPages = (int)(ceil(((float)size / (float)pageSize)));
            page** anythingreally = new page * [numPages];
            for (int i = 0; i < numPages; i++) {
                anythingreally[i] = new page;
                anythingreally[i]->pageNum = CurPageNum;
                CurPageNum++;
                anythingreally[i]->validBit = 0;
                anythingreally[i]->lastTimeAccessed = 0;
            }
            table.push_back(anythingreally);
            plistLines++;
            currSize = "";
            currPID = "";
            readPID = false;
        }
        else if (!readPID && c != ' ') {
            currPID = currPID + c;
        }
        else if (c == ' ') {
            readPID = true;
        }
        else {
            currSize = currSize + c;
        }
        lastChar = c;
    }
    plist.close();

    numPages = NUM_ADDRESSES / pageSize;
    int pagesPerProgram = numPages / plistLines;
    printf("Pages per program = %d \n", pagesPerProgram);

    // Declare and initialize mainMemory
    page** mainMemory = new page*[numPages];
    
    // Initializing main memory and FIFO queue
    for (int i = 0; i < plistLines; ++i) {
        for (int j = 0; j < pagesPerProgram; ++j) {
            mainMemory[j + pagesPerProgram * i] = new page;
            mainMemory[j + pagesPerProgram * i] = table[i][j];
            table[i][j]->validBit = 1;
            table[i][j]->lastTimeAccessed = TimeOfAccess;
            TimeOfAccess++;
            fifoQueue.push(j + pagesPerProgram * i); // Push the initial pages into FIFO queue
        }
    }
    numPages = pagesPerProgram * plistLines;
    printf("Main memory number of pages = %d \n", numPages);

    ifstream ptrace(argv[2], ifstream::in);
    lastChar = '\n';
    readPID = false;
    currPID = "";
    pid = 0;
    currSize = "";
    size = 0;
    int clockIndex = 0;
    int ptraceLines = 0;
    printf("Finished Plist, now ptracing\n");
    int faultCounter = 0;
    int indexToSwap = 0;
    int tempsize;
    int numMemoryAccesses = 0;

    while (ptrace.good()) {
        c = ptrace.get();
        if (c == '\n' && lastChar != '\n') {
            size = atoi(currSize.c_str());
            pid = atoi(currPID.c_str());

            if (table[pid][lookupMemoryLoc(pid, size, pageSize)]->validBit == 0) {
                faultCounter++;

                prepagecase:
                if (algCase == 0) {
                    indexToSwap = fifoQueue.front();
                    fifoQueue.pop();
                    fifoQueue.push(indexToSwap);
                }
                else if (algCase == 1) {
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
                else if (algCase == 2) {
                    while (mainMemory[clockIndex]->validBit == 1) {
                        mainMemory[clockIndex]->validBit = 2;
                        clockIndex = (clockIndex + 1) % numPages;
                    }
                    indexToSwap = clockIndex;
                    clockIndex = (clockIndex + 1) % numPages;
                }
                else if (algCase == 3) {
                    indexToSwap = findOptimalIndex(mainMemory, table, numPages, pid, pageSize, size);
                }

                mainMemory[indexToSwap]->validBit = 0;
                int checkPageNum = mainMemory[indexToSwap]->pageNum;
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
                        goto prepagecase;
                    }
                }
            }
            else if (algCase == 1) {
                table[pid][lookupMemoryLoc(pid, size, pageSize)]->lastTimeAccessed = TimeOfAccess;
                TimeOfAccess++;
            }
            else if (table[pid][lookupMemoryLoc(pid, size, pageSize)]->validBit == 2) {
                table[pid][lookupMemoryLoc(pid, size, pageSize)]->validBit = 1;
            }

            notYetPrepaged = true;
            ptraceLines++;
            currSize = "";
            currPID = "";
            readPID = false;
            numMemoryAccesses++;
        }
        else if (!readPID && c != ' ') {
            currPID = currPID + c;
        }
        else if (c == ' ') {
            readPID = true;
        }
        else {
            currSize = currSize + c;
        }
        lastChar = c;
    }

    ptrace.close();

    // Calculate performance metrics
    float pageFaultRate = (float)faultCounter / (float)numMemoryAccesses;
    float averageAccessTime = (float)TimeOfAccess / (float)numMemoryAccesses;

    printf("Number of memory accesses: %d\n", numMemoryAccesses);
    printf("Number of page faults: %d\n", faultCounter);
    printf("Page Fault Rate: %.2f\n", pageFaultRate);
    printf("Average Access Time: %.2f\n", averageAccessTime);
    return 0;
}
