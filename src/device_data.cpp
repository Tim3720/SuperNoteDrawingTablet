#include "device_data.hpp"

void getPenData(std::map<int, std::string>& keyMap, std::map<int, std::string>& codeMap)
{
    keyMap[BTN_TOUCH] = "BTN_TOUCH";
    keyMap[BTN_TOOL_PEN] = "BTN_TOOL_PEN";
    keyMap[BTN_TOOL_RUBBER] = "BTN_TOOL_RUBBER";
    keyMap[BTN_STYLUS] = "BTN_STYLUS";
    keyMap[BTN_STYLUS2] = "BTN_STYLUS2";
    keyMap[BTN_DIGI] = "BTN_DIGI";

    codeMap[ABS_X] = "ABS_X"; // 0-20967
    codeMap[ABS_Y] = "ABS_Y"; // 0-17527 (On A5X!)
    codeMap[ABS_PRESSURE] = "ABS_PRESSURE";
    codeMap[ABS_DISTANCE] = "ABS_DISTANCE";
    codeMap[ABS_TILT_X] = "ABS_TILT_X";
    codeMap[ABS_TILT_Y] = "ABS_TILT_Y";
}

void getTouchData(std::map<int, std::string>& keyMap, std::map<int, std::string>& codeMap)
{
    keyMap[KEY_W] = "KEY_W";
    keyMap[KEY_S] = "KEY_S";
    keyMap[KEY_V] = "KEY_V";
    keyMap[KEY_E] = "KEY_E";
    keyMap[KEY_L] = "KEY_L";
    keyMap[KEY_M] = "KEY_M";
    keyMap[KEY_U] = "KEY_U";
    keyMap[KEY_Z] = "KEY_Z";
    keyMap[KEY_O] = "KEY_O";
    keyMap[KEY_C] = "KEY_C";
    keyMap[KEY_RIGHT] = "KEY_RIGHT";
    keyMap[KEY_LEFT] = "KEY_LEFT";
    keyMap[KEY_DOWN] = "KEY_DOWN";
    keyMap[KEY_UP] = "KEY_UP";
    keyMap[KEY_POWER] = "KEY_POWER";
    keyMap[BTN_TOUCH] = "BTN_TOUCH";

    codeMap[ABS_MT_SLOT] = "ABS_MT_SLOT";
    codeMap[ABS_MT_TOUCH_MAJOR] = "ABS_MT_TOUCH_MAJOR";
    codeMap[ABS_MT_POSITION_X] = "ABS_MT_POSITION_X";
    codeMap[ABS_MT_POSITION_Y] = "ABS_MT_POSITION_Y";
    codeMap[ABS_MT_TRACKING_ID] = "ABS_MT_TRACKING_ID";
    codeMap[ABS_MT_PRESSURE] = "ABS_MT_PRESSURE";
}
