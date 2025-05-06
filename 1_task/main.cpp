#include <chrono>
#include <fstream>
#include <iomanip>
#include <sstream>

#include "Computer_Club.hpp"

void processInputFile(const std::string& filename) {
  std::ifstream file(filename);
  if (!file.is_open()) {
    std::cerr << "Error: could not open file " << filename << "\n";
    exit(1);
  }

  std::string line;
  std::vector<std::string> lines;

  for (size_t i = 0; i < 3 && getline(file, line); i++) {
    if (!line.empty()) {
      lines.push_back(line);
    }
  }

  if (lines.size() < 3) {
    std::cerr << "Error: not enough input data" << "\n";
    exit(1);
  }

  int tableCount;
  try {
    tableCount = stoi(lines[0]);
    if (tableCount <= 0)
      throw std::invalid_argument("Table count must be positive");
  } catch (...) {
    std::cerr << "Error: invalid table count" << "\n";
    exit(1);
  }

  std::chrono::minutes openTime, closeTime;  // test chrono 1
  try {
    size_t spacePos = lines[1].find(' ');
    if (spacePos == std::string::npos)
      throw std::invalid_argument("Invalid time range format");

    std::string openStr = lines[1].substr(0, spacePos);
    std::string closeStr = lines[1].substr(spacePos + 1);

    std::istringstream issOpen(openStr);
    issOpen >> std::chrono::parse("%H:%M", openTime);

    std::istringstream issClose(closeStr);
    issClose >> std::chrono::parse("%H:%M", closeTime);

    if (closeTime <= openTime)
      throw std::invalid_argument("Close time must be after open time");
  } catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    exit(1);
  }

  int hourCost;
  try {
    hourCost = stoi(lines[2]);
    if (hourCost <= 0)
      throw std::invalid_argument("Hour cost must be positive");
  } catch (...) {
    std::cerr << "Error: invalid hour cost" << "\n";
    exit(1);
  }

  ComputerClub club(tableCount, openTime, closeTime, hourCost);

  size_t num_str = 4;
  while (getline(file, line)) {
    if (line.empty()) {
      ++num_str;
      continue;
    }

    std::stringstream ss(line);
    std::string timeStr, eventIdStr;
    std::vector<std::string> eventBody;

    ss >> timeStr >> eventIdStr;
    std::string bodyPart;
    while (ss >> bodyPart) {
      eventBody.push_back(bodyPart);
    }

    try {
      std::chrono::minutes time;
      std::istringstream iss(timeStr);
      iss >> parse("%H:%M", time);

      int eventId = stoi(eventIdStr);

      club.processEvent(time, eventId, eventBody);
    } catch (const std::exception& e) {
      std::cerr << "Error in line " << (num_str + 1) << ": " << e.what()
                << std::endl;
      exit(1);
    }
    ++num_str;
  }

  club.endDay();

  std::vector<std::string> report = club.generateReport();
  for (const auto& line : report) {
    std::cout << line << std::endl;
  }
}

int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <input_file>" << std::endl;
    return 1;
  }

  try {
    processInputFile(argv[1]);
  } catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}