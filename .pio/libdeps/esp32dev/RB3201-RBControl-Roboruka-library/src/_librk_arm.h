#pragma once

#include <memory>

#include "roboruka.h"

#include "RBControl_arm.hpp"
#include "rbprotocol.h"

namespace rk {

class ArmWrapper {
public:
    ArmWrapper();
    ~ArmWrapper();

    void setup(const rkConfig& cfg);

    std::unique_ptr<rbjson::Object> getInfo();
    void sendInfo();
    bool moveTo(double x, double y);
    void setGrabbing(bool grab);
    bool isGrabbing() const;

    bool getCurrentPosition(double& outX, double& outY) const;

private:
    ArmWrapper(const ArmWrapper&) = delete;

    rb::Arm* m_arm;
    std::vector<rb::Angle> m_bone_trims;
};

}; // namespace rk
