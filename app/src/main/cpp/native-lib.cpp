#include <android/log.h>
#include <jni.h>

#include <functional>
#include <memory>
#include <string>

#include <onnxruntime/onnxruntime_cxx_api.h>

#define LogF(fmt, ...) \
    do {               \
        __android_log_print(ANDROID_LOG_INFO, "phi3splittest", fmt, __VA_ARGS__); \
    } while (0)

void Log(const char* message) {
    __android_log_write(ANDROID_LOG_INFO, "phi3splittest", message);
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
Java_com_example_phi3splittest_MainActivity_runModel(JNIEnv *jniEnv, jobject thiz,
                                                     jbyteArray modelBytesArray) {
    Ort::Env ortEnv{ORT_LOGGING_LEVEL_VERBOSE};
    Ort::SessionOptions sessionOptions{};
    sessionOptions.AppendExecutionProvider("QNN",
                                           {{"backend_path", "libQnnHtp.so"}});

    // manage model bytes in unique_ptr
    jbyte* modelBytesRaw = jniEnv->GetByteArrayElements(modelBytesArray, nullptr);
    size_t modelBytesLength = jniEnv->GetArrayLength(modelBytesArray);

    auto modelBytes = std::unique_ptr<jbyte, std::function<void(jbyte*)>>{
        modelBytesRaw,
        [&modelBytesArray, &jniEnv](jbyte* p) {
            if (p) jniEnv->ReleaseByteArrayElements(modelBytesArray, p, JNI_ABORT);
        }};

    Ort::Session session{ortEnv, modelBytesRaw, modelBytesLength, sessionOptions};

    using T = float;
    Ort::AllocatorWithDefaultOptions allocator{};
    const auto shape = std::array{int64_t{1}};

    auto a = Ort::Value::CreateTensor<T>(allocator, shape.data(), shape.size());
    *a.GetTensorMutableData<T>() = T{1};

    auto b = Ort::Value::CreateTensor<T>(allocator, shape.data(), shape.size());
    *b.GetTensorMutableData<T>() = T{2};

    auto inputs = std::vector<Ort::Value>{};
    inputs.emplace_back(std::move(a));
    inputs.emplace_back(std::move(b));
    auto input_names = std::array{"A", "B"};

    auto output_names = std::array{"C"};
    auto output = session.Run(Ort::RunOptions{}, input_names.data(), inputs.data(), inputs.size(),
                              output_names.data(), output_names.size());

    std::string result = "ORT says 1 + 2 = " + std::to_string(*output[0].GetTensorMutableData<T>());

    return jniEnv->NewStringUTF(result.c_str());
}
