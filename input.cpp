#include <cstdint>
#include <iostream>
#include <linux/input-event-codes.h>
#include <map>
#include <sstream>

//// ADB shell getevent translation

void decode(const std::string& line, std::map<int, std::string>& eventmap,
    std::map<int, std::string>& codemap)
{
    std::istringstream stream(line);
    std::string event_type, event_code, event_value;
    if (stream >> event_type >> event_code >> event_value) {

        int event, code, value;
        std::sscanf(event_type.data(), "%x", &event);
        std::sscanf(event_code.data(), "%x", &code);
        std::sscanf(event_value.data(), "%x", &value);

        if (event == 3) {
            // std::cout << "Event code: " << code << std::endl;
            std::cout << "Event code: " << codemap[code]
                      << " | Event value: " << value << std::endl;
        }
    }
}

int main()
{
    std::map<int, std::string> eventmap;
    eventmap[0x0000] = "SYN";
    eventmap[0x0001] = "KEY";
    eventmap[0x0003] = "ABS";

    std::map<int, std::string> codemap;
    // codemap[0x0000] = "ABS_X"; // 0-20967
    // codemap[0x0001] = "ABS_Y"; // 0-17527 (On A5X!)
    // codemap[0x0018] = "ABS_PRESSURE";
    // codemap[0x0019] = "ABS_DISTANCE";
    // codemap[0x001a] = "ABS_TILT_X";
    // codemap[0x001b] = "ABS_TILT_Y";
    //

    codemap[ABS_MT_SLOT] = "ABS_MT_SLOT";
    codemap[ABS_MT_TOUCH_MAJOR] = "ABS_MT_TOUCH_MAJOR";
    codemap[ABS_MT_POSITION_X] = "ABS_MT_POSITION_X";
    codemap[ABS_MT_POSITION_Y] = "ABS_MT_POSITION_Y";
    codemap[ABS_MT_TRACKING_ID] = "ABS_MT_TRACKING_ID";
    codemap[ABS_MT_PRESSURE] = "ABS_MT_PRESSURE";

    std::cout << 0x0018 << std::endl;

    // codemap[0x0140] = "BTN_DIGI"; // 1 = down, 0 = up
    // codemap[0x0141] = "BTN_TOOL_RUBBER";
    // codemap[0x014a] = "BTN_TOUCH"; // 1 = down, 0 = up
    // codemap[0x014b] = "BTN_STYLUS";
    // codemap[0x014c] = "BTN_STYLUS2";

    // 20967 and 15725 are the limits of the display on the A5X. Other devices may
    // differ.
    int xscale = 20967;
    int yscale = 15725;

    std::cout << "Starting shell\n";
    FILE* fp = popen("adb shell -x getevent /dev/input/event1", "r");
    if (fp == nullptr) {
        std::cerr << "Failed to run adb command. " << std::endl;
        return 1;
    }
    std::cout << "\x1B[2J\x1B[H";
    char buffer[256];

    std::cout << sizeof(buffer) << std::endl;
    // Read each line from the output of the getevent command
    while (fgets(buffer, sizeof(buffer), fp) != nullptr) {
        // Convert the buffer to a string
        std::string line(buffer);
        // line.erase(std::remove(line.begin(), line.end(), '\n'), line.end());
        decode(line, eventmap, codemap);

        // parse_getevent_output(line, device, inputInfo, xscale, yscale);
    }

    // Close the file if something bad happens
    fclose(fp);

    return 0;
}
