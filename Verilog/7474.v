// Dual D flip-flop with set and clear; positive-edge-triggered

// Note: Preset_bar is synchronous, not asynchronous as specified in datasheet for this device,
//       in order to meet requirements for FPGA circuit design (see IceChips Technical Notes)

module ttl_7474 #(parameter DELAY_RISE = 25, DELAY_FALL = 25)
(
  input Preset_bar,
  input Clear_bar,
  input D,
  input Clk,
  output Q,
  output Q_bar
);

//------------------------------------------------//
reg Q_current;
reg Preset_bar_previous;

  initial begin
        Q_current = 1'b0;

  end


    always @(posedge Clk or negedge Clear_bar)
    begin
      if (!Clear_bar)
        Q_current <= 1'b0;
      else if (!Preset_bar && Preset_bar_previous)  // falling edge has occurred
        Q_current <= 1'b1;
      else
      begin
        Q_current <= D;
        Preset_bar_previous <= Preset_bar;
      end
    end
//------------------------------------------------//

assign #(DELAY_RISE, DELAY_FALL) Q = Q_current;
assign #(DELAY_RISE, DELAY_FALL) Q_bar = ~Q_current;

endmodule
