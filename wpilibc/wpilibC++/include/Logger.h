/**
 * \file Logger.h
 * \brief Contains the log class for logging information to the driverstation
 * and to arbitrary files.
 */
#pragma once

#include <atomic>
#include <condition_variable>
#include <chrono>
#include <string>
#include <sstream>
#include <map>
#include <functional>
#include <queue>
#include <set>
#include <vector>
#include <memory>
#include <thread>
#include "HAL/cpp/priority_mutex.h"
#include "HAL/cpp/ErrorCodes.h"
#include "LogLocation.h"
#include "Notifier.h"

// MSVC does not have __PRETTY_FUNCTION__.
#ifdef _MSC_VER
#define __PRETTY_FUNCTION__ __FUNCSIG__
#endif  // ifdef _MSC_VER

/**
 * Log a message with the provided level and msg.
 * Equivalent to LOG_ERRNO(level, 0, msg).
 * @param level The integer level to use--see the Logger::Level enum.
 * @param msg The message to be printed.
 */
#define LOG(level, msg) LOG_ERRNO((level), 0, (msg))

/**
 * Log a message at the DEBUG level.
 * Equivalent to LOG(Logger::kDEBUG, msg).
 */
#define LOG_DEBUG(msg) LOG(Logger::kDEBUG, (msg))
/**
 * Log a message at the WARNING level.
 * Equivalent to LOG(Logger::kWARNING, msg).
 */
#define LOG_WARNING(msg) LOG(Logger::kWARNING, (msg))
/**
 * Log a message at the ERROR level.
 * Equivalent to LOG(Logger::kERROR, msg).
 */
#define LOG_ERROR(msg) LOG(Logger::kERROR, (msg))

// TODO: Convert to using SFINAE to detect whether the GetName() mthod exists,
// and then call different functions depending on the result.
// Note: Do not break up the comment at the end of this macro; as is, it will
// show up more nicely whena compiler error is thrown, whereas if broken up, the
// user will only be able to see a portion of the note.
/**
 * Log a message with the given level and errno.
 * Sample usage would be LOG_ERRNO(Logger::kWARNING, ArgumentOutOfBounds, "The
 * Gyro can only be used on specific ports."), or LOG_ERRNO(Logger::kERROR, 100,
 * "Some special error occured and whatever message corresponds to errno 100
 * will be printed.")
 * Passing an integer or name in as the errno has the same effect, although it
 * is generally easier to remember a name than a number, so both are supported.
 */
#define LOG_ERRNO(level, errno, msg)                                    \
  (Logger::GetInstance().Log((level), #errno, (msg),                    \
                             {__FILE__, __PRETTY_FUNCTION__, __LINE__}, \
                             this->GetName())) // If your current object does not have a GetName() method, then this call will fail; either write a GetName() method or call LOG_FUNC().

/**
 * The LOG_FUNC macros are for use when either not in a class at all (and so the
 * this pointer is invalid), or when in a class that does not have a GetName()
 * method (and so calling this->GetName() will be invalid).
 */
#define LOG_FUNC(level, msg) LOG_FUNC_ERRNO((level), 0, (msg))

/**
 * A combination of LOG_FUNC and LOG_ERRNO.
 */
#define LOG_FUNC_ERRNO(level, errno, msg)             \
  (Logger::GetInstance().Log((level), (errno), (msg), \
                             {__FILE__, __PRETTY_FUNCTION__, __LINE__}, ""))

/**
 * A class to be used for logging messages to stdout, the driverstation, and
 * arbitrary ostream.
 */
class Logger {
 public:
  struct LogMessage {
    // Note: Order matters; braced initializer list used for construction.
    // Use int instead of Level so that custom levels can be more easily used.
    int level;
    int code;
    std::string details;
    LogLocation location;
    // TODO: Consider steady_clock vs. system_clock.
    std::chrono::steady_clock::time_point timestamp;
    std::string stacktrace;
    std::string originatingObject;  // Used to group messages by object.
  };

  enum NILevel { kNIWarning = 2, kNIError = 3 };

  /**
   * Common logging levels (note that 0 is reserved).
   * Using kDEBUG results in stdout printouts, while kWARNING and kERROR
   * get ouptut through the Driverstation log viewer.
   */
  enum Level { kDEBUG = 1, kWARNING = 2, kERROR = 3};

  typedef std::function<std::string(const LogMessage &)> FormatLogFunc;

  static Logger &GetInstance();
  // TODO: Convert to std::string
  void Log(int level, const char *code, const std::string &details,
           const LogLocation &location, const std::string &originator = "");
  void Log(int level, int code, const std::string &details,
           const LogLocation &location, const std::string &originator = "",
           int depth = 3);

  void FlushStream();
  void SetStdoutEnable(bool enabled);
  void SetNIEnable(bool enabled);
  void AddLevel(int level, const std::string &name);
  void AddFormatter(int level, FormatLogFunc formatter);
  void AddListener(std::weak_ptr<std::ostream> listener);
  void RemoveListener(std::weak_ptr<std::ostream> listener);

 protected:
  // Reserve 0 for special cases.
  std::map<int, std::string> m_levels = {
      {1, "DEBUG"}, {2, "WARNING"}, {3, "ERROR"}};

  std::map<int, FormatLogFunc> m_levelFormats;

  Logger(double period=0.1);
  virtual ~Logger();
  void ProcessNILog(const LogMessage &msg);
  void ProcessLog(const LogMessage &msg);
  void ProcessCommon(const LogMessage &msg, std::string formatted="");
  std::string FormatDefault(const LogMessage &msg);
  std::string FormatParseable(const LogMessage &msg);
  static void RunStream();

 private:
  std::atomic<bool> m_usestdout{true};
  std::atomic<bool> m_useNI{true};
  std::string m_pending;
  priority_mutex m_queueMutex;
  std::set<std::weak_ptr<std::ostream>,
           std::owner_less<std::weak_ptr<std::ostream>>> m_listeners;
  // For now, only one mutex for the whole set of streams (rather than for each
  // individual stream) for sake of simplicity.
  priority_mutex m_streamMutex;
  std::thread m_streamThread;
  std::atomic<bool> m_runThread{true};
  std::condition_variable_any m_flushStream;
  Notifier m_flusher;

  static void StaticFlush(void *);

  void FindAndReplace(std::string &source, const std::string &find,
                      const std::string &replace);
};
