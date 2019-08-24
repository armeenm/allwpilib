/*----------------------------------------------------------------------------*/
/* Copyright (c) 2008-2019 FIRST. All Rights Reserved.                        */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in the root directory of */
/* the project.                                                               */
/*----------------------------------------------------------------------------*/

#pragma once

#include <atomic>
#include <memory>
#include <thread>
#include <vector>

#include "frc/Counter.h"
#include "frc/ErrorBase.h"
#include "frc/PIDSource.h"
#include "frc/smartdashboard/SendableBase.h"

namespace frc {

class DigitalInput;
class DigitalOutput;

/**
 * Ultrasonic rangefinder class.
 *
 * The Ultrasonic rangefinder measures absolute distance based on the round-trip
 * time of a ping generated by the controller. These sensors use two
 * transducers, a speaker and a microphone both tuned to the ultrasonic range. A
 * common ultrasonic sensor, the Daventech SRF04 requires a short pulse to be
 * generated on a digital channel. This causes the chirp to be emitted. A second
 * line becomes high as the ping is transmitted and goes low when the echo is
 * received. The time that the line is high determines the round trip distance
 * (time of flight).
 */
class Ultrasonic final : public ErrorBase,
                         public SendableBase,
                         public PIDSource {
 public:
  enum DistanceUnit { kInches = 0, kMilliMeters = 1 };

  /**
   * Create an instance of the Ultrasonic Sensor.
   *
   * This is designed to support the Daventech SRF04 and Vex ultrasonic sensors.
   *
   * @param pingChannel The digital output channel that sends the pulse to
   *                    initiate the sensor sending the ping.
   * @param echoChannel The digital input channel that receives the echo. The
   *                    length of time that the echo is high represents the
   *                    round trip time of the ping, and the distance.
   * @param units       The units returned in either kInches or kMilliMeters
   */
  Ultrasonic(int pingChannel, int echoChannel, DistanceUnit units = kInches);

  /**
   * Create an instance of an Ultrasonic Sensor from a DigitalInput for the echo
   * channel and a DigitalOutput for the ping channel.
   *
   * @param pingChannel The digital output object that starts the sensor doing a
   *                    ping. Requires a 10uS pulse to start.
   * @param echoChannel The digital input object that times the return pulse to
   *                    determine the range.
   * @param units       The units returned in either kInches or kMilliMeters
   */
  Ultrasonic(DigitalOutput* pingChannel, DigitalInput* echoChannel,
             DistanceUnit units = kInches);

  /**
   * Create an instance of an Ultrasonic Sensor from a DigitalInput for the echo
   * channel and a DigitalOutput for the ping channel.
   *
   * @param pingChannel The digital output object that starts the sensor doing a
   *                    ping. Requires a 10uS pulse to start.
   * @param echoChannel The digital input object that times the return pulse to
   *                    determine the range.
   * @param units       The units returned in either kInches or kMilliMeters
   */
  Ultrasonic(DigitalOutput& pingChannel, DigitalInput& echoChannel,
             DistanceUnit units = kInches);

  /**
   * Create an instance of an Ultrasonic Sensor from a DigitalInput for the echo
   * channel and a DigitalOutput for the ping channel.
   *
   * @param pingChannel The digital output object that starts the sensor doing a
   *                    ping. Requires a 10uS pulse to start.
   * @param echoChannel The digital input object that times the return pulse to
   *                    determine the range.
   * @param units       The units returned in either kInches or kMilliMeters
   */
  Ultrasonic(std::shared_ptr<DigitalOutput> pingChannel,
             std::shared_ptr<DigitalInput> echoChannel,
             DistanceUnit units = kInches);

  ~Ultrasonic() override;

  Ultrasonic(Ultrasonic&&) = default;
  Ultrasonic& operator=(Ultrasonic&&) = default;

  /**
   * Single ping to ultrasonic sensor.
   *
   * Send out a single ping to the ultrasonic sensor. This only works if
   * automatic (round robin) mode is disabled. A single ping is sent out, and
   * the counter should count the semi-period when it comes in. The counter is
   * reset to make the current value invalid.
   */
  void Ping();

  /**
   * Check if there is a valid range measurement.
   *
   * The ranges are accumulated in a counter that will increment on each edge of
   * the echo (return) signal. If the count is not at least 2, then the range
   * has not yet been measured, and is invalid.
   */
  bool IsRangeValid() const;

  /**
   * Turn Automatic mode on/off.
   *
   * When in Automatic mode, all sensors will fire in round robin, waiting a set
   * time between each sensor.
   *
   * @param enabling Set to true if round robin scheduling should start for all
   *                 the ultrasonic sensors. This scheduling method assures that
   *                 the sensors are non-interfering because no two sensors fire
   *                 at the same time. If another scheduling algorithm is
   *                 prefered, it can be implemented by pinging the sensors
   *                 manually and waiting for the results to come back.
   */
  static void SetAutomaticMode(bool enabling);

  /**
   * Get the range in inches from the ultrasonic sensor.
   *
   * @return Range in inches of the target returned from the ultrasonic sensor.
   *         If there is no valid value yet, i.e. at least one measurement
   *         hasn't completed, then return 0.
   */
  double GetRangeInches() const;

  /**
   * Get the range in millimeters from the ultrasonic sensor.
   *
   * @return Range in millimeters of the target returned by the ultrasonic
   *         sensor. If there is no valid value yet, i.e. at least one
   *         measurement hasn't completed, then return 0.
   */
  double GetRangeMM() const;

  bool IsEnabled() const;

  void SetEnabled(bool enable);

  /**
   * Set the current DistanceUnit that should be used for the PIDSource base
   * object.
   *
   * @param units The DistanceUnit that should be used.
   */
  void SetDistanceUnits(DistanceUnit units);

  /**
   * Get the current DistanceUnit that is used for the PIDSource base object.
   *
   * @return The type of DistanceUnit that is being used.
   */
  DistanceUnit GetDistanceUnits() const;

  /**
   * Get the range in the current DistanceUnit for the PIDSource base object.
   *
   * @return The range in DistanceUnit
   */
  double PIDGet() override;

  void SetPIDSourceType(PIDSourceType pidSource) override;

  void InitSendable(SendableBuilder& builder) override;

 private:
  /**
   * Initialize the Ultrasonic Sensor.
   *
   * This is the common code that initializes the ultrasonic sensor given that
   * there are two digital I/O channels allocated. If the system was running in
   * automatic mode (round robin) when the new sensor is added, it is stopped,
   * the sensor is added, then automatic mode is restored.
   */
  void Initialize();

  /**
   * Background task that goes through the list of ultrasonic sensors and pings
   * each one in turn. The counter is configured to read the timing of the
   * returned echo pulse.
   *
   * DANGER WILL ROBINSON, DANGER WILL ROBINSON:
   * This code runs as a task and assumes that none of the ultrasonic sensors
   * will change while it's running. Make sure to disable automatic mode before
   * touching the list.
   */
  static void UltrasonicChecker();

  // Time (sec) for the ping trigger pulse.
  static constexpr double kPingTime = 10 * 1e-6;

  // Priority that the ultrasonic round robin task runs.
  static constexpr int kPriority = 64;

  // Max time (ms) between readings.
  static constexpr double kMaxUltrasonicTime = 0.1;
  static constexpr double kSpeedOfSoundInchesPerSec = 1130.0 * 12.0;

  // Thread doing the round-robin automatic sensing
  static std::thread m_thread;

  // Ultrasonic sensors
  static std::vector<Ultrasonic*> m_sensors;

  // Automatic round-robin mode
  static std::atomic<bool> m_automaticEnabled;

  std::shared_ptr<DigitalOutput> m_pingChannel;
  std::shared_ptr<DigitalInput> m_echoChannel;
  bool m_enabled = false;
  Counter m_counter;
  DistanceUnit m_units;
};

}  // namespace frc
