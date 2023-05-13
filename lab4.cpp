#include <iostream>
#include <vector>
#include <queue>
#include <list>
#include <string>
#include <bitset>
#include <climits>
#include <algorithm>
#include "lab4.h"
/*Используемые алгоритмы :
* Second chance
* Working set
*/
//таймер в данной задаче отсчитывает каждую инструкцию доступа к памяти
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

//страница из таблицы страниц
class Page {
    //20 бит под номер виртуальной страницы
    constexpr static int bitsInVPN = 20;
    //номер виртуальной страницы
    std::bitset<bitsInVPN> VPN;
    //биты: обращения, модификации, пустоты - (используется для корректного отображения содержимого страницы)

public:
    bool R, M, E;
    Page() {
        R = 0;
        M = 0;
        E = 1;
        VPN.reset(); //установка всех битов в 0
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
        maxVPN.set(); //установка всех битов в 1
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
    //вектор страниц, содержащихся таблице страниц (page table)
    std::vector<Page> PT;
    const int PTsize = 5;
    Timer tim;

    Memory() { PT.resize(PTsize); }

    //действия алгоритма при обращении к загруженной странице
    virtual void founded_page_actions(int PPN) {}
    //действия алгоритма при загрузке страницы на пустое место
    virtual void loaded_page_actions(int PPN) {}
    //действия алгоритма при вытеснении страницы
    virtual int swapped_page_actions() { return 0; }
public:
    unsigned int pageMissCounter = 0;
    virtual void operation(bool opCode, int VPN) {
        //поиск страницы в таблице страниц
        int PPN = find_page(VPN);
        //если страница не найдена
        if (PPN == -1) {
            //поиск пустой страницы
            PPN = find_empty();
            //если пустых страниц нет
            if (PPN == -1)
                PPN = swapped_page_actions();
            else loaded_page_actions(PPN);
            //поместить новую страницу 
            PT[PPN].set_VPN(VPN);
            ++pageMissCounter; //для анализа кол-во ошибок отсутствия страницы (отладочная информация)
        }
        else founded_page_actions(PPN);
        //произвести операцию
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
        /*Взять страницу из очереди, если R = 0 (pop)
        * иначе - сбросить R и поместить в конец (reset, pop, push)
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
        //установка текущего времени для всех страниц с R=1
        for (int i = 0; i < PTsize; ++i)
            if (PT[i].R && !PT[i].E)
                pagesTime[i] = tim.get();
        pagesTime[PPN] = tim.get();
    }

    virtual int swapped_page_actions() {
        //установка текущего времени для всех страниц с R=1
        for (int i = 0; i < PTsize; ++i)
            if (PT[i].R)
                pagesTime[i] = tim.get();

        //расчет возраста каждой страницы
        std::vector<std::uint64_t> pagesAge(PTsize);
        for (int i = 0; i < PTsize; ++i)
            pagesAge[i] = tim.get() - pagesTime[i];

        //выборка страниц с максимальным возрастом
        std::vector<std::uint64_t>::iterator maxAgeIt = std::max_element(pagesAge.begin(),
            pagesAge.end());
        std::vector<std::uint64_t> maxAgePages;
        maxAgePages.reserve(PTsize);
        for (int i = 0; i < PTsize; ++i)
            if (pagesAge[i] == *maxAgeIt)
                maxAgePages.push_back(i);

        int displacePageNum;
        //если страница с максимальным возрастом не одна
        if (maxAgePages.size() != 1) {
            //выбрать только не модифицированные
            std::vector<std::uint64_t> notModifiedPages;
            notModifiedPages.reserve(maxAgePages.size());
            for (int i = 0; i < maxAgePages.size(); ++i)
                if (!PT[maxAgePages[i]].M)
                    notModifiedPages.push_back(maxAgePages[i]);

            //если есть не модифицированные страницы -> случайно вернуть одну из них
            if (notModifiedPages.size() != 0)
                displacePageNum = notModifiedPages[uniform_rnd(0, notModifiedPages.size() - 1)];
            //если есть нет не модифицированных страниц -> случайно вернуть одну из самых старых
            else
                displacePageNum = maxAgePages[uniform_rnd(0, maxAgePages.size() - 1)];
        }
        else displacePageNum = maxAgePages[0];

        //присвоить новой странице текущее время
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