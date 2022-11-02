#include "_librk_motors.h"
#include "RBControl_manager.hpp"

namespace rk {

Motors::Motors()
    : m_id_left(rb::MotorId::M1)
    , m_id_right(rb::MotorId::M1) {
}

Motors::~Motors() {
}

void Motors::setup(const rkConfig& cfg) {
    m_id_left = (rb::MotorId)(cfg.motor_id_left - 1);
    m_id_right = (rb::MotorId)(cfg.motor_id_right - 1);
    m_polarity_switch_left = cfg.motor_polarity_switch_left;
    m_polarity_switch_right = cfg.motor_polarity_switch_right;

    // Set motor power limits
    rb::Manager::get().setMotors().pwmMaxPercent(m_id_left, cfg.motor_max_power_pct).pwmMaxPercent(m_id_right, cfg.motor_max_power_pct).set();
}

void Motors::set(int8_t left, int8_t right) {
    if (m_polarity_switch_left)
        left = -left;
    if (m_polarity_switch_right)
        right = -right;

    rb::Manager::get().setMotors().power(m_id_left, left).power(m_id_right, right).set();
}

void Motors::set(int8_t left, int8_t right, uint8_t power_left, uint8_t power_right) {
    if (m_polarity_switch_left)
        left = -left;
    if (m_polarity_switch_right)
        right = -right;

    rb::Manager::get().setMotors().pwmMaxPercent(m_id_left, power_left).pwmMaxPercent(m_id_right, power_right).power(m_id_left, left).power(m_id_right, right).set();
}

void Motors::setById(rb::MotorId id, int8_t power) {
    if ((m_polarity_switch_left && id == m_id_left) || (m_polarity_switch_right && id == m_id_right))
        power = - power;

    rb::Manager::get()
        .setMotors()
        .power(id, power)
        .set();
}

void Motors::joystick(int32_t x, int32_t y) {
    x = scale(x);
    y = scale(y);

    int r = ((y - (x / 1.5f)));
    int l = ((y + (x / 1.5f)));

    r = rb::clamp(r, -127, 127);
    l = rb::clamp(l, -127, 127);

    if (r < 0 && l < 0) {
        std::swap(r, l);
    }
    set(l, r);
}

int32_t Motors::scale(int32_t val) {
    if (val == 0)
        return 0;
    const float valf = val - RBPROTOCOL_AXIS_MIN;
    constexpr auto range = RBPROTOCOL_AXIS_MAX - RBPROTOCOL_AXIS_MIN;
    return (valf / range) * 200 - 100;
}

}; // namespacer rk
