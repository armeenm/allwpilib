#include "HAL/cpp/ErrorCodes.h"

#define CODE(name, code, msg) Add((code), (msg), #name)

/**
 * Create all of the various error codes; Add new entries to create new
 * errors.
 */
ErrorCodes::ErrorCodes() {
  /*
   * Errors
   */
  CODE(ModuleIndexOutOfRange, -1,
       "Allocating module that is out of range or not found");
  CODE(ChannelIndexOutOfRange, -1, "Allocating channel that is out of range");
  CODE(NotAllocated, -2, "Attempting to free unallocated resource");
  CODE(ResourceAlreadyAllocated, -3,
       "Attempted to reuse an allocated resource");
  CODE(NoAvailableResources, -4, "No available resources to allocate");
  CODE(NullParameter, -5, "A pointer parameter to a method is nullptr");
  CODE(Timeout, -6, "A timeout has been exceeded");
  CODE(CompassManufacturerError, -7,
       "Compass manufacturer doesn't match HiTechnic");
  CODE(CompassTypeError, -8,
       "Compass type doesn't match expected type for HiTechnic compass");
  CODE(IncompatibleMode, -9, "The object is in an incompatible mode");
  CODE(AnalogTriggerLimitOrderError, -10,
       "AnalogTrigger limits error.  Lower limit > Upper Limit");
  CODE(AnalogTriggerPulseOutputError, -11,
       "Attempted to read AnalogTrigger pulse output.");
  CODE(TaskError, -12, "Task can't be started");
  CODE(TaskIDError, -13, "Task error: Invalid ID.");
  CODE(TaskDeletedError, -14, "Task error: Task already deleted.");
  CODE(TaskOptionsError, -15, "Task error: Invalid options.");
  CODE(TaskMemoryError, -16,
       "Task can't be started due to insufficient memory.");
  CODE(TaskPriorityError, -17, "Task error: Invalid priority [1-255].");
  CODE(DriveUninitialized, -18,
       "RobotDrive not initialized for the C interface");
  CODE(CompressorNonMatching, -19,
       "Compressor slot/channel doesn't match previous instance");
  CODE(CompressorAlreadyDefined, -20, "Creating a second compressor instance");
  CODE(CompressorUndefined, -21,
       "Using compressor functions without defining compressor");
  CODE(InconsistentArrayValueAdded, -22,
       "When packing data into an array to the dashboard, not all values added "
       "were "
       "of the same type.");
  CODE(MismatchedComplexTypeClose, -23,
       "When packing data to the dashboard, a Close for a complex type was "
       "called "
       "without a matching Open.");
  CODE(DashboardDataOverflow, -24,
       "When packing data to the dashboard, too much data was packed and the "
       "buffer "
       "overflowed.");
  CODE(DashboardDataCollision, -25,
       "The same buffer was used for packing data and for printing.");
  CODE(EnhancedIOMissing, -26,
       "IO is not attached or Enhanced IO is not enabled.");
  CODE(LineNotOutput, -27,
       "Cannot SetDigitalOutput for a line not configured for output.");
  CODE(ParameterOutOfRange, -28, "A parameter is out of range.");
  CODE(SPIClockRateTooLow, -29,
       "SPI clock rate was below the minimum supported");
  CODE(JaguarVersionError, -30, "Jaguar firmware version error");
  CODE(JaguarMessageNotFound, -31, "Jaguar message not found");
  CODE(NetworkTablesReadError, -40, "Error reading NetworkTables socket");
  CODE(NetworkTablesBufferFull, -41,
       "Buffer full writing to NetworkTables socket");
  CODE(NetworkTablesWrongType, -42,
       "The wrong type was read from the NetworkTables entry");
  CODE(NetworkTablesCorrupt, -43, "NetworkTables data stream is corrupt");
  CODE(SmartDashboardMissingKey, -43, "SmartDashboard data does not exist");
  CODE(CommandIllegalUse, -50, "Illegal use of Command");
  CODE(UnsupportedInSimulation, -80, "Unsupported in simulation");

  /*
   * Warnings
   */
  CODE(SampleRateTooHigh, 1, "Analog module sample rate is too high");
  CODE(VoltageOutOfRange, 2,
       "Voltage to convert to raw value is out of range [-10; 10]");
  CODE(CompressorTaskError, 3, "Compressor task won't start");
  CODE(LoopTimingError, 4,
       "Digital module loop timing is not the expected value");
  CODE(NonBinaryDigitalValue, 5, "Digital output value is not 0 or 1");
  CODE(IncorrectBatteryChannel, 6,
       "Battery measurement channel is not correct value");
  CODE(BadJoystickIndex, 7, "Joystick index is out of range, should be 0-3");
  CODE(BadJoystickAxis, 8, "Joystick axis or POV is out of range");
  CODE(InvalidMotorIndex, 9, "Motor index is out of range, should be 0-3");
  CODE(DriverStationTaskError, 10, "Driver Station task won't start");
  CODE(EnhancedIOPWMPeriodOutOfRange, 11,
       "Driver Station Enhanced IO PWM Output period out of range.");
  CODE(SPIWriteNoMOSI, 12, "Cannot write to SPI port with no MOSI output");
  CODE(SPIReadNoMISO, 13, "Cannot read from SPI port with no MISO input");
  CODE(SPIReadNoData, 14, "No data available to read from SPI");
  CODE(IncompatibleState, 15,
       "Incompatible State: The operation cannot be completed");
}

/**
 * Gets the singleton instance of the ErrorCodes class.
 */
ErrorCodes &ErrorCodes::GetInstance() {
  static ErrorCodes instance;
  return instance;
}

/**
 * Get the string associated with an error code.
 * @param code The integer code to look up.
 * @return The correspond string, or an empty string if no codes is found.
 */
const char *ErrorCodes::Get(int code) {
  if (m_codes.count(code))
    return m_codes[code];
  else
    return "";
}

/**
 * Get the integer error code associated with an error name.
 * @param name The string name used for the error code.
 * @return The corresponding integer code, or 0 if not found.
 */
int ErrorCodes::GetCode(const char *name) {
  if (m_names.count(name))
    return m_names[name];
  else
    return 0;
}

/**
 * Add a new entry for an error code.
 * Does not create a new entry if the error code is already used.
 * @param code The integer code to create.
 * @param msg The string to be displayed to the user when the code is used.
 * @param name An easy-to-remember identifier (preferably something that is
 *   a valid C++ identifier so that the LOG macros can take the name without
 *   quotes) that will be used to identify the code; not strictly necessary,
 *   but encouraged to allow easier use of the LOG macros.
 * @return true if the new error code was created; false if the error code
 *   already exists.
 */
bool ErrorCodes::Add(int code, const char *msg, const char *name) {
  if (m_codes.count(code)) return false;
  m_codes[code] = msg;
  if (name != nullptr) {
    m_names[name] = code;
  }
  return true;
}
