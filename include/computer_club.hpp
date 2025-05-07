#ifndef COMPUTERCLUB_H
#define COMPUTERCLUB_H

#include "time.hpp"
#include <vector>
#include <string>
#include <map>
#include <queue>

struct Table {
    int number;
    int revenue;
    Time occupiedTime;
    std::string currentClient;
    Table(int num);
};

struct Event {
    Time time;
    int id;
    std::vector<std::string> body;
    std::string toString() const;
};

class ComputerClub {
private:
    int numTables;
    Time openTime;
    Time closeTime;
    int hourCost;
    std::vector<Table> tables;
    std::map<std::string, std::pair<int, Time>> clients; // <client, <table_num, start_time>>
    std::queue<std::string> waitingQueue;
    std::vector<std::string> output;

    bool isOpenAt(const Time& time) const;
    bool hasFreeTables() const;
    void freeTable(int tableNum, const Time& time);

public:
    ComputerClub(int num, Time open, Time close, int cost);
    void processEvent(const Event& event);
    void endDay();
    std::vector<std::string> getOutput() const;
    std::vector<Table> getTables() const;
    Time getOpenTime() const;
    Time getCloseTime() const;
};

#endif // COMPUTERCLUB_H