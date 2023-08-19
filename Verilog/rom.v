// 8K ROM component
// (c) 2019 Warren Toomey, GPL3

module rom (Address, Data, CS_n, OE_n);

  parameter AddressSize = 13;
  parameter WordSize = 8;
  parameter Filename= "romdata.hex";
  parameter DELAY_RISE = 150;
  parameter DELAY_FALL = 150;

  input  [AddressSize-1:0] Address;
  inout  [WordSize-1:0]    Data;
  input CS_n, OE_n;

  reg [WordSize-1:0] Mem [0:(1<<AddressSize)-1];

  // Initialise ROM from file
  initial begin
    $readmemh(Filename, Mem);
  end

/* verilator lint_off ASSIGNDLY */
  assign #(DELAY_RISE, DELAY_FALL)
	Data = (!CS_n && !OE_n) ? Mem[Address] : {WordSize{1'bz}};
/* verilator lint_on ASSIGNDLY */

endmodule
