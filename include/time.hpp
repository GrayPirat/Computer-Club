#ifndef TIME_H
#define TIME_H

#include <string>
#include <sstream>
#include <iomanip>

struct Time {
    int hours;
    int minutes;

    Time();
    Time(int h, int m);

    bool operator<(const Time& other) const;
    bool operator<=(const Time& other) const;
    Time operator+(const Time& other) const;
    Time operator-(const Time& other) const;
    std::string toString() const;
};

Time parseTime(const std::string& timeStr);

#endif // TIME_H