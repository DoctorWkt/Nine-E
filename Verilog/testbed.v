// Test module for the SDRAM Controller
// (c) 2022 Warren Toomey, GPL3

`timescale 1ns/1ps
`default_nettype none
`define TESTING
`include "top.v"
`include "tbhelper.v"

module testbed;

`TBASSERT_METHOD(tbassert)

  // DUT inputs
  reg        clk_in;     // Master clock
  reg        reset_n;    // Reset line

  // DUT outputs
  wire [15:0] addr;	  // Address bus

  // DUT
  top dut(clk_in, reset_n, addr);

  // Clock generator: 14.75MHz, so we
  // toggle the clock every 34ns
  always begin
    #34 clk_in = ~clk_in;
  end

  initial begin

    $dumpfile("output.vcd");
    $dumpvars;
    clk_in= 0;
    reset_n= 0;

    // Raise the reset line after 140ns
    #140 reset_n= 1;

    #100000 $finish;      // Terminate simulation
  end

endmodule
