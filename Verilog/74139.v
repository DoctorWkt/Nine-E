// 2-line to 4-line decoder/demultiplexer (inverted outputs)
// (c) 2023 Warren Toomey, GPL3

/* verilator lint_off DECLFILENAME */
module ttl_74139 #(parameter DELAY_RISE = 20, DELAY_FALL = 20)
(
  input Enable_n,
  input  [1:0] A,
  output [3:0] Y
);

assign #(DELAY_RISE, DELAY_FALL) Y[0]= (!Enable_n) ? ((A == 2'b00) ? 1'b0 : 1'b1) : 1'b1;
assign #(DELAY_RISE, DELAY_FALL) Y[1]= (!Enable_n) ? ((A == 2'b01) ? 1'b0 : 1'b1) : 1'b1;
assign #(DELAY_RISE, DELAY_FALL) Y[2]= (!Enable_n) ? ((A == 2'b10) ? 1'b0 : 1'b1) : 1'b1;
assign #(DELAY_RISE, DELAY_FALL) Y[3]= (!Enable_n) ? ((A == 2'b11) ? 1'b0 : 1'b1) : 1'b1;

endmodule
/* verilator lint_on DECLFILENAME */
