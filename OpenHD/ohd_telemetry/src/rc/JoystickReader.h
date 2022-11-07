//
// Created by consti10 on 22.08.22.
//

#ifndef OPENHD_OPENHD_OHD_TELEMETRY_SRC_RC_JOYSTICKREADER_H_
#define OPENHD_OPENHD_OHD_TELEMETRY_SRC_RC_JOYSTICKREADER_H_

#include <array>
#include <functional>
#include <mutex>
#include <optional>
#include <sstream>
#include <thread>
#include "openhd-spdlog.hpp"

/**
 * The Paradigm of this class is similar to how for example external devices
 * are handled in general in OpenHD: If the user says he wants RC joystick
 * control, try to open the joystick and read data, re-connect if anything goes
 * wrong during run time. This class reads does all the connecting, handle disconnecting
 * and reading values in its own thread - you can query a "state" from any thread at any
 * time though.
 */
class JoystickReader {
 public:
  // Called every time there is new joystick data
  typedef std::function<void(std::array<uint16_t,16> data)> NEW_JOYSTICK_DATA_CB;
  // thread-safe. Fetch new updated joystick values if there is any.
  //std::optional<std::array<uint16_t,16>> get_new_data_if_available();
 public:
  explicit JoystickReader();
  ~JoystickReader();
  static constexpr uint16_t DEFAULT_RC_CHANNELS_VALUE=UINT16_MAX;
  struct CurrChannelValues{
    // See mavlink RC override https://mavlink.io/en/messages/common.html#RC_CHANNELS_OVERRIDE
    std::array<uint16_t,16> values{DEFAULT_RC_CHANNELS_VALUE};
    // Time point when we received the last update to at least one of the channel(s)
    std::chrono::steady_clock::time_point last_update;
    // Weather we think the RC (joystick) is currently connected or not.
    bool considered_connected=false;
  };
  // Get the current "state", thread-safe
  CurrChannelValues get_current_state();
  // For debugging
  static std::string curr_state_to_string(const CurrChannelValues& curr_channel_values);
 private:
  void loop();
  void connect_once_and_read_until_error();
  // Wait up to timeout_ms for an event, and then read as many events as there are available
  // We are only interested in the Joystick events
  void wait_for_events(int timeout_ms);
  int process_event(void* event,std::array<uint16_t,16>& values);
  void reset_curr_values();
  std::unique_ptr<std::thread> m_read_joystick_thread;
  bool terminate=false;
  std::mutex m_curr_values_mutex;
  CurrChannelValues m_curr_values;
  std::shared_ptr<spdlog::logger> m_console;
};

#endif //OPENHD_OPENHD_OHD_TELEMETRY_SRC_RC_JOYSTICKREADER_H_
