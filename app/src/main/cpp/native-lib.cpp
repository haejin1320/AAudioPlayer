#include <jni.h>
#include <aaudio/AAudio.h>
#include <fstream>
#include <vector>
#include <thread>
#include <string>
#include <android/log.h>

#define LOG_TAG "AAudioPlayer"
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)


// 파일 경로 및 전역 변수
const char* FILE_PATH = "/sdcard/Download/test.wav";
std::vector<int16_t> pcmData;
AAudioStream* stream;
int32_t sampleRate = 44100;  // 기본값
int32_t channelCount = 1;    // 기본값
int32_t bitDepth = 16;

// WAV 헤더에서 샘플링 속도와 채널 수 읽기
std::vector<int16_t> readWavFile(const char* filePath) {
    std::ifstream file(filePath, std::ios::binary);

    file.seekg(24);  // 샘플 속도는 WAV 헤더의 24 바이트 위치에 있음
    file.read(reinterpret_cast<char*>(&sampleRate), sizeof(sampleRate));

    file.seekg(22);  // 채널 수는 WAV 헤더의 22 바이트 위치에 있음
    uint16_t channels;
    file.read(reinterpret_cast<char*>(&channels), sizeof(channels));
    channelCount = channels;

    // 비트 깊이 읽기
    file.seekg(34);
    uint16_t depth;
    file.read(reinterpret_cast<char*>(&depth), sizeof(depth));
    bitDepth = depth;

    file.seekg(44);  // PCM 데이터 시작 위치
    int16_t sample;
    while (file.read(reinterpret_cast<char*>(&sample), sizeof(sample))) {
        pcmData.push_back(sample);
    }
    file.close();
    return pcmData;
}


// AAudio 데이터 콜백 함수
aaudio_data_callback_result_t dataCallback(AAudioStream* stream,
                                           void* userData,
                                           void* audioData,
                                           int32_t numFrames) {
    static size_t dataIndex = 0;
    auto* outputData = static_cast<int16_t*>(audioData);

    for (int i = 0; i < numFrames; i++) {
        if (dataIndex < pcmData.size()) {
            outputData[i] = pcmData[dataIndex++];
        } else {
            outputData[i] = 0; // 데이터가 없을 때 0으로 채움
        }
    }

    return AAUDIO_CALLBACK_RESULT_CONTINUE;
}

// AAudio 스트림 생성 및 시작 함수
void createAndStartStream() {
    AAudioStreamBuilder* builder;
    AAudio_createStreamBuilder(&builder);

    if (bitDepth == 16) {
        AAudioStreamBuilder_setFormat(builder, AAUDIO_FORMAT_PCM_I16);
    } else if (bitDepth == 32) {
        AAudioStreamBuilder_setFormat(builder, AAUDIO_FORMAT_PCM_FLOAT);
    } else {
        LOGE("Unsupported bit depth: %d", bitDepth);
        return;
    }
    AAudioStreamBuilder_setChannelCount(builder, channelCount);
    AAudioStreamBuilder_setSampleRate(builder, sampleRate);
    AAudioStreamBuilder_setDataCallback(builder, dataCallback, nullptr);

    aaudio_result_t result = AAudioStreamBuilder_openStream(builder, &stream);
    if (result != AAUDIO_OK) {
        LOGE("Failed to open AAudio stream. Error: %d", result);
        return;
    }
    AAudioStreamBuilder_delete(builder);

    AAudioStream_requestStart(stream);
}

// AAudio 스트림 정지 및 종료 함수
void stopAndCleanUpStream() {
    if (stream) {
        AAudioStream_requestStop(stream);
        AAudioStream_close(stream);
        stream = nullptr;
    }
}

extern "C" JNIEXPORT void JNICALL
Java_com_aaudio_aaudioplayer_MainActivity_startPlayback(JNIEnv* env, jobject /* this */) {
    pcmData = readWavFile(FILE_PATH);
    if (pcmData.empty()) {
        LOGE("Failed to read PCM data from file.");
        return;
    }

    LOGI("Bit depth : %d", bitDepth);
    LOGI("Sample Rate: %d", sampleRate);
    LOGI("Channel Count: %d", channelCount);
    LOGI("PCM data successfully read, starting stream...");
    createAndStartStream();
}

extern "C" JNIEXPORT void JNICALL
Java_com_aaudio_aaudioplayer_MainActivity_stopPlayback(JNIEnv* env, jobject /* this */) {
    LOGI("Stopping playback...");
    stopAndCleanUpStream();
}

extern "C" JNIEXPORT void JNICALL
Java_com_aaudio_aaudioplayer_MainActivity_savePCMToFile(JNIEnv* env, jobject /* this */) {
    // 파일 이름 생성
    std::string filePath = "/sdcard/Download/";
    filePath += "output_" + std::to_string(channelCount) + "ch_";
    filePath += std::to_string(sampleRate) + "Hz_16bit.pcm";

    // PCM 파일 생성 및 데이터 쓰기
    std::ofstream outFile(filePath, std::ios::binary);
    if (!outFile) {
        LOGE("Failed to open file for writing PCM data.");
        return;
    }

    outFile.write(reinterpret_cast<const char*>(pcmData.data()), pcmData.size() * sizeof(int16_t));
    outFile.close();

    LOGI("PCM data saved to %s", filePath.c_str());
}