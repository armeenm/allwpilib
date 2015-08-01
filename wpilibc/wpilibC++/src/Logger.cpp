#include "Logger.h"

#include <iostream>
#include <cstdlib>

#include "Utility.h" // Used for stack trace.
#include "HAL/HAL.hpp"

/**
 * Constructs a logger (only used in the GetInstance() call).
 * @param period The rate at which to call FlushStream().
 */
Logger::Logger(double period)
    : m_streamThread(RunStream), m_flusher(StaticFlush) {
  m_flusher.StartPeriodic(period);
}

/**
 * Stop the stream writing thread from running.
 */
Logger::~Logger() {
  m_runThread = false;
  if (m_streamThread.joinable()) m_streamThread.join();
}

/**
 * @return A singleton instance of the Logger class.
 */
Logger &Logger::GetInstance() {
  static Logger instance;
  return instance;
}

/**
 * Log a single log message.
 * @param level The logging level to use; can be passed as the Level type
 *   (DEBUG, WARNING, or ERROR), or directly as a custom integer level.
 * @param codename If std::atoi(codename) returns a non-zero value, then
 *   codename is used as if it is an integer error code number; otherwise, it is
 *   treated as the name of an error code and ErrorCode.GetCode() is called on
 *   the string.
 * @param details The actual user message to use.
 * @param location The location the log message was created from.
 * @param originator A string corresonding and preferably unique to the calling
 *   class. Can be used to categorize log messages by object.
 */
void Logger::Log(int level, const char *codename, const std::string &details,
                 const LogLocation &location, const std::string &originator) {
  if (codename == nullptr) Log(level, 0, details, location, originator);
  else if (int code = std::atoi(codename))
    Log(level, code, details, location, originator, 3);
  else
    Log(level, ErrorCodes::GetInstance().GetCode(codename), details, location,
        originator, 3);
}

void Logger::Log(int level, int code, const std::string &details,
                 const LogLocation &location, const std::string &originator,
                 int depth) {
  auto now = std::chrono::steady_clock::now();
  LogMessage msg = {level, code,                  details,   location,
                    now,   GetStackTrace(depth), originator};
  if (level == kNIWarning || level == kNIError) {
    if (m_useNI) ProcessNILog(msg);
  }
  else {
    if (m_usestdout) ProcessLog(msg);
  }

  ProcessCommon(msg);
}

/**
 * Write a log message to the driverstation directly.
 */
void Logger::ProcessNILog(const LogMessage &msg) {
  // Call HAL-level senderror.
  HALSendError(msg.level == kNIError, msg.code, msg.details.c_str(),
               msg.location, msg.stacktrace.c_str());
}

/**
 * Write a log message to stdout.
 */
void Logger::ProcessLog(const LogMessage &msg) {
  std::string formatted;
  if (m_levelFormats.count(msg.level))
    formatted = m_levelFormats[msg.level](msg);
  else formatted = FormatDefault(msg);
  // Just redirect to stdout.
  std::cout << formatted;
}

/**
 * Write a log message to all outputstreams in m_listeners.
 */
void Logger::ProcessCommon(const LogMessage &msg, std::string formatted) {
  if (formatted.length() == 0) {
    if (m_levelFormats.count(msg.level))
      formatted = m_levelFormats[msg.level](msg);
    else formatted = FormatParseable(msg);
  }
  std::unique_lock<priority_mutex> lock(m_queueMutex);
  m_pending += formatted;
}

/**
 * Performs a simple, minimalist, formatting for the give LogMessage. Only
 * includes the level, the details (ie, the message itself), and the location.
 */
std::string Logger::FormatDefault(const LogMessage &msg) {
  // Use raw c-strings for easier formatting with snprintf.
  const int kBufLen = 512;
  char buf[kBufLen];
  // Note: The default will not contain a timestamp because NI is including it
  // in when they capture stdout.
  snprintf(buf, kBufLen, "[%s]: From %s: %s\n", m_levels[msg.level].c_str(),
           LogLocation::ShortFilename(msg.location).c_str(),
           msg.details.c_str());
  return buf;
}

/**
 * Captures as much information as possible from the LogMessage and returns a
 * string with all the information, separated by commas and terminated with a
 * semicolon and newline. All commas and semicolons in the generated output are
 * escaped with a backslash and all backslashes are escaped with backslashes
 * themselves.
 * The information will be formatted as follows:
 * level(int),level(name),code(string),code(string),location(string),timestamp(int, milliseconds),stacktrace(string),originatingObject(raw pointer),originatingObject(string),details(string)
 * Individual datasets are terminated with a semicolon and new-line character.
 * New line characters within the message will not be escaped.
 */
std::string Logger::FormatParseable(const LogMessage &msg) {
  std::string timestamp =
      std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(
                         msg.timestamp.time_since_epoch()).count());
  std::vector<std::string> components = {
      std::to_string(msg.level), m_levels[msg.level],
      std::to_string(msg.code),  ErrorCodes::GetInstance().Get(msg.code),
      msg.location,              timestamp,
      msg.stacktrace,            msg.originatingObject,
      msg.details};
  std::string result = "";
  for (auto &str : components) {
    FindAndReplace(str, "\\", "\\\\");
    FindAndReplace(str, ",", "\\,");
    FindAndReplace(str, ";", "\\;");
    result += str + ",";
  }
  // Get rid of last comma.
  result.pop_back();
  result += ";\n";
  return result;
}

/**
 * Actually write the data to the various ostreams.
 * Called in a separate thread once at initialization.
 */
void Logger::RunStream() {
  Logger &instance = GetInstance();
  while (instance.m_runThread) {
    std::unique_lock<priority_mutex> streamLock(instance.m_streamMutex);
    instance.m_flushStream.wait(streamLock);
    std::unique_lock<priority_mutex> queueLock(instance.m_queueMutex);
    if (instance.m_pending.size() == 0) continue;
    auto pending = instance.m_pending;
    instance.m_pending.clear();
    queueLock.unlock();
    // Copy so that iterators stay valid if we erase anything.
    auto listeners = instance.m_listeners;
    for (auto &weakstream : listeners) {
      printf("Writing outputs!\n");
      // Both instantiate stream and check its value.
      if (auto stream = weakstream.lock()) {
        *stream << pending;
        stream->flush();
      }
      else {
        LOG_FUNC(kWARNING, "Deleting log listener from listener list.");
        instance.m_listeners.erase(weakstream);
      }
    }
  }
}

/**
 * Finds and replaces all substrings in a string.
 * @param source The string to perform the find & replace on.
 * @param find The substring to be replaced with replace.
 * @param replace The substring to replace find with.
 */
void Logger::FindAndReplace(std::string &source, const std::string &find,
                            const std::string &replace) {
  for (std::string::size_type i = 0;
       (i = source.find(find, i)) != std::string::npos;) {
    source.replace(i, find.length(), replace);
    i += replace.length();
  }
}

// Various accessors and the such.

/**
 * Writes any pending logs messages to the ostream listeners.
 */
void Logger::FlushStream() { m_flushStream.notify_all(); }

/**
 * Enable or disable the printing of messages to standard out.
 * This is enabled by default.
 * @param enabled Whether or not to print log messages to stdout.
 */
void Logger::SetStdoutEnable(bool enabled) {
  m_usestdout = enabled;  // Enabled by default.
}

/**
 * Enable or disable the sending of warning/error messages directly to
 * the driverstation.
 * This is enabled by default.
 * Disabling this will mean that log messages can not be viewed in the
 * driverstation log viewer and is generally discouraged.
 */
void Logger::SetNIEnable(bool enabled) { m_useNI = enabled; }

/**
 * Add a named level for logging beyond the existing DEBUG/WARNING/ERROR.
 * @param level The integer number to use for the new level.
 * @param name The name to give to the level.
 */
void Logger::AddLevel(int level, const std::string &name) {
  m_levels[level] = name;
}

/**
 * Add a formatter function for the given level.
 * By default, the FormatDefault() is used for printing to stdout and
 * the FormatParseable() is used for printing to the various listeners.
 * Adding a formatter here overrides both of those for a given logging level,
 * allowing for special logging in the case of some special level (eg,
 * you may want to include an error code for errors but not for debug).
 * @param level The level to set the formatter for.
 * @param formatter The formatter to use for said level, with a function
 * returning a std::string and taking a const reference to a LogMessage.
 */
void Logger::AddFormatter(int level, FormatLogFunc formatter) {
  m_levelFormats[level] = formatter;
}

/**
 * Add an ostream to write parseable output to.
 * The ostream could write to a file, or could be some custom class
 * that does something special with the logs (eg, filters them in some
 * special way).
 * @param listener An ostream to write to.
 */
void Logger::AddListener(std::weak_ptr<std::ostream> listener) {
  std::unique_lock<priority_mutex> lock(m_streamMutex);
  m_listeners.insert(listener);
}


/**
 * Removes a previously added listener from the set of listeners to write to.
 */
void Logger::RemoveListener(std::weak_ptr<std::ostream> listener) {
  std::unique_lock<priority_mutex> lock(m_streamMutex);
  m_listeners.erase(listener);
}

/**
 * Passed to a Notifier to periodically call FlushStream().
 */
void Logger::StaticFlush(void *) {
  GetInstance().FlushStream();
}
