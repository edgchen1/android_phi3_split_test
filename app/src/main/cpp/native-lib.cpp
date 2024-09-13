#include <android/log.h>
#include <jni.h>

#include <string>

#include <ort_genai.h>

#define LogF(fmt, ...) \
    do {               \
        __android_log_print(ANDROID_LOG_INFO, "phi3splittest", fmt, __VA_ARGS__); \
    } while (0)

void Log(const char* message) {
    __android_log_write(ANDROID_LOG_INFO, "phi3splittest", message);
}

std::string RunPhi3(const char* model_path) {
    auto model = OgaModel::Create(model_path);
    auto tokenizer = OgaTokenizer::Create(*model);
    auto tokenizer_stream = OgaTokenizerStream::Create(*tokenizer);

    while (true) {
        const std::string prompt_text = "What is the square root of 16?";

        const std::string prompt = "<|user|>\n" + prompt_text + " <|end|>\n<|assistant|>";

        auto sequences = OgaSequences::Create();
        constexpr size_t seq_len = 512;
        constexpr int32_t space_token = 220;
        tokenizer->Encode(prompt.c_str(), *sequences);
        const size_t num_tokens = sequences->SequenceCount(0);
        LogF("num_tokens: %zu", num_tokens);
        sequences->PadSequence(space_token, seq_len, 0);

        std::array<float, seq_len> attn_mask{};
        for (size_t i = 0; i < std::min(seq_len, num_tokens); ++i) {
            attn_mask[i] = 1.0f;
        }
        for (size_t i = num_tokens; i < seq_len; ++i) {
            attn_mask[i] = 0.0f;
        }

        const std::array<int64_t, 1> shape{seq_len};

        auto attn_mask_tensor = OgaTensor::Create(
                attn_mask.data(), shape.data(), shape.size(), OgaElementType_float32);

        Log("Generating response...");
        auto params = OgaGeneratorParams::Create(*model);
        params->SetSearchOption("max_length", 1024);
        params->SetSearchOptionBool("do_sample", true);
        params->SetSearchOption("top_k", 5);
        params->SetSearchOption("top_p", 0.9);
        params->SetSearchOption("temperature", 0.1);
        params->SetInputSequences(*sequences);
        params->SetModelInput("attn_mask", *attn_mask_tensor);

        Log("Creating generator...");

        std::string output{};

        auto generator = OgaGenerator::Create(*model, *params);

        while (!generator->IsDone()) {
            generator->ComputeLogits();
            generator->GenerateNextToken();

            const auto num_tokens = generator->GetSequenceCount(0);
            const auto new_token = generator->GetSequenceData(0)[num_tokens - 1];
            const auto partial_output = tokenizer_stream->Decode(new_token);

            LogF("generated partial output: %s", partial_output);
            output.append(partial_output);
        }

        return output;
    }
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
