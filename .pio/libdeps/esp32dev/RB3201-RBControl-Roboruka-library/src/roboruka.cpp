#include <stdarg.h>
#include <stdio.h>

#include "_librk_context.h"
#include "roboruka.h"

#include "RBControl.hpp"

#define TAG "roboruka"

using namespace rb;
using namespace rk;
using namespace mcp3008;

// Empty loop in case the user won't supply one
void __attribute__((weak)) loop() {

}

void rkSetup(const rkConfig& cfg) {
    gCtx.setup(cfg);
}

void rkControllerSendLog(const char* format, ...) {
    if (gCtx.prot() == nullptr) {
        ESP_LOGE(TAG, "%s: protocol not initialized!", __func__);
        return;
    }
    va_list args;
    va_start(args, format);
    gCtx.prot()->send_log(format, args);
    va_end(args);
}

void rkControllerSendLog(const std::string& text) {
     if (gCtx.prot() == nullptr) {
        ESP_LOGE(TAG, "%s: protocol not initialized!", __func__);
        return;
    }

    gCtx.prot()->send_log(text);
}

void rkControllerSend(const char* cmd, rbjson::Object* data) {
    if (gCtx.prot() == nullptr) {
        ESP_LOGE(TAG, "%s: protocol not initialized!", __func__);
        return;
    }
    gCtx.prot()->send(cmd, data);
}

void rkControllerSendMustArrive(const char* cmd, rbjson::Object* data) {
    if (gCtx.prot() == nullptr) {
        ESP_LOGE(TAG, "%s: protocol not initialized!", __func__);
        return;
    }
    gCtx.prot()->send_mustarrive(cmd, data);
}

bool rkArmMoveTo(double x, double y) {
    if (!gCtx.arm().moveTo(x, y)) {
        ESP_LOGE(TAG, "%s: can't move to %.1f %.1f, failed to solve the movement!", __func__, x, y);
        return false;
    }
    return true;
}

bool rkArmPosition(double& outX, double& outY) {
    return gCtx.arm().getCurrentPosition(outX, outY);
}

void rkArmSetGrabbing(bool grab) {
    return gCtx.arm().setGrabbing(grab);
}

bool rkArmIsGrabbing() {
    return gCtx.arm().isGrabbing();
}

void rkArmSetServo(uint8_t id, float degrees) {
    Manager::get().servoBus().set(id, Angle::deg(degrees));
}

float rkArmGetServo(uint8_t id) {
    const auto pos = Manager::get().servoBus().pos(id);
    return pos.isNaN() ? nanf("") : pos.deg();
}

std::unique_ptr<rbjson::Object> rkArmGetInfo() {
    return gCtx.arm().getInfo();
}

float rkBatteryCoef() {
    return Manager::get().battery().fineTuneCoef();
}

uint32_t rkBatteryRaw() {
    return Manager::get().battery().raw();
}

uint32_t rkBatteryPercent() {
    return Manager::get().battery().pct();
}

uint32_t rkBatteryVoltageMv() {
    return Manager::get().battery().voltageMv();
}

void rkMotorsSetPower(int8_t left, int8_t right) {
    gCtx.motors().set(left, right);
}

void rkMotorsSetPowerLeft(int8_t power) {
    gCtx.motors().setById(gCtx.motors().idLeft(), power);
}

void rkMotorsSetPowerRight(int8_t power) {
    gCtx.motors().setById(gCtx.motors().idRight(), power);
}

void rkMotorsSetPowerById(int id, int8_t power) {
    id -= 1;
    if(id < 0 || id >= (int)rb::MotorId::MAX) {
        ESP_LOGE(TAG, "%s: invalid motor id, %d is out of range <1;8>.", __func__, id+1);
        return;
    }
    gCtx.motors().setById(rb::MotorId(id), power);
}

void rkMotorsJoystick(int32_t x, int32_t y) {
    gCtx.motors().joystick(x, y);
}

void rkLedRed(bool on) {
    Manager::get().leds().red(on);
}

void rkLedYellow(bool on) {
    Manager::get().leds().yellow(on);
}

void rkLedGreen(bool on) {
    Manager::get().leds().green(on);
}

void rkLedBlue(bool on) {
    Manager::get().leds().blue(on);
}

void rkLedAll(bool on) {
    auto& l = Manager::get().leds();
    l.red(on);
    l.yellow(on);
    l.green(on);
    l.blue(on);
}

void rkLedById(uint8_t id, bool on) {
    if (id == 0) {
        ESP_LOGE(TAG, "%s: invalid id %d, LEDs are indexed from 1, just like on the board (LED1, LED2...)!", __func__, id);
        return;
    } else if (id > 4) {
        ESP_LOGE(TAG, "%s: maximum LED id is 4, you are using %d!", __func__, id);
        return;
    }

    auto& l = Manager::get().leds();
    switch (id) {
    case 1:
        l.red(on);
        break;
    case 2:
        l.yellow(on);
        break;
    case 3:
        l.green(on);
        break;
    case 4:
        l.blue(on);
        break;
    }
}

bool rkButtonIsPressed(uint8_t id, bool waitForRelease) {
    if (id == 0) {
        ESP_LOGE(TAG, "%s: invalid id %d, buttons are indexed from 1, just like on the board (SW1, SW2...)!", __func__, id);
        return false;
    } else if (id > 3) {
        ESP_LOGE(TAG, "%s: maximum button id is 3, you are using %d!", __func__, id);
        return false;
    }

    int pin = SW1 + (id - 1);
    auto& exp = Manager::get().expander();
    for (int i = 0; i < 3; ++i) {
        const bool pressed = exp.digitalRead(pin) == 0;
        if (!pressed)
            return false;
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    if (waitForRelease) {
        rkButtonWaitForRelease(id);
    }
    return true;
}

void rkButtonWaitForRelease(uint8_t id) {
    if (id == 0) {
        ESP_LOGE(TAG, "%s: invalid id %d, buttons are indexed from 1, just like on the board (SW1, SW2...)!", __func__, id);
        return;
    } else if (id > 3) {
        ESP_LOGE(TAG, "%s: maximum button id is 3, you are using %d!", __func__, id);
        return;
    }

    int pin = SW1 + (id - 1);
    int counter = 0;
    auto& exp = Manager::get().expander();
    while (true) {
        const bool pressed = exp.digitalRead(pin) == 0;
        if (!pressed) {
            if (++counter > 3)
                return;
        } else {
            counter = 0;
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void rkLineCalibrate(float motor_time_coef) {
    auto& man = Manager::get();

    const auto l = gCtx.motors().idLeft();
    const auto r = gCtx.motors().idRight();

    const auto maxleft = man.motor(l).pwmMaxPercent();
    const auto maxright = man.motor(r).pwmMaxPercent();

    constexpr int8_t pwr = 40;

    auto cal = gCtx.line().startCalibration();

    gCtx.motors().set(-pwr, pwr, 100, 100);

    for (int i = 0; i < 30 * motor_time_coef; ++i) {
        cal.record();
        vTaskDelay(pdMS_TO_TICKS(20));
    }

    gCtx.motors().set(pwr, -pwr);
    for (int i = 0; i < 50 * motor_time_coef; ++i) {
        cal.record();
        vTaskDelay(pdMS_TO_TICKS(20));
    }

    gCtx.motors().set(-pwr, pwr);
    for (int i = 0; i < 30 * motor_time_coef; ++i) {
        cal.record();
        vTaskDelay(pdMS_TO_TICKS(20));
    }

    gCtx.motors().set(0, 0, maxleft, maxright);

    cal.save();
    gCtx.saveLineCalibration();
}

void rkLineClearCalibration() {
    LineSensor::CalibrationData cal;
    for (int i = 0; i < Driver::CHANNELS; ++i) {
        cal.min[i] = 0;
        cal.range[i] = Driver::MAX_VAL;
    }
    gCtx.line().setCalibration(cal);
    gCtx.saveLineCalibration();
}

uint16_t rkLineGetSensor(uint8_t sensorId) {
    return gCtx.line().calibratedReadChannel(sensorId);
}

float rkLineGetPosition(bool white_line, uint8_t line_threshold_pct) {
    return gCtx.line().readLine(white_line, float(line_threshold_pct) / 100.f);
}
