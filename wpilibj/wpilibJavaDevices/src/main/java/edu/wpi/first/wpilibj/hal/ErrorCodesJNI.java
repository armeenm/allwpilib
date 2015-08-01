package edu.wpi.first.wpilibj.hal;

import java.lang.String;

public class ErrorCodesJNI extends JNIWrapper {
  public static native String get(int code);
  public static native int getCode(String name);
  public static native boolean add(int code, String msg, String name);
}
