/**
 * @file roboruka.h
 *
 * Metody v tomto souboru vám dovolují jednoduše obsluhovat Roboruku.
 *
 */

#ifndef _LIBRB_H
#define _LIBRB_H

#include <memory>

#include <fmt/core.h>
#include <fmt/printf.h>

#include <Arduino.h>

#include "gridui.h"
#include "rbprotocol.h"

using namespace gridui;

/**
 * \defgroup general Inicializace
 *
 * Tato sekce je určená k počátečnímu nastavení knihovny Roboruka.
 *
 * @{
 */

/**
 * \brief Nastavení čísel pinů různých periferií.
 *
 * Zde můžete přenastavit piny, pokud máte periferie připojené na desce na jíném pinu.
 */
struct rkPinsConfig {
    rkPinsConfig()
        : arm_servos(32)
        , line_cs(4)
        , line_mosi(14)
        , line_miso(27)
        , line_sck(26) {
    }

    uint8_t arm_servos; //!< Signál pro serva ruky. Výchozí: pin 32

    uint8_t line_cs;
    uint8_t line_mosi;
    uint8_t line_miso;
    uint8_t line_sck;
};

#define RK_DEFAULT_WIFI_AP_PASSWORD "flusflus" //!< Výchozí heslo pro WiFi AP

/**
 * \brief Nastavení SW pro Roboruku
 *
 * Tato struktura obsahuje konfigurační hodnoty pro software Roboruky.
 * Předává se funkci rkSetup(). Ve výchozím stavu má smysluplné hodnoty
 * a není třeba nastavovat všechny, ale jen ty, které chcete změnit.
 */
struct rkConfig {
    rkConfig()
        : rbcontroller_app_enable(false)
        , owner("")
        , name("")
        , battery_coefficient(1.0)
        , wifi_name("")
        , wifi_password("")
        , wifi_default_ap(false)
        , wifi_ap_password(RK_DEFAULT_WIFI_AP_PASSWORD)
        , wifi_ap_channel(1)
        , motor_id_left(2)
        , motor_id_right(1)
        , motor_max_power_pct(60)
        , motor_polarity_switch_left(false)
        , motor_polarity_switch_right(false)
        , motor_enable_failsafe(false)
        , arm_bone_trims { 0, 0, 0 } {
    }

    bool rbcontroller_app_enable; //!< povolit komunikaci s aplikací RBController. Výchozí: `false`

    const char* owner; //!< Jméno vlastníka robota. Podle tohoto jména filtruje RBController roboty. Výchozí: `""`
    const char* name; //!< Jméno robota. Výchozí: ""

    float battery_coefficient; //!< koeficient pro kalibraci měření napětí baterie. Výchozí: `1.0`

    const char* wifi_name; //!< Jméno WiFi sítě, na kterou se připojovat. Výchozí: `""`
    const char* wifi_password; //!< Heslo k WiFi, na kterou se připojit. Výchozí: `""`

    bool wifi_default_ap; //!< Vytvářet WiFi síť místo toho, aby se připojovalo k wifi_name. Výchozí: `false`
    const char* wifi_ap_password; //!< Heslo k vytvořené síti. Výchozí: `"flusflus"`
    uint8_t wifi_ap_channel; //!< Kanál WiFi vytvořené sítě. Výchozí: `1`

    uint8_t motor_id_left; //!< Které M číslo motoru patří levému, podle čísla na desce. Výchozí: `2`
    uint8_t motor_id_right; //!< Které M číslo motoru patří pravému, podle čísla na desce. Výchozí: `1`
    uint8_t motor_max_power_pct; //!< Limit výkonu motoru v procentech od 0 do 100. Výchozí: `60`
    bool motor_polarity_switch_left; //!< Prohození polarity levého motoru. Výchozí: `false`
    bool motor_polarity_switch_right; //!< Prohození polarity pravého motoru. Výchozí: `false`
    bool motor_enable_failsafe; //!< Zastaví motory po 500ms, pokud není zavoláno rkSetMotorPower. Výchozí: `false`

    float arm_bone_trims[3]; //!< Korekce úhlů pro serva v ruce, ve stupních. Pole je indexované stejně, jako serva,
        //!< hodnota z tohoto pole je vždy přičtena k úhlu poslenému do serva.
        //!< Určeno pro korekci nepřesně postavených rukou, kde fyzické postavení ruky
        //!< neodpovídá vypočítanému postavení.

    rkPinsConfig pins; //!< Konfigurace pinů pro periferie, viz rkPinsConfig
};

/**
 * \brief Inicializační funkce Roboruky
 *
 * Tuhle funci MUSÍTE zavolat vždy na začátku vaší funkce setup() v main.cpp.
 * Můžete jí předat nastavení ve formě struktury rkConfig.
 */
void rkSetup(const rkConfig& cfg = rkConfig());

/**@}*/
/**
 * \defgroup motors Motory
 *
 * Metody pro obsluhu motorů.
 * @{
 */

/**
 * \brief Nastavení výkonu motorů.
 *
 * \param left výkon levého motoru od od -100 do 100
 * \param right výkon pravého motoru od od -100 do 100
 */
void rkMotorsSetPower(int8_t left, int8_t right);

/**
 * \brief Nastavení výkonu levého motoru.
 *
 * \param power výkon levého motoru od od -100 do 100
 */
void rkMotorsSetPowerLeft(int8_t power);

/**
 * \brief Nastavení výkonu pravého motoru.
 *
 * \param power výkon levého motoru od od -100 do 100
 */
void rkMotorsSetPowerRight(int8_t power);

/**
 * \brief Nastavení výkonu motoru podle jeho čísla (M1...M8) na desce.
 *
 * \param id číslo motoru od 1 do 8 včetně
 * \param power výkon motoru od od -100 do 100
 */
void rkMotorsSetPowerById(int id, int8_t power);

/**
 * \brief Nastavení motorů podle joysticku.
 *
 * Tato funkce nastaví výkon motorů podle výstupu z joysticku. Očekává dvě
 * hodnoty od -32768 do 32768, posílané například aplikací RBController.
 * Funkce tyto hodnoty převede na výkon a nastaví ho.
 *
 * \param x X hodnota z joysticku.
 * \param y Y hodnota z joysticku.
 */
void rkMotorsJoystick(int32_t x, int32_t y);

/**@}*/
/**
 * \defgroup arm Ruka
 *
 * Metody pro obsluhu samotné ruky.
 * @{
 */

/**
 * \brief Pohnutí rukou na souřadnice X Y.
 *
 * Zadávají se cílové souřadnice pro konec ruky s čelistmi.
 *
 * Souřadnice jsou počítany v milimetrech. Souřadnicový systém má počátek v ose
 * prostředního serva (id 0), doprava od ní se počítá +X, nahoru -Y a dolů +Y.
 * Při pohledu na robota z pravého boku:
 *
 * <pre>
 *                       Y-
 *                       |     /\   <---- Ruka
 *                       |    /  \
 *                       |   /    \
 *                       |  /      \
 *                       | /        C  <-- Cílová pozice, "prsty rukou"
 *                       |/
 * Osa prostředního ---> 0--------------------- X+
 * serva na robotovi.    |
 *                       |
 *                       |
 *                       |
 *                       |
 *                       Y+
 * </pre>
 *
 * Souřadnice nemusíte měřit ručně, jsou vidět v aplikaci RBController nahoře vlevo nad rukou.
 * Vždy je to X a Y nad sebou, zleva první dvojice je pozice kurzoru (červené čárkované kolečko)
 * a druhá je opravdová pozice, na kterou se ruka dostala.
 *
 * Varování: ruka se nedokáže ze všech pozic dostat na všechny ostatní pozice, typicky musí "objet" tělo, aby nenarazila.
 * Pokud pohyb nefunguje, zkuste zadat nějakou pozici, na kterou by se ruka měla dostat odkudkoliv (např. 145;-45)
 * a až pak kýženou cílovou pozici.
 *
 * \param x Cílová souřadnice X
 * \param y Cílová souřadnice Y
 * \return Pokud se podařilo najít řešení a ruka se pohnula, vrátí funkce `true`. Pokud ne, vrátí `false`.
 */
bool rkArmMoveTo(double x, double y);

/**
 * \brief Na kterých souřadnicích je teď konec ruky?
 *
 * Tato funkce má dvě výstupní hodnoty, proto jsou outX a outY výstupní proměnné, i když jsou předávány jako parametry.
 *
 * \param outX Do této proměnné bude zapsána souřadnice X.
 * \param outY Do této proměnné bude zapsána souřadnice Y.
 * \return Vrátí `false` pokud se pozici nepodaří zjistit, jinak `true`.
 */
bool rkArmPosition(double& outX, double& outY);

/**
 * \brief Jsou prsty ruky rozevřené?
 *
 * \return Vrátí `true`, pokud jsou prsty ruky momentálně plně rozevřeny.
 */
bool rkArmIsGrabbing();

/**
 * \brief Otevřít/zavřít prsty ruky.
 *
 * \param grab Pokud `true`, prsty ruky se sevřou, pokud `false` tak se uvolní.
 */
void rkArmSetGrabbing(bool grab);

/**
 * \brief Pohnout servo na úhel.
 *
 * \param id ID serva, 0 až 2 včetně.
 * \param degrees Úhel ve stupních, na který se má servo pohnout.
 */
void rkArmSetServo(uint8_t id, float degrees);

/**
 * \brief Na jaké pozici je servo?
 *
 * \param id ID serva, 0 až 2 včetně.
 * \return Vrací úhel ve stupních, nebo NaN pokud se úhel nepodaří zjistit. Použijte funkci `isnan` na kontrolu tohoto stavu.
 */
float rkArmGetServo(uint8_t id);

/**
 * \brief Informace o mechanice ruky.
 *
 * Tato funkce vrací JSON objekt který obsahuje informace o rozměrech ruky,
 * limitech jejích kloubů a další. Je určena pro předání informací do webového
 * rozhraní v aplikaci RBControl.
 *
 * \return JSON objekt obsahující informace o ruce.
 */
std::unique_ptr<rbjson::Object> rkArmGetInfo();

/**@}*/
/**
 * \defgroup battery Baterie
 *
 * Metody pro získání informací o stavu baterie.
 * @{
 */

/**
 * \brief Úroveň baterie v procentech
 *
 * \return Hodnota od 0 do 100 udávající nabití baterie.
 */
uint32_t rkBatteryPercent();

/**
 * \brief Úroveň baterie v mV.
 *
 * \return Naměřené napětí na baterii v milivoltech.
 */
uint32_t rkBatteryVoltageMv();

uint32_t rkBatteryRaw();
float rkBatteryCoef();

/**@}*/
/**
 * \defgroup rbcontroller Aplikace RBController
 *
 * Metody pro komunikaci s aplikací RBController pro Android.
 * @{
 */

/**
 * \brief Odeslat text do aplikace.
 *
 * Tato metoda odešle text, který se zobrazí v aplikaci v černém okně nahoře.
 * Příklad:
 *
 *     rkControllerSendLog("Test logu! Stav Baterie: %u mv", rkBatteryVoltageMv());
 *
 * \param format Text k odeslání, může obsahovat formátovací značky jako C funkce printf().
 * \param ... argumenty pro format, funguje stejně jako printf().
 */
void rkControllerSendLog(const char* format, ...);
void rkControllerSendLog(const std::string& text);

void rkControllerSend(const char* cmd, rbjson::Object* data = nullptr);
void rkControllerSendMustArrive(const char* cmd, rbjson::Object* data = nullptr);

/**@}*/
/**
 * \defgroup leds LEDky
 *
 * Metody pro zapínaní a vypínání LEDek na desce.
 * @{
 */

/**
 * \brief Zapnout/vypnout červenou LED
 * \param on `true` pro zapnuto, `false` pro vypnuto.
 */
void rkLedRed(bool on = true);
/**
 * \brief Zapnout/vypnout žlutou LED
 * \param on `true` pro zapnuto, `false` pro vypnuto.
 */
void rkLedYellow(bool on = true);
/**
 * \brief Zapnout/vypnout zelenou LED
 * \param on `true` pro zapnuto, `false` pro vypnuto.
 */
void rkLedGreen(bool on = true);
/**
 * \brief Zapnout/vypnout modrou LED
 * \param on `true` pro zapnuto, `false` pro vypnuto.
 */
void rkLedBlue(bool on = true);

/**
 * \brief Zapnout/vypnout všechny LED zaráz
 * \param on `true` pro zapnuto, `false` pro vypnuto.
 */
void rkLedAll(bool on = true);

/**
 * \brief Zapnout/vypnout LED podle jejich čísla na desce, od 1 do 4 včetně.
 * \param on `true` pro zapnuto, `false` pro vypnuto.
 */
void rkLedById(uint8_t id, bool on = true);

/**@}*/
/**
 * \defgroup buttons Tlačítka
 *
 * Funkce pro vyčítání stavu tlačítek.
 * @{
 */

/**
 * \brief Je teď stisknuto tlačítko?
 *
 * \param id číslo tlačítka jako na desce, od 1 do 3 včetně.
 * \param waitForRelease pokud je stisknuto, počká před vrácením výsledku na jeho uvolnění (default: false)
 * \return Vrátí `true` pokud je tlačítko stisknuto.
 */
bool rkButtonIsPressed(uint8_t id, bool waitForRelease = false);

/**
 * \brief Počkat, dokud není tlačítko uvolněno.
 * 
 * Pokud tlačítko není stisknuté, počká pouze několik desítek ms, tedy nečeká na stisknutí.
 *
 * \param id číslo tlačítka jako na desce, od 1 do 3 včetně.
 */
void rkButtonWaitForRelease(uint8_t id);

/**@}*/

/**@}*/
/**
 * \defgroup line Sledování čáry
 *
 * Funkce pro komunikaci se senzory na čáru.
 * @{
 */

/**
 * \brief Kalibrovat senzory na čáru.
 *
 * Otočí robota doprava a pak doleva a zase zpět tak, aby lišta se senzory
 * prošla celá nad čárou i nad okolím. Trvá to 2.2s, funkce po celou dobu
 * kalibrace čeká a vrátí se až po dokončení.
 *
 * Předpokládá se, že před zavoláním této metody roboto stojí tak, že prostřední senzory
 * jsou nad čárou.
 *
 * Kalibrační hodnoty se ukládají do paměti, kalibraci je třeba dělat pouze když
 * je ruka přesunuta na jiný podklad.
 *
 * \param motor_time_coef tento koeficient změní jak dlouho se roboto otáčí, změňte pokud se vaše ruka
 *                        neotočí tak, že senzory projedou všechny nad čárou.
 */
void rkLineCalibrate(float motor_time_coef = 1.0);

/**
 * \brief Vymazat kalibraci
 *
 * Vymazat nastavenou kalibraci a dále používat nezkalibrované hodnoty.
 */
void rkLineClearCalibration();

/**
 * \brief Hodnota z jednoho senzoru na čáru.
 *
 * \param sensorId číslo senzoru od 0 do 7 včetně
 * \return naměřená hodnota od 0 do 1023
 */
uint16_t rkLineGetSensor(uint8_t sensorId);

/**
 * \brief Pozice čáry pod senzory
 *
 * Tato funkce se pokouší najít černou čáru pod senzory.
 *
 * \param white_line nastavte na true, pokud sledujete bílou čáru na černém podkladu. Výchozí: false
 * \param line_threshold_pct Jak velký rozdíl v procentech musí mezi hodnotami být, aby byla čára považována za nalezenou. Výchozí: 25%
 * \return Desetinná hodnota od -1 do +1. -1 znamená, že čára je úplně vlevo, 0 že je uprostřed a 1 že je úplně vpravo.
 *         Vrátí NaN, pokud nenalezne čáru - výsledek otestujte funkcí isnan() - `isnan(line_position)`
 */
float rkLineGetPosition(bool white_line = false, uint8_t line_threshold_pct = 25);

/**@}*/

#endif // LIBRB_H
