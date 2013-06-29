/**
 * @file Cosa/OWI.hh
 * @version 1.0
 *
 * @section License
 * Copyright (C) 2012-2013, Mikael Patel
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA  02111-1307  USA
 *
 * This file is part of the Arduino Che Cosa project.
 */

#ifndef __COSA_OWI_HH__
#define __COSA_OWI_HH__

#include "Cosa/Types.h"
#include "Cosa/Pins.hh"
#include "Cosa/ExternalInterrupt.hh"
#include "Cosa/IOStream.hh"

/**
 * 1-wire device driver support class. Allows device rom search
 * and connection to multiple devices on one-wire bus. 
 *
 * @section Limitations
 * The driver will turn off interrupt handling during data read 
 * from the device. 
 */
class OWI : private IOPin {
public:
  /**
   * ROM Commands and Size
   */
  enum {
    SEARCH_ROM = 0xF0,
    READ_ROM = 0x33,
    MATCH_ROM = 0x55,
    SKIP_ROM = 0xCC,
    ALARM_SEARCH = 0xEC
  } __attribute__((packed));
  static const uint8_t ROM_MAX = 8;
  static const uint8_t ROMBITS = ROM_MAX * CHARBITS;

  /**
   * Driver for device connected to a one-wire bus.
   */
  class Driver {
    friend class OWI;
    friend IOStream& operator<<(IOStream& outs, OWI& owi);
    friend IOStream& operator<<(IOStream& outs, Driver& dev);
  public:
    enum {
      FIRST = -1,
      ERROR = -1,
      LAST = ROMBITS
    } __attribute__((packed));
  protected:
    uint8_t m_rom[ROM_MAX];
    const uint8_t* ROM;
    Driver* m_next;
    OWI* m_pin;

    /**
     * Search device rom given the last position of discrepancy.
     * Negative value for start from the beginning.
     * @param[in] last position of discrepancy.
     * @return position of difference or negative error code.
     */
    int8_t search(int8_t last = FIRST);

  public:
    /** Name of device driver instance */
    const char* NAME;
    
    /**
     * Construct one wire device driver. Use one wire bus on given pin.
     * @param[in] pin one wire bus.
     * @param[in] name of device driver instance.
     */
    Driver(OWI* pin, const char* name = 0) : 
      ROM(0), 
      NAME(name),
      m_pin(pin) 
    {}

    /**
     * Construct one wire device driver. Use one wire bus on given pin,
     * and given rom identity in EEPROM (or NULL).
     * @param[in] pin one wire bus.
     * @param[in] rom identity.
     */
    Driver(OWI* pin, const uint8_t* rom);

    /**
     * Return pointer to device rom. 
     * @return device rom buffer.
     */
    uint8_t* get_rom() 
    {
      return (m_rom);
    }

    /**
     * Update the rom identity (in EEPROM). Return true(1) if 
     * successful otherwise false(0). Typically used to save
     * configuration after connect().
     * return bool.
     */
    bool update_rom();

    /**
     * Search device rom given the last position of discrepancy.
     * Negative value for start from the beginning.
     * @param[in] last position of discrepancy.
     * @return position of difference or negative error code.
     */
    int8_t search_rom(int8_t last = FIRST);

    /**
     * Read device rom. This can only be used when there is only
     * one slave on the bus.
     * @return true(1) if successful otherwise false(0).
     */
    bool read_rom();

    /**
     * Match device rom. Address the slave device with the
     * rom code. Device specific function command should follow.
     * May be used to verify rom code.
     * @return true(1) if successful otherwise false(0).
     */
    bool match_rom();

    /**
     * Skip device rom for boardcast or single device access.
     * Device specific function command should follow.
     * @return true(1) if successful otherwise false(0).
     */
    bool skip_rom();

    /**
     * Search alarming device given the last position of discrepancy.
     * Negative value for start from the beginning.
     * @param[in] last position of discrepancy.
     * @return position of difference or negative error code.
     */
    int8_t alarm_search(int8_t last = FIRST);

    /**
     * Connect to one-wire device with given family code and index.
     * @param[in] family device family code.
     * @param[in] index device order.
     * @return true(1) if successful otherwise false(0).
     */
    bool connect(uint8_t family, uint8_t index);

    /**
     * @override
     * Callback on alarm dispatch. Default is empty function.
     */
    virtual void on_alarm() {}
  };

  /**
   * Alarm search iterator class.
   */
  class Search : private Driver {
  private:
    uint8_t m_family;
    int8_t m_last;
  public:
    /**
     * Initiate an alarm search iterator for the given one-wire bus and 
     * device family code.
     * @param[in] owi one-wire bus.
     * @param[in] family code (default all).
     */
    Search(OWI* owi, uint8_t family = 0) : 
      Driver(owi),
      m_family(family),
      m_last(FIRST)
    {
    }

    /**
     * Get the next device with an active alarm.
     * @return pointer to driver or null(0).
     */
    Driver* next();

    /**
     * Reset iterator.
     */
    void reset()
    {
      m_last = FIRST;
    }
  };

  /**
   * Act as slave device connected to a one-wire bus.
   */
  class Device : public ExternalInterrupt {
    friend class OWI;
  private:
    // One-wire slave pin mode
    enum Mode {
      OUTPUT_MODE = 0,
      INPUT_MODE = 1
    } __attribute__((packed));

    // One-wire slave states
    enum State {
      IDLE_STATE,
      RESET_STATE,
      PRESENCE_STATE,
      ROM_STATE,
      FUNCTION_STATE
    } __attribute__((packed));

    /**
     * Set slave device pin input/output mode.
     * @param[in] mode pin mode.
     */
    void set_mode(Mode mode)
    {
      synchronized {
	if (mode == OUTPUT_MODE)
	  *DDR() |= m_mask; 
	else
	  *DDR() &= ~m_mask; 
      }
    }

    /**
     * Set slave device pin.
     */
    void set() 
    { 
      synchronized {
	*PORT() |= m_mask; 
      }
    }

    /**
     * Clear slave device pin.
     */
    void clear() 
    { 
      synchronized {
	*PORT() &= ~m_mask; 
      }
    }

    /**
     * Read the given number of bits from the one wire bus (master).
     * Default number of bits is 8. Returns the value read LSB aligned.
     * or negative error code.
     * @param[in] bits to be read.
     * @return value read.
     */
    int read(uint8_t bits = CHARBITS);

    /**
     * Write the given value to the one wire bus. The bits are written
     * from LSB to MSB. Return true(1) if successful otherwise false(0).
     * @param[in] value.
     * @param[in] bits to be written.
     */
    bool write(uint8_t value, uint8_t bits);

    /**
     * @override
     * Slave device event handler function. Handle presence pulse and
     * rom/function command parsing.
     * @param[in] type the type of event.
     * @param[in] value the event value.
     */
    virtual void on_event(uint8_t type, uint16_t value);

    /**
     * @override
     * Slave device interrupt handler function. Detect reset and initiate
     * presence pulse. Push service_request event for further handling.
     * @param[in] arg argument from interrupt service routine.
     */
    virtual void on_interrupt(uint16_t arg = 0);

  protected:
    uint8_t* m_rom;
    volatile uint32_t m_time;
    volatile uint8_t m_crc;
    volatile State m_state;

  public:
    // Slave function codes
    enum {
      STATUS = 0x11
    } __attribute__((packed));

    /**
     * Construct one wire slave device connected to the given pin and
     * rom identity code. Note: crc is generated automatically.
     * @param[in] pin number.
     * @param[in] rom identity number.
     */
    Device(Board::ExternalInterruptPin pin, uint8_t* rom) : 
      ExternalInterrupt(pin, ExternalInterrupt::ON_CHANGE_MODE),
      m_rom(rom),
      m_time(0),
      m_crc(0),
      m_state(IDLE_STATE)
    {
    }
  };
  
private:
  /** Number of devices */
  uint8_t m_devices;

  /** List of slave devices */
  Driver* m_device;

  /** Intermediate CRC sum */
  uint8_t m_crc;

public:
  /**
   * Construct one wire bus connected to the given pin.
   * @param[in] pin number.
   */
  OWI(Board::DigitalPin pin) : 
    IOPin(pin), 
    m_devices(0),
    m_device(0),
    m_crc(0)
  {}

  /**
   * Reset the one wire bus and check that at least one device is
   * presence.
   * @return true(1) if successful otherwise false(0).
   */
  bool reset();

  /**
   * Read the given number of bits from the one wire bus (slave).
   * Default number of bits is 8. Returns the value read LSB aligned.
   * @param[in] bits to be read.
   * @return value read.
   */
  uint8_t read(uint8_t bits = CHARBITS);

  /**
   * Read given number of bytes from one wire bus (slave) to given
   * buffer. Return true(1) if correctly read otherwise false(0).
   * @param[in] buf buffer pointer.
   * @param[in] size number of bytes to read.
   * @return bool.
   */
  bool read(void* buf, uint8_t size);

  /**
   * Write the given value to the one wire bus. The bits are written
   * from LSB to MSB. Pass true(1) for power parameter to allow 
   * parasite devices to be powered. Should be turned off with power_off().
   * @param[in] value to write.
   * @param[in] bits to be written.
   * @param[in] power on for parasite device.
   */
  void write(uint8_t value, uint8_t bits = CHARBITS, bool power = false);

  /**
   * Write the given value and given number of bytes from buffer to
   * the one wire bus (slave).
   * @param[in] value to write.
   * @param[in] buf buffer pointer.
   * @param[in] size number of bytes to write.
   */
  void write(uint8_t value, void* buf, uint8_t size);

  /**
   * Turn off parasite powering of pin. See also write().
   */
  void power_off()
  {
    set_mode(INPUT_MODE);
    clear();
  }

  /**
   * Lookup the driver instance with the given rom address.
   * @return driver pointer or null(0).
   */
  Driver* lookup(uint8_t* rom);

  /**
   * Search drivers with alarm setting and call on_alarm().
   * Return true(1) if there was at least one driver with an alarm,
   * otherwise when no alarms false(0).
   * @return bool.
   */
  bool alarm_dispatch();
};

/**
 * Print device driver name and rom to output stream. 
 * @param[in] outs stream to print to.
 * @param[in] dev owi device driver.
 * @return output stream.
 */
IOStream& operator<<(IOStream& outs, OWI::Driver& dev);

/**
 * Print list of connected devices on given stream.
 * @param[in] outs stream to print device information to.
 * @param[in] owi one-wire bus.
 * @return output stream.
 */
IOStream& operator<<(IOStream& outs, OWI& owi);

#endif

