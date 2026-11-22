#pragma once
/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2013 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 *
 * @modified    Tom Clay, 2026 - Adapted for ReXGlue runtime
 */

#include <functional>
#include <memory>
#include <vector>

#include <array>
#include <atomic>

#include <rex/input/device_assignment.h>
#include <rex/input/input.h>
#include <rex/input/input_driver.h>
#include <rex/input/state_merge.h>
#include <rex/system/interfaces/input.h>

namespace rex::ui {
class Window;
}

namespace rex::input {

class InputSystem : public system::IInputSystem {
 public:
  explicit InputSystem(rex::ui::Window* window);
  ~InputSystem() override;

  rex::ui::Window* window() const { return window_; }

  X_STATUS Setup() override;
  void Shutdown() override;

  void AddDriver(std::unique_ptr<InputDriver> driver);
  void AttachWindow(rex::ui::Window* window);
  void SetActiveCallback(std::function<bool()> callback);
  // Whether one of the app's overlays (debug, console, settings) is open.
  void SetOverlayOpenCallback(std::function<bool()> callback) { overlay_open_ = std::move(callback); }
  bool IsOverlayOpen() const { return overlay_open_ && overlay_open_(); }

  /// Replaces any previous assignment. Call before the guest starts polling.
  void SetDeviceAssignment(std::unique_ptr<DeviceAssignment> assignment);

  X_RESULT GetCapabilities(uint32_t user_index, uint32_t flags, X_INPUT_CAPABILITIES* out_caps);
  X_RESULT GetState(uint32_t user_index, X_INPUT_STATE* out_state);
  X_RESULT SetState(uint32_t user_index, X_INPUT_VIBRATION* vibration);
  X_RESULT GetKeystroke(uint32_t user_index, uint32_t flags, X_INPUT_KEYSTROKE* out_keystroke);

  /// What the device last used by the user is (any thread; kUnknown until a
  /// device has been used).
  DeviceKind ActiveKind(uint32_t user_index) const;

 private:
  std::function<bool()> overlay_open_;
  /// Re-enumerates every driver and notifies the assignment when the set
  /// changed.
  void RefreshDevices();
  InputDriver* DriverForDevice(DeviceId id);
  const DeviceInfo* DeviceInfoFor(DeviceId id) const;

  rex::ui::Window* window_ = nullptr;

  std::vector<std::unique_ptr<InputDriver>> drivers_;

  std::unique_ptr<DeviceAssignment> assignment_;
  ActiveDeviceTracker active_devices_;
  std::array<std::atomic<uint8_t>, kMaxGuestUsers> active_kind_{};
  // input_last_used_only: each device's state at its last read, and the
  // device last used by each user (the last to change while off neutral).
  std::vector<std::pair<DeviceId, X_INPUT_GAMEPAD>> last_seen_;
  std::array<DeviceId, kMaxGuestUsers> last_used_{};
  // The keyboard pressed while a controller is in use: 1 pressed, 2 pressed
  // and let go (its next press takes over).
  std::array<uint8_t, kMaxGuestUsers> keyboard_armed_{};

  // Ordered by ordinal. Ordinals are never recycled, so unplugging pad one
  // does not renumber pad two.
  std::vector<DeviceInfo> devices_;
  std::vector<InputDriver*> device_owners_;
};

/// Create a default InputSystem with SDL + NOP drivers.
/// In tool mode, only the NOP driver is added.
std::unique_ptr<InputSystem> CreateDefaultInputSystem(bool tool_mode);

}  // namespace rex::input
