/*----------------------------------------------------------------------------*/
/* Copyright (c) 2020 FIRST. All Rights Reserved.                             */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in the root directory of */
/* the project.                                                               */
/*----------------------------------------------------------------------------*/

#include <chrono>
#include <iostream>

#include "wpi/DataLog.h"

int main() {
  using std::chrono::duration_cast;
  using std::chrono::high_resolution_clock;
  using std::chrono::microseconds;

  {
    auto log = wpi::log::DoubleLog::Open("test.log", wpi::log::CD_CreateAlways);
    if (!log) {
      wpi::errs() << "could not open log\n";
      return EXIT_FAILURE;
    }
    for (int i = 0; i < 50; ++i) log->Append(20000 * i, 1.3 * i);
  }

  {
    auto start = high_resolution_clock::now();
    {
      wpi::log::DoubleLog::Config config;
      config.periodicFlush = 1000;
      auto log =
          wpi::log::DoubleLog::Open("test2.log", wpi::log::CD_CreateAlways);
      if (!log) {
        wpi::errs() << "could not open log\n";
        return EXIT_FAILURE;
      }
      for (int i = 0; i < 500000; ++i) log->Append(20000 * i, 1.3 * i);
    }
    auto stop = high_resolution_clock::now();
    std::cout << " time: " << duration_cast<microseconds>(stop - start).count()
              << "\n";
  }
#if 0
  {
    std::vector<uint8_t> data;
    data.resize(5000);
    auto start = high_resolution_clock::now();
    {
      auto log = wpi::log::DataLog::Open("test2.log", "image", "image", 0,
                                         wpi::log::CD_OpenAlways);
      if (!log) {
        wpi::errs() << "could not open log\n";
        return EXIT_FAILURE;
      }
      for (int i = 0; i < 1000; ++i) log->Append(20000 * i, data);
    }
    auto stop = high_resolution_clock::now();
    std::cout << " time: " << duration_cast<microseconds>(stop - start).count()
              << "\n";
  }
#endif
  {
    auto log =
        wpi::log::StringLog::Open("test-string.log", wpi::log::CD_CreateAlways);
    if (!log) {
      wpi::errs() << "could not open log\n";
      return EXIT_FAILURE;
    }
    for (int i = 0; i < 50; ++i) log->Append(20000 * i, "hello");
  }

  {
    auto log = wpi::log::DoubleArrayLog::Open("test-double-array.log",
                                              wpi::log::CD_CreateAlways);
    if (!log) {
      wpi::errs() << "could not open log\n";
      return EXIT_FAILURE;
    }
    log->Append(20000, {1, 2, 3});
    log->Append(30000, {4, 5});
  }

  {
    auto log = wpi::log::StringArrayLog::Open("test-string-array.log",
                                              wpi::log::CD_CreateAlways);
    if (!log) {
      wpi::errs() << "could not open log\n";
      return EXIT_FAILURE;
    }
    log->Append(20000, {"Hello", "World"});
    log->Append(30000, {"This", "Is", "Fun"});
  }

  return EXIT_SUCCESS;
}
