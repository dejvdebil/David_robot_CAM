#pragma once

#include <stdint.h>

#include "RBControl_pinout.hpp"
#include "roboruka.h"

namespace rk {

class Motors {
public:
    Motors();
    ~Motors();

    void setup(const rkConfig& cfg);

    void set(int8_t left, int8_t right);
    void set(int8_t left, int8_t right, uint8_t power_left, uint8_t power_right);
    void setById(rb::MotorId id, int8_t power);
    void joystick(int32_t x, int32_t y);

    rb::MotorId idLeft() const { return m_id_left; }
    rb::MotorId idRight() const { return m_id_right; }

private:
    Motors(const Motors&) = delete;

    int32_t scale(int32_t val);

    rb::MotorId m_id_left;
    rb::MotorId m_id_right;
    bool m_polarity_switch_left;
    bool m_polarity_switch_right;
};

}; // namespace rk
