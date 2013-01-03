/**
 * @file CosaPinsBenchmark.ino
 * @version 1.0
 *
 * @section License
 * Copyright (C) 2012, Mikael Patel
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
 * @section Description
 * Cosa Pins Benchmark; number of micro-seconds for pin operations.
 *
 * This file is part of the Arduino Che Cosa project.
 */

#include "Cosa/Pins.hh"
#include "Cosa/Memory.h"

InputPin inPin(7);
OutputPin outPin(8);
AnalogPin analogPin(0);

void setup()
{
  uint32_t start, stop;

  // Start the trace output stream
  trace.begin(9600, PSTR("CosaPinsBenchmark: started"));

  // Check amount of free memory and size of instance
  TRACE(free_memory());
  TRACE(sizeof(InputPin));
  TRACE(sizeof(OutputPin));

  // Measure the time to perform 1,000,000 input pin reads
  start = micros();
  for (uint16_t i = 0; i < 1000; i++)
    for (uint16_t j = 0; j < 1000; j++) {
      inPin.is_set();
    }
  stop = micros();
  INFO("Cosa: %ul us per 1000, is_set()", (stop - start) / 1000L);

  start = micros();
  for (uint16_t i = 0; i < 1000; i++)
    for (uint16_t j = 0; j < 1000; j++) {
      digitalRead(7);
    }
  stop = micros();
  INFO("Arduino: %ul us per 1000, digitalRead()", (stop - start) / 1000L);

  // Measure the time to perform 1,000,000 output pin set/clear
  start = micros();
  for (uint16_t i = 0; i < 1000; i++)
    for (uint16_t j = 0; j < 1000; j++) {
      outPin.set();
      outPin.clear();
    }
  stop = micros();
  INFO("Cosa: %ul us per 1000, set()-clear()", (stop - start) / 1000L);

  start = micros();
  for (uint16_t i = 0; i < 1000; i++)
    for (uint16_t j = 0; j < 1000; j++) {
      digitalWrite(8, 1);
      digitalWrite(8, 0);
    }
  stop = micros();
  INFO("Arduino: %ul us per 1000, digitalWrite(1)-digitalWrite(0)", 
       (stop - start) / 1000L);

  // Measure the time to perform 1,000,000 output pin toggle
  start = micros();
  for (uint16_t i = 0; i < 1000; i++)
    for (uint16_t j = 0; j < 1000; j++) {
      outPin.toggle();
    }
  stop = micros();
  INFO("Cosa: %ul us per 1000 toggle()", (stop - start) / 1000L);

  // Measure the time to perform 1,000 analog pin samples
  start = micros();
  for (uint16_t i = 0; i < 1000; i++)
    analogPin.sample();
  stop = micros();
  INFO("Cosa: %ul us per sample()", (stop - start) / 1000L);

  // Measure the time to perform 1,000 analogRead
  start = micros();
  for (uint16_t i = 0; i < 1000; i++)
    analogRead(0);
  stop = micros();
  INFO("Arduino: %ul us per analogRead()", (stop - start) / 1000L);
}

void loop()
{
}