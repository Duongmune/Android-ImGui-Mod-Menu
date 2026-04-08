//
// Created by reveny on 21/08/2023.
// Edited by ChanelTeam VIP
//

#include "../Include/KittyMemory/MemoryPatch.h"
#include "../Include/ImGui.h"
#include "../Include/RemapTools.h"
#include "../Include/Drawing.h"
#include "../Include/Unity.h"
#include <unistd.h> // Thư viện cần thiết cho lệnh sleep()

// ================= KHAI BÁO BIẾN & HÀM CAM XA =================
bool isCamXa = false;
float doXaCam = 1.5f;

// Con trỏ lưu hàm gốc của game
float (*old_GetCameraHeightRateValue)(void* instance, int type);

// Hàm giả mạo sẽ đè lên hàm gốc
float hook_GetCameraHeightRateValue(void* instance, int type) {
    if (isCamXa) {
        return doXaCam; // Trả về độ xa do mình kéo trên menu
    }
    return old_GetCameraHeightRateValue(instance, type); // Trả về bình thường
}
// ==============================================================


// ================== VẼ GIAO DIỆN IMGUI MENU ===================
void DrawMenu() {
    // Xóa cái ShowDemoWindow của bản gốc đi, vẽ menu của riêng mình:
    ImGui::Begin("ChanelTeam Mod VIP"); 

    ImGui::Text("Menu Hack Lien Quan Mobile");
    ImGui::Separator();

    // Checkbox bật/tắt
    ImGui::Checkbox("Bat Cam Xa", &isCamXa);

    // Nếu bật thì hiện thanh kéo chỉnh độ xa
    if (isCamXa) {
        ImGui::SliderFloat("Do Xa Camera", &doXaCam, 1.0f, 3.0f, "%.1f");
    }

    ImGui::End();
}
// ==============================================================


void *thread(void *) {
    LOGI(OBFUSCATE("Main Thread Loaded: %d"), gettid());

    // ĐỢI GAME LOAD XONG THƯ VIỆN (Tránh văng game)
    do {
        sleep(1);
    } while (getAbsoluteAddress("libil2cpp.so", 0) == 0);

    // Khởi tạo Menu ImGui
    initModMenu((void *)DrawMenu);

    // ================= TIẾN HÀNH HOOK BỘ NHỚ ====================
    // Lấy địa chỉ gốc của thư viện
    uintptr_t il2cppBase = getAbsoluteAddress("libil2cpp.so", 0);
    
    // Ép DobbyHook vào cái offset 0x8D546F4 mà tui tìm cho bro
    DobbyHook((void*)(il2cppBase + 0x8D546F4), (void*)hook_GetCameraHeightRateValue, (void**)&old_GetCameraHeightRateValue);
    // ==============================================================

    LOGI("Main thread done");
    pthread_exit(0);
}


// Call anything from JNI_OnLoad here
extern "C" {
    // JNI Support
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

    //Don't leave any traces, remap the loader lib as well
    RemapTools::RemapLibrary("libLoader.so");
}
