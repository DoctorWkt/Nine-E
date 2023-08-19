a.out: 74139.v 7474.v mc6809e.v mc6809i.v ram.v rom.v tbhelper.v testbed.v top.v uart.v
	iverilog testbed.v

run: a.out romdata.hex
	vvp a.out
	gtkwave --rcvar 'fontname_signals Monospace 16' --rcvar 'fontname_waves Monospace 16' output.gtkw

romdata.hex: ../Monitor/data.rom
	./makerom ../Monitor/data.rom romdata.hex

clean:
	rm -f a.out output.vcd romdata.hex
