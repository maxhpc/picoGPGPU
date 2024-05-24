// maxhpc: Maxim Vorontsov

// Copyright © 2019-2023
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "processor.h"

#include <verilated.h>

#include "VVortex.h"
#include "VVortex__Syms.h"

#ifdef VCD_OUTPUT
#include <verilated_vcd_c.h>
#endif

#include <iostream>
#include <fstream>
#include <iomanip>
#include <mem.h>

#include <VX_config.h>
#include <ostream>
#include <list>
#include <queue>
#include <vector>
#include <sstream> 
#include <unordered_map>

#ifndef TRACE_START_TIME
#define TRACE_START_TIME 0ull
#endif

#ifndef TRACE_STOP_TIME
#define TRACE_STOP_TIME -1ull
#endif

#ifndef VERILATOR_RESET_VALUE
#define VERILATOR_RESET_VALUE 2
#endif

typedef uint32_t Word;

#define VL_WDATA_GETW(lwp, i, n, w) \
 VL_SEL_IWII(0, n * w, 0, 0, lwp, i * w, w)

using namespace vortex;

static uint64_t timestamp = 0;

double sc_time_stamp() { 
 return timestamp;
}

///////////////////////////////////////////////////////////////////////////////

static bool trace_enabled = 0;
static uint64_t trace_start_time = TRACE_START_TIME;
static uint64_t trace_stop_time = TRACE_STOP_TIME;

bool sim_trace_enabled() {
 if (timestamp >= trace_start_time 
  && timestamp < trace_stop_time)
  return 1;
 return trace_enabled;
}

void sim_trace_enable(bool enable) {
 trace_enabled = enable;
}

///////////////////////////////////////////////////////////////////////////////

class Processor::Impl {
public:
 Impl() {
  // force random values for unitialized signals 
  Verilated::randReset(VERILATOR_RESET_VALUE);
  Verilated::randSeed(50);

  // turn off assertion before reset
  Verilated::assertOn(0);

  // create RTL module instance
  device_ = new VVortex();

 #ifdef VCD_OUTPUT
  Verilated::traceEverOn(1);
  trace_ = new VerilatedVcdC();
  device_->trace(trace_, 99);
  trace_->open("trace.vcd");
 #endif

  // reset the device
  this->reset();
  
  // Turn on assertion after reset
  Verilated::assertOn(1);
 }

 ~Impl() {
 #ifdef VCD_OUTPUT
  trace_->close();
  delete trace_;
 #endif
  
  delete device_;
 }

 void attach_ram(void* ram) {
  ram_ = ram;
 }

 int run() {
  int exitcode = 0;

 #ifndef NDEBUG
  std::cout << std::dec << timestamp << ": [sim] run()" << std::endl;
 #endif

  // start execution
  running_ = 1;
  this->wait(1000);

  // reset device
  this->reset();

  return exitcode;
 }

 void write_dcr(uint32_t addr, uint32_t value) {
  device_->dcr_wr_valid = 1;
  device_->dcr_wr_addr = addr;
  device_->dcr_wr_data = value;
  this->tick();
  device_->dcr_wr_valid = 0;
 }


private:

 void reset() {
  running_ = 0;

  device_->rstn = 0;
  device_->dcr_wr_valid = 0;

  for (uint32_t i = 0; i < RESET_DELAY; ++i) {
   this->tick();
  }

  device_->rstn = 1;
 }

 void tick() {
  // clk rising edge
  device_->clk = 1;
  this->eval(device_->clk, device_->rstn);
  // clk falling edge
  device_->clk = 0;
  this->eval(device_->clk, device_->rstn);

 #ifndef NDEBUG
  fflush(stdout);
 #endif
 }

 void eval(bool clk, bool rstn) {
  this->eval_avs_bus(clk, rstn);
  device_->eval();
 #ifdef VCD_OUTPUT
  if (sim_trace_enabled()) {
   trace_->dump(timestamp);
   ++timestamp;
  }
 #endif
 }

 void eval_avs_bus(bool clk, bool rstn) {
  if (!rstn) {
   device_->mem_ready[0][0] = 0;
  }
   else {
    if (clk) {
     if (device_->mem_valid[0][0]) {
      if (ram_ != nullptr) {
printf("zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz %x\n", *(uint32_t*)(ram_+0x80000000));
printf("zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz %x, %x\n", device_->mem_addr[0][0], *(uint32_t*)(ram_+device_->mem_addr[0][0]));
       device_->mem_rdata[0][0] = *(uint32_t*)(ram_ +device_->mem_addr[0][0]);
device_->mem_rdata[0][0] = 0x12345678;
      }
     }
     device_->mem_ready[0][0] = device_->mem_valid[0][0];
    }
   }
 }

 void wait(uint32_t cycles) {
  for (int i = 0; i < cycles; ++i) {
   this->tick();
  }
 }

 bool get_ebreak() const {
  return (bool)device_->Vortex->sim_ebreak;
 }

 uint64_t get_last_wb_value(int reg) const {
//  return ((Word*)device_->Vortex->sim_wb_value.data())[reg];
 }

private:
 VVortex *device_;
#ifdef VCD_OUTPUT
 VerilatedVcdC *trace_;
#endif

 bool running_;

 void *ram_ = nullptr;
};

///////////////////////////////////////////////////////////////////////////////

Processor::Processor() 
 : impl_(new Impl())
{}

Processor::~Processor() {
 delete impl_;
}

void Processor::attach_ram(void* ram) {
  impl_->attach_ram(ram);
}

int Processor::run() {
 return impl_->run();
}

void Processor::write_dcr(uint32_t addr, uint32_t value) {
 return impl_->write_dcr(addr, value);
}
