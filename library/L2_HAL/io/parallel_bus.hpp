#pragma once

#include <cstdint>

#include "L1_Peripheral/gpio.hpp"

namespace sjsu
{
class ParallelBus
{
  // ===========================================================================
  // Interface Methods
  // ===========================================================================
  virtual void Initialize()                                  = 0;
  virtual void Write(uint32_t data)                          = 0;
  virtual uint32_t Read()                                    = 0;
  virtual size_t BusWidth() const                            = 0;
  virtual void SetDirection(sjsu::Gpio::Direction direction) = 0;
  // ===========================================================================
  // Utility Methods
  // ===========================================================================
  void SetAsOutput(sjsu::Gpio::State state)
  {
    SetDirection(sjsu::Gpio::Direction::kOutput);
  }
  void SetAsInput(sjsu::Gpio::State state)
  {
    SetDirection(sjsu::Gpio::Direction::kInput);
  }
};
}  // namespace sjsu
