#pragma once
#include <linux/gpio.h>
#include <thread>
#include <atomic>
#include "DriverBus.hpp"
#include "Pulse.hpp"
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <chrono>
#include <sys/ioctl.h>
#include <poll.h>

class FlowmeterComms
{
public:
    FlowmeterComms(
        int gpioPin, 
        core::ThreadSafeDeque<Pulse>& queue);
    ~FlowmeterComms();
    void start();
    void stop();

private:
    int flowmeterPin;
    std::thread flowmeterThread;
    std::atomic<bool> running{false};
    core::ThreadSafeDeque<Pulse>& pulseQueue;
    void readPulses();
};