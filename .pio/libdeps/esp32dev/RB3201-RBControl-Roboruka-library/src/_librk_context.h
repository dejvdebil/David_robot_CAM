#pragma once

#include <atomic>

#include "RBControl_arm.hpp"

#include "mcp3008_linesensor.h"

#include "_librk_arm.h"
#include "_librk_motors.h"
#include "_librk_wifi.h"

namespace rk {

class Context {
public:
    Context();
    ~Context();

    void setup(const rkConfig& cfg);

    rb::Protocol* prot() const { return m_prot; }
    ArmWrapper& arm() { return m_arm; }
    Motors& motors() { return m_motors; }
    mcp3008::LineSensor& line();

    void saveLineCalibration();

private:
    void handleRbcontrollerMessage(const std::string& cmd, rbjson::Object* pkt);
    bool loadLineCalibration(mcp3008::LineSensor::CalibrationData& data);

    ArmWrapper m_arm;
    Motors m_motors;
    WiFi m_wifi;
    rb::Protocol* m_prot;

    std::atomic<bool> m_initialized;

    std::atomic<bool> m_line_installed;
    mcp3008::Driver::Config m_line_cfg;
    mcp3008::LineSensor m_line;
};

extern Context gCtx;

};
