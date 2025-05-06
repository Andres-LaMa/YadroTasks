#include "Computer_Club.hpp"

#include <sstream>

bool ComputerClub::isOpenAt(const std::chrono::minutes& time) const {
  return time >= openTime && time < closeTime;
}

void ComputerClub::freeTable(int tableNum,
                             const std::chrono::minutes& currentTime) {
  if (tableNum < 1 || tableNum > tableCount) return;
  Table& table = tables[tableNum - 1];

  if (table.isBusy) {
    auto duration = currentTime - table.startTime;
    int hours = duration.count() / 60;
    if (duration.count() % 60 != 0) hours++;
    table.revenue += hours * hourCost;

    table.busyTime += duration;

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

      std::stringstream ss;
      ss << format("{:%H:%M}", currentTime) << " 12 " << nextClient << " "
         << tableNum;
      eventsOutput.push_back(ss.str());
    }
  }
}

ComputerClub::ComputerClub(int tables, const std::chrono::minutes& open,
                           const std::chrono::minutes& close, int cost)
    : tableCount(tables), openTime(open), closeTime(close), hourCost(cost) {
  for (int i = 1; i <= tableCount; ++i) {
    this->tables.emplace_back(i);
  }
}

void ComputerClub::processEvent(const std::chrono::minutes& time, int eventId,
                                const std::vector<std::string>& eventBody) {
  std::string timeStr = format("{:%H:%M}", time);

  if (!isOpenAt(time) && eventId != 11 && eventId != 12 && eventId != 13) {
    if (eventBody.size() != 1) {
      eventsOutput.push_back(timeStr + " " + std::to_string(eventId) + " " +
                             eventBody[0] + " " + eventBody[1]);
    } else {
      eventsOutput.push_back(timeStr + " " + std::to_string(eventId) + " " +
                             eventBody[0]);
    }

    eventsOutput.push_back(timeStr + " 13 NotOpenYet");
    return;
  }

  switch (eventId) {
    case 1: {
      if (eventBody.size() != 1) {
        throw std::invalid_argument("Invalid event format");
      }
      std::string clientName = eventBody[0];

      eventsOutput.push_back(timeStr + " " + std::to_string(eventId) + " " +
                             eventBody[0]);
      if (clients.count(clientName)) {
        eventsOutput.push_back(timeStr + " 13 YouShallNotPass");
      } else if (!isOpenAt(time)) {
        eventsOutput.push_back(timeStr + " 13 NotOpenYet");
      } else {
        clients[clientName] = 0;
      }
      break;
    }
    case 2: {
      if (eventBody.size() != 2) {
        throw std::invalid_argument("Invalid event format");
      }

      eventsOutput.push_back(timeStr + " " + std::to_string(eventId) + " " +
                             eventBody[0] + " " + eventBody[1]);

      std::string clientName = eventBody[0];
      int tableNum = stoi(eventBody[1]);

      if (tableNum < 1 || tableNum > tableCount) {
        throw std::invalid_argument("Invalid table number");
      }

      if (clients.count(clientName) == 0) {
        eventsOutput.push_back(timeStr + " 13 ClientUnknown");
        return;
      }

      Table& table = tables[tableNum - 1];
      if (table.isBusy && table.currentClient != clientName) {
        eventsOutput.push_back(timeStr + " 13 PlaceIsBusy");
        return;
      }

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

      eventsOutput.push_back(timeStr + " " + std::to_string(eventId) + " " +
                             eventBody[0]);

      std::string clientName = eventBody[0];

      if (clients.count(clientName) == 0) {
        eventsOutput.push_back(timeStr + " 13 ClientUnknown");
        return;
      }

      bool hasFreeTables = false;
      for (const auto& table : tables) {
        if (!table.isBusy) {
          hasFreeTables = true;
          break;
        }
      }

      if (hasFreeTables) {
        eventsOutput.push_back(timeStr + " 13 ICanWaitNoLonger!");
        return;
      }

      if (waitingQueue.size() >= static_cast<size_t>(tableCount)) {
        clients.erase(clientName);
        eventsOutput.push_back(timeStr + " 11 " + clientName);
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

      eventsOutput.push_back(timeStr + " " + std::to_string(eventId) + " " +
                             eventBody[0]);

      std::string clientName = eventBody[0];

      if (clients.count(clientName) == 0) {
        eventsOutput.push_back(timeStr + " 13 ClientUnknown");
        return;
      }

      int tableNum = clients[clientName];
      if (tableNum > 0) {
        freeTable(tableNum, time);
      } else {
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

void ComputerClub::endDay() {
  std::vector<std::string> remainingClients;
  for (const auto& client : clients) {
    remainingClients.push_back(client.first);
  }
  std::sort(remainingClients.begin(), remainingClients.end());

  for (const auto& client : remainingClients) {
    int tableNum = clients[client];
    if (tableNum > 0) {
      freeTable(tableNum, closeTime);
    }
    eventsOutput.push_back(format("{:%H:%M}", closeTime) + " 11 " + client);
  }

  clients.clear();
  while (!waitingQueue.empty()) waitingQueue.pop();
}

std::vector<std::string> ComputerClub::generateReport() const {
  std::vector<std::string> report;
  report.push_back(format("{:%H:%M}", openTime));

  for (const auto& event : eventsOutput) {
    report.push_back(event);
  }

  report.push_back(format("{:%H:%M}", closeTime));

  for (const auto& table : tables) {
    std::stringstream ss;
    auto hours = table.busyTime.count() / 60;
    auto mins = table.busyTime.count() % 60;
    ss << table.number << " " << table.revenue << " " << std::setw(2)
       << std::setfill('0') << hours << ":" << std::setw(2) << std::setfill('0')
       << mins;
    report.push_back(ss.str());
  }

  return report;
}