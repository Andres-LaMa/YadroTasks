#pragma once
#include <algorithm>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <map>
#include <queue>
#include <string>
#include <vector>

class ComputerClub
{
 private:
  struct Table
  {
    int number;
    int revenue = 0;
    std::chrono::minutes busyTime{0};
    std::string currentClient;
    std::chrono::minutes startTime;
    bool isBusy = false;

    Table(int num)
        : number(num)
    {}
  };

  int tableCount;
  std::chrono::minutes openTime;
  std::chrono::minutes closeTime;
  int hourCost;
  std::vector<Table> tables;
  std::map<std::string, int> clients;
  std::queue<std::string> waitingQueue;
  std::vector<std::string> eventsOutput;

  void freeTable(int tableNum, const std::chrono::minutes &currentTime);

 public:
  bool isOpenAt(const std::chrono::minutes &time) const;

  ComputerClub(int tables, const std::chrono::minutes &open,
               const std::chrono::minutes &close, int cost);

  void processEvent(const std::chrono::minutes &time, int eventId,
                    const std::vector<std::string> &eventBody);
  void endDay();
  std::vector<std::string> generateReport() const;
};