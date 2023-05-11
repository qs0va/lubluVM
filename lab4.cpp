#include <iostream>
#include <array>
#include <queue>

#include "lab4.h"

#define READ 0
#define WRITE 1
#define SIZE 5

struct PhysicalPage {
    int VPN;
    bool R;
    bool M;
    int que;
    unsigned int counter;
};

struct Command {
    int type;
    int VPN;
};

typedef std::array<PhysicalPage, SIZE> Table;

void run(Table::iterator(*alg) (Table&));

void print(const Table& table);
void dprint(const Table& table);

void insert(Table& table, const Table::iterator& iter, int VPN);

void doReset(Table& table);

Table::iterator find(Table& table, int VPN);

Table::iterator getBlockToReplace(Table& table, Table::iterator(*alg) (Table&));
Table::iterator getEmpty(Table& table);

Table::iterator FIFO(Table& table);


int main(int argc, char *argv[]) {
    //if (argc > 1) {
    //    if (strcmp("1", argv[1]) == 0) {
    //        secondChance();
    //    }
    //    else if (strcmp("2", argv[1]) == 0) {
    //        NFU();
    //    }
    //    else {
    //        std::cout << "Wrong input\n";
    //    }
    //}
    //else {
    //    std::cout << "No input\n";
    //}
    run(FIFO);
    return 0;
}

void run(Table::iterator(*alg) (Table&)) {
    Table table;
    PhysicalPage temp;
    temp.VPN = -1;
    temp.R = false;
    temp.M = false;
    temp.que = -1;
    temp.counter = 0;
    for (auto it = table.begin(); it < table.end(); it++) {
        *it = temp;
    }

    Command command;
    int resetCounter = 0;
    while (std::cin >> command.type >> command.VPN) {
        if (resetCounter == 5) {
            doReset(table);
            resetCounter = 0;
        }
    
        Table::iterator block = find(table, command.VPN);
        if (block == table.end()) {
            block = getBlockToReplace(table, alg);
            insert(table, block, command.VPN);
            block->M = false;
            block->counter = 0;
        }

        block->R = true;
        if (command.type == WRITE) {
            block->M = true;
        }

        dprint(table);
        resetCounter++;
    }
}

void print(const Table& table) {
    for (auto it = table.begin(); it < table.end(); it++) {
        if (it->VPN == -1) {
            std::cout << '#';
        }
        else {
            std::cout << it->VPN;
        }
        if (it + 1 != table.end()) std::cout << ' ';
    }
    std::cout << '\n';
}

void dprint(const Table& table) {
    for (auto p : table) {
        if (p.VPN == -1) {
            std::cout << '#';
        }
        else {
            std::cout << p.VPN;
        }
        std::cout << '(' << (int)p.R << ',' << (int)p.M << ',' << p.counter << ") ";
    }
    std::cout << "\n\n";
}

Table::iterator find(Table& table, int VPN) {
    for (auto it = table.begin(); it < table.end(); it++) {
        if (it->VPN == VPN) return it;
    }
    return table.end();
}

void doReset(Table& table) {
    for (auto it = table.begin(); it < table.end(); it++) {
        it->counter += it->R;
        it->R = false;
    }
}

Table::iterator getBlockToReplace(Table& table, Table::iterator (*alg) (Table&)) {
    auto block = getEmpty(table);
    if (block != table.end()) return block;
    return alg(table);
}

Table::iterator getEmpty(Table& table) {
    for (auto it = table.begin(); it < table.end(); it++) {
        if (it->VPN == -1) return it;
    }
    return table.end();
}


Table::iterator FIFO(Table& table) {
    Table::iterator iter;
    for (auto it = table.begin(); it < table.end(); it++) {
        if (it->que == 0) {
            iter = it;
            break;
        }
    }
    return iter;
}


void insert(Table& table, const Table::iterator& iter, int VPN) {
    for (auto it = table.begin(); it < table.end(); it++) {
        if (it->que != -1) it->que--;
    }
    iter->VPN = VPN;
    iter->que = SIZE - 1;
}
