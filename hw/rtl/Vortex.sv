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

`include "VX_define.vh"

`include "picorv32/picorv32.v"
module Vortex(
 /* */
  input clk,
  input rstn,
 /**/
 /* */
  output mem_valid[`NUM_SOCKETS][`NUM_CLUSTERS],
  output mem_instr[`NUM_SOCKETS][`NUM_CLUSTERS],
  input  mem_ready[`NUM_SOCKETS][`NUM_CLUSTERS],
  //
  output[31:0] mem_addr [`NUM_SOCKETS][`NUM_CLUSTERS],
  output[31:0] mem_wdata[`NUM_SOCKETS][`NUM_CLUSTERS],
  output[ 3:0] mem_wstrb[`NUM_SOCKETS][`NUM_CLUSTERS],
  input [31:0] mem_rdata[`NUM_SOCKETS][`NUM_CLUSTERS],
 /**/
 /* DCR write request */
  input                         dcr_wr_valid,
  input[`VX_DCR_ADDR_WIDTH-1:0] dcr_wr_addr,
  input[`VX_DCR_DATA_WIDTH-1:0] dcr_wr_data,
 /**/
 /* Status */
  output busy
 /**/
);

 wire sim_ebreak /* verilator public */;
  assign sim_ebreak = 1'b0;

 genvar X, Y;
 generate
  for (Y=0; Y<`NUM_CLUSTERS; Y++) begin
   for (X=0; X<`NUM_SOCKETS; X++) begin
    picorv32 #(
     .ENABLE_MUL(1'b1),
     .ENABLE_FAST_MUL(1'b0),
     .ENABLE_DIV(1'b1),
     .PROGADDR_RESET(32'h8000_0000)
    ) picorc32 (
     /* */
      .clk   (clk),
      .resetn(rstn),
      .trap  (),
     /**/
     /* */
      .mem_valid(mem_valid[X][Y]),
      .mem_instr(mem_instr[X][Y]),
      .mem_ready(mem_ready[X][Y]),
      //
      .mem_addr (mem_addr [X][Y]),
      .mem_wdata(mem_wdata[X][Y]),
      .mem_wstrb(mem_wstrb[X][Y]),
      .mem_rdata(mem_rdata[X][Y]),
     /**/
     /* Look-Ahead Interface */
      .mem_la_read (),
      .mem_la_write(),
      .mem_la_addr (),
      .mem_la_wdata(),
      .mem_la_wstrb(),
     /* Pico Co-Processor Interface (PCPI) */
      .pcpi_valid(),
      .pcpi_insn (),
      .pcpi_rs1  (),
      .pcpi_rs2  (),
      .pcpi_wr   (),
      .pcpi_rd   (),
      .pcpi_wait (),
      .pcpi_ready(1'b1),
     /**/
     /* IRQ Interface */
      .irq(32'h00000000),
      .eoi(),
     /**/
     `ifdef RISCV_FORMAL
      .rvfi_valid,
      .rvfi_order    (),
      .rvfi_insn     (),
      .rvfi_trap     (),
      .rvfi_halt     (),
      .rvfi_intr     (),
      .rvfi_mode     (),
      .rvfi_ixl      (),
      .rvfi_rs1_addr (),
      .rvfi_rs2_addr (),
      .rvfi_rs1_rdata(),
      .rvfi_rs2_rdata(),
      .rvfi_rd_addr  (),
      .rvfi_rd_wdata (),
      .rvfi_pc_rdata (),
      .rvfi_pc_wdata (),
      .rvfi_mem_addr (),
      .rvfi_mem_rmask(),
      .rvfi_mem_wmask(),
      .rvfi_mem_rdata(),
      .rvfi_mem_wdata(),
      //
      .rvfi_csr_mcycle_rmask(),
      .rvfi_csr_mcycle_wmask(),
      .rvfi_csr_mcycle_rdata(),
      .rvfi_csr_mcycle_wdata(),
      //
      .rvfi_csr_minstret_rmask(),
      .rvfi_csr_minstret_wmask(),
      .rvfi_csr_minstret_rdata(),
      .rvfi_csr_minstret_wdata(),
     `endif
     /* Trace Interface */
      .trace_valid(),
      .trace_data ()
     /**/
    );
   end
  end
 endgenerate

endmodule
