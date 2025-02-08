#include "connection.hpp"
#include <cmath>
#include <cstdio>
#include <fcntl.h>
#include <iostream>
#include <linux/uinput.h>
#include <sstream>
#include <string.h>
#include <thread>
#include <unistd.h>

// TODO: Button positions for landscape mode, hand detection via tracking id??

SuperNoteConnector::SuperNoteConnector()
{
    getPenData(penKeyMap, penCodeMap);
    getTouchData(touchKeyMap, touchCodeMap);

    landscapeMode = true;
    penNear = false;
    pressedButton = 0;
    buttonPressed = false;

    std::cout << "Connecting to SuperNote...\n";
    initSuperNoteConnection();
    std::cout << "Successfully connected to SuperNote\n";

    std::cout << "Initializing pen...\n";
    initPen();
    std::cout << "Successfully initialized pen\n";

    std::cout << "Initializing touch screen...\n";
    initTouchScreen();
    std::cout << "Successfully initialized touch screen\n";

    std::cout << "Initializing keyboard...\n";
    initKeyBoard();
    std::cout << "Successfully initialized keyboard\n";

    std::thread tPen(&SuperNoteConnector::runPen, this);
    std::thread tTouch(&SuperNoteConnector::runTouch, this);

    tPen.join();
    tTouch.join();
    std::cout << "Quitting...\n";
    superNotePenConnection.reset(nullptr);
    ioctl(touchFD, UI_DEV_DESTROY);
    close(touchFD);
    ioctl(penFD, UI_DEV_DESTROY);
    close(penFD);
}

void SuperNoteConnector::runPen()
{
    char penBuffer[256];
    char touchBuffer[256];
    std::string penLine;
    std::string penKey, penCode, penValue;
    std::string touchLine;
    std::string touchKey, touchCode, touchValue;
    int event, code, value;
    std::istringstream stream;
    while (fgets(penBuffer, sizeof(penBuffer), superNotePenConnection.get()) != nullptr) {
        penLine = std::string(penBuffer);
        stream = std::istringstream(penLine);
        if (stream >> penKey >> penCode >> penValue) {
            std::sscanf(penKey.data(), "%x", &event);
            std::sscanf(penCode.data(), "%x", &code);
            std::sscanf(penValue.data(), "%x", &value);

            // if (event == EV_SYN) {
            //     std::cout << "#################################\n";
            // }
            // if (event == EV_KEY)
            //     std::cout << keyMap[code] << std::endl;
            // if (event == EV_ABS)
            //     std::cout << codeMap[code] << "  " << value << std::endl;

            if (event == EV_SYN) {
                pressButton();
            } else if (event == EV_ABS) {
                if (code == ABS_X) {
                    // remap value to preset bounds
                    // value = int(PEN_X_LOWER_BOUND + (PEN_X_UPPER_BOUND - PEN_X_LOWER_BOUND) * float(value) / PEN_ABS_X_MAX);
                    // when not using landscape mode, the x and y values need to be switched
                    if (!landscapeMode) {
                        code = ABS_Y;
                        value = int(round(PEN_ABS_Y_MAX * (1 - float(value) / PEN_ABS_X_MAX)));
                    }
                } else if (code == ABS_Y) {
                    // value = int(PEN_Y_LOWER_BOUND + (PEN_Y_UPPER_BOUND - PEN_Y_LOWER_BOUND) * float(value) / PEN_ABS_Y_MAX);
                    if (!landscapeMode) {
                        code = ABS_X;
                        value = int(round(PEN_ABS_X_MAX * (float(value) / PEN_ABS_Y_MAX)));
                    }
                }
            }
            if (event == EV_KEY && code == BTN_DIGI) {
                penNear = value;
            }
            sendToPen(event, code, value);
        }
    }
}

void SuperNoteConnector::runTouch()
{
    char touchBuffer[256];
    std::string touchLine;
    std::string touchKey, touchCode, touchValue;
    int event, code, value;
    std::istringstream stream;
    int x, y;
    while (fgets(touchBuffer, sizeof(touchBuffer), superNoteTouchConnection.get()) != nullptr) {
        touchLine = std::string(touchBuffer);
        stream = std::istringstream(touchLine);
        if (stream >> touchKey >> touchCode >> touchValue) {
            std::sscanf(touchKey.data(), "%x", &event);
            std::sscanf(touchCode.data(), "%x", &code);
            std::sscanf(touchValue.data(), "%x", &value);
            if (event == EV_SYN) {
                std::cout << "#################################\n";
            }
            if (event == EV_KEY)
                std::cout << touchKeyMap[code] << std::endl;
            if (event == EV_ABS)
                std::cout << touchCodeMap[code] << "  " << value << std::endl;
            if (event == EV_ABS) {
                if (code == ABS_MT_POSITION_X) {
                    x = value;
                } else if (code == ABS_MT_POSITION_Y) {
                    y = value;
                    checkButtons(x, y);
                }
            }
            // if (!penNear)
            //     sendToTouch(event, code, value);
            // else
            //     sendToPen(EV_KEY, BTN_TOOL_PEN, 1);
        }
    }
}

void SuperNoteConnector::pressButton()
{
    switch (pressedButton) {
    case 0: // default button
        if (buttonPressed) {
            pressKeyComb({ KEY_LEFTSHIFT, KEY_LEFTCTRL, KEY_D }, 1);
            pressKeyComb({ KEY_LEFTSHIFT, KEY_LEFTCTRL, KEY_D }, 0);
            buttonPressed = false;
        }
        break;
    case 1: // pen2
        sendToPen(EV_KEY, BTN_STYLUS, 0);
        sendToPen(EV_KEY, BTN_STYLUS, 1);
        break;
    case 2: // marker
        sendToPen(EV_KEY, BTN_STYLUS2, 0);
        sendToPen(EV_KEY, BTN_STYLUS2, 1);
        break;
    case 3: // ereaser
        if (buttonPressed) {
            pressKeyComb({ KEY_LEFTSHIFT, KEY_LEFTCTRL, KEY_E }, 1);
            pressKeyComb({ KEY_LEFTSHIFT, KEY_LEFTCTRL, KEY_E }, 0);
            buttonPressed = false;
        }
        break;
    case 4: // selection tool
        if (buttonPressed) {
            pressKeyComb({ KEY_LEFTSHIFT, KEY_LEFTCTRL, KEY_G }, 1);
            pressKeyComb({ KEY_LEFTSHIFT, KEY_LEFTCTRL, KEY_G }, 0);
            buttonPressed = false;
        }
        break;
    case 5: // undo
        if (buttonPressed) {
            // fix for german layout y <-> z
            pressKeyComb({ KEY_LEFTCTRL, KEY_Y }, 1);
            pressKeyComb({ KEY_LEFTCTRL, KEY_Y }, 0);
            buttonPressed = false;
        }
        break;
    case 6: // redo
        if (buttonPressed) {
            // fix for german layout y <-> z
            pressKeyComb({ KEY_LEFTCTRL, KEY_Z }, 1);
            pressKeyComb({ KEY_LEFTCTRL, KEY_Z }, 0);
            buttonPressed = false;
        }
        break;
    }

    buttonPressed = false;
}

void SuperNoteConnector::checkButtons(int x, int y)
{
    bool pressed = true;
    // check Pen1-Button:
    if ((landscapeMode && x > 1790 && y > 1120 && y < 1210) || (y > 1320 && x > 200 && x < 280)) {
        pressedButton = 0;
    }
    // check Pen2-Button:
    else if ((landscapeMode && x > 1790 && y > 1030 && y < 1120) || (y > 1320 && x > 280 && x < 380)) {
        pressedButton = 1;
    }
    // check Marker-Button:
    else if ((landscapeMode && x > 1790 && y > 940 && y < 1030) || (y > 1320 && x > 380 && x < 470)) {
        pressedButton = 2;
    }
    // check Ereaser-Button:
    else if ((landscapeMode && x > 1790 && y > 750 && y < 850) || (y > 1320 && x > 560 && x < 650)) {
        pressedButton = 3;
    }
    // check Select-Button:
    else if ((landscapeMode && x > 1790 && y > 560 && y < 650) || (y > 1320 && x > 720 && x < 840)) {
        pressedButton = 4;
    }
    // check undo-Button:
    else if ((landscapeMode && x > 1790 && y > 370 && y < 460) || (y > 1320 && x > 940 && x < 1030)) {
        pressedButton = 5;
    }
    // check redo-Button:
    else if ((landscapeMode && x > 1790 && y > 280 && y < 370) || (y > 1320 && x > 1030 && x < 1120)) {
        pressedButton = 6;
    } else {
        pressed = false;
    }
    if (pressed) {
        buttonPressed = true;
        pressButton();
    }
}

void SuperNoteConnector::sendToPen(uint16_t type, uint16_t code, int32_t value)
{
    struct input_event ev;
    memset(&ev, 0, sizeof(ev));
    ev.type = type;
    ev.code = code;
    ev.value = value;
    write(penFD, &ev, sizeof(ev));
}

void SuperNoteConnector::sendToTouch(uint16_t type, uint16_t code, int32_t value)
{
    struct input_event ev;
    memset(&ev, 0, sizeof(ev));
    ev.type = type;
    ev.code = code;
    ev.value = value;
    write(touchFD, &ev, sizeof(ev));
}

void SuperNoteConnector::sendToKeyboard(uint16_t type, uint16_t code, int32_t value)
{
    struct input_event ev;
    memset(&ev, 0, sizeof(ev));
    ev.type = type;
    ev.code = code;
    ev.value = value;
    write(keyboardFD, &ev, sizeof(ev));

    ev.type = EV_SYN;
    ev.code = SYN_REPORT;
    ev.value = 0;
    write(keyboardFD, &ev, sizeof(ev));
}

void SuperNoteConnector::pressKeyComb(std::vector<uint16_t> keys, bool value)
{
    for (uint16_t key : keys) {
        sendToKeyboard(EV_KEY, key, value);
    }
}

void SuperNoteConnector::initSuperNoteConnection()
{
    auto p = popen("adb shell -x getevent /dev/input/event5", "r");
    if (!p) {
        throw std::runtime_error("Failed to run adb command.");
    }
    sleep(1);
    superNotePenConnection = unique_PIPE_handle { p };

    auto p2 = popen("adb shell -x getevent /dev/input/event1", "r");
    if (!p2) {
        throw std::runtime_error("Failed to run adb command.");
    }
    sleep(1);
    superNoteTouchConnection = unique_PIPE_handle { p2 };
}

void SuperNoteConnector::initPen()
{
    // command to debug virtual devices: libinput list-devices
    penFD = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if (penFD < 0) {
        std::cerr << "Error: Cannot open /dev/uinput. Try running with sudo.\n";
        return;
    }

    ioctl(penFD, UI_SET_EVBIT, EV_KEY);
    ioctl(penFD, UI_SET_KEYBIT, BTN_TOUCH);
    ioctl(penFD, UI_SET_KEYBIT, BTN_TOOL_PEN);
    ioctl(penFD, UI_SET_KEYBIT, BTN_TOOL_RUBBER);
    ioctl(penFD, UI_SET_KEYBIT, BTN_STYLUS);
    ioctl(penFD, UI_SET_KEYBIT, BTN_STYLUS2);
    ioctl(penFD, UI_SET_KEYBIT, BTN_DIGI);

    ioctl(penFD, UI_SET_EVBIT, EV_ABS);
    ioctl(penFD, UI_SET_ABSBIT, ABS_X);
    ioctl(penFD, UI_SET_ABSBIT, ABS_Y);
    ioctl(penFD, UI_SET_ABSBIT, ABS_PRESSURE);
    ioctl(penFD, UI_SET_ABSBIT, ABS_DISTANCE);
    ioctl(penFD, UI_SET_ABSBIT, ABS_TILT_X);
    ioctl(penFD, UI_SET_ABSBIT, ABS_TILT_Y);

    // Set absolute input range and resolution
    struct uinput_abs_setup abs_x = { ABS_X, { .value = 0, .minimum = 0, .maximum = PEN_ABS_X_MAX, .fuzz = 0, .flat = 0, .resolution = 1 } };
    struct uinput_abs_setup abs_y = { ABS_Y, { .value = 0, .minimum = 0, .maximum = PEN_ABS_Y_MAX, .fuzz = 0, .flat = 0, .resolution = 1 } };
    struct uinput_abs_setup abs_pressure = { ABS_PRESSURE, { .value = 0, .minimum = 0, .maximum = 4095, .fuzz = 0, .flat = 0, .resolution = 1 } };
    struct uinput_abs_setup abs_distance = { ABS_DISTANCE, { .value = 0, .minimum = 0, .maximum = 255, .fuzz = 0, .flat = 0, .resolution = 1 } };
    struct uinput_abs_setup abs_tilt_x = { ABS_TILT_X, { .value = 0, .minimum = -9000, .maximum = 9000, .fuzz = 0, .flat = 0, .resolution = 1 } };
    struct uinput_abs_setup abs_tilt_y = { ABS_TILT_Y, { .value = 0, .minimum = -9000, .maximum = 9000, .fuzz = 0, .flat = 0, .resolution = 1 } };

    ioctl(penFD, UI_ABS_SETUP, &abs_x);
    ioctl(penFD, UI_ABS_SETUP, &abs_y);
    ioctl(penFD, UI_ABS_SETUP, &abs_pressure);
    ioctl(penFD, UI_ABS_SETUP, &abs_distance);
    ioctl(penFD, UI_ABS_SETUP, &abs_tilt_x);
    ioctl(penFD, UI_ABS_SETUP, &abs_tilt_y);

    struct uinput_setup usetup;
    memset(&usetup, 0, sizeof(usetup));
    usetup.id.bustype = BUS_USB;
    usetup.id.vendor = 0x1234;
    usetup.id.product = 0x5678;
    strcpy(usetup.name, PEN_DEVICE_NAME);

    ioctl(penFD, UI_DEV_SETUP, &usetup);
    ioctl(penFD, UI_DEV_CREATE);
    sleep(1);
}

void SuperNoteConnector::initTouchScreen()
{
    touchFD = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if (touchFD < 0) {
        std::cerr << "Error: Cannot open /dev/uinput. Try running with sudo.\n";
        return;
    }

    ioctl(touchFD, UI_SET_EVBIT, EV_KEY);
    ioctl(touchFD, UI_SET_KEYBIT, KEY_W);
    ioctl(touchFD, UI_SET_KEYBIT, KEY_S);
    ioctl(touchFD, UI_SET_KEYBIT, KEY_V);
    ioctl(touchFD, UI_SET_KEYBIT, KEY_E);
    ioctl(touchFD, UI_SET_KEYBIT, KEY_L);
    ioctl(touchFD, UI_SET_KEYBIT, KEY_M);
    ioctl(touchFD, UI_SET_KEYBIT, KEY_U);
    ioctl(touchFD, UI_SET_KEYBIT, KEY_Z);
    ioctl(touchFD, UI_SET_KEYBIT, KEY_O);
    ioctl(touchFD, UI_SET_KEYBIT, KEY_C);
    ioctl(touchFD, UI_SET_KEYBIT, KEY_RIGHT);
    ioctl(touchFD, UI_SET_KEYBIT, KEY_LEFT);
    ioctl(touchFD, UI_SET_KEYBIT, KEY_DOWN);
    ioctl(touchFD, UI_SET_KEYBIT, KEY_UP);
    ioctl(touchFD, UI_SET_KEYBIT, KEY_POWER);
    ioctl(touchFD, UI_SET_KEYBIT, BTN_TOUCH);

    ioctl(touchFD, UI_SET_EVBIT, EV_ABS);
    ioctl(touchFD, UI_SET_ABSBIT, ABS_MT_SLOT);
    ioctl(touchFD, UI_SET_ABSBIT, ABS_MT_TOUCH_MAJOR);
    ioctl(touchFD, UI_SET_ABSBIT, ABS_MT_POSITION_X);
    ioctl(touchFD, UI_SET_ABSBIT, ABS_MT_POSITION_Y);
    ioctl(touchFD, UI_SET_ABSBIT, ABS_MT_TRACKING_ID);
    ioctl(touchFD, UI_SET_ABSBIT, ABS_MT_PRESSURE);

    // Set absolute input range and resolution
    struct uinput_abs_setup abs_slot = { ABS_MT_SLOT, { .value = 0, .minimum = 0, .maximum = 9, .fuzz = 0, .flat = 0, .resolution = 1 } };
    struct uinput_abs_setup abs_major = { ABS_MT_TOUCH_MAJOR, { .value = 0, .minimum = 0, .maximum = 255, .fuzz = 0, .flat = 0, .resolution = 1 } };
    struct uinput_abs_setup abs_pos_x = { ABS_MT_POSITION_X, { .value = 0, .minimum = 0, .maximum = 1872, .fuzz = 0, .flat = 0, .resolution = 1 } };
    struct uinput_abs_setup abs_pos_y = { ABS_MT_POSITION_Y, { .value = 0, .minimum = 0, .maximum = 1404, .fuzz = 0, .flat = 0, .resolution = 1 } };
    struct uinput_abs_setup abs_tracking = { ABS_MT_TRACKING_ID, { .value = 0, .minimum = 0, .maximum = 65535, .fuzz = 0, .flat = 0, .resolution = 1 } };
    struct uinput_abs_setup abs_pressure = { ABS_MT_PRESSURE, { .value = 0, .minimum = 0, .maximum = 255, .fuzz = 0, .flat = 0, .resolution = 1 } };

    ioctl(touchFD, UI_ABS_SETUP, &abs_slot);
    ioctl(touchFD, UI_ABS_SETUP, &abs_major);
    ioctl(touchFD, UI_ABS_SETUP, &abs_pos_x);
    ioctl(touchFD, UI_ABS_SETUP, &abs_pos_y);
    ioctl(touchFD, UI_ABS_SETUP, &abs_tracking);
    ioctl(touchFD, UI_ABS_SETUP, &abs_pressure);

    struct uinput_setup usetup;
    memset(&usetup, 0, sizeof(usetup));
    usetup.id.bustype = BUS_USB;
    usetup.id.vendor = 0x1234;
    usetup.id.product = 0x5678;
    strcpy(usetup.name, TOUCH_SCREEN_DEVICE_NAME);

    ioctl(touchFD, UI_DEV_SETUP, &usetup);
    ioctl(touchFD, UI_DEV_CREATE);
    sleep(1);
}

void SuperNoteConnector::initKeyBoard()
{
    keyboardFD = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if (keyboardFD < 0) {
        std::cerr << "Error: Cannot open /dev/uinput. Try running with sudo.\n";
        return;
    }

    ioctl(keyboardFD, UI_SET_EVBIT, EV_KEY);
    ioctl(keyboardFD, UI_SET_KEYBIT, KEY_LEFTCTRL);
    ioctl(keyboardFD, UI_SET_KEYBIT, KEY_LEFTSHIFT);
    ioctl(keyboardFD, UI_SET_KEYBIT, KEY_D);
    ioctl(keyboardFD, UI_SET_KEYBIT, KEY_E);
    ioctl(keyboardFD, UI_SET_KEYBIT, KEY_G);
    ioctl(keyboardFD, UI_SET_KEYBIT, KEY_Z);
    ioctl(keyboardFD, UI_SET_KEYBIT, KEY_Y);

    struct uinput_setup usetup;
    memset(&usetup, 0, sizeof(usetup));
    usetup.id.bustype = BUS_USB;
    usetup.id.vendor = 0x1234;
    usetup.id.product = 0x5678;
    strcpy(usetup.name, "Virtual Keyboard");

    ioctl(keyboardFD, UI_DEV_SETUP, &usetup);
    ioctl(keyboardFD, UI_DEV_CREATE);
    sleep(1);
}
