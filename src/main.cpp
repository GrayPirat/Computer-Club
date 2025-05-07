#include "computer_club.hpp"
#include <fstream>
#include <iostream>
using namespace std; 

ComputerClub readInput(const string& filename) {
  ifstream file(filename);
  if (!file.is_open()) {
    throw runtime_error("Cannot open file");
  }

  string line;
  int lineNum = 0;

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
        std::cerr << "Usage: " << argv[0] << " <input_file>\n";
        return 1;
    }
    
    try {
        ComputerClub club = readInput(argv[1]);
        club.endDay();
        
        std::cout << club.getOpenTime().toString() << "\n";
        for (const auto& line : club.getOutput()) 
            std::cout << line << "\n";
        std::cout << club.getCloseTime().toString() << "\n";
        
        for (const auto& table : club.getTables()) {
            std::cout << table.number << " " << table.revenue << " "
                      << table.occupiedTime.toString() << "\n";
        }
    } catch (const std::exception& e) {
        std::cerr << e.what() << "\n";
        return 1;
    }
    return 0;
}