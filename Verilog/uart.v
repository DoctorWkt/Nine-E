// Simulation of UART
// (c) 2023 Warren Toomey, GPL3

// Because Icarus Verilog cannot do keyboard input, we
// simply send back a sequence of eight characters when
// there is a UART read :-)

module uart (
        input [7:0] indata,	// Input data
	input TX_n,		// Transmit control line, falling edge
	input RD_n,		// Receive control line, falling edge
	output [7:0] outdata	// Output data
  );

  reg [2:0] state = 3'b000;
  reg [7:0] outval = "H";
  assign outdata= outval;

  // UART output: write data on a falling TX edge
  always @(negedge TX_n)
    $write("%c", indata);

  // UART output: return data on a falling RX edge
  always @(negedge RD_n) begin
    case (state)
      3'b000: begin outval <= "H";  state= 3'b001; end
      3'b001: begin outval <= "e";  state= 3'b010; end
      3'b010: begin outval <= "l";  state= 3'b011; end
      3'b011: begin outval <= "l";  state= 3'b100; end
      3'b100: begin outval <= "o";  state= 3'b101; end
      3'b101: begin outval <= "!";  state= 3'b110; end
      3'b110: begin outval <= "\r"; state= 3'b111; end
      3'b111: begin outval <= "\n"; state= 3'b000; end
    endcase
  end

endmodule
