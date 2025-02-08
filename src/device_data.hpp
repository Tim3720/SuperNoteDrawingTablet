#pragma once

#include <linux/uinput.h>
#include <map>
#include <string>

#define PEN_DEVICE_NAME "Virtual SuperNote Stylus"
#define TOUCH_SCREEN_DEVICE_NAME "Virtual SuperNote Touchscreen"

#define PEN_ABS_X_MAX 20967
#define PEN_ABS_Y_MAX 15725
#define PEN_X_UPPER_BOUND 18000
#define PEN_X_LOWER_BOUND 500
#define PEN_Y_UPPER_BOUND 13000
#define PEN_Y_LOWER_BOUND 0

struct ButtonPressEvent {
    struct input_event ev;
    int FD;
    bool pressed;
    bool continuous;
};

void getPenData(std::map<int, std::string>& keyMap, std::map<int, std::string>& codeMap);
void getTouchData(std::map<int, std::string>& keyMap, std::map<int, std::string>& codeMap);
