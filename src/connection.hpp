#pragma once
#include "device_data.hpp"
#include <atomic>
#include <cstdio>
#include <iostream>
#include <map>
#include <memory>
#include <vector>

struct piped_file_closer {
    void operator()(std::FILE* fp) const
    {
        std::cout << "Deleting pipe\n";
        pclose(fp);
    }
};

// custom unique_ptr for piped FILE*
using unique_PIPE_handle = std::unique_ptr<std::FILE, piped_file_closer>;

class SuperNoteConnector {
public:
    SuperNoteConnector();

private:
    void initPen();
    void initSuperNoteConnection();
    void initTouchScreen();
    void initKeyBoard();
    void closeListener();
    void runPen();
    void runTouch();
    void sendToPen(uint16_t type, uint16_t code, int32_t value);
    void sendToTouch(uint16_t type, uint16_t code, int32_t value);
    void sendToKeyboard(uint16_t type, uint16_t code, int32_t value);
    void pressKeyComb(std::vector<uint16_t> keys, bool value);
    void checkButtons(int x, int y);
    void pressButton();

    unique_PIPE_handle superNotePenConnection;
    unique_PIPE_handle superNoteTouchConnection;
    int touchFD;
    int penFD;
    int keyboardFD;
    int pressedButton;
    bool landscapeMode;
    bool penNear;
    bool buttonPressed;
    // ButtonPressEvent pressedButton;

    std::map<int, std::string> penKeyMap;
    std::map<int, std::string> penCodeMap;
    std::map<int, std::string> touchKeyMap;
    std::map<int, std::string> touchCodeMap;
    std::map<std::string, struct input_event> buttonCommands;
};
