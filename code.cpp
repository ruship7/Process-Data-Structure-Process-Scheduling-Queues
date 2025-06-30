#include <iostream>
#include <cstring>
#include <fstream>
#include <queue>
#include <bits/stdc++.h>
#define TIME_QUANTUM 5
#define N 40
#define SIZE 300
#define K 4
using namespace std;

enum STATUS { Loading, Loaded, Ready, Execute, Terminate };

int GLOBAL_TIME = 0;

class PCB {
public:
    int ID;
    int BT;  
    int TTC;
    int TSC;
    int priority;
    int AT;
    STATUS status;
    string terminateMsg;

    PCB(PCB* pcb) : ID(pcb->ID), BT(pcb->BT), priority(pcb->priority), 
      TTC(pcb->TTC), TSC(pcb->TSC), AT(pcb->AT), 
      status(pcb->status), terminateMsg(pcb->terminateMsg) {}

    PCB() : BT(0), priority(0), TTC(0), TSC(0), ID(0), AT(0), 
      status(STATUS::Loading), terminateMsg("") {}
};

class MOS {
private:
    char MainMemory[SIZE][K];
    bool mark[30];
    queue<PCB*> loadQ;
    queue<PCB*> readyQ;
    queue<PCB*> terminateQ;
    ifstream fi;
    ofstream fo;

    int allocate(PCB* p) {
        for (int i = 0; i < SIZE; i += 10) {  // 1 page = 10 rows
            if (!mark[i / 10]) {
                mark[i / 10] = true;
    
                // Store ID into the first word of the page
                string idStr = to_string(p->ID);
                int len = min((int)idStr.length(), K);
                for (int j = 0; j < len; ++j)
                    MainMemory[i][j] = idStr[j];
    
                return i;  // return starting index of the allocated page
            }
        }
        return -1; // No free page
    }

    void terminate(int EM, PCB* temp) {
        string msg = (EM == 3) ? "TIME LIMIT EXCEEDED" : "NORMAL TERMINATION";
        fo << "[TIME " << GLOBAL_TIME << "] Process " << temp->ID << " Terminated: " << msg 
           << " (TTC:" << temp->TTC << "/BT:" << temp->BT 
           << " Priority:" << temp->priority << ")\n";
    }

    void displayQueues() {
        fo << "\n--- Current Queues at TIME " << GLOBAL_TIME << " ---\n";

        fo << "Load Queue: ";
        queue<PCB*> tempLoad = loadQ;
        while (!tempLoad.empty()) {
            fo << tempLoad.front()->ID << " ";
            tempLoad.pop();
        }
        fo << "\n";

        fo << "Ready Queue: ";
        queue<PCB*> tempReady = readyQ;
        while (!tempReady.empty()) {
            fo << tempReady.front()->ID << " ";
            tempReady.pop();
        }
        fo << "\n";

        fo << "Terminated Processes: ";
        queue<PCB*> tempTerm = terminateQ;
        while (!tempTerm.empty()) {
            fo << tempTerm.front()->ID << " ";
            tempTerm.pop();
        }
        fo << "\n-----------------------\n";
    }

    
    

public:
    MOS() {
        memset(mark, 0, sizeof(mark));
        for(int i=0; i<SIZE; i++)
            memset(MainMemory[i], ' ', K);

        fi.open("input.txt");
        fo.open("output.txt");
    }

    void loadProcesses() {
        string line;
        while(getline(fi, line)) {
            if(line.find("$AMJ") != string::npos) {
                PCB* p = new PCB();
                p->ID = stoi(line.substr(4,4));
                p->BT = stoi(line.substr(8,4));
                p->AT = stoi(line.substr(12,4));
                p->priority = stoi(line.substr(16,4));
                p->status = Loading;
                loadQ.push(p);
                fo << "[TIME " << GLOBAL_TIME << "] Process " << p->ID << " status changed to LOADING\n";
            }
        }
    }

    void printMemory() {
        fo << "\n====== MAIN MEMORY DUMP ======\n";
        for (int i = 0; i < SIZE; ++i) {
            fo << setw(3) << i << ": ";
            for (int j = 0; j < K; ++j) {
                if (MainMemory[i][j] == ' ')
                    fo << ". ";
                else
                    fo << MainMemory[i][j] << " ";
            }
            fo << "\n";
        }
        fo << "==============================\n";
    }
    

    void executeRR() {
        while(!loadQ.empty() || !readyQ.empty()) {
            int sz = loadQ.size();
            for (int i = 0; i < sz; ++i) {
                PCB* p = loadQ.front();
                loadQ.pop();
                if (p->AT <= GLOBAL_TIME && count(mark, mark+30, false) > 0) {
                    allocate(p);
                    p->status = Ready;
                    fo << "[TIME " << GLOBAL_TIME << "] Process " << p->ID << " status changed to READY\n";
                    readyQ.push(p);
                    displayQueues();
                } else {
                    loadQ.push(p);
                }
            }

            if(!readyQ.empty()) {
                PCB* current = readyQ.front();
                readyQ.pop();

                current->status = Execute;
                fo << "[TIME " << GLOBAL_TIME << "] Process " << current->ID << " status changed to EXECUTE\n";
                displayQueues();

                for(int t = 0; t < TIME_QUANTUM; t++) {
                    current->TTC++;
                    GLOBAL_TIME++;
                    if(current->TTC >= current->BT) {
                        current->status = Terminate;
                        terminateQ.push(current);
                        fo << "[TIME " << GLOBAL_TIME << "] Process " << current->ID << " status changed to TERMINATE\n";
                        terminate(0, current);
                        displayQueues();
                        break;
                    }
                }
                if(current->TTC < current->BT) {
                    current->status = Ready;
                    fo << "[TIME " << GLOBAL_TIME << "] Process " << current->ID << " time slice completed. Status: READY\n";
                    readyQ.push(current);
                    displayQueues();
                }
            } else {
                GLOBAL_TIME++;
            }
        }
    }

    void executeFCFS() {
        while (!loadQ.empty() || !readyQ.empty()) {
            int sz = loadQ.size();
            for (int i = 0; i < sz; ++i) {
                PCB* p = loadQ.front();
                loadQ.pop();
                if (p->AT <= GLOBAL_TIME && count(mark, mark+30, false) > 0) {
                    allocate(p);
                    p->status = Ready;
                    fo << "[TIME " << GLOBAL_TIME << "] Process " << p->ID << " status changed to READY\n";
                    readyQ.push(p);
                    displayQueues();
                } else {
                    loadQ.push(p);
                }
            }

            if (!readyQ.empty()) {
                PCB* current = readyQ.front();
                readyQ.pop();

                current->status = Execute;
                fo << "[TIME " << GLOBAL_TIME << "] Process " << current->ID << " status changed to EXECUTE\n";
                displayQueues();

                while (current->TTC < current->BT) {
                    current->TTC++;
                    GLOBAL_TIME++;
                }

                current->status = Terminate;
                terminateQ.push(current);
                fo << "[TIME " << GLOBAL_TIME << "] Process " << current->ID << " status changed to TERMINATE\n";
                terminate(0, current);
                displayQueues();
            } else {
                GLOBAL_TIME++;
            }
        }
    }

    void executeSJF() {
        while (!loadQ.empty() || !readyQ.empty()) {
            int sz = loadQ.size();
            for (int i = 0; i < sz; ++i) {
                PCB* p = loadQ.front();
                loadQ.pop();
                if (p->AT <= GLOBAL_TIME && count(mark, mark+30, false) > 0) {
                    allocate(p);
                    p->status = Ready;
                    fo << "[TIME " << GLOBAL_TIME << "] Process " << p->ID << " status changed to READY\n";
                    readyQ.push(p);
                    displayQueues();
                } else {
                    loadQ.push(p);
                }
            }

            if (!readyQ.empty()) {
                PCB* shortest = nullptr;
                int n = readyQ.size();
                for (int i = 0; i < n; ++i) {
                    PCB* curr = readyQ.front();
                    readyQ.pop();
                    if (!shortest || curr->BT < shortest->BT) {
                        shortest = curr;
                    }
                    readyQ.push(curr);
                }

                bool selected = false;
                for (int i = 0; i < n; ++i) {
                    PCB* curr = readyQ.front();
                    readyQ.pop();
                    if (!selected && curr == shortest) {
                        selected = true;
                        continue;
                    }
                    readyQ.push(curr);
                }

                shortest->status = Execute;
                fo << "[TIME " << GLOBAL_TIME << "] Process " << shortest->ID << " status changed to EXECUTE\n";
                displayQueues();

                while (shortest->TTC < shortest->BT) {
                    shortest->TTC++;
                    GLOBAL_TIME++;
                }

                shortest->status = Terminate;
                terminateQ.push(shortest);
                fo << "[TIME " << GLOBAL_TIME << "] Process " << shortest->ID << " status changed to TERMINATE\n";
                terminate(0, shortest);
                displayQueues();
            } else {
                GLOBAL_TIME++;
            }
        }
    }

    void executePriority() {
        while (!loadQ.empty() || !readyQ.empty()) {
            int sz = loadQ.size();
            for (int i = 0; i < sz; ++i) {
                PCB* p = loadQ.front();
                loadQ.pop();
                if (p->AT <= GLOBAL_TIME && count(mark, mark + 30, false) > 0) {
                    allocate(p);
                    p->status = Ready;
                    fo << "[TIME " << GLOBAL_TIME << "] Process " << p->ID << " status changed to READY\n";
                    readyQ.push(p);
                    displayQueues();
                } else {
                    loadQ.push(p);
                }
            }

            if (!readyQ.empty()) {
                PCB* highest = nullptr;
                int n = readyQ.size();
                for (int i = 0; i < n; ++i) {
                    PCB* curr = readyQ.front();
                    readyQ.pop();
                    if (!highest || curr->priority < highest->priority) {
                        highest = curr;
                    }
                    readyQ.push(curr);
                }

                bool selected = false;
                for (int i = 0; i < n; ++i) {
                    PCB* curr = readyQ.front();
                    readyQ.pop();
                    if (!selected && curr == highest) {
                        selected = true;
                        continue;
                    }
                    readyQ.push(curr);
                }

                highest->status = Execute;
                fo << "[TIME " << GLOBAL_TIME << "] Process " << highest->ID << " status changed to EXECUTE\n";
                displayQueues();

                while (highest->TTC < highest->BT) {
                    highest->TTC++;
                    GLOBAL_TIME++;
                }

                highest->status = Terminate;
                terminateQ.push(highest);
                fo << "[TIME " << GLOBAL_TIME << "] Process " << highest->ID << " status changed to TERMINATE\n";
                terminate(0, highest);
                displayQueues();
            } else {
                GLOBAL_TIME++;
            }
        }
    }

    ~MOS() {
        fi.close();
        fo.close();
    }
};

int main() {
    srand(time(NULL));
    MOS os;
    os.loadProcesses();

    os.executeRR();
    // os.executeFCFS();
    // os.executeSJF();
    // os.executePriority();
    os.executeRR();

    os.printMemory();

    
    return 0;
}