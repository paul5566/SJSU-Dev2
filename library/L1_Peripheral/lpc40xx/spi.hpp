/// SSP provides the ability for serial communication over SPI, SSI, or
/// Microwire on the LPC407x chipset. NOTE: The SSP2 peripheral is a
/// selectable option in this driver, it is not currently available on
/// the SJTwo board. Only one set of pins are available for either SSP0
/// or SSP1 as follows:
///      Peripheral  |   MISO    |   MOSI    |   SCK
///          SSP0    |   P0.17   |   P0.18   |   P0.15
///          SSP1    |   P0.8    |   P0.9    |   P0.7
/// Order of function calls should be as follows:
///
///     1. Constructor (create object)
///     2. SetClock(...)
///     3. SetPull(...)
///     4. Initialize()
///
/// Note that all register modifications must be made before the SSP
/// is enabled in the CR1 register (see page 612 of user manual UM10562)
/// If changes are desired after the Initialize function is called, the
/// peripheral must be disabled, and then re-enabled after changes are made.

#pragma once

#include "L1_Peripheral/spi.hpp"

#include "L0_Platform/lpc40xx/LPC40xx.h"
#include "L1_Peripheral/lpc40xx/pin.hpp"
#include "L1_Peripheral/lpc40xx/system_controller.hpp"
#include "utility/bit.hpp"
#include "utility/enum.hpp"
#include "utility/status.hpp"

namespace sjsu
{
namespace lpc40xx
{
class Spi final : public sjsu::Spi
{
 public:
  // SSPn Control Register 0
  struct ControlRegister0  // NOLINT
  {
    static constexpr bit::Mask kDataBit     = bit::CreateMaskFromRange(0, 3);
    static constexpr bit::Mask kFrameBit    = bit::CreateMaskFromRange(4, 5);
    static constexpr bit::Mask kPolarityBit = bit::CreateMaskFromRange(6);
    static constexpr bit::Mask kPhaseBit    = bit::CreateMaskFromRange(7);
    static constexpr bit::Mask kDividerBit  = bit::CreateMaskFromRange(8, 15);
  };
  // SSPn Control Register 1
  struct ControlRegister1  // NOLINT
  {
    static constexpr bit::Mask kSpiEnable    = bit::CreateMaskFromRange(1);
    static constexpr bit::Mask kSlaveModeBit = bit::CreateMaskFromRange(2);
  };
  // SSPn Status Register
  struct StatusRegister  // NOLINT
  {
    static constexpr bit::Mask kDataLineIdleBit = bit::CreateMaskFromRange(4);
  };

  // SSP data size for frame packets
  static constexpr uint8_t kDataSizeLUT[] = {
    0b0011,  // 4-bit  transfer
    0b0100,  // 5-bit  transfer
    0b0101,  // 6-bit  transfer
    0b0110,  // 7-bit  transfer
    0b0111,  // 8-bit  transfer
    0b1000,  // 9-bit  transfer
    0b1001,  // 10-bit transfer
    0b1010,  // 11-bit transfer
    0b1011,  // 12-bit transfer
    0b1100,  // 13-bit transfer
    0b1101,  // 14-bit transfer
    0b1110,  // 15-bit transfer
    0b1111,  // 16-bit transfer
  };

  struct Bus_t
  {
    LPC_SSP_TypeDef * registers;
    sjsu::lpc40xx::SystemController::PeripheralID power_on_bit;
    const sjsu::Pin & mosi;
    const sjsu::Pin & miso;
    const sjsu::Pin & sck;
    uint8_t pin_function_id;
  };

  struct Bus  // NOLINT
  {
   private:
    // SSP0 pins
    inline static const sjsu::lpc40xx::Pin kMosi0 =
        sjsu::lpc40xx::Pin::CreatePin<0, 18>();
    inline static const sjsu::lpc40xx::Pin kMiso0 =
        sjsu::lpc40xx::Pin::CreatePin<0, 17>();
    inline static const sjsu::lpc40xx::Pin kSck0 =
        sjsu::lpc40xx::Pin::CreatePin<0, 15>();
    // SSP1 pins
    inline static const sjsu::lpc40xx::Pin kMosi1 =
        sjsu::lpc40xx::Pin::CreatePin<0, 9>();
    inline static const sjsu::lpc40xx::Pin kMiso1 =
        sjsu::lpc40xx::Pin::CreatePin<0, 8>();
    inline static const sjsu::lpc40xx::Pin kSck1 =
        sjsu::lpc40xx::Pin::CreatePin<0, 7>();
    // SSP2 pins
    inline static const sjsu::lpc40xx::Pin kMosi2 =
        sjsu::lpc40xx::Pin::CreatePin<1, 1>();
    inline static const sjsu::lpc40xx::Pin kMiso2 =
        sjsu::lpc40xx::Pin::CreatePin<1, 4>();
    inline static const sjsu::lpc40xx::Pin kSck2 =
        sjsu::lpc40xx::Pin::CreatePin<1, 0>();

   public:
    inline static const Bus_t kSpi0 = {
      .registers       = LPC_SSP0,
      .power_on_bit    = sjsu::lpc40xx::SystemController::Peripherals::kSsp0,
      .mosi            = kMosi0,
      .miso            = kMiso0,
      .sck             = kSck0,
      .pin_function_id = 0b010,
    };
    inline static const Bus_t kSpi1 = {
      .registers       = LPC_SSP1,
      .power_on_bit    = sjsu::lpc40xx::SystemController::Peripherals::kSsp1,
      .mosi            = kMosi1,
      .miso            = kMiso1,
      .sck             = kSck1,
      .pin_function_id = 0b010,
    };
    inline static const Bus_t kSpi2 = {
      .registers       = LPC_SSP2,
      .power_on_bit    = sjsu::lpc40xx::SystemController::Peripherals::kSsp2,
      .mosi            = kMosi2,
      .miso            = kMiso2,
      .sck             = kSck2,
      .pin_function_id = 0b100,
    };
  };

  explicit constexpr Spi(const Bus_t & bus,
                         const sjsu::SystemController & system_controller =
                             DefaultSystemController())
      : bus_(bus), system_controller_(system_controller)
  {
  }

  /// This METHOD MUST BE EXECUTED before any other method can be called.
  /// Powers on the peripheral, activates the SSP pins and enables the SSP
  /// peripheral.
  /// See page 601 of user manual UM10562 LPC408x/407x for more details.
  Status Initialize() const override
  {
    constexpr uint8_t kSpiFormatCode = 0b00;

    // Power up peripheral
    system_controller_.PowerUpPeripheral(bus_.power_on_bit);
    // Enable SSP pins
    bus_.mosi.SetPinFunction(bus_.pin_function_id);
    bus_.miso.SetPinFunction(bus_.pin_function_id);
    bus_.sck.SetPinFunction(bus_.pin_function_id);
    // Set SSP frame format to SPI
    bus_.registers->CR0 = bit::Insert(
        bus_.registers->CR0, kSpiFormatCode, ControlRegister0::kFrameBit);
    // Set SPI to master mode by clearing
    bus_.registers->CR1 = bit::Clear(bus_.registers->CR1,
                                     ControlRegister1::kSlaveModeBit.position);
    // Enable SSP
    bus_.registers->CR1 =
        bit::Set(bus_.registers->CR1, ControlRegister1::kSpiEnable.position);
    return Status::kSuccess;
  }

  /// An easy way to sets up an SPI peripheral as SPI master with default clock
  /// rate at 1Mhz.
  void SetSpiDefault() const
  {
    constexpr bool kPositiveClockOnIdle  = false;
    constexpr bool kReadMisoOnRising     = false;
    constexpr units::frequency::hertz_t kDefaultFrequency = 1_MHz;

    SetDataSize(DataSize::kEight);
    SetClock(kDefaultFrequency, kPositiveClockOnIdle, kReadMisoOnRising);
  }

  /// Checks if the SSP controller is idle.
  /// @returns true if the controller is sending or receiving a data frame and
  /// false if it is idle.
  bool IsBusBusy() const
  {
    return bit::Read(bus_.registers->SR,
                     StatusRegister::kDataLineIdleBit.position);
  }

  /// Transfers a data frame to an external device using the SSP
  /// data register. This functions for both transmitting and
  /// receiving data. It is recommended this region be protected
  /// by a mutex.
  /// @param data - information to be placed in data register
  /// @return - received data from external device
  uint16_t Transfer(uint16_t data) const override
  {
    bus_.registers->DR = data;
    while (IsBusBusy())
    {
      continue;
    }
    return static_cast<uint16_t>(bus_.registers->DR);
  }

  /// Sets the various modes for the Peripheral
  /// @param size - number of bits per frame
  void SetDataSize(DataSize size) const override
  {
    // NOTE: In UM10562 page 611, you will see that DSS (Data Size Select) is
    // equal to the bit transfer minus 1. So we can add 3 to our DataSize enum
    // to get the appropriate tranfer code.
    constexpr uint8_t kBitTransferCodeOffset = 3;
    uint8_t size_code =
        static_cast<uint8_t>(util::Value(size) + kBitTransferCodeOffset);

    bus_.registers->CR0 =
        bit::Insert(bus_.registers->CR0, size_code, ControlRegister0::kDataBit);
  }

  /// Sets the clock rate for the Peripheral
  /// @param positive_clock_on_idle - maintain bus on clock false=low or
  ///        false=high between frames
  /// @param read_miso_on_rising - capture serial data on true=first or
  ///        1=second clock cycle
  /// @param frequency - serial clock rate
  void SetClock(units::frequency::hertz_t frequency,
                bool positive_clock_on_idle = false,
                bool read_miso_on_rising    = false) const override
  {
    bus_.registers->CR0 = bit::Insert(bus_.registers->CR0,
                                      positive_clock_on_idle,
                                      ControlRegister0::kPolarityBit);

    bus_.registers->CR0 = bit::Insert(
        bus_.registers->CR0, read_miso_on_rising, ControlRegister0::kPhaseBit);

    uint16_t prescaler =
        (system_controller_.GetPeripheralFrequency(bus_.power_on_bit) /
         frequency)
            .to<uint16_t>();
    // Store lower half of precalar in clock prescalar register
    bus_.registers->CPSR = prescaler & 0xFF;
    // Store upper 8 bit half of the prescalar in control register 0
    bus_.registers->CR0 = bit::Insert(
        bus_.registers->CR0, prescaler >> 8, ControlRegister0::kDividerBit);
  }

 private:
  const Bus_t & bus_;
  const sjsu::SystemController & system_controller_;
};
}  // namespace lpc40xx
}  // namespace sjsu
