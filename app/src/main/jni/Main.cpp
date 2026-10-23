#include <list>
#include <vector>
#include <cstring>
#include <pthread.h>
#include <thread>
#include <string>
#include <jni.h>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <dlfcn.h>
#include "Includes/Logger.h"
#include "Includes/obfuscate.h"
#include "Includes/Utils.hpp"
#include "Menu/Menu.hpp"
#include "Menu/Jni.hpp"
#include "Includes/Macros.h"
#include "dobby.h"

// ==================== متغيرات التحكم ====================
bool godMode = false;
bool highDamage = false;
bool infiniteAmmo = false;
bool speedHack = false;

// ==================== عناوين RVA (من الدامب) ====================
// دالة OnHPUpdated في كلاس BaseNetworkPlayer
#define RVA_Player_OnHPUpdated 0xB1EA80 

// ==================== الأوفستات (Offsets) ====================
// BaseNetworkPlayer
#define OFFSET_PLAYER_HP 0x6C
#define OFFSET_PLAYER_CHARACTER 0x68

// Gun (Weapons) - الأوفستات الصح من الدامب
#define OFFSET_GUN_DAMAGE 0x7C 
#define OFFSET_GUN_AMMO_IN_MAG 0xB0 
#define OFFSET_GUN_MAG_SIZE 0xAC 

// ==================== دوال الهوك (Hook) ====================
void* (*old_Player_OnHPUpdated)(void* instance, float oldValue, float newValue);

void* Player_OnHPUpdated(void* instance, float oldValue, float newValue) {
    if (instance != NULL) {
        // 1. God Mode
        if (godMode) {
            *(float*)((uintptr_t)instance + OFFSET_PLAYER_HP) = 100.0f;
        }
        
        // 2. High Damage & Infinite Ammo (مثال)
        if (highDamage || infiniteAmmo) {
            void* character = *(void**)((uintptr_t)instance + OFFSET_PLAYER_CHARACTER);
            if (character != NULL) {
                // كود تعديل السلاح (محتاج عنوان السلاح)
            }
        }
    }
    return old_Player_OnHPUpdated(instance, oldValue, newValue);
}

// ==================== دالة تفعيل الهوك ====================
void* hack_thread(void* arg) {
    while (!isLibraryLoaded("libil2cpp.so")) {
        sleep(1);
    }
    
    uintptr_t libBase = (uintptr_t)getAbsoluteAddress("libil2cpp.so", 0);
    
    // تفعيل الهوك للاعب
    void* addr_Player = (void*)(libBase + RVA_Player_OnHPUpdated);
    DobbyHook(addr_Player, (void*)Player_OnHPUpdated, (void**)&old_Player_OnHPUpdated);
    
    return NULL;
}

// ==================== دالة القائمة (Menu) ====================
jobjectArray GetFeatureList(JNIEnv *env, jobject context) {
    jobjectArray ret;
    const char *features[] = {
        OBFUSCATE("Toggle_God Mode"),
        OBFUSCATE("Toggle_High Damage"),
        OBFUSCATE("Toggle_Infinite Ammo"),
        OBFUSCATE("Toggle_Speed Hack"),
    };
    int TotalFeatures = sizeof(features) / sizeof(features[0]);
    jclass stringClass = env->FindClass("java/lang/String");
    ret = env->NewObjectArray(TotalFeatures, stringClass, NULL);
    for (int i = 0; i < TotalFeatures; i++) {
        jstring str = env->NewStringUTF(features[i]);
        env->SetObjectArrayElement(ret, i, str);
    }
    return ret;
}

// ==================== دالة التحكم في الأزرار ====================
void Changes(JNIEnv *env, jclass clazz, jobject obj, jint feature, jstring text, jint intValue, jlong longValue, jboolean boolValue, jstring stringValue) {
    switch (feature) {
        case 0: godMode = boolValue; break;
        case 1: highDamage = boolValue; break;
        case 2: infiniteAmmo = boolValue; break;
        case 3: speedHack = boolValue; break;
    }
}
