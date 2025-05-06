#include <gtest/gtest.h>

#include <chrono>
#include <format>
#include <sstream>

#include "Computer_Club.hpp"

using namespace std::chrono_literals;

TEST(ComputerClubTest, Initialization)
{
  ComputerClub club(3, 8h, 20h, 10);
  EXPECT_EQ(club.generateReport().size(), 5);
}

TEST(ComputerClubTest, IsOpenAt)
{
  ComputerClub club(3, 8h, 20h, 10);
  EXPECT_TRUE(club.isOpenAt(8h));
  EXPECT_TRUE(club.isOpenAt(10h));
  EXPECT_TRUE(club.isOpenAt(19h + 59min));
  EXPECT_FALSE(club.isOpenAt(7h + 59min));
  EXPECT_FALSE(club.isOpenAt(20h));
  EXPECT_FALSE(club.isOpenAt(20h + 1min));
}

TEST(ComputerClubTest, ClientArrival)
{
  ComputerClub club(2, 8h, 20h, 10);
  club.processEvent(9h, 1, {"Client1"});
  auto report = club.generateReport();
  EXPECT_EQ(report.size(), 5);  // hmmmmm...

  club.processEvent(9h + 30min, 1, {"Client1"});
  report = club.generateReport();
  EXPECT_TRUE(std::find(report.begin(), report.end(),
                        "09:30 13 YouShallNotPass") != report.end());
}

TEST(ComputerClubTest, TakeTable)
{
  ComputerClub club(2, 8h, 20h, 10);
  club.processEvent(9h, 1, {"Client1"});
  club.processEvent(9h + 30min, 2, {"Client1", "1"});

  club.processEvent(10h, 2, {"UnknownClient", "1"});
  auto report = club.generateReport();
  EXPECT_TRUE(std::find(report.begin(), report.end(),
                        "10:00 13 ClientUnknown") != report.end());

  club.processEvent(10h, 1, {"Client2"});
  club.processEvent(10h + 30min, 2, {"Client2", "1"});
  report = club.generateReport();
  EXPECT_TRUE(std::find(report.begin(), report.end(), "10:30 13 PlaceIsBusy") !=
              report.end());
}

TEST(ComputerClubTest, WaitingQueue)
{
  ComputerClub club(1, 8h, 20h, 10);
  club.processEvent(9h, 1, {"Client1"});
  club.processEvent(9h, 2, {"Client1", "1"});
  club.processEvent(9h + 30min, 1, {"Client2"});
  club.processEvent(9h + 45min, 3, {"Client2"});

  auto report = club.generateReport();
  EXPECT_FALSE(std::find(report.begin(), report.end(),
                         "09:45 13 ICanWaitNoLonger!") !=
               report.end());  // hmmmmm

  club.processEvent(10h, 1, {"Client3"});
  club.processEvent(10h, 3, {"Client3"});
  report = club.generateReport();
  EXPECT_TRUE(std::find(report.begin(), report.end(), "10:00 11 Client3") !=
              report.end());
}

TEST(ComputerClubTest, ClientLeave)
{
  ComputerClub club(1, 8h, 20h, 10);
  club.processEvent(9h, 1, {"Client1"});
  club.processEvent(9h, 2, {"Client1", "1"});
  club.processEvent(10h, 4, {"Client1"});

  auto report = club.generateReport();
  EXPECT_TRUE(std::find(report.begin(), report.end(), "10:00 4 Client1") !=
              report.end());

  club.processEvent(10h + 30min, 4, {"UnknownClient"});
  report = club.generateReport();
  EXPECT_TRUE(std::find(report.begin(), report.end(),
                        "10:30 13 ClientUnknown") != report.end());
}

TEST(ComputerClubTest, EndDay)
{
  ComputerClub club(2, 8h, 20h, 10);
  club.processEvent(9h, 1, {"Client1"});
  club.processEvent(9h, 2, {"Client1", "1"});
  club.processEvent(10h, 1, {"Client2"});
  club.processEvent(10h, 3, {"Client2"});

  club.endDay();
  auto report = club.generateReport();

  EXPECT_TRUE(std::find(report.begin(), report.end(), "20:00 11 Client1") !=
              report.end());
  EXPECT_TRUE(std::find(report.begin(), report.end(), "20:00 11 Client2") !=
              report.end());
}

TEST(ComputerClubTest, RevenueCalculation)
{
  ComputerClub club(1, 8h, 20h, 10);
  club.processEvent(9h, 1, {"Client1"});
  club.processEvent(9h, 2, {"Client1", "1"});
  club.processEvent(11h, 4, {"Client1"});

  club.endDay();
  auto report = club.generateReport();

  EXPECT_TRUE(std::find(report.begin(), report.end(), "1 20 02:00") !=
              report.end());
}

TEST(ComputerClubTest, PartialHourBilling)
{
  ComputerClub club(1, 8h, 20h, 10);
  club.processEvent(9h, 1, {"Client1"});
  club.processEvent(9h, 2, {"Client1", "1"});
  club.processEvent(9h + 30min, 4, {"Client1"});

  club.endDay();
  auto report = club.generateReport();

  EXPECT_TRUE(std::find(report.begin(), report.end(), "1 10 00:30") !=
              report.end());
}

TEST(ComputerClubTest, TableReassignment)
{
  ComputerClub club(1, 8h, 20h, 10);
  club.processEvent(9h, 1, {"Client1"});
  club.processEvent(9h, 2, {"Client1", "1"});
  club.processEvent(10h, 1, {"Client2"});
  club.processEvent(10h, 3, {"Client2"});
  club.processEvent(11h, 4, {"Client1"});

  auto report = club.generateReport();

  EXPECT_TRUE(std::find(report.begin(), report.end(), "11:00 12 Client2 1") !=
              report.end());
}

TEST(ComputerClubTest, InvalidEvents)
{
  ComputerClub club(1, 8h, 20h, 10);

  EXPECT_THROW(club.processEvent(9h, 99, {"Client1"}), std::invalid_argument);

  EXPECT_THROW(club.processEvent(9h, 1, {"Client1", "extra"}),
               std::invalid_argument);

  club.processEvent(9h, 1, {"Client1"});
  EXPECT_THROW(club.processEvent(9h, 2, {"Client1", "2"}),
               std::invalid_argument);
}

TEST(ComputerClubTest, BeforeOpening)
{
  ComputerClub club(1, 8h, 20h, 10);
  club.processEvent(7h, 1, {"Client1"});

  auto report = club.generateReport();
  EXPECT_TRUE(std::find(report.begin(), report.end(), "07:00 13 NotOpenYet") !=
              report.end());
}

TEST(ComputerClubTest, AfterClosing)
{
  ComputerClub club(1, 8h, 20h, 10);
  club.processEvent(20h, 1, {"Client1"});

  auto report = club.generateReport();
  EXPECT_TRUE(std::find(report.begin(), report.end(), "20:00 13 NotOpenYet") !=
              report.end());
}