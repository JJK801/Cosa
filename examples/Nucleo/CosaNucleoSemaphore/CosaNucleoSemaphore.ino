/**
 * @file CosaNucleoSemaphore.ino
 * @version 1.0
 *
 * @section License
 * Copyright (C) 2014, Mikael Patel
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
 * Demonstration of Cosa Nucleo Threads and Semaphores.
 *
 * This file is part of the Arduino Che Cosa project.
 */

#include "Cosa/Trace.hh"
#include "Cosa/Watchdog.hh"
#include "Cosa/Nucleo/Thread.hh"
#include "Cosa/Nucleo/Semaphore.hh"
#include "Cosa/IOStream/Driver/UART.hh"

Nucleo::Semaphore sem(0);

class Thread1 : public Nucleo::Thread {
public:
  virtual void run();
};

class Thread2 : public Nucleo::Thread {
public:
  virtual void run();
};

void
Thread1::run()
{
  uint8_t nr = 0;
  while (1) {
    trace << PSTR("Thread1:") << nr << endl;
    if (nr == 10) sem.signal();
    if (nr == 15) sem.signal();
    if (nr == 20) sem.wait();
    if (nr == 40) sem.wait();
    nr += 1;
    delay(1000);
  }
}

void
Thread2::run()
{
  uint8_t nr = 0;
  sem.wait(2);
  while (1) {
    trace << PSTR("Thread2:") << nr << endl;
    if (nr == 20) sem.signal();
    if (nr == 40) sem.wait();
    nr += 1;
    delay(1000);
  }
}

Thread1 foo;
Thread2 fie;

void setup()
{
  // Setup trace output stream and start watchdog timer
  uart.begin(9600);
  trace.begin(&uart, PSTR("CosaNucleoSemaphore: started"));
  Watchdog::begin();

  // Some information about memory foot print
  TRACE(sizeof(jmp_buf));
  TRACE(sizeof(Nucleo::Thread));
  TRACE(sizeof(Nucleo::Semaphore));

  // Initiate the two threads (stack size 128)
  Nucleo::Thread::begin(&foo, 128);
  Nucleo::Thread::begin(&fie, 128);
}

void loop()
{
  // Run the threads; start the main thread
  Nucleo::Thread::begin();

  // Sanity check; should never come here
  ASSERT(true == false);
}
