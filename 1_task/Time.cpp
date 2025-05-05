#pragma once
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

class Time {
 private:
  int hours;
  int minutes;

 public:
  Time() : hours(0), minutes(0) {}
  Time(int h, int m) : hours(h), minutes(m) {}

  static Time fromString(const std::string& timeStr) {
    size_t colonPos = timeStr.find(':');
    if (colonPos == std::string::npos || colonPos == 0 ||
        colonPos == timeStr.length() - 1) {
      throw std::invalid_argument("Invalid time format");
    }

    int h = stoi(timeStr.substr(0, colonPos));
    int m = stoi(timeStr.substr(colonPos + 1));

    if (h < 0 || h > 23 || m < 0 || m > 59) {
      throw std::invalid_argument("Invalid time value");
    }

    return Time(h, m);
  }

  std::string toString() const {
    std::stringstream ss;
    ss << std::setw(2) << std::setfill('0') << hours << ":" << std::setw(2)
       << std::setfill('0') << minutes;
    return ss.str();
  }

  bool operator<(const Time& other) const {
    if (hours != other.hours) return hours < other.hours;
    return minutes < other.minutes;
  }

  bool operator<=(const Time& other) const { return !(other < *this); }

  bool operator>(const Time& other) const { return other < *this; }

  bool operator>=(const Time& other) const { return !(*this < other); }

  bool operator==(const Time& other) const {
    return hours == other.hours && minutes == other.minutes;
  }

  int toMinutes() const { return hours * 60 + minutes; }

  static Time fromMinutes(int totalMinutes) {
    totalMinutes = totalMinutes % (24 * 60);
    if (totalMinutes < 0) totalMinutes += 24 * 60;
    return Time(totalMinutes / 60, totalMinutes % 60);
  }

  Time operator+(const Time& other) const {
    int total = toMinutes() + other.toMinutes();
    return fromMinutes(total);
  }

  Time operator-(const Time& other) const {
    int total = toMinutes() - other.toMinutes();
    return fromMinutes(total);
  }
};
