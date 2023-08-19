// Top level module for the Nine_E 6809 SBC
// (c) 2023 Warren Toomey, GPL3

`default_nettype none
`include "tbhelper.v"
`include "7474.v"
`include "74138.v"
`include "74139.v"
`include "mc6809i.v"
`include "mc6809e.v"
`include "ram.v"
`include "rom.v"
`include "uart.v"

module top (i_clk, i_reset_n, addr);

  input i_clk;                          // Master clock
  input i_reset_n;                      // Reset line
  output [15:0] addr;                   // Address bus value

  // Internal signals
  wire qclk;                            // Q clock
  wire eclk;                            // E clock
  wire [7:0] datain;                    // Data into the CPU
  wire [7:0] dataout;                   // Data out from the CPU
  wire rw;                              // Read/write line
  wire bs;                              // BS line
  wire ba;                              // BA line
  wire irq_n;                           // IRQ line
  wire firq_n;                          // FIRQ line
  wire nmi_n;                           // NMI line
  wire avma;                            // AVMA line
  wire busy;                            // BUSY line
  wire lic;                             // LIC line
  wire halt_n;                          // HALT line

  wire romcs_n;                         // ROM chip select
  wire ramcs_n;                         // RAM chip select
  wire uartrd_n;                        // UART read select
  wire uartwr_n;                        // UART write select
  wire chrd_n;                          // CH135 read select
  wire chwr_n;                          // CH135 write select

  wire [7:0] romdata;                   // Data from the ROM
  wire [7:0] ramdata;                   // Data from the RAM
  wire [7:0] uartdata;                  // Data from the UART

  // The Q & E clock generation logic
  // using two J/K flip-flops.
  wire eclk_n;
  wire unused1;

  ttl_7474 FF1( 1'b1, 1'b1, eclk_n, i_clk, qclk, unused1);
  ttl_7474 FF2( 1'b1, 1'b1, qclk, i_clk, eclk, eclk_n);

  // Address decoding wires
  wire a15_n = ~addr[15];		// Inverted A15 line
  wire [3:0] dm1out;			// Output from Demux 1
  wire [3:0] dm2out;			// Output from Demux 2
  wire [2:0] dm3in;			// Input  into Demux 3
  wire [7:0] dm3out;			// Output from Demux 3

  // Demux 1 is only enabled for the top 32K
  // and divides it into four 8K areas.
  ttl_74139 Demux1(a15_n, addr[14:13], dm1out);

  // Demux 2 takes the third 8K area from Demux 1
  // and divides it into four 2K areas.
  ttl_74139 Demux2(dm1out[2], addr[12:11], dm2out);

  // Demux 3 is enabled by the E clock signal and the highest of the
  // Demux 2 outputs. It selects using the read/write line and one more
  // address line. It generates the four falling edge strobe lines to read from
  // and write to the UART and the CH135. These can only fall when E is high.

  assign dm3in[0] = 1'b0;
  assign dm3in[1] = addr[10];
  assign dm3in[2] = rw;

  ttl_74138 Demux3(dm2out[3], 1'b0, eclk, dm3in, dm3out);

  assign romcs_n=  dm1out[3];		// Top 8K of memory: $E000 up
  assign uartwr_n= dm3out[0];		// Writes to  $D800 - $DBFF
  assign chwr_n=   dm3out[2];		// Writes to  $DC00 - $DFFF
  assign uartrd_n= dm3out[4];		// Reads from $D800 - $DBFF
  assign chrd_n=   dm3out[6];		// Reads from $DC00 - $DFFF

  // RAM enabled for lower 32K of memory, then the next two
  // 8K areas, then the next three 2K areas, i.e $0000 - $D7FF
  assign ramcs_n=  addr[15] & dm1out[0] & dm1out[1] &
		   dm2out[0] & dm2out[1] & dm2out[2];

  // Memory devices: 8K of ROM and 64K of RAM.
  // Only 54K of the RAM is mapped
  rom ROM(addr[12:0], romdata, romcs_n, 1'b0);
  ram RAM(addr[15:0], dataout, ramcs_n, rw, !rw, ramdata);

  // The UART device.
  uart UART(dataout, uartwr_n, uartrd_n, uartdata);

  // The CPU device
  mc6809e CPU(datain, dataout, addr, rw, eclk, qclk, bs, ba, irq_n,
              firq_n, nmi_n, avma, busy, lic, halt_n, i_reset_n);

  // Multiplex the data from ROM, RAM and UART onto the 6809 datain.
  assign datain= (!romcs_n)  ? romdata  :
		 (!ramcs_n)  ? ramdata  :
		 (!uartrd_n) ? uartdata : 8'h00;

  // Prevent halts and interrupts
  assign irq_n=  1'b1;
  assign firq_n= 1'b1;
  assign nmi_n=  1'b1;
  assign halt_n= 1'b1;

endmodule
