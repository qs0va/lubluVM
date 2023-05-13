#include <iostream>
#include <vector>
#include <queue>
#include <list>
#include <string>
#include <bitset>
#include <climits>
#include <algorithm>
#include "lab4.h"
/*������������ ��������� :
* Second chance
* Working set
*/
//������ � ������ ������ ����������� ������ ���������� ������� � ������
class Timer {
    std::uint64_t counter;
    std::uint64_t interruptEveryN = 5;
public:
    Timer() { counter = 0; }
    std::uint64_t inc() {
        if (ULLONG_MAX == ++counter)
            counter = 0;
        return counter;
    }
    std::uint64_t get() const { return counter; }
    bool is_interrupt() { return !(counter % interruptEveryN); }
};

//�������� �� ������� �������
class Page {
    //20 ��� ��� ����� ����������� ��������
    constexpr static int bitsInVPN = 20;
    //����� ����������� ��������
    std::bitset<bitsInVPN> VPN;
    //����: ���������, �����������, ������� - (������������ ��� ����������� ����������� ����������� ��������)

public:
    bool R, M, E;
    Page() {
        R = 0;
        M = 0;
        E = 1;
        VPN.reset(); //��������� ���� ����� � 0
    }
    void reference() { R = 1; }
    void modify() { M = 1; R = 1; }
    bool is_empty() const { return E; }
    void reset_R() { R = 0; }
    bool is_equal(std::bitset<bitsInVPN> VPN) const {
        return (!E && (this->VPN == VPN));
    }
    bool set_VPN(std::uint32_t VPN) {
        std::bitset<bitsInVPN> maxVPN;
        maxVPN.set(); //��������� ���� ����� � 1
        if (VPN > maxVPN.to_ulong()) return false;
        this->VPN = VPN;
        E = 0;
        R = 0;
        M = 0;
        return true;
    }
    std::string get_VPN() const {
        return E ? "#" : (std::to_string(VPN.to_ulong())
            //+ "(" + std::to_string(R) + ", " + std::to_string(M) + ')'
            );
    }

};

class Memory {
private:
    //VPN - virtual page number
    // PPN - physical page number
    int find_empty() const {
        for (int i = 0; i < PTsize; ++i)
            if (PT[i].is_empty()) return i;
        return -1;
    }
    int find_page(std::uint32_t VPN) const {
        for (int i = 0; i < PTsize; ++i)
            if (PT[i].is_equal(VPN)) return i;
        return -1;
    }
    void after_op_action() {
        tim.inc();
        if (tim.is_interrupt())
            for (Page& page : PT)
                page.reset_R();
    }
protected:
    //������ �������, ������������ ������� ������� (page table)
    std::vector<Page> PT;
    const int PTsize = 5;
    Timer tim;

    Memory() { PT.resize(PTsize); }

    //�������� ��������� ��� ��������� � ����������� ��������
    virtual void founded_page_actions(int PPN) {}
    //�������� ��������� ��� �������� �������� �� ������ �����
    virtual void loaded_page_actions(int PPN) {}
    //�������� ��������� ��� ���������� ��������
    virtual int swapped_page_actions() { return 0; }
public:
    unsigned int pageMissCounter = 0;
    virtual void operation(bool opCode, int VPN) {
        //����� �������� � ������� �������
        int PPN = find_page(VPN);
        //���� �������� �� �������
        if (PPN == -1) {
            //����� ������ ��������
            PPN = find_empty();
            //���� ������ ������� ���
            if (PPN == -1)
                PPN = swapped_page_actions();
            else loaded_page_actions(PPN);
            //��������� ����� �������� 
            PT[PPN].set_VPN(VPN);
            ++pageMissCounter; //��� ������� ���-�� ������ ���������� �������� (���������� ����������)
        }
        else founded_page_actions(PPN);
        //���������� ��������
        opCode ? PT[PPN].modify() : PT[PPN].reference();
        after_op_action();
    }

    virtual std::string get_PT() const {
        std::string str;
        for (Page page : PT)
            str += (page.get_VPN() + ' ');
        str.pop_back();
        return str;
    };
};

class MemorySecondChance : public Memory {
private:
    std::queue<int> pagesNums;
    int swapped_page_actions() override {
        int pageNum;
        bool exit = false;
        /*����� �������� �� �������, ���� R = 0 (pop)
        * ����� - �������� R � ��������� � ����� (reset, pop, push)
        */
        while (!exit) {
            pageNum = pagesNums.front();
            if (PT[pageNum].R)
                PT[pageNum].reset_R();
            else exit = true;
            pagesNums.push(pageNum);
            pagesNums.pop();
        };
        return pageNum;
    }

    void loaded_page_actions(int PPN) override { pagesNums.push(PPN); };
public:
    MemorySecondChance() : Memory() {};
};


class MemoryWorkingSet : public Memory {
private:
    std::vector<std::uint64_t> pagesTime;

    virtual void loaded_page_actions(int PPN) {
        //��������� �������� ������� ��� ���� ������� � R=1
        for (int i = 0; i < PTsize; ++i)
            if (PT[i].R && !PT[i].E)
                pagesTime[i] = tim.get();
        pagesTime[PPN] = tim.get();
    }

    virtual int swapped_page_actions() {
        //��������� �������� ������� ��� ���� ������� � R=1
        for (int i = 0; i < PTsize; ++i)
            if (PT[i].R)
                pagesTime[i] = tim.get();

        //������ �������� ������ ��������
        std::vector<std::uint64_t> pagesAge(PTsize);
        for (int i = 0; i < PTsize; ++i)
            pagesAge[i] = tim.get() - pagesTime[i];

        //������� ������� � ������������ ���������
        std::vector<std::uint64_t>::iterator maxAgeIt = std::max_element(pagesAge.begin(),
            pagesAge.end());
        std::vector<std::uint64_t> maxAgePages;
        maxAgePages.reserve(PTsize);
        for (int i = 0; i < PTsize; ++i)
            if (pagesAge[i] == *maxAgeIt)
                maxAgePages.push_back(i);

        int displacePageNum;
        //���� �������� � ������������ ��������� �� ����
        if (maxAgePages.size() != 1) {
            //������� ������ �� ����������������
            std::vector<std::uint64_t> notModifiedPages;
            notModifiedPages.reserve(maxAgePages.size());
            for (int i = 0; i < maxAgePages.size(); ++i)
                if (!PT[maxAgePages[i]].M)
                    notModifiedPages.push_back(maxAgePages[i]);

            //���� ���� �� ���������������� �������� -> �������� ������� ���� �� ���
            if (notModifiedPages.size() != 0)
                displacePageNum = notModifiedPages[uniform_rnd(0, notModifiedPages.size() - 1)];
            //���� ���� ��� �� ���������������� ������� -> �������� ������� ���� �� ����� ������
            else
                displacePageNum = maxAgePages[uniform_rnd(0, maxAgePages.size() - 1)];
        }
        else displacePageNum = maxAgePages[0];

        //��������� ����� �������� ������� �����
        pagesTime[displacePageNum] = tim.get();
        return displacePageNum;
    }

public:
    MemoryWorkingSet() : Memory() { pagesTime.resize(PTsize); }
};

void do_operations(Memory* mem) {
    bool opCode;
    int VPN;
    while (std::cin >> opCode >> VPN) {
        (*mem).operation(opCode, VPN);
        std::cout << (*mem).get_PT() << std::endl;
        //std::cout << (*mem).pageMissCounter << std::endl;
    }
}

int main(int argc, char* argv[])
{
    int alg = 1;
    Memory* memory;
    if (argc > 1)
        alg = std::atoi(argv[1]);
    if (alg == 1) memory = new MemorySecondChance();
    else memory = new MemoryWorkingSet();
    do_operations(memory);
    delete memory;
    return 0;
}