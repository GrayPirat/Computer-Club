#include <algorithm>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <queue>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

// Структура для хранения времени
struct Time {
  int hours;
  int minutes;

  Time() : hours(0), minutes(0) {}
  Time(int h, int m) : hours(h), minutes(m) {}

  bool operator<(const Time& other) const {
    if (hours == other.hours) {
      return minutes < other.minutes;
    }
    return hours < other.hours;
  }

  bool operator<=(const Time& other) const {
    return *this < other || (hours == other.hours && minutes == other.minutes);
  }

  Time operator+(const Time& other) const {
    int total_minutes = minutes + other.minutes;
    int extra_hours = total_minutes / 60;
    total_minutes %= 60;
    return Time(hours + other.hours + extra_hours, total_minutes);
  }

  Time operator-(const Time& other) const {
    int total_minutes = (hours * 60 + minutes) - (other.hours * 60 + other.minutes);
    if (total_minutes < 0) total_minutes = 0;
    return Time(total_minutes / 60, total_minutes % 60);
  }

  string toString() const {
    stringstream ss;
    ss << setw(2) << setfill('0') << hours << ":" << setw(2) << setfill('0') << minutes;
    return ss.str();
  }
};

// Функция для парсинга времени из строки
Time parseTime(const string& timeStr) {
  size_t colonPos = timeStr.find(':');
  if (colonPos == string::npos || colonPos == 0 || colonPos == timeStr.length() - 1) {
    throw invalid_argument("Invalid time format");
  }
  int hours = stoi(timeStr.substr(0, colonPos));
  int minutes = stoi(timeStr.substr(colonPos + 1));
  if (hours < 0 || hours > 23 || minutes < 0 || minutes > 59) {
    throw invalid_argument("Invalid time value");
  }
  return Time(hours, minutes);
}

// Структура для хранения информации о столе
struct Table {
  int number;
  int revenue;
  Time occupiedTime;
  string currentClient;

  Table(int num) : number(num), revenue(0), occupiedTime(0, 0), currentClient("") {}
};

// Структура для хранения события
struct Event {
  Time time;
  int id;
  vector<string> body;

  string toString() const {
    stringstream ss;
    ss << time.toString() << " " << id;
    for (const auto& part : body) {
      ss << " " << part;
    }
    return ss.str();
  }
};

// Класс для управления компьютерным клубом
class ComputerClub {
 private:
  int numTables;
  Time openTime;
  Time closeTime;
  int hourCost;
  vector<Table> tables;
  map<string, int> clients;  // клиент -> номер стола (0 если в очереди)
  queue<string> waitingQueue;
  vector<string> output;

  // Проверка, открыт ли клуб в указанное время
  bool isOpenAt(const Time& time) const { return openTime <= time && time <= closeTime; }

  // Проверка, есть ли свободные столы
  bool hasFreeTables() const {
    for (const auto& table : tables) {
      if (table.currentClient.empty()) {
        return true;
      }
    }
    return false;
  }

  // Освобождение стола и обработка очереди ожидания
  void freeTable(int tableNum, const Time& time) {
    if (tableNum < 1 || tableNum > numTables) return;

    Table& table = tables[tableNum - 1];
    if (!table.currentClient.empty()) {
      // Расчет времени занятия и выручки
      Time duration = time - parseTime(clients[table.currentClient] == tableNum ? table.currentClient : "00:00");
      int hours = duration.hours + (duration.minutes > 0 ? 1 : 0);
      table.revenue += hours * hourCost;
      table.occupiedTime = table.occupiedTime + duration;

      clients.erase(table.currentClient);
      table.currentClient = "";

      // Обработка очереди ожидания
      if (!waitingQueue.empty()) {
        string nextClient = waitingQueue.front();
        waitingQueue.pop();
        clients[nextClient] = tableNum;
        table.currentClient = nextClient;

        // Генерация события ID 12
        Event sitEvent;
        sitEvent.time = time;
        sitEvent.id = 12;
        sitEvent.body = {nextClient, to_string(tableNum)};
        output.push_back(sitEvent.toString());
      }
    }
  }

 public:
  ComputerClub(int num, Time open, Time close, int cost)
      : numTables(num), openTime(open), closeTime(close), hourCost(cost) {
    for (int i = 1; i <= numTables; ++i) {
      tables.emplace_back(i);
    }
  }

  // Обработка входящего события
  void processEvent(const Event& event) {
    output.push_back(event.toString());

    try {
      switch (event.id) {
        case 1: {  // Клиент пришел
          const string& client = event.body[0];
          if (clients.count(client)) {
            throw runtime_error("YouShallNotPass");
          }
          if (!isOpenAt(event.time)) {
            throw runtime_error("NotOpenYet");
          }
          clients[client] = 0;  // 0 означает, что клиент в клубе, но не за столом
          break;
        }
        case 2: {  // Клиент сел за стол
          const string& client = event.body[0];
          int tableNum = stoi(event.body[1]);
          if (tableNum < 1 || tableNum > numTables) {
            throw runtime_error("PlaceIsBusy");
          }
          if (!clients.count(client)) {
            throw runtime_error("ClientUnknown");
          }
          if (!tables[tableNum - 1].currentClient.empty() && tables[tableNum - 1].currentClient != client) {
            throw runtime_error("PlaceIsBusy");
          }

          // Если клиент уже сидит за другим столом, освобождаем его
          if (clients[client] > 0 && clients[client] != tableNum) {
            freeTable(clients[client], event.time);
          }

          tables[tableNum - 1].currentClient = client;
          clients[client] = tableNum;
          break;
        }
        case 3: {  // Клиент ожидает
          const string& client = event.body[0];
          if (!clients.count(client)) {
            throw runtime_error("ClientUnknown");
          }
          if (hasFreeTables()) {
            throw runtime_error("ICanWaitNoLonger!");
          }
          if (waitingQueue.size() >= numTables) {
            // Клиент уходит, генерируем событие ID 11
            Event leaveEvent;
            leaveEvent.time = event.time;
            leaveEvent.id = 11;
            leaveEvent.body = {client};
            output.push_back(leaveEvent.toString());
            clients.erase(client);
          } else {
            waitingQueue.push(client);
            clients[client] = 0;  // 0 означает в очереди
          }
          break;
        }
        case 4: {  // Клиент ушел
          const string& client = event.body[0];
          if (!clients.count(client)) {
            throw runtime_error("ClientUnknown");
          }
          int tableNum = clients[client];
          if (tableNum > 0) {
            freeTable(tableNum, event.time);
          } else {
            // Удаляем из очереди ожидания
            queue<string> newQueue;
            while (!waitingQueue.empty()) {
              string next = waitingQueue.front();
              waitingQueue.pop();
              if (next != client) {
                newQueue.push(next);
              }
            }
            waitingQueue = newQueue;
            clients.erase(client);
          }
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

  // Завершение рабочего дня
  void endDay() {
    // Собираем всех клиентов в алфавитном порядке
    vector<string> remainingClients;
    for (const auto& client : clients) {
      remainingClients.push_back(client.first);
    }
    sort(remainingClients.begin(), remainingClients.end());

    // Генерируем события ухода для каждого клиента
    for (const string& client : remainingClients) {
      Event leaveEvent;
      leaveEvent.time = closeTime;
      leaveEvent.id = 11;
      leaveEvent.body = {client};
      output.push_back(leaveEvent.toString());

      // Освобождаем стол, если клиент за ним сидел
      if (clients[client] > 0) {
        freeTable(clients[client], closeTime);
      }
    }

    clients.clear();
    while (!waitingQueue.empty()) {
      waitingQueue.pop();
    }
  }

  // Получение результатов
  vector<string> getOutput() const { return output; }

  vector<Table> getTables() const { return tables; }

  Time getOpenTime() const { return openTime; }

  Time getCloseTime() const { return closeTime; }
};

// Функция для чтения и проверки входных данных
ComputerClub readInput(const string& filename) {
  ifstream file(filename);
  if (!file.is_open()) {
    throw runtime_error("Cannot open file");
  }

  string line;
  int lineNum = 0;

  // Чтение количества столов
  int numTables = 0;
  if (getline(file, line)) {
    lineNum++;
    try {
      numTables = stoi(line);
      if (numTables <= 0) {
        throw runtime_error("Number of tables must be positive");
      }
    } catch (...) {
      throw runtime_error("Error in line " + to_string(lineNum) + ": invalid number of tables");
    }
  } else {
    throw runtime_error("Missing number of tables");
  }

  // Чтение времени работы
  Time openTime, closeTime;
  if (getline(file, line)) {
    lineNum++;
    size_t spacePos = line.find(' ');
    if (spacePos == string::npos || spacePos == 0 || spacePos == line.length() - 1) {
      throw runtime_error("Error in line " + to_string(lineNum) + ": invalid time range format");
    }
    try {
      openTime = parseTime(line.substr(0, spacePos));
      closeTime = parseTime(line.substr(spacePos + 1));
      if (closeTime < openTime) {
        throw runtime_error("Close time must be after open time");
      }
    } catch (const exception& e) {
      throw runtime_error("Error in line " + to_string(lineNum) + ": " + e.what());
    }
  } else {
    throw runtime_error("Missing working hours");
  }

  // Чтение стоимости часа
  int hourCost = 0;
  if (getline(file, line)) {
    lineNum++;
    try {
      hourCost = stoi(line);
      if (hourCost <= 0) {
        throw runtime_error("Hour cost must be positive");
      }
    } catch (...) {
      throw runtime_error("Error in line " + to_string(lineNum) + ": invalid hour cost");
    }
  } else {
    throw runtime_error("Missing hour cost");
  }

  ComputerClub club(numTables, openTime, closeTime, hourCost);

  // Чтение событий
  while (getline(file, line)) {
    lineNum++;
    if (line.empty()) continue;

    istringstream iss(line);
    vector<string> parts;
    string part;
    while (iss >> part) {
      parts.push_back(part);
    }

    if (parts.size() < 2) {
      throw runtime_error("Error in line " + to_string(lineNum) + ": event must have at least time and ID");
    }

    try {
      Time time = parseTime(parts[0]);
      int id = stoi(parts[1]);

      Event event;
      event.time = time;
      event.id = id;
      for (size_t i = 2; i < parts.size(); ++i) {
        event.body.push_back(parts[i]);
      }

      // Проверка формата событий
      switch (id) {
        case 1:
          if (event.body.size() != 1) {
            throw runtime_error("Event ID 1 must have 1 parameter (client name)");
          }
          break;
        case 2:
          if (event.body.size() != 2) {
            throw runtime_error("Event ID 2 must have 2 parameters (client name and table number)");
          }
          try {
            stoi(event.body[1]);
          } catch (...) {
            throw runtime_error("Invalid table number in event ID 2");
          }
          break;
        case 3:
          if (event.body.size() != 1) {
            throw runtime_error("Event ID 3 must have 1 parameter (client name)");
          }
          break;
        case 4:
          if (event.body.size() != 1) {
            throw runtime_error("Event ID 4 must have 1 parameter (client name)");
          }
          break;
        default:
          throw runtime_error("Unknown event ID: " + to_string(id));
      }

      club.processEvent(event);
    } catch (const exception& e) {
      throw runtime_error("Error in line " + to_string(lineNum) + ": " + e.what());
    }
  }

  return club;
}

int main(int argc, char* argv[]) {
  if (argc != 2) {
    cerr << "Usage: " << argv[0] << " <input_file>" << endl;
    return 1;
  }

  try {
    ComputerClub club = readInput(argv[1]);
    club.endDay();

    // Вывод результатов
    cout << club.getOpenTime().toString() << endl;
    for (const auto& line : club.getOutput()) {
      cout << line << endl;
    }
    cout << club.getCloseTime().toString() << endl;

    for (const auto& table : club.getTables()) {
      cout << table.number << " " << table.revenue << " " << table.occupiedTime.toString() << endl;
    }
  } catch (const exception& e) {
    cerr << e.what() << endl;
    return 1;
  }

  return 0;
}