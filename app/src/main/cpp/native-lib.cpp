#include <android/log.h>
#include <jni.h>

#include <string>

#define LogF(fmt, ...) \
    do {               \
        __android_log_print(ANDROID_LOG_INFO, "phi3splittest", fmt, __VA_ARGS__); \
    } while (0)

void Log(const char* message) {
    __android_log_write(ANDROID_LOG_INFO, "phi3splittest", message);
}

std::string RunPhi3(const char* model_path) {
    return "placeholder";
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_example_phi3splittest_MainActivity_stringFromJNI(
        JNIEnv* env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}

extern "C"
JNIEXPORT jstring JNICALL
Java_com_example_phi3splittest_MainActivity_runPhi3(JNIEnv *env, jobject thiz) {
    const auto adsp_library_path = std::getenv("ADSP_LIBRARY_PATH");
    LogF("ADSP_LIBRARY_PATH = %s", (adsp_library_path != nullptr) ? adsp_library_path : "not set");

    std::string result = RunPhi3("/data/local/tmp/phi3-split/phi3-split-qnn-updated");
    return env->NewStringUTF(result.c_str());
}
