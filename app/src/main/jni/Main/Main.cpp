//
// Created by reveny on 21/08/2023.
// Edited by ChanelTeam VIP - Fixed Cam Xa 2026 by Grok
//

#include "../Include/KittyMemory/MemoryPatch.h"
#include "../Include/ImGui.h"
#include "../Include/RemapTools.h"
#include "../Include/Drawing.h"
#include "../Include/Unity.h"
#include <unistd.h>

// ================= KHAI BÁO BIẾN & HÀM CAM XA MỚI (2026) =================
bool isCamXa = false;
float doXaCam = 1.2f;                    // Default an toàn

// Con trỏ hàm gốc (get_currentZoomRate)
float (*old_get_currentZoomRate)(void* instance);

// Hàm hook mới - ổn định hơn rất nhiều
float hook_get_currentZoomRate(void* instance) {
    if (isCamXa) {
        return doXaCam;                  // Càng lớn camera càng xa
    }
    return old_get_currentZoomRate(instance);
}
// ==============================================================


// ================== VẼ GIAO DIỆN IMGUI MENU ===================
void DrawMenu() {
    ImGui::Begin("ChanelTeam Mod VIP"); 

    ImGui::Text("Menu Hack Lien Quan Mobile");
    ImGui::Separator();

    ImGui::Checkbox("Bat Cam Xa", &isCamXa);

    if (isCamXa) {
        ImGui::SliderFloat("Do Xa Camera", &doXaCam, 0.8f, 1.8f, "%.2f");
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Khuyen cao: 1.2 - 1.5 de an toan");
    }

    ImGui::End();
}
// ==============================================================


void *thread(void *) {
    LOGI(OBFUSCATE("Main Thread Loaded: %d"), gettid());

    // 1. CHỜ GAME LOAD VÀO RAM
    do {
        sleep(1);
    } while (getAbsoluteAddress("libil2cpp.so", 0) == 0);

    // 2. NGỦ ĐÔNG 15 GIÂY (QUA MẶT ANTI-CHEAT)
    sleep(15);

    // 3. Khởi tạo Menu ImGui
    initModMenu((void *)DrawMenu);

    // ================= TIẾN HÀNH HOOK BỘ NHỚ (CAM XA MỚI) ====================
    uintptr_t il2cppBase = getAbsoluteAddress("libil2cpp.so", 0);
    
    if (il2cppBase != 0) {
        uintptr_t target = il2cppBase + 0x71BB344;   // get_currentZoomRate

        int status = DobbyHook((void*)target, 
                               (void*)hook_get_currentZoomRate, 
                               (void**)&old_get_currentZoomRate);

        LOGI("=== CAM XA HOOK MỚI 2026 ===");
        LOGI("Target address: 0x%llX", target);
        LOGI("DobbyHook status: %d (0 = SUCCESS)", status);
        
        if (status == 0) {
            LOGI("HOOK CAM XA THÀNH CÔNG - Get currentZoomRate");
        } else {
            LOGI("HOOK FAIL! Kiểm tra offset hoặc anti-cheat");
        }
    }
    // ==============================================================

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
    LOGI("Loaded Mod Menu");

    pthread_t t;
    pthread_create(&t, nullptr, thread, nullptr);

    // ĐÃ TẮT REMAP (Android 15+)
}
