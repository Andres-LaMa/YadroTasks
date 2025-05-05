#pragma once
#include <algorithm>
#include <iostream>
#include <map>
#include <queue>
#include <string>
#include <vector>

#include "Time.cpp"

struct Table {
  int number;
  int revenue = 0;
  Time busyTime;
  std::string currentClient;
  Time startTime;
  bool isBusy = false;

  Table(int num) : number(num) {}
};

class ComputerClub {
 private:
  int tableCount;
  Time openTime;
  Time closeTime;
  int hourCost;
  std::vector<Table> tables;
  std::map<std::string, int> clients;
  std::queue<std::string> waitingQueue;
  std::vector<std::string> eventsOutput;

  bool isOpenAt(const Time& time) const {
    return time >= openTime && time < closeTime;
  }

  void freeTable(int tableNum, const Time& currentTime) {
    if (tableNum < 1 || tableNum > tableCount) return;
    Table& table = tables[tableNum - 1];

    if (table.isBusy) {
      Time duration = currentTime - table.startTime;
      int hours = duration.toMinutes() / 60;
      if (duration.toMinutes() % 60 != 0) hours++;
      table.revenue += hours * hourCost;

      table.busyTime = table.busyTime + (currentTime - table.startTime);

      table.isBusy = false;
      clients.erase(table.currentClient);
      table.currentClient = "";

      if (!waitingQueue.empty()) {
        std::string nextClient = waitingQueue.front();
        waitingQueue.pop();
        clients[nextClient] = tableNum;
        table.currentClient = nextClient;
        table.startTime = currentTime;
        table.isBusy = true;

        eventsOutput.push_back(currentTime.toString() + " 12 " + nextClient +
                               " " + std::to_string(tableNum));
      }
    }
  }

 public:
  ComputerClub(int tables, const Time& open, const Time& close, int cost)
      : tableCount(tables), openTime(open), closeTime(close), hourCost(cost) {
    for (int i = 1; i <= tableCount; ++i) {
      this->tables.emplace_back(i);
    }
  }

  void processEvent(const Time& time, int eventId,
                    const std::vector<std::string>& eventBody) {
    if (!isOpenAt(time) && eventId != 11 && eventId != 12 && eventId != 13) {
      if (eventBody.size() != 1) {
        eventsOutput.push_back(time.toString() + " " + std::to_string(eventId) +
                               " " + eventBody[0] + " " + eventBody[1]);
      } else {
        eventsOutput.push_back(time.toString() + " " + std::to_string(eventId) +
                               " " + eventBody[0]);
      }

      eventsOutput.push_back(time.toString() + " 13 NotOpenYet");
      return;
    }

    switch (eventId) {
      case 1: {
        if (eventBody.size() != 1) {
          throw std::invalid_argument("Invalid event format");
        }
        std::string clientName = eventBody[0];

        eventsOutput.push_back(time.toString() + " " + std::to_string(eventId) +
                               " " + eventBody[0]);
        if (clients.count(clientName)) {
          eventsOutput.push_back(time.toString() + " 13 YouShallNotPass");
        } else if (!isOpenAt(time)) {
          eventsOutput.push_back(time.toString() + " 13 NotOpenYet");
        } else {
          clients[clientName] = 0;
        }
        break;
      }
      case 2: {
        if (eventBody.size() != 2) {
          throw std::invalid_argument("Invalid event format");
        }

        eventsOutput.push_back(time.toString() + " " + std::to_string(eventId) +
                               " " + eventBody[0] + " " + eventBody[1]);

        std::string clientName = eventBody[0];
        int tableNum = stoi(eventBody[1]);

        if (tableNum < 1 || tableNum > tableCount) {
          throw std::invalid_argument("Invalid table number");
        }

        if (clients.count(clientName) == 0) {
          eventsOutput.push_back(time.toString() + " 13 ClientUnknown");
          return;
        }

        Table& table = tables[tableNum - 1];
        if (table.isBusy && table.currentClient != clientName) {
          eventsOutput.push_back(time.toString() + " 13 PlaceIsBusy");
          return;
        }

        // если клиент сидит за другим столом, освобождаем его и сажаем за
        // другой
        int prevTable = clients[clientName];
        if (prevTable > 0 && prevTable != tableNum) {
          freeTable(prevTable, time);
        }

        table.currentClient = clientName;
        table.startTime = time;
        table.isBusy = true;
        clients[clientName] = tableNum;
        break;
      }
      case 3: {
        if (eventBody.size() != 1) {
          throw std::invalid_argument("Invalid event format");
        }

        eventsOutput.push_back(time.toString() + " " + std::to_string(eventId) +
                               " " + eventBody[0]);

        std::string clientName = eventBody[0];

        if (clients.count(clientName) == 0) {
          eventsOutput.push_back(time.toString() + " 13 ClientUnknown");
          return;
        }

        // есть свободные столы
        bool hasFreeTables = false;
        for (const auto& table : tables) {
          if (!table.isBusy) {
            hasFreeTables = true;
            break;
          }
        }

        if (hasFreeTables) {
          eventsOutput.push_back(time.toString() + " 13 ICanWaitNoLonger!");
          return;
        }
        // если свободных стоолов нет, то проверяем есть ли место в очереди.
        // если места нет, то клиент уходит
        if (waitingQueue.size() >= tableCount) {
          clients.erase(clientName);
          eventsOutput.push_back(time.toString() + " 11 " + clientName);
        } else {
          waitingQueue.push(clientName);
          clients[clientName] = 0;
        }
        break;
      }
      case 4: {
        if (eventBody.size() != 1) {
          throw std::invalid_argument("Invalid event format");
        }

        eventsOutput.push_back(time.toString() + " " + std::to_string(eventId) +
                               " " + eventBody[0]);

        std::string clientName = eventBody[0];

        if (clients.count(clientName) == 0) {
          eventsOutput.push_back(time.toString() + " 13 ClientUnknown");
          return;
        }

        int tableNum = clients[clientName];
        if (tableNum > 0) {
          freeTable(tableNum, time);
        } else {
          // Удаляем из очереди ожидания
          std::queue<std::string> newQueue;
          while (!waitingQueue.empty()) {
            std::string client = waitingQueue.front();
            waitingQueue.pop();
            if (client != clientName) {
              newQueue.push(client);
            }
          }
          waitingQueue = newQueue;
          clients.erase(clientName);
        }
        break;
      }
      default:
        throw std::invalid_argument("Unknown event ID");
    }
  }

  void endDay() {
    std::vector<std::string> remainingClients;
    for (const auto& client : clients) {
      remainingClients.push_back(client.first);
    }
    std::sort(remainingClients.begin(), remainingClients.end());

    // выгныть клиентов
    for (const auto& client : remainingClients) {
      int tableNum = clients[client];
      if (tableNum > 0) {
        freeTable(tableNum, closeTime);
      }
      eventsOutput.push_back(closeTime.toString() + " 11 " + client);
    }

    clients.clear();
    while (!waitingQueue.empty()) waitingQueue.pop();
  }

  std::vector<std::string> generateReport() const {
    std::vector<std::string> report;
    report.push_back(openTime.toString());

    for (const auto& event : eventsOutput) {
      report.push_back(event);
    }

    report.push_back(closeTime.toString());

    for (const auto& table : tables) {
      std::stringstream ss;
      ss << table.number << " " << table.revenue << " "
         << table.busyTime.toString();
      report.push_back(ss.str());
    }

    return report;
  }
};