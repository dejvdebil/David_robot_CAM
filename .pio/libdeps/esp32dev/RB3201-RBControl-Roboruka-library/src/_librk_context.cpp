#include "nvs_flash.h"

#include "roboruka.h"

#include "RBControl.hpp"
#include "rbprotocol.h"
#include "rbwebserver.h"
#include "rbwifi.h"
#include <stdio.h>

#include "_librk_context.h"

using namespace rb;
using namespace mcp3008;

#define TAG "roboruka"

namespace rk {

Context gCtx;

Context::Context() {
    m_prot = nullptr;
}

Context::~Context() {
}

void Context::setup(const rkConfig& cfg) {
    bool expected = false;
    if (!m_initialized.compare_exchange_strong(expected, true)) {
        ESP_LOGE(TAG, "rkSetup was called more than once, this is WRONG!");
        return;
    }

    rb::Timers::deleteFreeRtOsTimerTask();

    // Initialize the robot manager
    auto& man = Manager::get();

    auto man_flags = MAN_NONE;
    if (!cfg.motor_enable_failsafe) {
        man_flags = ManagerInstallFlags(man_flags | MAN_DISABLE_MOTOR_FAILSAFE);
    }

    man.install(man_flags);

    m_arm.setup(cfg);
    m_motors.setup(cfg);

    m_line_cfg.pin_cs = (gpio_num_t)cfg.pins.line_cs;
    m_line_cfg.pin_mosi = (gpio_num_t)cfg.pins.line_mosi;
    m_line_cfg.pin_miso = (gpio_num_t)cfg.pins.line_miso;
    m_line_cfg.pin_sck = (gpio_num_t)cfg.pins.line_sck;

    // Set the battery measurement coeficient
    auto& batt = man.battery();
    batt.setFineTuneCoef(cfg.battery_coefficient);

    // Set-up servos
    auto& servos = man.initSmartServoBus(3, (gpio_num_t)cfg.pins.arm_servos);
    if (!servos.posOffline(2).isNaN())
        servos.setAutoStop(2);
    servos.limit(0, 0_deg, 220_deg);
    servos.limit(1, 85_deg, 210_deg);
    servos.limit(2, 75_deg, 160_deg);

    m_wifi.init(cfg);

    if (cfg.rbcontroller_app_enable) {
        // Start web server with control page (see data/index.html)
        rb_web_start(80);

        // Initialize the communication protocol
        using namespace std::placeholders;
        m_prot = new Protocol(cfg.owner, cfg.name, "Compiled at " __DATE__ " " __TIME__,
            std::bind(&Context::handleRbcontrollerMessage, this, _1, _2));
        m_prot->start();

        UI.begin(m_prot);
    }
}

void Context::handleRbcontrollerMessage(const std::string& cmd, rbjson::Object* pkt) {
    m_wifi.disableBle();

    if (UI.handleRbPacket(cmd, pkt))
        return;
}

LineSensor& Context::line() {
    bool ex = false;
    if (!m_line_installed.compare_exchange_strong(ex, true))
        return m_line;

    auto res = m_line.install(m_line_cfg);
    if (res != ESP_OK) {
        ESP_LOGE(TAG, "failed to install linesensor: %d!", res);
        m_line_installed = false;
        return m_line;
    }

    LineSensor::CalibrationData data;
    if (loadLineCalibration(data)) {
        m_line.setCalibration(data);
    }

    return m_line;
}

bool Context::loadLineCalibration(LineSensor::CalibrationData& data) {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ret = nvs_flash_erase();
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "failed to nvs_flash_erase: %d!", ret);
            return false;
        }
        ret = nvs_flash_init();
    }

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "failed to nvs_flash_init: %d!", ret);
        return false;
    }

    nvs_handle nvs_ns;
    ret = nvs_open("roboruka", NVS_READONLY, &nvs_ns);
    if (ret != ESP_OK) {
        if (ret != ESP_ERR_NVS_NOT_FOUND) {
            ESP_LOGE(TAG, "failed to nvs_open: %d", ret);
        }
        return false;
    }

    size_t size = sizeof(data);
    ret = nvs_get_blob(nvs_ns, "linecal", &data, &size);
    nvs_commit(nvs_ns);
    nvs_close(nvs_ns);

    if (ret == ESP_OK) {
        return true;
    } else if (ret != ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGE(TAG, "failed to nvs_get_blob: %d", ret);
    }
    return false;
}

void Context::saveLineCalibration() {
    const auto& data = m_line.getCalibration();

    nvs_handle nvs_ns;
    esp_err_t ret = nvs_open("roboruka", NVS_READWRITE, &nvs_ns);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "failed to nvs_open: %d", ret);
        return;
    }

    ret = nvs_set_blob(nvs_ns, "linecal", &data, sizeof(data));
    nvs_close(nvs_ns);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "failed to nvs_set_blob: %d", ret);
    }
}

}; // namespace rk
