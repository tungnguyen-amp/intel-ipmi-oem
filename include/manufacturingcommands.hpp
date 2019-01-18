/*
// Copyright (c) 2018 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
*/

#pragma once

#include <ipmid/api-types.hpp>
#include <ipmid/utils.hpp>
#include <phosphor-logging/log.hpp>
#include <sdbusplus/bus.hpp>
#include <sdbusplus/message.hpp>
#include <sdbusplus/timer.hpp>
#include <variantvisitors.hpp>
#include <vector>

#define FAN_SENSOR_NOT_PRESENT (0 << 0)
#define FAN_SENSOR_PRESENT (1 << 0)
#define FAN_NOT_PRESENT (0 << 1)
#define FAN_PRESENT (1 << 1)

namespace ipmi
{

// TODO: Service names may change. Worth to consider dynamic detection.
static constexpr const char* fanService = "xyz.openbmc_project.FanSensor";
static constexpr const char* gpioService = "xyz.openbmc_project.Gpio";
static constexpr const char* ledServicePrefix =
    "xyz.openbmc_project.LED.Controller.";

static constexpr const char* ledPathPrefix =
    "/xyz/openbmc_project/led/physical/";
static constexpr const char* fanPwmPath =
    "/xyz/openbmc_project/sensors/fan_pwm/Pwm_";
static constexpr const char* fanTachPathPrefix =
    "/xyz/openbmc_project/sensors/fan_tach/Fan_";

static constexpr const char* fanIntf = "xyz.openbmc_project.Sensor.Value";
static constexpr const char* gpioIntf = "xyz.openbmc_project.Control.Gpio";
static constexpr const char* ledIntf = "xyz.openbmc_project.Led.Physical";

static constexpr const char* busPropertyIntf =
    "org.freedesktop.DBus.Properties";
static constexpr const char* ledStateStr =
    "xyz.openbmc_project.Led.Physical.Action."; // Comes with postfix Off/On
static constexpr const char* smGetSignalPathPrefix =
    "/xyz/openbmc_project/control/gpio/";

/** @enum MtmLvl
.*
 *  Manufacturing command access levels
 */
enum class MtmLvl
{
    mtmNotRunning = 0x00,
    mtmExpired = 0x01,
    mtmAvailable = 0x02,
};

enum class SmActionGet : uint8_t
{
    sample = 0,
    ignore = 1,
    revert = 2
};

enum class SmActionSet : uint8_t
{
    forceDeasserted = 0,
    forceAsserted = 1,
    revert = 2
};

enum class SmSignalGet : uint8_t
{
    smPowerButton = 0,
    smResetButton = 1,
    smSleepButton,
    smNmiButton = 3,
    smChassisIntrusion = 4,
    smPowerGood,
    smPowerRequestGet,
    smSleepRequestGet,
    smFrbTimerHaltGet,
    smForceUpdate,
    smRingIndication,
    smCarrierDetect,
    smIdentifyButton = 0xc,
    smFanPwmGet = 0xd,
    smSignalReserved,
    smFanTachometerGet = 0xf,
    smNcsiDiag = 0x10,
    smFpLcpLeftButton = 0x11,
    smFpLcpRightButton,
    smFpLcpEnterButton,
    smGetSignalMax
};

enum class SmSignalSet : uint8_t
{
    smPowerLed = 0,
    smPowerFaultLed,
    smClusterLed,
    smDiskFaultLed,
    smCoolingFaultLed,
    smFanPowerSpeed = 5,
    smPowerRequestSet,
    smSleepRequestSet,
    smAcpiSci,
    smSpeaker,
    smFanPackFaultLed,
    smCpuFailLed,
    smDimmFailLed,
    smIdentifyLed,
    smHddLed,
    smSystemReadyLed,
    smLcdBacklight = 0x10,
    smSetSignalMax
};

struct SetSmSignalReq
{
    SmSignalSet Signal;
    uint8_t Instance;
    SmActionSet Action;
    uint8_t Value;
};

struct GetSmSignalReq
{
    SmSignalGet Signal;
    uint8_t Instance;
    SmActionGet Action;
};

struct GetSmSignalRsp
{
    uint8_t SigVal;
    uint8_t SigVal1;
    uint8_t SigVal2;
};

class LedProperty
{
    SmSignalSet signal;
    std::string name;
    std::string prevState;
    std::string currentState;
    bool isLocked;

  public:
    LedProperty(SmSignalSet signal_, std::string name_) :
        signal(signal_), name(name_), prevState(""), isLocked(false)
    {
    }

    LedProperty() = delete;

    SmSignalSet getSignal() const
    {
        return signal;
    }

    void setLock(const bool& lock)
    {
        isLocked = lock;
    }
    void setPrevState(const std::string& state)
    {
        prevState = state;
    }
    void setCurrState(const std::string& state)
    {
        currentState = state;
    }
    std::string getCurrState() const
    {
        return currentState;
    }
    bool getLock() const
    {
        return isLocked;
    }
    std::string getPrevState() const
    {
        return prevState;
    }
    std::string getName() const
    {
        return name;
    }
};

/** @class Manufacturing
 *
 *  @brief Implemet commands to support Manufacturing Test Mode.
 */
class Manufacturing
{
    std::string path;
    std::string gpioPaths[(uint8_t)SmSignalGet::smGetSignalMax];
    std::vector<LedProperty> ledPropertyList;
    void initData();

  public:
    Manufacturing();

    ipmi_return_codes detectAccessLvl(ipmi_request_t request,
                                      ipmi_data_len_t req_len);

    LedProperty* findLedProperty(const SmSignalSet& signal)
    {
        auto it = std::find_if(ledPropertyList.begin(), ledPropertyList.end(),
                               [&signal](const LedProperty& led) {
                                   return led.getSignal() == signal;
                               });
        if (it != ledPropertyList.end())
        {
            return &(*it);
        }
        return nullptr;
    }

    std::string getLedPropertyName(const SmSignalSet signal)
    {
        LedProperty* led = findLedProperty(signal);
        if (led != nullptr)
        {
            return led->getName();
        }
        else
        {
            return "";
        }
    }

    int8_t getProperty(const char* service, std::string path,
                       const char* interface, std::string propertyName,
                       ipmi::Value* value);
    int8_t setProperty(const char* service, std::string path,
                       const char* interface, std::string propertyName,
                       ipmi::Value value);
    int8_t disablePidControlService(const bool disable);

    void revertTimerHandler();

    std::tuple<uint8_t, ipmi_ret_t, uint8_t> proccessSignal(SmSignalGet signal,
                                                            SmActionGet action);

    std::string getGpioPathForSmSignal(uint8_t gpioInstane)
    {
        return smGetSignalPathPrefix + gpioPaths[gpioInstane];
    }

    MtmLvl getAccessLvl(void)
    {
        static MtmLvl mtmMode = MtmLvl::mtmNotRunning;
        if (mtmMode != MtmLvl::mtmExpired)
        {
            ipmi::Value mode;
            if (getProperty("xyz.openbmc_project.SpeciaMode",
                            "/xyz/openbmc_project/security/specialMode",
                            "xyz.openbmc_project.Security.SpecialMode",
                            "SpecialMode", &mode) != 0)
            {
                mtmMode = MtmLvl::mtmExpired;
                return mtmMode;
            }
            mtmMode = static_cast<MtmLvl>(std::get<std::uint8_t>(mode));
        }
        return mtmMode;
    }

    std::vector<SmSignalGet> revertSmSignalGetVector;
    bool revertFanPWM = false;
    bool revertLedCallback = false;
    phosphor::Timer revertTimer;
};

} // namespace ipmi