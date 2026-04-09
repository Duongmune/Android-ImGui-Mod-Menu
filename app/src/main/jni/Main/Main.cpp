//
// Created by reveny on 21/08/2023.
// Edited by ChanelTeam VIP - BẢN HACK TÀNG HÌNH (CHỐNG CRASH 100%)
//

#include "../Include/KittyMemory/MemoryPatch.h"
#include "../Include/ImGui.h" 
#include "../Include/RemapTools.h"
#include "../Include/Drawing.h"
#include "../Include/Unity.h"
#include <unistd.h>

// ================= KHAI BÁO BIẾN CAM XA TỰ ĐỘNG =================
bool isCamXa = true;      // ÉP BẬT LUÔN (Không cần Menu)
float doXaCam = 1.3f;     // Độ xa camera (Bro có thể chỉnh 1.2 đến 1.5)

float (*old_get_currentZoomRate)(void* instance);

float hook_get_currentZoomRate(void* instance) {
    if (isCamXa) {
        return doXaCam; // Ép camera xa ra
    }
    if (old_get_currentZoomRate != nullptr) {
        return old_get_currentZoomRate(instance);
    }
    return 1.0f;
}
// ================================================================


void *thread(void *) {
    LOGI(OBFUSCATE("Main Thread Loaded: %d"), gettid());

    // 1. NGỦ 15 GIÂY: Qua ải Logo và Anti-cheat ban đầu một cách êm ái
    sleep(15);

    // 2. LẤY LÕI GAME (Chỉ mở sổ Maps 1 lần duy nhất)
    uintptr_t il2cppBase = getAbsoluteAddress("libil2cpp.so", 0);
    
    if (il2cppBase == 0) {
        LOGI("Không tìm thấy libil2cpp.so - Tự thoát Mod để cứu game");
        pthread_exit(0);
        return nullptr;
    }

    // ==============================================================
    // [QUAN TRỌNG NHẤT]: ĐÃ XÓA CODE GỌI MENU IMGUI! 
    // Không vẽ Menu = Không xung đột đồ họa Vulkan = Không Crash!
    // ==============================================================

    // 3. CẮM HOOK CAM XA NGẦM VÀO GAME
    uintptr_t target = il2cppBase + 0x71BB344;
    
    int status = DobbyHook((void*)target, (void*)hook_get_currentZoomRate, (void**)&old_get_currentZoomRate);

    if (status == 0) {
        LOGI("HOOK CAM XA NGẦM THÀNH CÔNG! Vào game quẩy thôi.");
    } else {
        LOGI("HOOK LỖI! DobbyHook status: %d", status);
    }

    LOGI("Main thread done");
    pthread_exit(0);
}


// Call anything from JNI_OnLoad here
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
    LOGI("Loaded Mod Menu - Stealth Mode");

    pthread_t t;
    pthread_create(&t, nullptr, thread, nullptr);

    // ============================================================
    // TẮT REMAP: Để Android 15 không phát hiện và bóp cổ văng game
    // ============================================================
    // RemapTools::RemapLibrary("libLoader.so");
}
