#include "RBControl.hpp"

#include "_librk_arm.h"
#include "_librk_context.h"
#include "roboruka.h"

using namespace rb;

#define TAG "roboruka"

namespace rk {

ArmWrapper::ArmWrapper() {
    m_arm = nullptr;
}

ArmWrapper::~ArmWrapper() {
}

void ArmWrapper::setup(const rkConfig& cfg) {
    ArmBuilder builder;
    builder.body(51, 130).armOffset(0, 20);

    auto b0 = builder.bone(0, 110);
    b0.relStops(-95_deg, 0_deg);
    b0.calcServoAng([](Angle absAngle, Angle) -> Angle {
        return Angle::Pi + absAngle + 30_deg;
    });
    b0.calcAbsAng([](Angle servoAng) -> Angle {
        return servoAng - Angle::Pi - 30_deg;
    });

    auto b1 = builder.bone(1, 130);
    b1.relStops(30_deg, 170_deg)
        .absStops(-20_deg, Angle::Pi)
        .baseRelStops(40_deg, 160_deg);
    b1.calcServoAng([](Angle absAngle, Angle) -> Angle {
        absAngle = rb::Arm::clamp(absAngle + Angle::Pi * 1.5);
        return Angle::Pi + absAngle + 25_deg;
    });
    b1.calcAbsAng([](Angle servoAng) -> Angle {
        auto a = servoAng - Angle::Pi - 25_deg;
        return rb::Arm::clamp(a - Angle::Pi * 1.5);
    });

    m_arm = builder.build().release();

    m_bone_trims.resize(sizeof(cfg.arm_bone_trims) / sizeof(float));
    for (size_t i = 0; i < m_bone_trims.size(); ++i) {
        m_bone_trims[i] = Angle::deg(cfg.arm_bone_trims[i]);
    }
}

std::unique_ptr<rbjson::Object> ArmWrapper::getInfo() {
    const auto& def = m_arm->definition();

    auto info = std::make_unique<rbjson::Object>();
    info->set("height", def.body_height);
    info->set("radius", def.body_radius);
    info->set("off_x", def.arm_offset_x);
    info->set("off_y", def.arm_offset_y);

    auto* bones = new rbjson::Array();
    info->set("bones", bones);

    auto& servo = Manager::get().servoBus();
    for (const auto& b : def.bones) {
        auto pos = servo.pos(b.servo_id);
        if (pos.isNaN()) {
            while (bones->size() != 0)
                bones->remove(bones->size() - 1);
            break;
        }

        pos -= m_bone_trims[b.servo_id];

        auto* info_b = new rbjson::Object();
        info_b->set("len", b.length);
        info_b->set("angle", b.calcAbsAng(pos).rad());
        info_b->set("rmin", b.rel_min.rad());
        info_b->set("rmax", b.rel_max.rad());
        info_b->set("amin", b.abs_min.rad());
        info_b->set("amax", b.abs_max.rad());
        info_b->set("bmin", b.base_rel_min.rad());
        info_b->set("bmax", b.base_rel_max.rad());
        bones->push_back(info_b);
    }
    return info;
}

void ArmWrapper::sendInfo() {
    gCtx.prot()->send_mustarrive("arminfo", getInfo().release());
}

bool ArmWrapper::moveTo(double x, double y) {
    if (!m_arm->syncBonesWithServos()) {
        ESP_LOGE(TAG, "failed to syncBonsWithServos, are they connected correctly?");
        return false;
    }

    if (!m_arm->solve(round(x), round(y)))
        return false;

    auto& servos = Manager::get().servoBus();
    for (const auto& b : m_arm->bones()) {
        servos.set(b.def.servo_id, b.servoAng() + m_bone_trims[b.def.servo_id], 200);
    }
    return true;
}

void ArmWrapper::setGrabbing(bool grab) {
    auto& servos = Manager::get().servoBus();
    const auto angle = grab ? 75_deg : 160_deg;
    servos.set(2, angle + m_bone_trims[2], 200.f, 1.f);
}

bool ArmWrapper::isGrabbing() const {
    return Manager::get().servoBus().posOffline(2).deg() < 140;
}

bool ArmWrapper::getCurrentPosition(double& outX, double& outY) const {
    outX = outY = 0;

    if (!m_arm->syncBonesWithServos()) {
        ESP_LOGE(TAG, "failed to syncBonsWithServos, are they connected correctly?");
        return false;
    }

    const auto& end = m_arm->bones().back();
    outX = end.x;
    outY = end.y;
    return true;
}

};
