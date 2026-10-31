#include <FlowmeterComms.hpp>

FlowmeterComms::FlowmeterComms(
    int gpioPin,
    core::ThreadSafeDeque<Pulse>& queue)
    : flowmeterPin(gpioPin), pulseQueue(queue)
{
}

void FlowmeterComms::start() {
    if (flowmeterThread.joinable()) {
        return;
    }
    running = true;
    flowmeterThread = std::thread(&FlowmeterComms::readPulses, this);


}

void FlowmeterComms::stop() {
    running = false;

    if (flowmeterThread.joinable()) {
        flowmeterThread.join();
    }
}

FlowmeterComms::~FlowmeterComms() {
    stop();
}

void FlowmeterComms::readPulses() {
    int gpioChipFileDescriptor =
        open("/dev/gpiochip0", O_RDONLY);

    if (gpioChipFileDescriptor < 0) {
        std::cerr << "Failed to open GPIO controller\n";
        return;
    }

    gpio_v2_line_request lineRequest{};
    lineRequest.offsets[0] = flowmeterPin;
    lineRequest.num_lines = 1;
    lineRequest.config.flags = GPIO_V2_LINE_FLAG_INPUT | GPIO_V2_LINE_FLAG_EDGE_RISING;

    int requestResult = ioctl(gpioChipFileDescriptor, GPIO_V2_GET_LINE_IOCTL, &lineRequest);
    
    if (requestResult < 0) {
        std::cerr << "Failed to request GPIO line\n";
        close(gpioChipFileDescriptor);
        return;
    }

    int gpioLineFileDescriptor = lineRequest.fd;
    pollfd gpioEventMonitor{};
    gpioEventMonitor.fd = gpioLineFileDescriptor;
    gpioEventMonitor.events = POLLIN;

    while (running) {
        int waitResult = poll(&gpioEventMonitor, 1, 400);

        if (waitResult == 0) {
            continue;
        }

        if (waitResult < 0) {
            std::cerr << "Error while polling GPIO line\n";
            break;
        }

        
        if (gpioEventMonitor.revents & (POLLERR | POLLHUP | POLLNVAL)) {
            std::cerr << "GPIO polling reported an error\n";
            break;
        }   

        if (!(gpioEventMonitor.revents & POLLIN)) {
            continue;
        }

        gpio_v2_line_event pulseEvent{};
        ssize_t readResult = read(gpioLineFileDescriptor, &pulseEvent, sizeof(pulseEvent));

        if (readResult != sizeof(pulseEvent)) {
            std::cerr << "Error while reading GPIO event\n";
            break;
        }

        if (pulseEvent.id == GPIO_V2_LINE_EVENT_RISING_EDGE) {
            Pulse pulse;
            pulse.timestamp_ns = pulseEvent.timestamp_ns;
            {
                std::lock_guard<std::mutex> lock(pulseQueue.mtx);
                pulseQueue.deque.push_back(pulse);

            }
            pulseQueue.cv.notify_one();
        }

    }
    close(gpioLineFileDescriptor);
    close(gpioChipFileDescriptor);
}