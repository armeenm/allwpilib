/*----------------------------------------------------------------------------*/
/* Copyright (c) 2020 FIRST. All Rights Reserved.                             */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in the root directory of */
/* the project.                                                               */
/*----------------------------------------------------------------------------*/

package edu.wpi.first.wpiutil.datalog;

@SuppressWarnings("MemberName")
public final class DataLogConfig {
  /**
   * Start out timestamp file with space for this many records.  Note the
   * actual file size will start out this big, but it's a sparse file.
   */
  public int initialSize = 1000;

  /**
   * Once size has reached this size, grow by this number of records each
   * time.  Prior to it reaching this size, the space is doubled.
   */
  public int maxGrowSize = 60000;

  /**
   * Maximum map window size.  Larger is more efficient, but may have
   * issues on 32-bit systems.  Defaults to unlimited.
   */
  public int maxMapSize;

  /**
   * Periodic flush setting.  Flushes log to disk every N appends.
   * Defaults to no periodic flush.
   */
  public int periodicFlush;

  /**
   * Start out data file with space for this many bytes.  Note the
   * actual file size will start out this big, but it's a sparse file.
   */
  public long initialDataSize = 100000;

  /**
   * Once data file has reached this size, grow by this number of bytes each
   * time.  Prior to it reaching this size, the space is doubled.
   */
  public long maxDataGrowSize = 1024 * 1024;

  /**
   * Use large (e.g. 64-bit) variable-sized data files when creating a new
   * log.  The default is to use 32-bit sizes for the variable-sized data.
   */
  public boolean largeData;

  /**
   * Fill data to put in between each record of variable-sized data in data
   * file.  Useful for e.g. making strings null terminated.  Defaults to
   * nothing.
   */
  public String gapData;

  /**
   * Check data type when opening existing file.  Defaults to checking.
   */
  public boolean checkType = true;

  /**
   * Check record size when opening existing file.  Defaults to checking.
   */
  public boolean checkSize = true;

  /**
   * Check data layout when opening existing file.  Defaults to checking.
   */
  public boolean checkLayout = true;

  /**
   * Check timestamp is monotonically increasing and don't save the value if
   * timestamp decreased.  Defaults to true.
   */
  public boolean checkMonotonic = true;

  /**
   * Open file in read-only mode.
   */
  public boolean readOnly;
}
