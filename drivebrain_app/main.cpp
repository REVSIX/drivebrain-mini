#include <FoxgloveServer.hpp>
#include <MCAPLogger.hpp>
#include <drivebrain_msgs.pb.h>
#include <atomic>
#include <chrono>
#include <csignal>
#include <thread>
#include <fmt/chrono.h>
#include <spdlog/spdlog.h>
#include <FlowmeterComms.hpp>
#include <deque>
#include <mutex>
#include <cstdint>
#include <memory>

std::atomic<bool> running{true};
std::atomic<bool> isPulseProducerFinished{false};
core::ThreadSafeDeque<Pulse> pulseQueue;
std::atomic<uint64_t> pulseCount{0};

void sig_handler(int signal) {
    if(signal == SIGINT || signal == SIGTERM) {
      spdlog::warn("Interrupted, stopping mini-drivebrain");
      running = false;
    }
}

void processPulses() {
    while (true) {
        std::deque<Pulse> pulsesToProcess;
        {
            std::unique_lock<std::mutex> lock(pulseQueue.mtx);
            pulseQueue.cv.wait(lock, [] {
                return !pulseQueue.deque.empty() || isPulseProducerFinished;
            });

            if (isPulseProducerFinished && pulseQueue.deque.empty()) {
                break;
            }

            pulsesToProcess.swap(pulseQueue.deque);
        }
        for (const Pulse& pulse : pulsesToProcess) {
            pulseCount++;
            auto flowmeterMessage = std::make_shared<drivebrain_msgs::FlowmeterData>();
            flowmeterMessage->set_pulse_count(pulseCount.load());
            flowmeterMessage->set_timestamp_ns(pulse.timestamp_ns);
            core::FoxgloveServer::instance().send_live_telem_msg(flowmeterMessage);
            core::MCAPLogger::instance().log_msg(flowmeterMessage);
        }
    }
}

int main() {
  std::signal(SIGINT, sig_handler);
  std::signal(SIGTERM, sig_handler);

  core::MCAPLogger::create(mcap::McapWriterOptions(""));
  core::FoxgloveServer::create();

  core::MCAPLogger::instance().open_new_mcap();
  core::MCAPLogger::instance().init_logging();

  spdlog::info("Constructed logging singletons");

  FlowmeterComms flowmeter(17, pulseQueue);
  std::thread consumerThread(processPulses);  

  auto loop_time = 0.004;
  std::chrono::microseconds loop_time_ms((int) (loop_time * 1000000.0f));
  auto next_tick = std::chrono::steady_clock::now();
  
  flowmeter.start();

  while(running) {
    next_tick += loop_time_ms;
    auto now = std::chrono::steady_clock::now();
    if(now > next_tick) {
      spdlog::warn("Loop overrun by {}", now-next_tick);
      next_tick = now;
    }
    std::this_thread::sleep_until(next_tick);
  }

  flowmeter.stop();

  isPulseProducerFinished = true;
  pulseQueue.cv.notify_all();

  consumerThread.join();

  core::MCAPLogger::instance().destroy();
  core::FoxgloveServer::instance().destroy();
  spdlog::info("Logging singletons safely destroyed");

  return 0;
}