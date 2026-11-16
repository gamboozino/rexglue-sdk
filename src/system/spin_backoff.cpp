/**
 * The back-off of the guest's busy-waits (rex/ppc/intrinsics.h): db16cyc, and
 * the head of each loop the recompiler finds only reading memory until
 * another thread changes it (REX_SPIN_WAIT). After IzunaRecomp v0.2.2.1.
 */
#include <rex/ppc/intrinsics.h>

#include <chrono>
#include <thread>

namespace rex::ppc {

void SpinBackoff() noexcept {
#if defined(__x86_64__) || defined(_M_X64)
  __builtin_ia32_pause();
#elif defined(__aarch64__) || defined(_M_ARM64)
  __asm__ __volatile__("yield");
#endif
  // A pause for the first 50 us of a wait (a handoff between threads answers
  // at once), a yield each time after. A call more than 200 us after the
  // thread's last one starts a new wait.
  static thread_local std::chrono::steady_clock::time_point last{}, start{};
  const auto now = std::chrono::steady_clock::now();
  if (now - last > std::chrono::microseconds(200)) {
    start = now;
  }
  last = now;
  if (now - start >= std::chrono::microseconds(50)) {
    std::this_thread::yield();
  }
}

}  // namespace rex::ppc
