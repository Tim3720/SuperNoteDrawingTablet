#include <cstdint>
#include <fcntl.h>
#include <iostream>
#include <linux/uinput.h>
#include <string.h>
#include <unistd.h>

#define DEVICE_NAME "Virtual Stylus"

void send_event(int fd, uint16_t type, uint16_t code, int32_t value)
{
    struct input_event ev;
    memset(&ev, 0, sizeof(ev));
    ev.type = type;
    ev.code = code;
    ev.value = value;
    write(fd, &ev, sizeof(ev));
}

void draw_line(int fd, int start_x, int start_y, int end_x, int end_y,
    int steps)
{
    int dx = (end_x - start_x) / steps;
    int dy = (end_y - start_y) / steps;

    send_event(fd, EV_ABS, ABS_X, start_x);
    send_event(fd, EV_ABS, ABS_Y, start_y);
    // send_event(fd, EV_KEY, BTN_TOOL_PEN, 1);
    send_event(fd, EV_KEY, BTN_TOUCH, 1);
    // send_event(fd, EV_KEY, BTN_STYLUS, 1); // Add stylus button
    send_event(fd, EV_ABS, ABS_PRESSURE, 1023);
    send_event(fd, EV_SYN, SYN_REPORT, 0);

    for (int i = 0; i <= steps; i++) {
        int x = start_x + (dx * i);
        int y = start_y + (dy * i);
        send_event(fd, EV_ABS, ABS_X, x);
        send_event(fd, EV_ABS, ABS_Y, y);
        send_event(fd, EV_ABS, ABS_PRESSURE, 1023);
        send_event(fd, EV_SYN, SYN_REPORT, 0);
        usleep(5000);
    }
    send_event(fd, EV_KEY, BTN_TOUCH, 0);
    send_event(fd, EV_KEY, BTN_STYLUS, 0);
    send_event(fd, EV_ABS, ABS_PRESSURE, 0);
    send_event(fd, EV_SYN, SYN_REPORT, 0);
}

int main()
{
    int fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if (fd < 0) {
        std::cerr << "Error: Cannot open /dev/uinput. Try running with sudo.\n";
        return 1;
    }

    ioctl(fd, UI_SET_EVBIT, EV_KEY);
    ioctl(fd, UI_SET_KEYBIT, BTN_TOUCH);
    ioctl(fd, UI_SET_KEYBIT, BTN_TOOL_PEN);
    ioctl(fd, UI_SET_KEYBIT,
        BTN_STYLUS); // Important: Required for stylus detection

    ioctl(fd, UI_SET_EVBIT, EV_ABS);
    ioctl(fd, UI_SET_ABSBIT, ABS_X);
    ioctl(fd, UI_SET_ABSBIT, ABS_Y);
    ioctl(fd, UI_SET_ABSBIT, ABS_PRESSURE);

    // Set absolute input range and resolution
    struct uinput_abs_setup abs_x = {
        ABS_X, { .minimum = 0, .maximum = 32767, .resolution = 4000 }
    };
    struct uinput_abs_setup abs_y = {
        ABS_Y, { .minimum = 0, .maximum = 32767, .resolution = 4000 }
    };
    struct uinput_abs_setup abs_pressure = { ABS_PRESSURE,
        { .minimum = 0, .maximum = 1023 } };

    ioctl(fd, UI_ABS_SETUP, &abs_x);
    ioctl(fd, UI_ABS_SETUP, &abs_y);
    ioctl(fd, UI_ABS_SETUP, &abs_pressure);

    struct uinput_setup usetup;
    memset(&usetup, 0, sizeof(usetup));
    usetup.id.bustype = BUS_USB;
    usetup.id.vendor = 0x1234;
    usetup.id.product = 0x5678;
    strcpy(usetup.name, DEVICE_NAME);

    ioctl(fd, UI_DEV_SETUP, &usetup);
    ioctl(fd, UI_DEV_CREATE);
    sleep(3);

    // Draw a diagonal line
    draw_line(fd, 10000, 10000, 10000, 30000, 200);
    // draw_line(fd, 10000, 30000, 30000, 30000, 200);
    // draw_line(fd, 30000, 30000, 30000, 10000, 200);
    // draw_line(fd, 30000, 10000, 10000, 10000, 200);

    ioctl(fd, UI_DEV_DESTROY);
    close(fd);
    return 0;
}
