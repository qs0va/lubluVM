#include <iostream>
#include <array>
#include <queue>

#include "lab4.h"

#define READ 0
#define WRITE 1
#define SIZE 12

struct PhysicalPage {
    int VPN;
    bool R;
    bool M;
    unsigned int counter;
};

struct Command {
    int type;
    int VPN;
};

typedef std::array<PhysicalPage, SIZE> Table;

class CycleQueue {
private:
    class Node {
    public:
        int value;
        Node* next;
        Node* prev;

        Node(int n, Node* prev, Node* next) {
            value = n;
            this->prev = prev;
            this->next = next;
        }
    };

    Node* head = nullptr;

public:
    void push(int n) {
        if (head == nullptr) {
            head = new Node(n, head, head);
            head->next = head;
            head->prev = head;
        }
        else {
            Node* temp = head->prev;
            head->prev = new Node(n, temp, head);
            temp->next = head->prev;
        }
    }

    int look() {
        return head->value;
    }

    int pop() {
        int out = head->value;
        Node* temp = head;
        head = temp->next;
        temp->prev->next = head;
        head->prev = temp->prev;

        delete temp;
        return out;
    }

    void headInc() {
    if (head)
        head = head->next;
    }
};

CycleQueue que;

void run(Table::iterator(*alg) (Table&));

void print(const Table& table);
void dprint(const Table& table);

void doReset(Table& table);

Table::iterator find(Table& table, int VPN);

Table::iterator getBlockToReplace(Table& table, Table::iterator(*alg) (Table&));
Table::iterator getEmpty(Table& table);

Table::iterator clock(Table& table);
Table::iterator NFU(Table& table);


int main(int argc, char *argv[]) {
    if (argc > 1) {
        if (strcmp("1", argv[1]) == 0) {
            run(clock);
        }
        else if (strcmp("2", argv[1]) == 0) {
            run(NFU);
        }
    }
    return 0;
}

void run(Table::iterator(*alg) (Table&)) {
    Table table;
    PhysicalPage temp;
    temp.VPN = -1;
    temp.R = false;
    temp.M = false;
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
            block->VPN = command.VPN;
            que.push(command.VPN);
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

Table::iterator clock(Table& table) {
    while (true) {
        auto iter = find(table, que.look());
        if (iter->R) {
            iter->R = false;
            que.headInc();
        }
        else {
            que.pop();
            return iter;
        }
    }
}

Table::iterator NFU(Table& table) {
    unsigned int minCounter = 4000000;
    for (auto p : table) {
        if (p.counter < minCounter) minCounter = p.counter;
    }

    std::vector<int> subList;
    for (auto p : table) {
        if (p.counter == minCounter) {
            subList.push_back(p.VPN);
        }
    }

    int VPNToReplace;
    if (subList.size() > 1) {
        VPNToReplace = subList.at(uniform_rnd(0, subList.size() - 1));
    }
    else {
        VPNToReplace = subList.at(0);
    }
    return find(table, VPNToReplace);
}
