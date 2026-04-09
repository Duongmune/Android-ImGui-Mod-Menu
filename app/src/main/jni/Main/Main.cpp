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

    // 1. CHỜ GAME LOAD VÀO RAM
    do {
        sleep(1);
    } while (getAbsoluteAddress("libil2cpp.so", 0) == 0);

    // ============================================================
    // 2. NGỦ ĐÔNG 15 GIÂY (TUYỆT CHIÊU QUA MẶT ANTI-CHEAT)
    // Chờ Garena giải mã xong hoàn toàn libil2cpp.so mới được hành động
    // ============================================================
    sleep(15);

    // 3. Khởi tạo Menu ImGui (Sau khi đồ họa đã load mượt)
    initModMenu((void *)DrawMenu);

    // ================= TIẾN HÀNH HOOK BỘ NHỚ ====================
    // Lấy địa chỉ gốc của thư viện (Lúc này đã giải mã sạch sẽ)
    uintptr_t il2cppBase = getAbsoluteAddress("libil2cpp.so", 0);
    
    // Móc DobbyHook an toàn
    if (il2cppBase != 0) {
        DobbyHook((void*)(il2cppBase + 0x8D546F4), (void*)hook_GetCameraHeightRateValue, (void**)&old_GetCameraHeightRateValue);
    }
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

    // ============================================================
    // ĐÃ TẮT REMAP: Để Android 15 không phát hiện và bóp cổ văng game
    // ============================================================
    // RemapTools::RemapLibrary("libLoader.so");
}
