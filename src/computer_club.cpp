#include "computer_club.hpp"
#include <algorithm>
#include <stdexcept>
#include <sstream>
#include <iomanip>

using namespace std;

Table::Table(int num) : number(num), revenue(0), occupiedTime(0,0), currentClient("") {}

string Event::toString() const {
    stringstream ss;
    ss << time.toString() << " " << id;
    for (const auto& part : body) ss << " " << part;
    return ss.str();
}

ComputerClub::ComputerClub(int num, Time open, Time close, int cost)
    : numTables(num), openTime(open), closeTime(close), hourCost(cost) {
    for (int i=1; i<=num; ++i) tables.emplace_back(i);
}

bool ComputerClub::isOpenAt(const Time& time) const {
    return openTime <= time && time <= closeTime;
}

bool ComputerClub::hasFreeTables() const {
    for (const auto& table : tables) {
        if (table.currentClient.empty()) return true;
    }
    return false;
}

void ComputerClub::freeTable(int tableNum, const Time& time) {
    if (tableNum < 1 || tableNum > numTables) return;

    Table& table = tables[tableNum-1];
    if (!table.currentClient.empty()) {
        // Получаем время начала занятия
        Time startTime = clients[table.currentClient].second;
        Time duration = time - startTime;
        
        // Расчет выручки (округляем вверх до часа)
        int hours = duration.hours + (duration.minutes > 0 ? 1 : 0);
        table.revenue += hours * hourCost;
        table.occupiedTime = table.occupiedTime + duration;

        // Удаляем клиента
        clients.erase(table.currentClient);
        table.currentClient = "";

        // Обработка очереди ожидания
        if (!waitingQueue.empty()) {
            string nextClient = waitingQueue.front();
            waitingQueue.pop();
            
            // Сажаем клиента за стол
            table.currentClient = nextClient;
            clients[nextClient] = {tableNum, time}; // Запоминаем время начала
            
            // Генерируем событие 12
            Event sitEvent;
            sitEvent.time = time;
            sitEvent.id = 12;
            sitEvent.body = {nextClient, to_string(tableNum)};
            output.push_back(sitEvent.toString());
        }
    }
}

void ComputerClub::processEvent(const Event& event) {
    output.push_back(event.toString());

    try {
        switch (event.id) {
            case 1: { // Клиент пришел
                const string& client = event.body[0];
                if (clients.count(client)) {
                    throw runtime_error("YouShallNotPass");
                }
                if (!isOpenAt(event.time)) {
                    throw runtime_error("NotOpenYet");
                }
                clients[client] = {0, Time()}; // 0 - не за столом
                break;
            }
            case 2: { // Клиент сел за стол
                const string& client = event.body[0];
                int tableNum = stoi(event.body[1]);
                
                if (tableNum < 1 || tableNum > numTables) {
                    throw runtime_error("PlaceIsBusy");
                }
                if (!clients.count(client)) {
                    throw runtime_error("ClientUnknown");
                }
                if (!tables[tableNum-1].currentClient.empty() && 
                    tables[tableNum-1].currentClient != client) {
                    throw runtime_error("PlaceIsBusy");
                }

                // Если клиент уже сидит за другим столом, освобождаем его
                int oldTable = clients[client].first;
                if (oldTable > 0 && oldTable != tableNum) {
                    freeTable(oldTable, event.time);
                }

                // Сажаем за новый стол
                tables[tableNum-1].currentClient = client;
                clients[client] = {tableNum, event.time}; // Запоминаем время начала
                break;
            }
            case 3: { // Клиент ожидает
                const string& client = event.body[0];
                if (!clients.count(client)) {
                    throw runtime_error("ClientUnknown");
                }
                if (hasFreeTables()) {
                    throw runtime_error("ICanWaitNoLonger!");
                }
                if (waitingQueue.size() >= numTables) {
                    // Клиент уходит (событие 11)
                    Event leaveEvent;
                    leaveEvent.time = event.time;
                    leaveEvent.id = 11;
                    leaveEvent.body = {client};
                    output.push_back(leaveEvent.toString());
                    clients.erase(client);
                } else {
                    waitingQueue.push(client);
                    clients[client].first = 0; // Помечаем как ожидающего
                }
                break;
            }
            case 4: { // Клиент ушел
                const string& client = event.body[0];
                if (!clients.count(client)) {
                    throw runtime_error("ClientUnknown");
                }
                
                int tableNum = clients[client].first;
                if (tableNum > 0) {
                    freeTable(tableNum, event.time);
                } else {
                    // Удаляем из очереди ожидания
                    queue<string> newQueue;
                    while (!waitingQueue.empty()) {
                        string next = waitingQueue.front();
                        waitingQueue.pop();
                        if (next != client) newQueue.push(next);
                    }
                    waitingQueue = newQueue;
                }
                clients.erase(client);
                break;
            }
            default:
                throw invalid_argument("Unknown event ID");
        }
    } catch (const exception& e) {
        // Генерация события ошибки
        Event errorEvent;
        errorEvent.time = event.time;
        errorEvent.id = 13;
        errorEvent.body = {e.what()};
        output.push_back(errorEvent.toString());
    }
}

void ComputerClub::endDay() {
    // Собираем клиентов в алфавитном порядке
    vector<string> remainingClients;
    for (const auto& client : clients) {
        remainingClients.push_back(client.first);
    }
    sort(remainingClients.begin(), remainingClients.end());

    // Обрабатываем уход клиентов
    for (const string& client : remainingClients) {
        Event leaveEvent;
        leaveEvent.time = closeTime;
        leaveEvent.id = 11;
        leaveEvent.body = {client};
        output.push_back(leaveEvent.toString());

        // Освобождаем стол, если клиент за ним сидел
        int tableNum = clients[client].first;
        if (tableNum > 0) {
            freeTable(tableNum, closeTime);
        }
    }

    // Очищаем данные
    clients.clear();
    while (!waitingQueue.empty()) waitingQueue.pop();
}

vector<string> ComputerClub::getOutput() const {
    return output;
}

vector<Table> ComputerClub::getTables() const {
    return tables;
}

Time ComputerClub::getOpenTime() const {
    return openTime;
}

Time ComputerClub::getCloseTime() const {
    return closeTime;
}