#include "roboruka.h"
#include <Arduino.h>

//70cm rovne,
// pak otocit o 90 stupnu doprava o polomeru max 25cm -> min 10cm, pak kousek (50 - r)
//        znovu                                                        kousek muze byt klidne delsi
//        znovu           doleva
//70cm rovne
int scaleSpeedToDist = 66;

void driveForward(int distance,int speed){
    rkMotorsSetPower(-speed,-speed);        //scalenout aby to bylo fakt rovne
    delay(distance*scaleSpeedToDist);
    rkMotorsSetPower(0,0);
}

void driveS(){   
    driveForward(70,100);
    delay(2000);
    
    //2200 daley je 360+90 pri 100 Power a 40 limit Power  450
    //440 je 45
    //3000 s je asi 45cm

    //otoci se o 90 stupnu (snad)
    rkMotorsSetPower(-100, 100);
    delay(750);
    rkMotorsSetPower(0,0);

    delay(2000);
    driveForward(25, 100);
    delay(2000);

    //otoci se o 90 stupnu (snad)
    rkMotorsSetPower(-100, 100);
    delay(750);
    rkMotorsSetPower(0,0);

    delay(2000);
    driveForward(70,100);
    delay(2000);

    rkMotorsSetPower(100, -100);
    delay(750);
    rkMotorsSetPower(0,0);

    delay(2000);
    driveForward(25, 100);
    delay(2000);

    rkMotorsSetPower(100, -100);
    delay(750);
    rkMotorsSetPower(0,0);
}

//TODO urcit jak moc zataci robot v pravo, aby jel rovne...

//TODO udelat aby jel o urcitem polomeru (optional)

//TODO senzory

void setup() {
    rkConfig cfg;
    cfg.motor_max_power_pct = 40; // limit výkonu motorů na 30%
    rkSetup(cfg);

    rkLedBlue();
    delay(3000);    
    rkLedRed(true);

    //driveS();

    delay(3000);
    rkLedRed(false);
}