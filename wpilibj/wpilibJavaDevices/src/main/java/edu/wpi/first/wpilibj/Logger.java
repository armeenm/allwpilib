package edu.wpi.first.wpilibj;

import java.io.IOException;
import java.io.OutputStream;
import java.lang.String;
import java.lang.Thread;
import java.lang.reflect.Array;
import java.util.Arrays;
import java.util.Collections;
import java.util.Iterator;
import java.util.Map;
import java.util.HashMap;
import java.util.Set;
import java.util.HashSet;
import java.util.concurrent.atomic.AtomicBoolean;

import edu.wpi.first.wpilibj.communication.FRCNetworkCommunicationsLibrary;
import edu.wpi.first.wpilibj.hal.ErrorCodesJNI;

/**
 * A class to be used for logging messages to stdout, the driverstation, and
 * arbitrary OutputStreams.
 */
public class Logger {
  // The singleton instance of this class.
  private static Logger instance = null;

  /**
   * Contains all the information needed in a LogMessage.
   */
  public class LogMessage {
    // The logging level; higher number=higher priority.
    public int level;
    // Error code; 0 means no issues.
    public int code;
    // The message provided by the user.
    public String details;
    // The timestamp, in milliseconds as specified by System.currentTimeMillis().
    public long timeMillis;
    // The stacktrace for whereever the log was created.
    public StackTraceElement[] stacktrace;
    // The result of calling toString() on the object passed to the logger.
    public String originatingObject;
  }

  static public class LogFormatter {
    /**
     * Returns a simply formatted, easy to read log message.
     * @param msg A log message to format.
     * @return A string of the form "[level]: From location: details" (without
     *   any terminating period or new line).
     */
    public String format(LogMessage msg) {
      int level = msg.level;
      String levelmsg = m_levels.get(level);
      StackTraceElement location = msg.stacktrace[0];
      String locationmsg = location.toString();
      String details = msg.details;

      return String.format("[%s]: From %s: %s", levelmsg,
                           locationmsg, details);
    }
  }

  static public class FormatParseable extends LogFormatter {
    /**
     * Returns an easily parseable version of the provided message.
     * The returned message will be a comma separated list of the various
     * components of the log message and be terminated by a semicolon and
     * newline (";\n"). Any commas, semicolons, or backslashes within the
     * body of the message are escaped with backslashes.
     * @param msg The log message to format.
     * @return A string formatted as described above, with the order:
     *   "level,levelmsg,code,codemsg,time,stack,originator,details;\n"
     */
    public String format(LogMessage msg) {
      // level, levelmsg, code, codemsg, time, stack, originator, details.
      String format = "%d,%s,%d,%s,%d,%s,%s,%s;\n";
      String levelmsg = sanitize(m_levels.get(msg.level));
      String codemsg = sanitize(ErrorCodesJNI.get(msg.code));
      String stack = sanitize(Arrays.toString(msg.stacktrace));
      String originator = sanitize(msg.originatingObject);
      String details = sanitize(msg.details);
      return String.format(format, msg.level, levelmsg, msg.code, codemsg,
                           msg.timeMillis, stack, originator, details);
    }

    /**
     * Escape all backslashes, commas, and semicolons in the source string.
     */
    private String sanitize(String source) {
      if (source == null) return "";
      source = source.replaceAll("\\\\", "\\\\\\\\");
      source = source.replaceAll(",", "\\\\,");
      source = source.replaceAll(";", "\\\\;");
      return source;
    }
  }

  /**
   * The logging levels currently supported by NI.
   * Unfortunately, NI only supports a warning and error level at the
   * moment and so these are the only ones that show up nicely in
   * the driverstation.
   */
  public enum NILevel {
    WARNING (2),
    ERROR (3);

    private final int value;
    NILevel(int value) { this.value = value; }
    public int getValue() { return value; }
  }

  /**
   * A convenient enum to use for the more common levels.
   * If a user wants to define their own levels, then this enum becomes useless.
   */
  public enum Level {
    DEBUG (1),
    WARNING (2),
    ERROR (3);

    private final int value;
    Level(int value) { this.value = value; }
    public int getValue() { return value; }
  }

  // A map of all the various logging levels.
  static Map<Integer, String> m_levels =
      Collections.synchronizedMap(new HashMap<Integer, String>());
  // The formatters to use for each level; if there is an entry in this
  // map for a given level, then any log messages of that level will be
  // logged solely using the provided formatter.
  Map<Integer, LogFormatter> m_levelFormats =
      Collections.synchronizedMap(new HashMap<Integer, LogFormatter>());
  // A set of outputstreams to send all log messages to.
  Set<OutputStream> m_listeners =
      Collections.synchronizedSet(new HashSet<OutputStream>());
  // Whether or not we should print debug information to stdout.
  AtomicBoolean m_usestdout = new AtomicBoolean(true);
  // Whether or not warnings/errors should be sent to NI's logger.
  AtomicBoolean m_useNI = new AtomicBoolean(true);

  protected Logger() {
    // Put in the default levels from the Level enum.
    m_levels.put(1, "DEBUG");
    m_levels.put(2, "WARNING");
    m_levels.put(3, "ERROR");
  }

  /**
   * Return the singleton instance of the class.
   */
  public static synchronized Logger getInstance() {
    if (instance == null) instance = new Logger();
    return instance;
  }

  /**
   * Log a single log message with the provided information.
   * @param level The integer level to use for logging.
   * @param code The integer error code to use for logging.
   * @param details The main message to log.
   * @param originator The object from which the logging is taking place;
   *   originator.toString() is called and can be used to categorize logs
   *   from any given Object.
   * @param depth The number of entries in the stack trace to ignore so
   *   that the log isn't polluted with the stack information on the Log()
   *   call itself.
   */
  public void log(int level, int code, String details, Object originator,
                  int depth) {
    LogMessage msg = new LogMessage();
    msg.level = level;
    msg.code = code;
    msg.details = details;
    // Accuracy and precision are relatively unimportant for logging, so we
    // don't bother using the FPGA for time..
    msg.timeMillis = System.currentTimeMillis();
    // We don't want to include the Log() call itself in the stack trace.
    msg.stacktrace = Thread.currentThread().getStackTrace();
    msg.stacktrace = Arrays.copyOfRange(msg.stacktrace, depth,
                                        msg.stacktrace.length);

    if (level == NILevel.WARNING.getValue() ||
        level == NILevel.ERROR.getValue()) {
      if (m_useNI.get()) processNILog(msg);
    }
    else {
      if (m_usestdout.get()) processLog(msg);
    }

    processCommon(msg);
  }

  /**
   * Add an outputstream to write parseable output to.
   * The OutputStream could write to a file, or could be some custom class
   * that does something special with the logs (eg, filters them in some
   * special way).
   * @param listener An outputstream to write to.
   */
  public void addListener(OutputStream listener) {
    m_listeners.add(listener);
  }

  /**
   * Removes a previously added listener from the set of listeners to write to.
   */
  public void removeListener(OutputStream listener) {
    m_listeners.remove(listener);
  }

  /**
   * Use a particular formatter for a given level.
   * By default, the LogFormatter is used for printing to stdout and
   * the FormatParseable is used for printing to the various listeners.
   * Adding a formatter here overrides both of those for a given logging level,
   * allowing for special logging in the case of some special level (eg,
   * you may want to include an error code for errors but not for debug).
   * @param level The level to set the formatter for.
   * @param formatter The formatter to use for said level.
   */
  public void addFormatter(int level, LogFormatter formatter) {
    m_levelFormats.put(level, formatter);
  }

  /**
   * Add a named level for logging beyond the existing DEBUG/WARNING/ERROR.
   * @param level The integer number to use for the new level.
   * @param name The name to give to the level.
   */
  public void addLevel(int level, String name) {
    m_levels.put(level, name);
  }

  /**
   * Enable or disable the printing of messages to standard out.
   * This is enabled by default.
   * @param enabled Whether or not to print log messages to stdout.
   */
  public void setStdoutEnable(boolean enabled) {
    m_usestdout.set(enabled);
  }

  /**
   * Enable or disable the sending of warning/error messages directly to
   * the driverstation.
   * This is enabled by default.
   * Disabling this will mean that log messages can not be viewed in the
   * driverstation log viewer and is generally discouraged.
   */
  public void setNIEnable(boolean enabled) {
    m_useNI.set(enabled);
  }

  /**
   * Write a log message to the driverstation directly.
   */
  protected void processNILog(LogMessage msg) {
    FRCNetworkCommunicationsLibrary.HALSendError(
        msg.level == NILevel.ERROR.getValue(), msg.code, msg.details,
        msg.stacktrace[0].toString(), msg.stacktrace.toString());
  }

  /**
   * Write a log message to stdout.
   */
  protected void processLog(LogMessage msg) {
    String formatted;
    if (m_levelFormats.containsKey(msg.level))
      formatted = m_levelFormats.get(msg.level).format(msg);
    else formatted = (new LogFormatter()).format(msg);
    System.out.println(formatted);
  }

  /**
   * Write a log message to all outputstreams in m_listeners.
   */
  protected void processCommon(LogMessage msg) {
    String formatted;
    if (m_levelFormats.containsKey(msg.level))
      formatted = m_levelFormats.get(msg.level).format(msg);
    else formatted = (new FormatParseable()).format(msg);

    for (Iterator<OutputStream> it = m_listeners.iterator(); it.hasNext();) {
      OutputStream stream = it.next();
      try {
      stream.write(formatted.getBytes());
      }
      catch (IOException ex) {
        System.out.println("Writing to stream failed.");
      }
    }
  }

  /**
   * Call flush() on all of the listeners.
   */
  public void flush() throws IOException {
    for (Iterator<OutputStream> it = m_listeners.iterator(); it.hasNext();) {
      it.next().flush();
    }
  }
}
