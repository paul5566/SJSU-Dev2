#pragma once

#include "L2_HAL/io/parallel_bus.hpp"
#include "utility/bit.hpp"
#include "utility/log.hpp"

namespace sjsu
{
class ParallelGpio : public sjsu::ParallelBus
{
 public:
  ParallelGpio(sjsu::Gpio * array[], size_t length) : io_(array), width_(length)
  {
  }
  void Initialize()
  {
    bool parallel_gpio_bus_initialized_successfully = true;
    for (size_t i = 0; i < width_; i++)
    {
      if (io_[i] != nullptr)
      {
        LOG_ERROR("Gpio %d of parallel bus is NULL.", i);
      }
    }

    SJ2_ASSERT_FATAL(parallel_gpio_bus_initialized_successfully,
                     "ParallelGpio initialization failed.");
  }

  void SetAsOpenDrain(bool set_as_open_drain)
  {
    for (size_t i = 0; i < width_; i++)
    {
      io_[i]->GetPin().SetAsOpenDrain(set_as_open_drain);
    }
  }

  void Write(uint32_t data) override
  {
    SetDirection(sjsu::Gpio::Direction::kOutput);
    for (size_t i = 0; i < width_; i++)
    {
      io_[i]->Set(static_cast<sjsu::Gpio::State>(bit::Read(data, i)));
    }
  }

  uint32_t Read() override
  {
    uint32_t read_value;
    SetDirection(sjsu::Gpio::Direction::kInput);

    for (size_t i = 0; i < width_; i++)
    {
      read_value |= io_[i]->Read() << i;
    }

    return read_value;
  }
  size_t BusWidth() const override
  {
    return width_;
  }
  void SetDirection(sjsu::Gpio::Direction direction) override
  {
    for (size_t i = 0; i < width_; i++)
    {
      io_[i]->SetDirection(direction);
    }
  }

 private:
  sjsu::Gpio ** io_;
  const size_t width_;
};
}  // namespace sjsu
