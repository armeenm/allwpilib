#include "edu_wpi_first_wpilibj_hal_ErrorCodesJNI.h"
#include "HAL/cpp/ErrorCodes.h"

// Initialization order is unimportant to us.
ErrorCodes &codes = ErrorCodes::GetInstance();

/*
 * Class:     edu_wpi_first_wpilibj_hal_ErrorCodesJNI
 * Method:    get()
 * signature: (I)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL
    Java_edu_wpi_first_wpilibj_hal_ErrorCodesJNI_get(JNIEnv *env, jclass,
                                                     jint code) {
  return env->NewStringUTF(codes.Get(code));
}

/*
 * Class:     edu_wpi_first_wpilibj_hal_ErrorCodesJNI
 * Method:    getCode()
 * signature: (Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL
    Java_edu_wpi_first_wpilibj_hal_ErrorCodesJNI_getCode(JNIEnv *env, jclass,
                                                         jstring name) {
  const char *str = env->GetStringUTFChars(name, NULL);
  int retval = codes.GetCode(str);
  env->ReleaseStringUTFChars(name, str);
  return retval;
}

/*
 * Class:     edu_wpi_first_wpilibj_hal_ErrorCodesJNI
 * Method:    add()
 * signature: (ILjava/lang/String;Ljava/lang/string;)Z
 */
JNIEXPORT jboolean JNICALL
    Java_edu_wpi_first_wpilibj_hal_ErrorCodesJNI_add(JNIEnv *env, jclass,
                                                     jint code, jstring msg,
                                                     jstring name) {
  const char *msgStr = env->GetStringUTFChars(msg, NULL);
  const char *nameStr = env->GetStringUTFChars(name, NULL);
  jboolean retval = codes.Add(code, msgStr, nameStr);
  env->ReleaseStringUTFChars(msg, msgStr);
  env->ReleaseStringUTFChars(name, nameStr);
  return retval;
}
