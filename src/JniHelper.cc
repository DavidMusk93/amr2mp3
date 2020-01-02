#include <jni.h>
#include "Mp3Converter.h"
#include "data.h"
#include "SafeString.h"

#define dimension_of(a) (sizeof(a)/sizeof(*a))

class JString
{
 public:
  JString(JNIEnv *env, jstring& j_str)
    : env_(env), j_str_(j_str) { c_str_ = env_->GetStringUTFChars(j_str_, nullptr); }

  ~JString() { env_->ReleaseStringUTFChars(j_str_, c_str_); }

  operator const char *() const { return c_str_; }

 private:
  JNIEnv *env_;
  jstring& j_str_;
  const char *c_str_;
};

static jint decode(JNIEnv *env, jobject j_obj, jstring j_amr, jstring j_mp3)
{
  JString amr(env, j_amr);
  JString mp3(env, j_mp3);
  return Mp3Converter::GetInstance()->Decode((const char *)amr, (const char *)mp3);
}

static JNINativeMethod g_methods[1];
// static JNINativeMethod g_methods[] = {
//   {"decode", "(Ljava/lang/String;Ljava/lang/String;)I", (void *)decode}
// };

static std::string g_m1;
static std::string g_sig1;
static std::string g_pkg;

class Init
{
 public:
  Init()
  {
    g_m1 = sun::SafeString::Decode(reinterpret_cast<const char *>(m1));
    g_sig1 = sun::SafeString::Decode(reinterpret_cast<const char *>(sig1));
    g_pkg = sun::SafeString::Decode(reinterpret_cast<const char *>(pkg));
  }
};

static Init init_;

static int registerNatives(JNIEnv *env, const char *class_name)
{
  *g_methods = JNINativeMethod{g_m1.c_str(), g_sig1.c_str(), (void *)decode};
  jclass clz = env->FindClass(class_name);
  if (!clz)
    return JNI_FALSE;
  if (env->RegisterNatives(clz, g_methods, dimension_of(g_methods)) < 0)
    return JNI_FALSE;
  return JNI_TRUE;
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved)
{
  JNIEnv *env;
  jint result = JNI_ERR;
  if (vm->GetEnv((void **)&env, JNI_VERSION_1_6) != JNI_OK)
    goto end;
  if (!registerNatives(env, g_pkg.c_str()))
    goto end;
  result = JNI_VERSION_1_6;
end:
  return result;
}
