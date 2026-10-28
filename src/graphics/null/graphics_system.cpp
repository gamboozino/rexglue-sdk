/**
 * @file        graphics/null/graphics_system.cpp
 * @brief       Null GPU backend: processes the guest command stream (fences,
 *              interrupts, vblank, swaps) without rendering anything.
 *
 * For titles whose host renderer draws from hooked graphics API calls: the
 * guest's own graphics library keeps running unmodified against a GPU that
 * retires its work at once. Modelled on Xenia's former null backend.
 *
 * @license     BSD 3-Clause License
 */

#include <memory>
#include <string>

#include <rex/graphics/command_processor.h>
#include <rex/graphics/graphics_system.h>

namespace rex::graphics::null {

class NullCommandProcessor : public CommandProcessor {
 public:
  using CommandProcessor::CommandProcessor;

  void IssueSwap(uint32_t, uint32_t, uint32_t) override {}

 protected:
  bool SetupContext() override { return true; }
  void ShutdownContext() override {}

  Shader* LoadShader(xenos::ShaderType, uint32_t, const uint32_t*, uint32_t) override {
    return nullptr;
  }

  bool IssueDraw(xenos::PrimitiveType, uint32_t, IndexBufferInfo*, bool) override { return true; }
  bool IssueCopy() override { return true; }
};

class NullGraphicsSystem : public GraphicsSystem {
 public:
  std::string name() const override { return "null"; }

  // The host renderer owns the window and presents; nothing to set up here.
  X_STATUS SetupPresentation(ui::WindowedAppContext*) override { return X_STATUS_SUCCESS; }

 protected:
  void CreateProvider(bool) override {}

 private:
  std::unique_ptr<CommandProcessor> CreateCommandProcessor() override {
    return std::make_unique<NullCommandProcessor>(this, kernel_state_);
  }
};

system::IGraphicsSystem* CreateNullGraphicsSystem() {
  return new NullGraphicsSystem();
}

}  // namespace rex::graphics::null
