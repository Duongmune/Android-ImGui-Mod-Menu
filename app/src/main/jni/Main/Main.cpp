//
// Created by reveny on 21/08/2023.
// Edited by ChanelTeam VIP - HỆ THỐNG ANTI-CRASH & LOGGING ĐỈNH CAO 2026
//

#include "../Include/KittyMemory/MemoryPatch.h"
#include "../Include/ImGui.h"
#include "../Include/RemapTools.h"
#include "../Include/Drawing.h"
#include "../Include/Unity.h"

// THÊM THƯ VIỆN ĐỂ NUỐT LỖI VÀ GHI LOG
#include <unistd.h>
#include <signal.h>
#include <setjmp.h>
#include <fcntl.h>
#include <stdio.h>

// ================= HỆ THỐNG NUỐT LỖI (SAFE-MODE) =================
sigjmp_buf crash_env;        // Biến lưu tọa độ "Checkpoint"
bool isCrashHandled = false; // Cờ kiểm tra xem đã nuốt lỗi chưa

// Cờ quản lý Mod
bool isCamXa = false;
float doXaCam = 1.2f;
bool isHookFailed = false; 

// HÀM GHI LOG VÀ BẮT LỖI VĂNG GAME
void CrashHandler(int sig, siginfo_t *info, void *context) {
    // 1. GHI LOG RA MÁY (KHÔNG CẦN ROOT)
    // Ưu tiên 1: Ghi ra thư mục Download cho dễ tìm
    const char* path1 = "/storage/emulated/0/Download/ChanelTeam_CrashLog.txt";
    // Ưu tiên 2: Nếu máy bảo mật cao cấm ghi Download, ném vào Data gốc của game
    const char* path2 = "/storage/emulated/0/Android/data/com.garena.game.kgvn/files/ChanelTeam_CrashLog.txt"; 

    int fd = open(path1, O_WRONLY | O_CREAT | O_APPEND, 0666);
    if (fd < 0) fd = open(path2, O_WRONLY | O_CREAT | O_APPEND, 0666);

    if (fd >= 0) {
        char buffer[512];
        int len = snprintf(buffer, sizeof(buffer), 
            "\n========== [CHANEL TEAM - CRASH REPORT] ==========\n"
            "Phat hien loi văng game (Signal: %d)\n"
            "Dia chi gay loi (Bad Offset/RAM): %p\n"
            "Hanh dong: Da NUOT LOI. Tu dong tat Mod de cuu Game!\n"
            "==================================================\n", 
            sig, info->si_addr);
        write(fd, buffer, len);
        close(fd);
    }

    // 2. KÍCH HOẠT CỜ CHẾT
    isHookFailed = true;

    // 3. THUẬT TOÁN XUYÊN KHÔNG (NUỐT LỖI)
    // Sẽ bê toàn bộ luồng xử lý bay ngược về cái Checkpoint an toàn đã tạo
    if (!isCrashHandled) {
        isCrashHandled = true;
        siglongjmp(crash_env, 1); // Lệnh bay về điểm an toàn
    } else {
        // Nếu đã cố cứu 1 lần mà vẫn lỗi -> Buông xuôi cho Thread này chết
        pthread_exit(0);
    }
}

// BẬT LÁ CHẮN BẢO VỆ
void InitCrashHandler() {
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO | SA_NODEFER; 
    sa.sa_sigaction = CrashHandler;
    sigemptyset(&sa.sa_mask);

    // Bắt 3 cái lỗi tồi tệ nhất gây văng game trên Android
    sigaction(SIGSEGV, &sa, NULL); // Bắt lỗi tràn RAM / Sai Offset
    sigaction(SIGABRT, &sa, NULL); // Bắt lỗi hệ thống tự đóng
    sigaction(SIGBUS, &sa, NULL);  // Bắt lỗi rác bộ nhớ
}
// =================================================================


// ================= KHAI BÁO HÀM HOOK CAM XA =================
float (*old_get_currentZoomRate)(void* instance);

float hook_get_currentZoomRate(void* instance) {
    if (isCamXa && !isHookFailed) {
        return doXaCam;
    }
    if (old_get_currentZoomRate != nullptr) {
        return old_get_currentZoomRate(instance);
    }
    return 1.0f;
}
// ============================================================


// ================== VẼ GIAO DIỆN IMGUI MENU ===================
void DrawMenu() {
    ImGui::Begin("ChanelTeam Mod VIP"); 

    ImGui::Text("Menu Hack Lien Quan Mobile");
    ImGui::Separator();

    if (isHookFailed) {
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "[!] HE THONG LOI [!]");
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Da nuot loi va dong bang chuc nang.");
        ImGui::Text("Check thu muc Download de xem Log!");
    } else {
        ImGui::Checkbox("Bat Cam Xa", &isCamXa);
        if (isCamXa) {
            ImGui::SliderFloat("Do Xa Camera", &doXaCam, 0.8f, 1.8f, "%.2f");
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Khuyen cao: 1.2 - 1.5 de an toan");
        }
    }

    ImGui::End();
}
// ==============================================================


void *thread(void *) {
    LOGI(OBFUSCATE("Main Thread Loaded: %d"), gettid());

    // 1. DỰNG LÁ CHẮN BẮT LỖI LÊN TRƯỚC TIÊN
    InitCrashHandler();

    // 2. TẠO CHECKPOINT (ĐIỂM LƯU GAME)
    // NẾU BẤT CỨ DÒNG CODE NÀO BÊN DƯỚI GÂY VĂNG GAME (VD: LỖI OFFSET, LỖI HOOK)
    // HỆ THỐNG SẼ TỰ ĐỘNG BẮN LUỒNG XỬ LÝ QUAY NGƯỢC LẠI ĐÚNG VỊ TRÍ NÀY!
    if (sigsetjmp(crash_env, 1) != 0) {
        LOGI("Đã kích hoạt bảo vệ. Hủy Mod để Game gốc vào bình thường.");
        // Rút điện bản Mod, hủy Thread này. Game sẽ chạy như chưa hề cài Mod.
        pthread_exit(0);
        return nullptr;
    }

    // 3. KỊCH BẢN BÌNH THƯỜNG: NGỦ ĐÔNG 14 GIÂY
    sleep(14);

    uintptr_t il2cppBase = getAbsoluteAddress("libil2cpp.so", 0);
    
    // Nếu rác/mã hóa quá nặng không tìm thấy lõi -> tự sát để game an toàn
    if (il2cppBase == 0) {
        pthread_exit(0);
        return nullptr;
    }

    // Đồ họa load xong an toàn -> bung Menu ImGui
    initModMenu((void *)DrawMenu);

    // 4. TIẾN HÀNH HOOK
    uintptr_t target = il2cppBase + 0x71BB344;
    int status = DobbyHook((void*)target, (void*)hook_get_currentZoomRate, (void**)&old_get_currentZoomRate);

    // Nếu lệnh DobbyHook phía trên mà chọc nhầm ổ kiến gây văng game -> Lá chắn sẽ kích hoạt
    // Còn nếu DobbyHook báo lỗi nhẹ (status != 0) -> Ta tự khóa Menu
    if (status != 0) {
        isHookFailed = true;
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
    LOGI("Loaded Mod Menu");

    pthread_t t;
    pthread_create(&t, nullptr, thread, nullptr);
}
