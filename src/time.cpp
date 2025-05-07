#include "time.hpp"
#include <stdexcept>

Time::Time() : hours(0), minutes(0) {}
Time::Time(int h, int m) : hours(h), minutes(m) {}

bool Time::operator<(const Time& other) const {
    return hours < other.hours || (hours == other.hours && minutes < other.minutes);
}

bool Time::operator<=(const Time& other) const {
    return *this < other || (hours == other.hours && minutes == other.minutes);
}

Time Time::operator+(const Time& other) const {
    int total = minutes + other.minutes;
    int h = hours + other.hours + total / 60;
    int m = total % 60;
    return Time(h, m);
}

Time Time::operator-(const Time& other) const {
    int this_min = hours * 60 + minutes;
    int other_min = other.hours * 60 + other.minutes;
    int diff = this_min - other_min;
    if (diff < 0) diff = 0;
    return Time(diff / 60, diff % 60);
}

std::string Time::toString() const {
    std::stringstream ss;
    ss << std::setw(2) << std::setfill('0') << hours << ":"
       << std::setw(2) << std::setfill('0') << minutes;
    return ss.str();
}

Time parseTime(const std::string& timeStr) {
    size_t colon = timeStr.find(':');
    if (colon == std::string::npos || colon == 0 || colon == timeStr.size()-1)
        throw std::invalid_argument("Invalid time format");
    
    int h = std::stoi(timeStr.substr(0, colon));
    int m = std::stoi(timeStr.substr(colon+1));
    
    if (h < 0 || h > 23 || m < 0 || m > 59)
        throw std::invalid_argument("Invalid time value");
    
    return Time(h, m);
}