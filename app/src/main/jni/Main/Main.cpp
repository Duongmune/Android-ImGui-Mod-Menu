//
// Created by reveny on 21/08/2023.
// Edited by ChanelTeam VIP - Fixed Cam Xa 2026 by Grok
// FIXED CRASH
//

#include "../Include/KittyMemory/MemoryPatch.h"
#include "../Include/ImGui.h"
#include "../Include/RemapTools.h"
#include "../Include/Drawing.h"
#include "../Include/Unity.h"
#include <unistd.h>
#include <pthread.h>
#include <atomic>
#include <thread>

// ================= KHAI BÁO BIẾN & HÀM CAM XA =================
std::atomic<bool> isCamXa(false);
float doXaCam = 1.2f;

// Con trỏ hàm gốc
float (*old_get_currentZoomRate)(void* instance);

// Hàm hook mới
float hook_get_currentZoomRate(void* instance) {
    if (isCamXa.load()) {
        return doXaCam;
    }
    return old_get_currentZoomRate(instance);
}

// ================== VẼ GIAO DIỆN IMGUI MENU ===================
void DrawMenu() {
    ImGui::Begin("ChanelTeam Mod VIP");
    ImGui::Text("Menu Hack Lien Quan Mobile");
    ImGui::Separator();

    bool camState = isCamXa.load();
    if (ImGui::Checkbox("Bat Cam Xa", &camState)) {
        isCamXa.store(camState);
    }

    if (isCamXa.load()) {
        ImGui::SliderFloat("Do Xa Camera", &doXaCam, 0.8f, 1.8f, "%.2f");
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Khuyen cao: 1.2 - 1.5 de an toan");
    }
    ImGui::End();
}

// Kiểm tra camera main đã sẵn sàng (dùng Unity API)
bool isCameraMainReady() {
    // Nếu bạn có Unity.h chuẩn, hãy dùng:
    // return Unity::Camera::get_main() != nullptr;
    // Nếu không, bạn có thể tìm bằng GameObject.Find (cần xử lý IL2CPP)
    // Tạm thời trả về true nếu chưa implement - NHƯNG BẠN CẦN SỬA LẠI
    static bool checked = false;
    static bool ready = false;
    if (!checked) {
        // TODO: Thay bằng code kiểm tra camera thực tế
        // Ví dụ dùng Unity::Camera::get_main() từ file Unity.h của bạn
        ready = true;  // Giả sử luôn ready (sẽ không an toàn)
        checked = true;
    }
    return ready;
}

void *thread_main(void *) {
    LOGI(OBFUSCATE("Main Thread Loaded: %d"), gettid());

    // 1. Chờ libil2cpp.so được map
    while (getAbsoluteAddress("libil2cpp.so", 0) == 0) {
        usleep(100000); // 0.1s
    }
    LOGI("libil2cpp.so found");

    // 2. Chờ camera main ready (đã vào game thực sự)
    while (!isCameraMainReady()) {
        sleep(1);
    }
    LOGI("Camera main ready");

    // 3. Đợi thêm 3 giây để game ổn định hoàn toàn
    sleep(3);

    // 4. Hook hàm get_currentZoomRate
    uintptr_t il2cppBase = getAbsoluteAddress("libil2cpp.so", 0);
    uintptr_t target = il2cppBase + 0x71BB344; // offset từ dump

    LOGI("Target address: 0x%llX", (unsigned long long)target);

    // Kiểm tra địa chỉ hợp lệ
    if (target > il2cppBase && target < il2cppBase + 0x2000000) {
        int status = DobbyHook((void*)target,
                               (void*)hook_get_currentZoomRate,
                               (void**)&old_get_currentZoomRate);
        LOGI("DobbyHook status: %d", status);
        if (status == 0) {
            LOGI("HOOK CAM XA THANH CONG");
        } else {
            LOGI("HOOK THAT BAI, code: %d", status);
        }
    } else {
        LOGI("Target address invalid!");
    }

    // 5. Khởi tạo menu ImGui sau khi hook thành công và game ổn định
    sleep(2);
    initModMenu((void*)DrawMenu);
    LOGI("Menu initialized");

    pthread_exit(nullptr);
}

// ==================== JNI & INIT ====================
extern "C" {
    JavaVM *jvm = nullptr;
    JNIEnv *env = nullptr;

    __attribute__((visibility ("default")))
    jint loadJNI(JavaVM *vm) {
        jvm = vm;
        vm->AttachCurrentThread(&env, nullptr);
        LOGI("loadJNI(): Initialized");
        return JNI_VERSION_1_6;
    }
}

__attribute__((constructor))
void init() {
    LOGI("Loaded Mod Menu");
    pthread_t pt;
    pthread_create(&pt, nullptr, thread_main, nullptr);
    pthread_detach(pt);  // không cần join
}
