// Simulation of CH375 USB controller
// (c) 2023 Warren Toomey, GPL3

module ch375 (
        input  [7:0] i_data,	// Input data
        output [7:0] o_data,	// Output data
	input        CS_n,	// Chip select, active low
	input 	     RD_n,	// Read from device, active low
	input 	     WR_n,	// Write to device, active low
	input	     A0,	// Command/data select line
	output	     INT_n	// Send interrupt, active low
  );

  // Writes take precedence over reads
  always @(negedge WR_n)
    if (!CS_n)
      // We've received a command in i_data
      if (A0)
        $write("%c", i_data);

  always @(negedge RD_n)
    if (!CS_n & WR_n)
      $write("%c", i_data);

endmodule
