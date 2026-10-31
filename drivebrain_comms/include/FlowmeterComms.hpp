#pragma once
#include <linux/gpio.h>
#include <thread>
#include <atomic>
#include "DriverBus.hpp"
#include "Pulse.hpp"

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
    int Flowmeterpin;
    std::thread FlowmeterThread;
    std::atomic<bool> running{false};
    core::ThreadSafeDeque<Pulse>& PulseQueue;
    void readPulses();
};