## Fri 12 May 2023 12:25:05 AEST

OK, so let's start with a simple 6809E on a breadboard.
6809, some ROM, some RAM, the UM245R UART for text I/O
and power.

If possible, 32K of RAM but only 16K of ROM. We can
map the UART into the remaining 16K of the address
space. The CPU boots to address $FFFE, so we need
the ROM to occupy $C000 upwards. The UART can be
at $8000, and the RAM from $0000 to $7FFF.

We need a crystal and a 74LS/HCT74 to make the Q and E
clock signals. We'll need a 74LS/HCT139 2:4 demux to
create the CE signals for the RAM, ROM and UART.

## Sat 13 May 2023 09:45:58 AEST

From previous projects, I can scrounge these devices:

* AS6C1008  128Kx8  RAM
* AT28C64B  8Kx8    EEPROM

I don't have any demultiplexer chips or the '574
flip-flop to generate the Q & E clock signals, so
I'm going to have to order some anyway. So I might as
well do a design where I can maximise the RAM space
and minimise the UART space.

There's also this CH375 chip which provides an
8-bit parallel interface to block USB storage.
So, I'm wondering if I can squeeze it into the design as well.
I've just ordered one from eBay. I haven't found
somewhere to buy the 6809E just yet.

So the CH375 has an active low chip select, so that's
good news. It's a pity the UM245R doesn't have any
chip select that I can see.

I've done a new demux design with gets me 48K of RAM,
8K of ROM, and three 4K address areas for the CH375,
the UM245R write and the UM245R read. It uses a '138
3:8 demux and the two halves of a '21 deal 4-input AND.
With LS devices, the total propagation time is around
30nS. With HCT devices, it would be about 70nS.

Maybe I should look at the 6809 SBC here:
https://jbevren.wordpress.com/2018/09/22/designing-a-6809-sbc/
It uses an MC68681 dual UART which has a chip select. So
maybe I could save some address space if I use this
instead of the UM245R.

## Sat 13 May 2023 12:19:50 AEST

The CH375 is a small board with a 8-row x 2 header
interface. Right-hand side is D7 .. D0 top to bottom.
Left-hand side is (top to bottom):
WR, RD, CS, A0, INT, VCC, GND, GND.

I can lose the barrel connector for power now that
I'm using the UM245R in the design.

## Sun 14 May 2023 11:19:09 AEST

Address decoding: I think I've found a way to get
more addressable RAM, by replacing the 3:8 demux
with a dual 2:4 demux. Damn, no it needs a NOT
gate. Anyway, I should also do some timing checks.
Here are the propagation delays for some devices:

* AS6C1008 RAM		 55ns
* AT28C64B EEPROM	150ns
* 74LS139  3:8 demux	 20ns
* 74LS139  2:4 demux	 25ns
* 74LS21   4-input AND	 10ns
* 74LS04   hex NOT	 10ns

It seems that the HCT devices have longer propagation
delays than the LS equivalents.

The 14.75MHz clock is divided by 4 to produce the Q
and E clock signals. So the actual clock is 3.6875MHz,
or a 271nS cycle time. We used the AT28C64B EEPROM in
the CSCvon8 design with a 3.57MHz clock signal, and
the decode was going through a ROM then a 139, so I
think I should be safe. Nothing I do with 7400 logic
will be as slow as going through a decode ROM.

So, assuming that I choose to use a NOT gate, dual
4-input AND gates and dual 2:4 demuxes: here is
the plan.

A15 (inverted) enables the first 2:4 demux. This
takes A13 & A14, and decodes the top 32K into four
8K areas. Output Q3 becomes ~{ROMCS}, giving us 8K
of ROM at the top. Outputs Q0 and Q1 go to a
4-input AND. Along with A15 into this AND, we now
have the bottom 32K plus another 16K of RAM.

Now, output Q2 of the first 2:4 demux enables
the second demux. This takes A11 and A12, and
decodes this 8K space into four 2K areas.
Output Q0 goes to the RAM's 4-input AND. So now
we have 32K + 16K + 2K = 50K of RAM.

The remaining outputs, Q1, Q2 and Q3, become
the ~{UARTRD}, ~{UARTWR} and {~CHCS} lines.
Thus, these only take up 2K of address space each.
That's better than taking up 4K with a single 3:8 demux.
Also, with the 3:8 demux, we had 6K less RAM than we
do now.

But we waste five NOT gates and a 4-input AND now :-(
And we have one extra chip in the design.

## Mon 15 May 2023 10:37:43 AEST

I've updated the schematic and the PCB design with
the 2:4 demux and the hex inverter. I've run
freerouter and we only have 20 vias. Some things to
change on the PCB design:

* the UM245R footprint is too narrow, must find out what I used previously
* move the CH375 pin header away from the ROM as it is going to be in a ZIF socket
* replace the 100uF power cap with a 220uF cap which I have; it needs legs 4mm apart.

I've found lots of the parts from my spares and
old projects. We now need to order these parts:

* MC6809E CPU
* 74LS74 dual flip flop
* 74LS09 hex inverter
* 74LS139 dual 2:4 demux
* 74LS21 dual 4-input AND
* 14\.75MHz oscillator
* DIP-32 socket, the cheap one for the ZIF socket

The CH375 is ordered. Once I get that, I can
improve the PCB design further.

## Mon 15 May 2023 11:44:24 AEST

I found this site that sells 6809E chips:
https://www.dragonplus-electronics.co.uk/product/1-x-hd63c09ep-hitachi-6809-compatible-cpu/
and they mention that most on-line chips are
fakes. But they don't have a way to pay for
shipping, so I've sent them a question on-line.

## Mon 15 May 2023 14:15:20 AEST

I just took the 74LS74 Verilog model and
build the Q & E clock circuit in Verilog.
Yes, Q and E are 1/4 the input clock and
E lags Q by one input clock, i.e. 1/4 of
the Q output. Phew, I was worried about
that.

I should think about building a simulator
so I can get some test code working.

## Mon 15 May 2023 14:57:26 AEST

I've modified Salmi's simulator to have 8K
of ROM and a UART output line at the 50K
mark. I've also downloaded and built asm6809.
I have some test code that stores literal
values to the UART address and jumps back
to the initial $FF00 boot address, so I
can print Hello indefinitely to the UART :-)
Next: work out how to check if a character
is available on stdin, send an IRQ if so,
and add code to read the UART at the 52K
address point.

I should also check and re-check the schematic.

## Tue 16 May 2023 08:13:44 AEST

I've just ordered a HD63C09EP CPU from
DragonPlus Electronics in the UK, so I
guess I'm now committed to do this thing.
Today, if I get a chance, I'll build a
Verilog model of the SBC and see if it
runs the same "Hello" code that the
Salmi simulator does.

## Tue 16 May 2023 10:02:30 AEST

I've got the Verilog model working and
printing out "Hello". I got the UARTWR
address wrong when writing the Salmi
simulator, but it's now corrected. So
I can assemble a ROM file and run it
in both Salmi and the verilog model
and they both print out "Hello". Yay!!

## Tue 16 May 2023 11:23:33 AEST

I tried to add some code to Salmi to
a) IRQ is some keyboard input available
and b) read a keyboard character when
a read at $D000. I'm using some of the
code from
https://github.com/czirkoszoltan/c-econio/blob/master/econio.c

But the select() isn't returning anything but 0.
Perhaps the readline() library is getting in the road?
Maybe I should open up /dev/tty so it's a separate fd?

OK I got it mostly working. Just a matter of fflush()ing
stdout at the right time. I should put in code to
exit(0) when we get a ctrl-C.

## Tue 16 May 2023 13:58:57 AEST

It's not going to be easy to do input with Icarus Verilog,
so I'm having a look at Verilator. I've copied the
`make_tracing_c` example and I've added the same C code
for checking/reading stdin as I put into Salmi. It looks
like I can get a line to strobe on available input, send
an output to ask for it and then get it. Yay!

So at some point I should switch over to Verilator.

The rain kept me awake last night. I was thinking of
the software for this SBC. So: Unix-like syscalls in
the kernel along with a shell. Each program gets
linked with a standard I/O library. We will need some
block cache for the file syscalls.

What syscalls?

* read
* write
* open
* close
* stat
* fstat
* lseek
* dup
* dup2
* exit
* getcwd
* chdir
* fchdir
* rename
* mkdir
* rmdir
* creat
* link
* unlink
* opendir
* closedir
* readdir

Both the C runtime (for returns from main()) and exit()
can simply jump to the start of the shell code in ROM.
The shell can read a line of text, insert NULs in the
line, build an argc and argv[], and then find the file
using the syscalls. It can parse the header, load the
file into memory and jump to the start address.

Which filesystem layout to use? Should I use xv6 or
FUZIX or something simpler? No idea. Can I borrow
code from these systems?

## Tue 16 May 2023 16:32:59 AEST

I wonder if I can write a Verilog model for the CH375.
Looks like there are some file I/O operations, see:

http://www.asic.co.in/Index_files/verilog_files/File_IO.htm
https://www.hdlworks.com/hdl_corner/verilog_ref/items/SystemFileTasks.htm

Actually, if I'm switching over to Verilator, I can
write C code for both the UART (input and output) and also
the CH375. And I can use this code for both the Salmi
simulator and the Verilator model.

The wiring for the CH375 is: ~{CHCS} to select the device,
addr0 -> A0 which determines if we are sending commands or
data, addr1 -> ~{RD} and addr2 -> ~{WR}. And the device
is mapped at $D800. So:

* write to  $D803 is a command write
* write to  $D802 is a data write
* read from $D804 is a data read

I want the actual device to arrive so I can fit it on the
PCB correctly.

Another 6809 SBC:
http://jefftranter.blogspot.com/2019/02/a-6809-single-board-computer.html

## Thu 18 May 2023 09:40:52 AEST

Still trying to get a monitor to work. I've given up on
ASSIST09, but I have copied the keyboard code out to
a test asm file and it works fine. Jeff Tranter has a monitor
but it uses FIRQ instead of IRQ. I'm wondering if I can
modify it to use IRQ. No, still no good. Oh well, I'll
just write my own ROM code!

I've rewired the CH375 pin socket so that, hopefully,
I can mount the board upside-down on the main PCB.

I wonder if I can use a Pico W to interface with the CH375
device and see how it reacts? I can write some Python
functions to read/write bytes and toggle the CS, RD, WR, A0
and INT lines. 13 lines, and the eight data lines are
bi-directional.

https://www.upesy.com/blogs/tutorials/micropython-raspberry-pi-pico-gpio-pins-usage

## Fri 19 May 2023 07:53:10 AEST

I'm just downloading the 6809 GCC compiler from https://github.com/jmatzen/gcc6809
to have a look at.

My thinking on filesystem is that the xv6 one is too complicated and
might not work on 16-bit systems. So I'm going to try the V6 filesystem.
I was going to modify the original code, but I've just found this
Github project: https://github.com/jaylogue/retro-fuse
This has the original code modified to compile under current systems,
with a FUSE interface. I hope it will be more easy to compile with the
6809 GCC compiler, and I can also mount images with FUSE on my Linux systems.

I'm also thinking of taking ideas/code from Mini-Unix, LSX and/or 2.11BSD.
LSX looks interesting in that there's only one process in memory, and
pipes are implemented using a temporary file. I wonder if it has a clock
to provide clock ticks? Anyway, I'll park that for a while.

My plan is to get the Salmi simulator as close to the real hardware as I
can. I'll use the Pico W to work out how to drive the CH375, then write
code for Salmi to simulate it. Then, start compiling the V6 filesystem
code for both Linux and the Nine-E device and get them to be consistent.
I could write a really simple ROM monitor to load binaries over the
terminal, and do the filesystem testing in RAM. Much easier than burning
ROMs all the time.

## Fri 19 May 2023 12:01:40 AEST

I've built a Debian VM so I can try to build GCC without destroying
my existing machine. I've copied the code over, gone into `build-6809`
and I'm doing a 'make everything'. Things required so far:

* mkdir -p /usr/local/lib/gcc/m6809-unknown-none/4.3.6
* apt-get install build-essential
* apt-get install libgmp-dev
* apt-get install libmpfr-dev
* apt-get install flex bison

Now I need `/usr/local/bin/m6809-unknown-none-ld`. Ah, I've copied the
`lwtools` source over to the VM and compiled/installed it. Then:

```
/usr/local/src/lwtools-4.19/extra# cp ld /usr/local/bin/m6809-unknown-none-ld
ln -s /usr/local/bin/lwasm /usr/local/bin/m6809-unknown-none-as
```

I got a fair way in but it still crashes, looks like the lwtools assembler
doesn't like the code that the compiler produces. I'll have to search on-line
to see if someone else has been able to build it.

## Fri 19 May 2023 15:03:43 AEST

The CH375 has arrived. I can mount it upside down, but some of it will
have to stick out over the PCB edge as there is a 3x2 pin header which
would scratch the PCB if not over the edge.

I've written some of the Pico code to test the device, but need some
time to wire it up and get things started.

## Sun 21 May 2023 08:32:47 AEST

There's this website from the CH375 manufacturer with driver code in C:
https://www.wch.cn/search?t=all&q=ch375
So far, CH375LIB/TOOLS/CH375EV4.C looks promising. I also found a
schematic of the little CH375 board here:
https://github.com/djuseeq/Ch376msc/blob/master/extras/schematic.png

Based on the red labels (of which VCC is missing), I think I connect
the VCC pin header to 5V. That powers the USB device and also (via the
LM1117) produces 3.3V for the CH375 device.

## Sun 21 May 2023 11:23:04 AEST

Well, I got the writing wrong to start with and had to bring out
the BitScope to see what was going wrong. It's now receiving commands
and responding to them. I'm up to the example code which sets the
USB host mode, loops waiting for a drive to connect and informs the
caller when the drive is there. Yay!!!

## Mon 22 May 2023 08:00:33 AEST

I looked at the example sector read code yesterday and the datasheet
for the CH375. You send CMD_DISK_READ, then the LBA address and # of
512-byte sectors to read. Then you read groups of 64 bytes as follows:

* wait for an interrupt, ensure the status is USB_INT_DISK_READ
* send a CMD_RD_USB_DATA command, read back the number of bytes (64)
* read 64 bytes from the device
* send CMD_DISK_RD_GO to request reading the next group of 64 bytes
* loop back

Once out of the loop, wait for an interrupt, ensure the status is
USB_INT_SUCCESS.

This means that, for a 512-byte sector, we take nine or ten interrupts!
Luckily, I've wired the CH375 to the FIRQ interrupt handler so the delay
to handle each interrupt will be minimal.

I was originally thinking of just setting a flag in the FIRQ interrupt
handler, and letting the upper-half kernel code do the CH375 commands.
But now I think I should do commands for the inner read loop in the
actual interrupt handler, and perhaps the actual 64-byte read.

## Mon 22 May 2023 09:03:44 AEST

I've replicated the read code in the C example into my Python code
on the Pico W. I'm trying to read one sector at LBA 0. I've issued
the CMD_DISK_READ command with the LBA and sector count. I should
get back the USB_INT_DISK_READ status but I'm getting back
USB_INT_SUCCESS instead. Not sure why, damn.

## Mon 22 May 2023 10:50:33 AEST

I've stopped work on the CH375 and I'm now writing some monitor
code. I want to be able to upload s19 files, run them, and dump
memory in hex format. Should be simple, but I need to learn
enough 6809 assembly code to write it. I've imported the keyboard
functions and now have putc(), getc() and puts() working. That's
all so far.

Also still trying to choose a C compiler that works. `cmoc` works,
but the output sucks badly. I can't get gcc6809 to compile. The
`lwcc` doesn't seem to be complete. Maybe I should go back to
writing my own compiler. I'd need to find some multiply & divide
assembly functions somewhere. And learn 6809 assembly code!!

## Mon 22 May 2023 14:51:40 AEST

Well, I've got a monitor with Dump, Go and S (upload s19 file) commands.
They all work, except ... I can manually type S19 lines, but I can't
paste them in. I'm guessing that the cbreak mode in Salmi is too slow
to keep up with the pasted characters. Damn!

## Mon 22 May 2023 16:20:49 AEST

So this page:
https://utcc.utoronto.ca/~cks/space/blog/unix/CBreakAndRaw
describes how to do 'cbreak' mode. I'm wondering if I can
go into cbreak mode, but back out to normal when we are in
the monitor.

## Tue 23 May 2023 10:13:32 AEST

OK, finally fixed. I used the cbreak code, and I am also buffering
input characters in the Salmi simulator. I had to put in a count
to stop the IRQ handler just eating all the available characters
before the monitor could consume them. So now, we get to run enough
monitor code to consume the current character before the IRQ handler
puts in a new character.

I also fixed up the s19 load code, and I've been able to upload an
s19 file and execute it. Yay, so it means I can burn the monitor
to ROM and not have to keep swapping it each time I want to run
another program.

I now want to add code to change memory contents.

## Tue 23 May 2023 14:31:13 AEST

I've done the alter code and it works.
I checked the schematic and found that TSC has to be tied low. Fixed.
I've swapped ROM and RAM on the PCB for the ZIF socket and for the
CH375. I've done the freerouting and there only 11 vias!

I've re-checked the BOM against what I have. I need to get:

* 74LS74
* 74LS04
* 74LS21
* 74LS139
* DIP-24 socket
* DIP-32 socket
* 14\.75MHz oscillator

I wonder if Element14 is cheaper than Digikey?
I also have to remember how to do the ground planes on the PCB :-)

## Tue 23 May 2023 15:01:51 AEST

Element14 didn't have the 4-input AND gate, so I ordered it all from
Digikey.

## Wed 24 May 2023 09:12:25 AEST

I had some trouble doing the freerouting and the copper pour. I've solved
it by removing all the pours, then doing the freerouting. Then I put back
in the front and back copper pours. When I run the DRC checker, I select
"Refill all zones before DRC" to get the pours filled. I've got two
slivers identified but I don't think that's a problem.

I also had errors with the thermal relief. I've done File -> Board Settings
\-> Constraints and set the minimum spoke count to 1.

## Wed 24 May 2023 14:51:19 AEST

I put the gerbers up on JLCPCB and ordered them. Yay!!
Here's the page to describe how they want the gerbers:
https://support.jlcpcb.com/article/194-how-to-generate-gerber-and-drill-files-in-kicad-6

## Wed 24 May 2023 16:54:44 AEST

I just found this project which has a C compiler in C for the 6809:
https://github.com/Deek/CoCoC and it seems to be recently maintained.

## Fri 26 May 2023 14:18:14 AEST

Just musing on the two stack pointers: S and U. So, all JSRs and RTS/RTIs
etc. go on the S stack. The user program can push, pop and index via the
U stack pointer. So we might be able to use it as a pointer for the locals
on the stack? I don't know yet.

I should also start building a syscall API using the SWI instruction
and put it into the Monitor ROM. That way I can write assembly code
independent of what is in the ROM and where it is. Maybe putchar()
when B is 0, A holds the char to send, and getchar() when B is 1.

## Sat 27 May 2023 10:13:54 AEST

I'm still annoyed that there's no C cross-compiler which builds on Linux
and is in C (CMOC is in C++). So perhaps I'll go back to writing a 6809
backend for acwj; I can look at the CMOC assembly output and compare.

Seeing that I want to port the V6 or V7 filesystem code, I can also do an
"apout" to the Salmi simulator and put in the file-oriented system calls
now. Then I can get a C compiler to work, bring over some of the 16-bit
userland librarys (from 2.11BSD?). In other words, I can do a few things
in parallel.

## Sun 28 May 2023 08:31:40 AEST

I've gone back to my C compiler. I see that I implemented an intermediate
representation before when I was working on a 6809 backend :-). I've just
got it to the point that I can use the LWTOOLS assembler and linker to
assemble .s files to .o files and link them together. I've implemented
exit() and printint() in the Salmi simulator, and I can compile and run
a program that does int a=2; printint(a);

## Sun 28 May 2023 09:56:23 AEST

I can compile and run test inputs 001, 002 003. Now doing 004 which
is compare and set. Looks like cmoc does a branch and sets D to 1 or
0\. So there's no easy way to do this. Sigh.

## Sun 28 May 2023 10:47:07 AEST

Now up to input008.c, OK so far. And now input009.c which took some effort.

## Sun 28 May 2023 11:43:01 AEST

input010.c works when the variables are global, just not local.
There's also a weird thing going on with the IF statement. Seems like
I'm making a compare and set boolean, then IFing on the boolean. So:

```
if (a<b) 
```

does a compare a<b and sets a register true/false, then does a
compare register to 1 and branch if true/false. Yuck!!

## Sun 28 May 2023 11:59:01 AEST

Hah, I know the problem now. I'm putting locals below the stack pointer.
But, of course, as soon as I do a JSR, the stack drops and tromps on
the locals. I need to move the SP down and make local offsets positive!

To drop the SP, I think I can TFR S,D, SUBD #number, TFR D,S.

## Mon 29 May 2023 20:18:56 AEST

I think I've got params and locals working right now, and I've modified
the Salmi simulator to optionally (not) start in the monitor. So I can
compile programs and simulate them with no user interaction. I need to
do a bit more to automate things.

I'm up to a tests/input010.c where a char arg is being passed to a function
with an int param. I need to widen it.

## Tue 30 May 2023 09:18:28 AEST

Fixed that and some other things. I can now pass up to tests/input012.c.
We're up to & and * in input013.c, and things need fixing.

## Wed 31 May 2023 09:30:24 AEST

Up to input016.c which is failing. Damn! I just found out that the 6809
is big-endian and not little-endian. So all my zero-page registers are now
backwards. If we have constant #1234, then it's stored as 34 at address
$0000 and 12 at address $0001. So I can't stb $0000 and ldd $0000.

I think I have a solution. From now on, literal integers values are always
P_INT. I've added code to allow `char x = 3;` by setting the '3' type back
to P_CHAR when the lvalue type is P_CHAR and we are in the range 0 ... 255.

But I need to re-run all the tests, and I should automate it. Right now
I'm doing it all by hand.

## Wed 31 May 2023 16:40:30 AEST

OK, I've modified `runtests` to call `lwasm`, `lwlink` and the Salmi
simulator so that I can automate the tests. I've removed the tests
which have `long`s in them. I'm now seg faulting on input017.c.

Also, the Digikey components arrived. So all that is missing now is
the CPU! That's coming from the UK and I have no idea when it will arrive.
But I can solder all the 74LS components, sockets, caps etc. and I can
check that the reset works, ditto the Q and E clock signals.

## Thu 01 Jun 2023 08:52:15 AEST

I'm up to input018a.c, having implemented deref'd writes. We have a problem.
We can't use 'a', 'b', 'd', 's', 'x', 'y' as they are register names known
to the assembler. What to do? Do we add a prefix to our symbol names? I was
thinking of a dot/period, e.g. .a, .b, .foo, .fred.

## Thu 01 Jun 2023 14:07:10 AEST

I've half implemented the dot thing above but need to do more. I also
got out my equipment and I've soldered all the logic chips, sockets,
caps etc. onto the board. I haven't put the ROM or RAM in, and no CPU
as yet of course. At some point I'll check the Q & E signals and the
reset signal.

## Thu 01 Jun 2023 14:21:11 AEST

Got the dot thing working. Now up to input020.c. The IR (intermediate)
code definitely doesn't look right. Fixed.

Now input021.c requires printf(). Which means that I need to add ellipsis
to the language. Argh!

## Fri 02 Jun 2023 08:58:41 AEST

I've nearly got ellipsis to work, except ... I'm pushing arguments in
left to right order. This works when # args == # params, but it doesn't
work for:

```
void mary(int x, ...);

main() { mary(3,4,5); }
```

3 gets pushed first, then 4, then 5. But mary() expects the first argument
just above the return address, and so it will access 5 and not 3. So now
I've got to a) push from last to first and b) rewrite the code to calculate
the argument positions. Sigh.

## Sat 03 Jun 2023 10:40:09 AEST

The 6309E turned up yesterday! I compiled minipro on the laptop so I
could burn the ROM but it failed. I was worried for a bit. Moved it
over to neddie the desktop and it burned there OK. Anyway, so now I
have my monitor in ROM. I'm prepared to do an all-up test with the
ROM, RAM and CPU. Later ....

## Sat 03 Jun 2023 10:59:00 AEST

AARGH! I've got a AS6C1008 RAM chip not a AS6C4008 chip! The difference
is that there's less memory (doesn't matter), but one of the address
lines on the AS6C4008 (which I'd tied to ground) is a CE line which needs
to be tied high. So I need to pull out the chip, push pin 30 out of the
socket and wire it to pin 32. Not too much of a drama but ... damn!

## Sun 04 Jun 2023 08:07:50 AEST

I fixed the RAM chip and put it all together. It runs ... sort of. It
exhibits the expected monitor behaviour: lots of output, then wait, then
echoing what I type in. Except that the output is completely garbled.

I hope/feel this means that the CPU is fetching and executing the right
instructions, but that somehow the UART control isn't working right.
I added a bunch of NOPs to the output code, in case the CPU was sending
bytes to the UART faster than it can cope. No change. I dropped to a
1MHz crystal too, no change. So I'll get the logic analyser out and
look at the data lines and the write strobe and see what's going on.

## Sun 04 Jun 2023 16:38:26 AEST

I wrote some code that loops printing out the ASCII character set. The
CPU is definitely running it fine. The output in hex is:

```
5a 12 da 12 52 12 12 12 52 12 52 52 52 52 52 5a
5a da da da 52 da da da 52 da 5a 5a 52 5a 52 12
12 52 12 52 52 5a 12 da 5a da da da da da da da
da da da 52 52 52 52 12 52 12 12 12 12 12 12 12
52 5a 5a 5a da 5a da da da 52 da 52 da 12 da 12
52 12 52 12 52 5a 52 5a 5a da 5a da da da da da
5A => 01011010
12 => 00010010
DA => 11011010
```

So that doesn't help, I will have to break out the logic analyser.
The only other thing I could try is to find the second UM245R and
swap it with the one that's on the board. Nope, tried that, no change.
Damn, so it will have to be the logic analyser.

## Mon 05 Jun 2023 08:39:22 AEST

I looked at Figure 2 in the 6809 datasheet which is the write data
timing, and I can see the problem. Here's the sequence:

* E drops with Q already low
* the R/W line drops
* the address lines become valid
* Q rises
* the data bus becomes valid at the same time that E rises

At the moment I'm simply decoding the address lines and sending a low WR
signal to the UM245R UART when they match $D000. But this happens way
before the data is on the data bus.

As it stands, the current PCB is not going to work. However, I do have
a spare 4-input AND gate and five inverters. I might be able to do this:

* The existing Q2 coming out of the second 2:4 decoder (which is currently UARTWR) goes to an inverter.
* The inverter's output goes to the AND gate, along with E. The remaining inputs are tied high.
* The AND gate's output goes to an inverter, whose output becomes the new UARTWR.

Thus, the AND gate's output is only high when we are decoding the $D000
address range and the E signal is high. This gets inverted to be the
active low UARTWR signal.

I could probably cut the track to the UART and do some extra wiring for
the above. What I should also do is:

* look at other 6809 SBCs and see what they do, and
* revive the Verilog model of my SBC and see what the timing is doing. Given that the 6809 model is cycle accurate, it should demonstrate the timing in the diagram.

## Mon 05 Jun 2023 10:07:51 AEST

Damn. I did the rewiring, checked it with multimeter, still garbage on
the output. I got out the logic probe to check that the line was
strobing. I'm getting different garbage than before, sigh.

## Mon 05 Jun 2023 10:15:36 AEST

I'm looking at the design for Simon6809: https://gitlab.com/8bitforce/simon6809.
They use a 3:8 decoder with A15 high enabling the demux and using A12-A14 as
the selector lines, thus decoding $8000, $9000, $A000 ... $E000, $F000.
Notably, the BA line from the 6809 is tied to one of the active low enable
lines (the second active low enable tied to ground).

The $D000 (active low) line, and the A11 line, go to an OR gate, whose output
is called FT245 (the FT245R is the same UART in the UM245R). This seems to
mean that the UART is only active when A12..A15 decodes to $D000 and A11 is
low, i.e. $D000 .. $D800.

There's a line called WE which is R/W inverted into a NAND gate with E as
the other input to the NAND gate. So, this means that WE is only low when
R/W is low and E is high.

Finally, the FT245 line and the WE line go to a NOR gate whose output
is called FT245_WE. This means that the FT245 write enable is active
low when the FT245 line is low AND the WE line is also low.

So, for the FT245 write enable to be low:

* address $D000 .. $D800
* E is high
* R/W is low

Ahah, this is different to my new wiring because R/W also has to be low.
So, I could invert R/W with a spare inverter and send it to one of the
spare 4-input AND inputs. Thus, the AND gate will have three inputs:

* The inverted Q2 output from the 2:4 decoder, indicating address $D000
* The E line
* The inverted R/W line

The output will be high when: address $D000, E high, R/W low.
This gets inverted to be the UM245R WR line.

## Mon 05 Jun 2023 11:08:27 AEST

Woohoo. I tried to use fine lacquer covered copper but no luck. I've
gone to using CAT-5 strands, and it works. I've got the monitor up
and running, without any NOPs or delays. This is with the 1MHz crystal.
Time to try the other crystals ...

The 14.7456MHz crystal works!! Wow, amazing :-) And, with it, I can upload
a S19 program and run it. Yay!!!

## Mon 05 Jun 2023 13:25:09 AEST

I suspect that this means that the CH375 isn't going to work as, when I
write a command to the device, the CHCS active low line will be enabled
when the address is $D803 but before there is real data on the data bus.
In other words, we need to have R/W low and E high as well as the
CH375 address. But I can always try...

## Tue 06 Jun 2023 07:50:01 AEST

Back to the compiler. I've rewritten the code that deals with params
and arguments. Both are output in the IR right to left now, which
makes dealing with varadiac functions much easier. The ellipsis (...)
code seems to be working. This now compiles and runs:

```
int printint(int x);

void mary(int a, int b, ...) {
  printint(a);
  printint(b);
}

int main() {
  mary(6, 20, 34);
  return(0);
}
```

which paves the way for printf(). But to do printf I'll need itoa() or
code to divide by 10 and get the dividend and remainder.

Looking at the compiler tests, I only need %s, %c, %d and %2d for
printf(). The first two are easy.

## Tue 06 Jun 2023 11:57:10 AEST

It's been hard work trying to get things fixed. I've got a library
function div16() which takes two numbers plus two int pointers and
returns the quotient and remainder via the pointers. I've just
implemented putchar() as a syscall and that works. I just tried to
write a puts() but that didn't work. Damn!

## Wed 07 Jun 2023 10:29:41 AEST

OK, I fixed the puts() problem. Lots of little things to do to make it
work. The only problem now is that ++ isn't being generated in the IR
code, so I'm doing `ptr= ptr+1` to move up the string in puts().

Trying to write a simple printf(). Right now, `while (1)` generates
incorrect code! Now using `while (7==7)`.

## Wed 07 Jun 2023 11:36:24 AEST

My idea of storing all INTLITs as type int might have backfired, as
I can't do `char c= '!';` as the exclamation mark is type int.

## Wed 07 Jun 2023 12:05:37 AEST

I went back to small INTLITS are char type, but now it's biting me.
I'm so close with printf() with %s %d %c. But I'm calling printf()
with a literal 23. That's being pushed as a byte not a 2-byte word
because it's small. And the printf() code is expecting a 2-byte
int of course.

So this works:

```
void main() {
  char *f= "Hello %c %s and %d bye\n";
  char mary= '!';
  char *foo= "Another thing";
  printf(f, mary, foo, 99);
}
```

but only when the three vars are global. My `cg.c` says I have to
handle IR opcode 55 (ldlab) size 2 :-)

## Wed 07 Jun 2023 15:13:03 AEST

OK, fixed the above. Now to do some regression tests. Yes, I had to
fix one small thing. Now I'm up to the bit where I need printf().
And now I have a printf.

I'm thinking that I might hand-edit the current assembly version of
printf to make it faster/cleaner. I gave up, it's too hard. The
current assembly code my compiler creates is pretty crappy.

I've installed puts() and printf() in the libc.a, modified the
Salmi simulator to not put \\n when printint()ing, and fixed up
runtests to use the original test C programs.

Doing regression testing, I'm up to input010.c which fails. OK, I see
why. We are effectively doing `char c='A'; printf("%d\n", c);`. So,
we only push 65 as a byte onto the stack, but printf is printing a
16-bit int out.

Not sure how to fix this. One way is to always push char arguments as
sign-extended 16-bit words when we can't check the parameter type
(i.e. when ...). That's a good idea!!

## Thu 08 Jun 2023 09:30:37 AEST

I applied the above fix and it works. Now up to input021.c which is
walking a string until NUL and printf("%c", *str);
Hah, *str is now being passed as an int, so I need to skip the first
byte of the argument in printf()! Yes, that's fixed it.

Now up to input023.c which is failing. I've implemented INV and NOT
IR instructions. But the logic for true/false is producing really
ugly IR code and, thus, assembly code. I'll have to look into it.

OK, I fixed the code somewhat. I went back to version 62 for much of it.
Now, postincrement isn't working.

## Thu 08 Jun 2023 19:09:57 AEST

I've added 2-byte AND OR XOR and also realised that I can put in some
INTLIT op INTLIT optimisations in opt.c for op= { AND, OR, XOR, SHL, SHR }.

Now we are up to input067.c, woo hoo! OK fixed that, back to using code
from version 62. Now up to input074.c, this is the first one with switch()
in it.

## Thu 08 Jun 2023 20:44:11 AEST

Fixed the switch problem, now up to input083.c. Ah, that was a %2d
format in printf() which I don't support yet. I changed the test.
Now up to input107.c. We have an array of strings, the last is NULL.
We're walking the list and either print the string or print NULL.
At the moment we print NULL for all of them. And I'm tired and will
stop now. But, not bad to get from input021.c to input107.c in a day!

## Fri 09 Jun 2023 10:27:13 AEST

I decided to pause the work on the compiler, mainly as it is
producing pretty bad code. I wanted to see if I could get cmoc
to work as a compiler for the Nine-E SBC. In particular, can
I get it to use the existing system calls that I've defined in
the Salmi simulator.

It's been a bit of hard work, but I can now putchar() and exit()
from a C program. I had to remove the crt0.o from the
/usr/local/share/cmoc/lib/libcmoc-crt-void.a library, which seems
to also have a bunch of routines for division etc. I now have
my own library in /home/wkt/wktcloud/Nine_E/Cmoc/lib with my
own crt0.o, exit.o and putchar.o. I also have a small script
called `cm` which is:

```
#!/bin/sh
cmoc --org=0100 --void-target --srec \
  -I/home/wkt/wktcloud/Nine_E/Cmoc/include \
  -L/home/wkt/wktcloud/Nine_E/Cmoc/lib \
  -nodefaultlibs -Wno-const $* -lninec
```

The only problem is that it compiles starting at $0000 not
$0100. Not sure why yet. Anyway, I just brought my own
printf() into my cmoc library and I've got it to work,
yay!!!

That sort of means that I might be able to run my
compiler tests with cmoc :-)

## Mon 12 Jun 2023 10:33:27 AEST

Back from the CDG dressage competition at Caboolture.
I've got cm running my acwj compiler tests. Obviously
the error messages are wrong. I've eliminated a few
tests which require things that don't yet exist:
fprintf(), malloc(), errno. But cm passes all the tests
in the test suite, so that gives me confidence that I
can use it to build the userland for Nine-E as well
as to compile the filesystem code to put into ROM.

While at CDG I looked at the 2.11BSD library functions
and the /usr/include. I think I'll add system calls
to the Salmi simulator and slowly bring up the userland.
Once I'm happy with the userland I can then work on
the ROM filesystem code to really implement the system
calls.

I still have to see if I can prod the CH375 to work
with the existing hardware, or if I need to redesign
the read/write decode logic to make it work.

## Mon 12 Jun 2023 13:06:24 AEST

Yay, I've got these variables/syscalls in Salmi and I can write
C code to call them: errno, exit, read, write, open,
close, putchar, printint.

## Tue 13 Jun 2023 11:51:26 AEST

I decided to give up on 2.11BSD's libc. I'm now using
the libc in FUZIX. I've spent several hours fixing up
the cmoc warnings and errors. Apart from a couple of
leftover warnings, I have no warnings/errors now and
I can build a libc.a. I haven't tested them yet.
I got a list of unknown symbols from the resulting .o
files, and here are the syscalls the functions need:

brk, close, dup2, execl, exit, fchmod, fclose, fork, getgrent, getpgrp, getpid,
getuid, ioctl, kill, lseek, mknod, open, pause, pipe, read, sbrk, signal,
stat, stime, sync, time, umount, unlink, utime, wait, waitpid, write

Obviously, some of them I won't implement like fork, pause, pipe, signal, wait.
A few I can make library functions like getuid(). So I can weed out some
of the libc functions, implement the sensible system calls in Salmi and
see how it all goes.

## Tue 13 Jun 2023 13:04:47 AEST

I added lseek() to Salmi and it works. I can open(), lseek(), read() and
write(1, ...) fine. I tried to use the FUZIX printf() but it doesn't work.
I wasn't expecting all the functions to work immediately. It will be a
matter of trying some and seeing what happens.

I've tried a few strXXX() functions and memcpy(), they work :-)
I renamed my functions to be myputchar(), myputs() and myprintf() for now!

## Tue 13 Jun 2023 14:10:17 AEST

Back to the address decode logic. Both the UART and the CH375 have RD# and
WR# lines. The CH375 has a CS# select line, but it can be tied low according
to the datasheet.

Here's my current idea. We keep the existing 2:4 decoder to make four 8K
areas above the 32K line. ROM still gets 8K.

We keep the second existing 2:4 decoder, which breaks the third 8K area
into four 2K areas. We change to an 8-input AND gate for RAM. We send a15,
two of the 8K decode lines and three of the 2K decode lines to the AND gate.
This gives us 32K + 16K + 6K = 54K of RAM.

This leaves a section of memory from $D800 to $DFFF unused.

Now we add in a 74LS138 3:8 decoder. The active high enable line is tied
to the E clock. One of the active low enable lines is tied to the Q2 (third)
output from the second 2:4 decoder. The other active low enable line is tied
low. Thus, this will decode addresses from $D800 to $DFFF but only when the
E clock is high.

For the 3-bit selector to the 74LS138 3:8 decoder, the lowest is ground,
the middle is address line a10 and the highest is the R/W line. The R/W
line now selects the bottom four outputs when it is high, the top four
when it is low.

R/W | A10 | Use
\----+-----+----
0  |  0  | UARTWR#
0  |  1  | CH375WR#
1  |  0  | UARTRD#
1  |  1  | CH375RD#

These can only go low when we have selected the corresponding address
range, E is high, and R/W is reading or writing (low).

So, I think this means that:

* $0000 to $D7FF is 54K of RAM
* $D800 write is UARTWR#, $D800 read is UARTRD#
* $DC00 write is CH375WR#, $DC00 read is CH375RD#
* $E000 to $FFFF is  8K of ROM

I've drawn it up on paper, but I need to implement it in the Verilog
model with the cycle accurate 6809 model to ensure that it works.

## Tue 13 Jun 2023 15:06:21 AEST

I've quickly added it to the Kicad schematic, only eyeballed it so far.
It needs a full check, plus updating the PCB layout etc.

## Wed 14 Jun 2023 12:03:59 AEST

Argh, I thought the 74LS30 was an 8-input AND but it's a NAND. No
worry, I can use both halves of the 74LS21 4-input AND instead.

I've rewired the Verilog model with the new decoding design and
I've checked & verified that the UART read and write stobes only
occur when the E clock is high. Ditto the CH375 strobes.

## Thu 15 Jun 2023 11:33:50 AEST

I've pulled out the existing PCB with the 14MHz crystal. I've put
the CH375 device on the board and I'm running a test to prod the
CH375 to see if I get anything:

```
chdatard equ    $D802
chdatawr equ    $D804
chcmdrd  equ    $D803
chcmdwr  equ    $D805
...
        lda     #$22            ; Send CMD_GET_STATUS to the CH375
        sta     chcmdwr
...
        lda     chdatard        ; Get the result back
...
        lda     #$01            ; Send CMD_GET_IC_VER to the CH375
        sta     chcmdwr
...
        lda     chdatard        ; Get the result back
```

But the result is:

```
About to send CMD_GET_STATUS                                                    
Got back 00                                                                     
About to send CMD_GET_IC_VER                                                    
Got back 00
```

I wish I'd kept a copy of the output when I did this with the Pico W
because then I'd know what to expect. I also set up an FIRQ handler
which should print out "FIRQ!" if we receive one. So, it's likely
that the board is sending gibberish to the CH375 like the UART,
because the CH375 chip select, RD# and WR# lines go active before E
goes high, and so the data bus has rubbish on it.

I'll pull out a logic analyzer and read the lines to see what's on them.

Aside: the board runs with the four crystals: 1MHz, 2MHz, 3.57MHz and
14\.75MHz. However, I can only paste an s19 binary into the monitor when
I'm running with the 14.75MHz crystal. I wonder if there's a way to tell
Minicom to put in delays between characters?

## Thu 15 Jun 2023 12:50:07 AEST

I've got the Bitscope Micro out to look at things. Yes, definitely the
chip select on the CH375 is going low when E is low. It stays low for
roughly 335nS. With a 14.7456MHz clock cycle, divided by four, each cycle is
271nS long. So I'm guessing that, at the Bitscope Micro's resolution, these
are equivalent.

I can see the E clock signal go up, then down again, slap bang in the
middle of the period when the CH375 chip select is down. Thus: the CH375
is being told to read data from the bus when it's not yet ready. Damn.
So I will definitely need to design, order, get a new PCB. Sigh.

## Thu 15 Jun 2023 13:09:04 AEST

Back to the software side for a bit. The next problem is how to determine
where the BSS ends, so I can set the initial brk value when I run a
program. I checked the S19 binaries and they don't include the BSS content
(of course), so there's no way to tell with S19 binaries. The lwlink tool
can do these output formats: decb, raw, lwex, os9, srec (s19). Now I need
to a) research which of these records the BSS information and b) then get
cmoc to tell lwlink to output in that format.

Did the above, no great luck. However, when I compile a C program to .o
format with `cmoc` and do this:

```
$ lwobjdump fred.o | grep -i bss
SECTION bss
    FLAG: BSS
        \02bss=0000
        bss_end=8001
        bss_start=0000
```

So I think I can do brk(bss_end+1) at the start of each program. Yes!
I don't have a brk() syscall yet, but I've added this code into crt0.s:

```
l_bss   IMPORT				; number of bytes in "bss" section
s_bss   IMPORT				; address of "bss" section

	SECTION code
INILIB
	export INILIB
	ldx	#(s_bss+l_bss+1)
```

From here, I can call a brk() system call to set where the heap is. Yay!!

2\.11BSD only has an sbrk() syscall, no brk() syscall. The Linux manpage
says:

```
int brk(void *addr);
   brk()  sets the end of the data segment to the value specified by addr,
   when that value is reasonable, the system has enough  memory,  and  the
   process does not exceed its maximum data size (see setrlimit(2)).

void *sbrk(intptr_t increment);
   sbrk() increments the program's data space by increment bytes.  Calling
   sbrk() with an increment of 0 can be used to find the current  location
   of the program break.
```

Can I implement brk() using sbrk()? I'm going to assume, at the start of
a program's execution that the brk value is zero. I can then do:

```
ldx     #(s_bss+l_bss+1)
call sbrk() to add this value to zero, setting the new brk value
```

Then, when we call sbrk(0), we get the current brk() value back. Now,
how to implement brk()?

```
int brk(void *addr) {
  void *curbrk, *newbrk;
  int change;

  curbrk= sbrk(0);			// Get the current brk value
  change= (int)addr - (int)curbrk;	// Determine the difference
  newbrk= sbrk(change);			// Try to move to that point
  if (newbrk== (void *)-1)		// Failure, return -1
    return(-1);				// and ENOMEM is already set
  else
    return(0);
}
```

cmoc compiles this, and I've added this to the library. Now to add
sbrk() to the 6809 simulator. Done. I'm running a program which
simply does printint(23), and I can see the sbrk() syscall with
the initial break value. But for some reason, sbrk() is called
several times which I don't know why.

Fixed. in crt0.s I pushed the sbrk() argument and forgot to pull
it back off the stack. It's working now. That means I can try
getting malloc() to work! Ooooh!

## Thu 15 Jun 2023 14:58:43 AEST

Trying malloc out. I malloc four areas and print their base addresses
out. I was worried that they were descending, so I added debug code
to sbrk() in Salmi. I'm seeing:

```
sbrk: change 908 newbrk 908		from crt0.s
sbrk: change 0 newbrk 908		from malloc
sbrk: change 0 newbrk 908		from malloc
sbrk: change 512 newbrk 1420		from malloc

1388	the four addresses
1364
1348
1244
```

So malloc() requests a 512-byte arena, and then allocates it in descending
order. Fine!!

I tried fopen(), fclose(), fread(), they work. Yay!!!

## Sat 17 Jun 2023 11:56:34 AEST

I did a bit of Kicad work on v2 of the PCB, need to do the freerouting and
copper pours, and some checking.

Over to the software. I want to add main(int argc, char *argv) to Salmi,
so I can run 6809 programs from the Linux command line, e.g. cat, cp.
The calling convention looks simple: push the base address of the array,
push the argc value.

I can rework the code from Apout which sets up the PDP-11 stack with argc/argv.

## Sat 17 Jun 2023 12:52:05 AEST

I've added the code and, in gdb, checked what's on the stack and it looks
good. I've compiled a main() with argc/v with cmoc. When I run it, the
argc is definitely getting passed. But the argvs don't seem to be there;
nothing gets printed when I do a puts(argv[i]). Hmm.

Ah, I had to push a pointer to the base of the argv[] list, not just have
the argv[] list straight after the argc. Works now. Yay!

## Tue 20 Jun 2023 10:12:04 AEST

I was looking at which filesystem code to use. I've gone for retro-fuse
(https://github.com/jaylogue/retro-fuse) because I can mount filesystems
under Linux easily, and it's already designed to be included into an
existing code base. So I can add it to Salmi etc. as I proposed above.

I think I'll use the 2.11BSD filesystem as I'm sure the filenames will
be larger than V7. I've compiled the code under 64-bit Linux and we get
these sizes:

```
$ size bsd211*o
   text	   data	    bss	    dec	    hex	filename
   3956	    140	  69128	  73224	  11e08	bsd211adapt.o
  10702	     64	      4	  10770	   2a12	bsd211fs.o
   8363	    360	      0	   8723	   2213	bsd211fuse.o
```

The actual fs code is only 10K. I'm pretty sure, with a 16-bit compiler,
that will shrink. I'm not concerned about the bss size in bsd211adapt.o
as I can tune down the number of buffers.

Here's a list of the filesystem 'calls' from bsd211fs.c:

```
extern int bsd211fs_init(int readonly);
extern int bsd211fs_shutdown();
extern int bsd211fs_mkfs(uint32_t fssize, uint32_t iratio, const struct bsd211fs_flparams *flparams);
extern int bsd211fs_open(const char * name, int flags, mode_t mode);
extern int bsd211fs_close(int fd);
extern off_t bsd211fs_seek(int fd, off_t offset, int whence);
extern int bsd211fs_link(const char *oldpath, const char *newpath);
extern int bsd211fs_mknod(const char *pathname, mode_t mode, dev_t dev);
extern ssize_t bsd211fs_read(int fd, void *buf, size_t count);
extern ssize_t bsd211fs_pread(int fd, void *buf, size_t count, off_t offset);
extern ssize_t bsd211fs_write(int fd, const void *buf, size_t count);
extern ssize_t bsd211fs_pwrite(int fd, const void *buf, size_t count, off_t offset);
extern int bsd211fs_truncate(const char *pathname, off_t length);
extern int bsd211fs_ftruncate(int fd, off_t length);
extern int bsd211fs_stat(const char *pathname, struct stat *statbuf);
extern int bsd211fs_unlink(const char *pathname);
extern int bsd211fs_rename(const char *oldpath, const char *newpath);
extern int bsd211fs_chmod(const char *pathname, mode_t mode);
extern int bsd211fs_chown(const char *pathname, uid_t owner, gid_t group);
extern int bsd211fs_utimens(const char *filename, const struct timespec times[2]);
extern int bsd211fs_access(const char *pathname, int mode);
extern ssize_t bsd211fs_readlink(const char *pathname, char *buf, size_t bufsiz);
extern int bsd211fs_symlink(const char *target, const char *linkpath);
extern int bsd211fs_mkdir(const char *pathname, mode_t mode);
extern int bsd211fs_rmdir(const char *pathname);
extern int bsd211fs_enumdir(const char *pathname, bsd211fs_enum_dir_funct enum_funct, void *context);
extern int bsd211fs_sync();
extern int bsd211fs_fsync(int fd);
extern int bsd211fs_statfs(const char *pathname, struct statvfs *buf);
extern int bsd211fs_setreuid(uid_t ruid, uid_t euid);
extern int bsd211fs_setregid(gid_t rgid, gid_t egid);
extern int bsd211fs_setgroups(size_t size, const gid_t *list);
extern void bsd211fs_refreshgroups();
extern int bsd211fs_adduidmap(uid_t hostuid, uint32_t fsuid);
extern int bsd211fs_addgidmap(uid_t hostgid, uint32_t fsgid);
```

There's a whole bunch I can lose when the code goes into the monitor:
mknod(), chown(), utimens(), access(), setreuid(), setregid(),
setgroups(), refreshgroups(), adduidmap(), addgidmap().

## Tue 20 Jun 2023 11:21:16 AEST

Wow. I've just been able to write a simple program that dsk_open()s
a 2.11BSD disk image, then init()s the 2.11BSD filesystem layer
(effectively mounts as /), then open() a file, read() from it,
close() it and shutdown() the filesystem. That's excellent :-)

Now I can look at trying to include this into Salmi. I wonder if I
can make a library to do this?

The nice thing is that there is no FUSE code in the resulting binary.

## Tue 20 Jun 2023 12:26:36 AEST

Wow!!! I just got the BSD filesystem integrated into Salmi to the
point where I can open(), lseek(), read() and close() a BSD file
with a 6809 binary. Yay!! I also set up the Salmi simulator so that
I can choose to use a BSD filesystem image, or the real Linux filesystem,
with a command-line argument.

Now I can't wait for the V2 PCB and '138 demuxers to arrive, so I can
build the new SBC and see if I can prod the CH375 and make it work.

## Tue 20 Jun 2023 12:36:12 AEST

Damn. There's this warning in the retro-fuse code:

```
/* Don't even think about trying to use this code on a big-endian machine. */
```

I'm guessing that either a) the underlying fs structures on disk are
little-endian and/or b) there's some 2.11BSD kernel code that relies
on the system being little-endian.

That's a problem because the 6809 is big-endian. I wonder if the above
will cancel out, i.e. we can mkfs a filesystem which is big-endian
and then just use it with the 6809 code?! No idea.

Looking at the 2.11BSD code, wow it's complicated. That's because it's
been pulled directly out from a full kernel.

I'm now considering using the original on-disk structures from either 2.11BSD
and/or V7, looking at the kernel code with my eyes, and rewriting it to have
just the essential code for Nine-E. That's going to be a lot of work. I can
probably use the Lion's Commentary as I go :-) V7 filenames are 14 bytes long,
which should be enough. I wonder what the biggest filesystem is, not that
I'm going to exceed it ever!

## Wed 21 Jun 2023 10:33:31 AEST

I've made a start at the fs code. I've got a very basic mkfs written
but I'm sure that it will have a heap of bugs in it. I haven't written
any code to use the fs once it's created.

## Thu 22 Jun 2023 13:43:05 AEST

I'm up to the block cache of 4 blocks and allocating a block from the
free list. I've fixed a few bugs and now I can allocate up until there
are no blocks left. Yay!

## Fri 23 Jun 2023 09:20:16 AEST

Now trying to get the code to free an in-use block correct, but it is
getting it wrong. Sigh. OK, now fixed (I hope!).

I just compiled umfs.c with cmoc to a .o file. lwobjdump says that there's
0x4A4 bytes of code which is 1,188 bytes. That's good because I don't think
I'm going to write so much more code that it will fill the 8K of ROM :-)

## Sat 24 Jun 2023 16:19:15 AEST

Added a locked bit for the block buffers, so that we can keep
one in while freeing the blocks in an i-node.

The v2 PCBs have arrived, not sure when I'll get a chance to solder
everything on to one of them.

## Sun 25 Jun 2023 16:04:40 AEST

Too busy yesterday & today doing horse stuff. I'm now looking at using
the xv6 filesystem code as it's roughly similar to what I'm doing and it's
tested. I've git cloned the x86 version and have it running. Now I want to
try and remove things I'll never use, and keep building it until I hit
something I can't remove :-)

## Mon 26 Jun 2023 12:08:39 AEST

I trimmed param.h down yesterday and kept xv6 working. Today, I've copied
the fs code out to a new directory, removed all the lock stuff, got it
to compile with no errors. The mkfs.c program still makes an image 100%
identical to the xv6 one. Now to start trying to get the code to run
standalone!

## Mon 26 Jun 2023 14:51:31 AEST

Woohoo, I can do cat(1) and the xv6 ls(1). The latter is identical to
the actual output, except for the console line:

```
$ ./ls
sb: size 1000 nblocks 963 ninodes 200 nlog 8 logstart 2 inodestart 10 bmap start 36
.              1 1 512
..             1 1 512
README         2 2 2286
cat            2 3 16712
echo           2 4 15600
forktest       2 5 9308
grep           2 6 19884
init           2 7 16132
kill           2 8 15660
ln             2 9 15528
ls             2 10 18112
mkdir          2 11 15688
rm             2 12 15664
sh             2 13 31496
stressfs       2 14 16540
usertests      2 15 52820
wc             2 16 17288
zombie         2 17 15240
```

## Tue 27 Jun 2023 10:10:31 AEST

I now have these programs that work: mkdir.c, catinto.c, cat.c, usertests.c.
catinto takes a Linux file and copies it into the simulated filesystem. For
usertests.c, I had to remove the non-FS tests and the FS tests that require
fork(), as the 6809 won't have fork. They all work.

Not sure what to do next. I want to try cmoc on the C files, but I want
to ensure that I'm using uint16_t where I can. At the moment, types.h is:

typedef unsigned int   uint;
typedef unsigned short ushort;
typedef unsigned char  uchar;

Maybe I should define things like off_t, size_t etc. Not sure. I will have
to, eventually.

## Tue 27 Jun 2023 10:28:35 AEST

I got cmoc (cfm) to compile the C files from xv6 with a few warnings.
Here are the code sizes:

```
bio.o: SECTION bss CODE 127E bytes	// Global initialisation, I guess
	SECTION rwdata CODE 0000 bytes
	SECTION rodata CODE 0011 bytes
	SECTION initgl CODE 0000 bytes
	SECTION code CODE 01B1 bytes
file.o: SECTION bss CODE 00E8 bytes
	SECTION rwdata CODE 0000 bytes
	SECTION rodata CODE 0035 bytes
	SECTION initgl CODE 0000 bytes
	SECTION code CODE 023D bytes
fs.o: SECTION bss CODE 01C8 bytes
	SECTION rwdata CODE 0000 bytes
	SECTION rodata CODE 00FE bytes
	SECTION initgl CODE 0000 bytes
	SECTION code CODE 0D17 bytes
log.o: SECTION bss CODE 001C bytes
	SECTION rwdata CODE 0000 bytes
	SECTION rodata CODE 005B bytes
	SECTION initgl CODE 0000 bytes
	SECTION code CODE 0398 bytes
sysfile.o: SECTION bss CODE 0020 bytes
	SECTION rwdata CODE 0000 bytes
	SECTION rodata CODE 0063 bytes
	SECTION initgl CODE 0000 bytes
	SECTION code CODE 078F bytes
```

That's a total of 13,208 bytes. If I take out the bss code, I get 8,104 bytes.
I think that's a good result. I'm sure I will be able to tighten up the code
in places.

## Wed 28 Jun 2023 10:12:31 AEST

I got time this morning to build the v2 PCB. It works! I've modified the
monitor with the new I/O locations, burned the ROM and it booted up first
time with a working monitor. This is at 1MHz. I also tried the minicom
1ms transmit delay, and I can defintely send an s19 file up to the board
without dropping any characters.

I also checked that I could write to the RAM addresses just below the I/O
space ($D800) and that also works. Now to try the 14MHz crystal ... It
certainly does, fantastic!

## Wed 28 Jun 2023 17:07:53 AEST

I updated the monitor code to have a getchar() syscall via SWI and also an
exit() syscall which simply restarts the monitor. I also moved a few things
around.

Now trying the CH375 test code I wrote before, with the new I/O addresses.
So far I get:

```
About to send CMD_GET_STATUS                                                    
Got back BD                                                                     
About to send CMD_GET_IC_VER                                                    
Got back B7
```

I wish I'd kept the output from the Pico from before, sigh :-(
If worse comes to worse, I can break out a logic analyser to see what I'm
sending to the device.

## Thu 29 Jun 2023 09:41:39 AEST

I'm getting more comfortable with the above. The user manual mentions $B7
as a version number. I also just tried using the CHECK_EXIST command, which
takes a data byte, inverts it and returns it. I send this data and got this
result: 93 -> 6C, or

```
10010011	93
01101100	6C
```

There's this C code from one of the CH375 Zip files: CH375EV4.C and .H.
I wonder if I could get it to compile with cmoc?

## Thu 29 Jun 2023 10:14:13 AEST

Progress. In the FIRQ handler I'm now sending CMD_GET_STATUS to disable
the interrupt and I get back the status. Doing this, I now get:

```
About to send CMD_GET_STATUS                                                    
Got back 15                                                                     
About to send CMD_CHECK_EXIST                                                   
Got back 6C                                                                     
About to send CMD_GET_IC_VER                                                    
Got back B7                                                                     
About to send CMD_SET_USB_MODE and 6                                            
FIRQ: 15
```

and 15 says that a USB key is detected. I'll try without a key ... With
no key, I see no FIRQ and this:

```
About to send CMD_GET_STATUS                                                    
Got back BD                                                                     
About to send CMD_CHECK_EXIST                                                   
Got back 6C                                                                     
About to send CMD_GET_IC_VER                                                    
Got back B7                                                                     
About to send CMD_SET_USB_MODE and 6                                            
Got back 51
```

I just modified the FIRQ handler to do nothing but store the result in
a 'status' location in memory. I initialise it to $EE before I run the
code. With a key, I get:

```
About to send CMD_GET_STATUS                                                    
Got back 15                                                                     
About to send CMD_CHECK_EXIST                                                   
Got back 6C                                                                     
About to send CMD_GET_IC_VER                                                    
Got back B7                                                                     
About to send CMD_SET_USB_MODE and 6                                            
Got back 15
```

Without a key, I get no FIRQ and:

```
About to send CMD_GET_STATUS                                                    
Got back BD                                                                     
About to send CMD_CHECK_EXIST                                                   
Got back 6C                                                                     
About to send CMD_GET_IC_VER                                                    
Got back B7                                                                     
About to send CMD_SET_USB_MODE and 6                                            
Got back EE
```

## Thu 29 Jun 2023 10:49:43 AEST

Now I'm sending CMD_DISK_INIT, waiting for the FIRQ handler to set the
result, and I get back 14 which means USB_INT_SUCCESS, yay! This is at
1MHz, so let's try 14MHz. Yes:

```
About to send CMD_GET_IC_VER, Got back B7                                       
About to send CMD_SET_USB_MODE and 6, Go back 15                                
Status: 15                                                                      
INIT_DISK Status: 14
```

## Thu 29 Jun 2023 10:58:33 AEST

I can get the disk's size (I think):

```
0801DD7F
FF000002
```

The datasheet says: The first 4 bytes constitute double-word data with
high bytes in the front, which is the total number of sectors of USB
storage device. The last 4 bytes constitute the double-word data with
high bytes in the front, which is the number of bytes of each sector.

The device is 64G. $0801DD7F is 134,339,967 decimal. Multiply by 512
and I get 68,782,063,104 which is 64G :-)

Not sure whay I'm getting 02 as the bytes in each sector? It's the same
at 1MHz, so it's not a clock speed issue.

## Thu 29 Jun 2023 15:01:14 AEST

I wrote a puts("Hello world\\n") program and compiled it with cmoc (cfm).
It runs under Salmi but not on the real board. So, something is different
between the two which I need to isolate.

Now I have an assembly file which has the myputchar() syscall, the
cmoc assembly of puts() which calls myputchar, and a cmoc main()
which calls puts(). I'm assembling my hand and it runs under Salmi.

## Thu 29 Jun 2023 16:41:26 AEST

Ah. The monitor uses the A register for character I/O, but the C library
is passing characters as int16s, so A==0 and B holds the character. Damn.
So I have to rewrite the monitor to use the B register. It's going to be
fiddly. Or, I rewrite the syscall code :-)

## Thu 29 Jun 2023 17:15:31 AEST

I rewrote the syscall code of course. The C runtime also calls \_sbrk(),
so I added a no-op handler in the monitor. I've re-burned the ROM with
the new monitor, and I can run this C program on the hardware:

```
int main() {
  myputs("Hello world\r\n");
  return(0);
}
```

## Fri 30 Jun 2023 10:17:32 AEST

I've split out the syscalls into separate .s files, and split myprintf()
and myputs() similarly. This is to get the linker to not include so much
code in the final executable. I also added printint() as a monitor syscall
but it calls prhex so the output is in hex not decimal. Again, to try and
make the C code produce a smaller binary.

I can now run this:

```
// Given a command and a string, print the string, send the
// command, get data back and print it in hex.
void cmddata(int cmd, char *str) {
  char data;
  myputs(str);
  *chcmd= (char) cmd;
  data= *chdata;
  printint(data);
  myputchar('\n');
}

int main() {
  cmddata(CMD_GET_IC_VER, "IC VER: ");
  return(0);
}
```

and I get "IC VER: FFB7", which should be just 00B7. OK, a bit of C
twiddling and I now get "IC VER: 00B7".

## Fri 30 Jun 2023 17:46:03 AEST

I'm trying to send CMD_SET_USB_MODE with data 6 to the CH375. This should
provoke an interrupt. In the ROM I now have:

```
		org     $d780
chstatus        fcb     #$00            ; CH375 status after an FIRQ
; When we get a fast IRQ, send CMD_GET_STATUS to the
; CH375 to stop the interrupt. Get the current status
; and store it in chstatus. Push/pop A to ensure it's intact
ch375firq       pshs    a
                lda     #$22            ; CMD_GET_STATUS
                sta     chcmdwr
                lda     chdatard        ; Get the result back
                sta     chstatus
                puls    a
                rti
```

When I try to do this (read from chstatus after setting it to dummy 0xee):

```
void cmdargstatus(int cmd, int arg, char *str) {
  *chstatus= 0xee;
  myputs(str); *chcmd= (char) cmd; *chdata= (char)arg;
  while (*chstatus == 0xee) ;
}

int main() {
  int status;
  cmddata(CMD_GET_IC_VER, "IC VER: ");
  cmdargstatus(CMD_SET_USB_MODE, 6, "USB_MODE: ");
  status= *chstatus;
  prbyte(status);
  return(0);
}
```

I keep getting the 0xee back at prbyte() even though the while loop will
break once it stops being 0xee. In the monitor I can do:

```
> dd780                                                                         
D780: 15 00 0D D7 80 80 09 06 C7 1E 6E 2F 0F 42 E4 F7
```

so I know the FIRQ handler fired. I suspect that the compiler is
optimising the access to chstatus and not generating code to get
the latest value. cmoc also doesn't know about volatile, sigh.

## Sat 01 Jul 2023 10:03:34 AEST

I've gone back to the assembly code that tests the CH375, but with
the FIRQ handler in ROM. The latter prints out a '@' when it gets run.
So I'm seeing:

```
CMD_SET_USB_MODE and 6, Go@t back 15                                            
Status: 15                                                                      
INIT_DISK, @Status: 14                                                          
SIZE_DISK, @Status: 14                                                          
0801DD7FFF000002
```

which shows that the FIRQ handler being run quite a while after we start
doing the puts() code in ROM.

I'm thinking of putting some CH375 routines in ROM. For the commands that
cause an interrupt, I'll loop in assembly until the status from the FIRQ
handler gets set. That way, I can call the routines from C and know that
I actually have a result.

Just reading the CH375 datasheet. The commands that generate interrupts
either send no (data) arguments with the command, or five bytes which
indicate the LBA and number of sectors to read/write. So I can make two
ROM subroutines to do these, then loop waiting for the FIRQ status to change.

## Mon 03 Jul 2023 11:51:06 AEST

Argh, I think I'm winning a bit!!! The first sector on the USB key has this 16 bytes
at the start (using `hd`):

```
23 23 20 46 72 69 20 31  32 20 4d 61 79 20 32 30  |## Fri 12 May 20|
```

I'm doing this in fred.c:

```
  // Read and print sector 0
  x= ch375cmdargstat(CMD_DISK_READ, 0, 1);
  prhexword(x); romputs(crlf);
  chcmd= CMD_RD_USB_DATA;
  for (int i=0; i<16; i++) {
    x= ch375getdata(); prhexword(x);
  }
```

and I'm getting this:

```
00400023002300200046007200690020003100320020004D0061007900200032
```

Lots of leading 00s as I print everything out as 16-bit ints. The 0040 is the
output from the ch375cmdargstat() which I guess is 64 bytes of data to read.
Skipping the 00's, we have:

```
23 23 20 46 72 69 20i 31 32 20 4D 61 79 20 32
```

which means that we read from sector 0. Yay!!

But I can't read sector 0, the program just locks up. I might go back to assembly code
for a while and try reading there.

## Mon 03 Jul 2023 13:06:54 AEST

Trying to work out the endianness of the CH375. The example test code has this:

```
UINT8   mReadSector( UINT32 iLbaStart, UINT8 iSectorCount, UINT8X *oDataBuffer ) {              

        UINT16  mBlockCount;
        UINT8   c;
        CH375_WR_CMD_PORT( CMD_DISK_READ );  
        CH375_WR_DAT_PORT( (UINT8)iLbaStart );  
        CH375_WR_DAT_PORT( (UINT8)( iLbaStart >> 8 ) );
        CH375_WR_DAT_PORT( (UINT8)( iLbaStart >> 16 ) );
        CH375_WR_DAT_PORT( (UINT8)( iLbaStart >> 24 ) );  
        CH375_WR_DAT_PORT( iSectorCount );
```

which seems to indicate that we send the low bytes of the LBA first, then the sector
count. OK, the manual says:

```
This command requires to input 5 data, respectively the lowest
byte of LBA address, the lower byte of LBA address, the higher
byte of LBA address, the highest byte of LBA address and the
number of sectors in turn.
```

So little-endian for this command. But, for the DISK_SIZE command, it says:

```
The data is usually 8 bytes. The first 4 bytes constitute
double-word data with high bytes in the front, which is the
total number of sectors of USB storage device. The last 4 bytes
constitute the double-word data with high bytes in the front,
which is the number of bytes of each sector.
```

So that's big-endian. Argh!!

I think it's a good idea to do a reset on the CH375 because it gets stuck
in a state if I talk to it incorrectly. We need to do a 40ms delay after
sending the reset. Damn! I wonder if I can send reset then GET_STATUS
and loop until the result changes? Or even just send reset and loop
until the data read changes?! Worth trying.

## Fri 07 Jul 2023 09:25:41 AEST

I've got a delay loop now and that works. I'm writing code for Salmi to
simulate the CH375. I just realised that when I get RD_USB_DATA, the first
byte is the number of bytes to read. So the previous disk size is not
0801DD7F but 08 then 01DD7FFF, which is 31,293,439 blocks times 512 or 16G.
It also means that the block size doesn't start with FF, but 00.

## Fri 07 Jul 2023 12:54:02 AEST

OK, I now have Salmi reading disk blocks which seems to be compatible
with the real CH375. I need to pull the hardware back out to compare.
Here is what the simulator outputs:

```
SET_USB_MODE 6, @Got back 15
INIT_DISK, @SIZE_DISK, @Status: 15
080000005A00000200
@40without even the implied warranty of
    MERCHANTABILITY or FITN
```

I now read 9 bytes for the disk size, hence the 08 which is the byte count
and now we get 00000200 as the block size, yay!

I also read the first byte from RD_USB_DATA which is the $40 after the @.
So, we receive an interrupt, then we are told to read $40 (64) bytes. I
also now put the count into the B register and loop reading that much from
the CH375.

Next:

* compare against the real hardware
* write the loop to read the eight 64 byte buffer to get a whole block!
* compare hardware vs simulator again

## Fri 07 Jul 2023 14:44:57 AEST

I've run testch375.asm version 1.15 on both Salmi and the hardware with
the same USB contents. Here is the hardware output:

```
SET_USB_MODE 6, Go@t back 15                                                    
INIT_DISK, @SIZE_DISK, @Status: 14                                              
0801DD7FFF00000200                                                              
@40reate the CE signals for the RAM, ROM and UART.                              
```

and here is the Salmi output:

```
SET_USB_MODE 6, @Got back 15
INIT_DISK, @SIZE_DISK, @Status: 15
080000008900000200
@40reate the CE signals for the RAM, ROM and UART.
```

Differences:

* The hardware gives status 14 after disk size, Salmi gives 15. Now fixed.
* I didn't have \\n mapped to \\r\\n in Minicom, hence the date over on the right
* The interrupts come later in the hardware than with Salmi
* The real USB key is bigger than the USB image in Salmi

Otherwise good!

## Sat 08 Jul 2023 10:34:37 AEST

Yay. I now have an asm routine which gets an LBA block number and a
buffer start address on the stack. On both Salmi and the hardware I can
read the entire block. I had a few issues to start with, but now it's
reliable. The only difference now between the Salmi and real CH375 is
size of USB key and when interrupts arrive.

At present the subroutine just prints out the bytes. I'll go change it
to actually store in the buffer. Done. Now the test program calls the
readblock subroutine, pushing the LBA and buffer address beforehand, then
prints out the block afterwards.  Works on Salmi and the SBC reliably.

## Sun 09 Jul 2023 11:20:18 AEST

I've added the write code to Salmi but I haven't tested it yet.

## Sun 09 Jul 2023 13:16:29 AEST

Got the write code working on Salmi. It also works on the hardware,
and I see this debug pattern: 0@1234@234@234@234@234@234@234@23
which indicates the eight stages and the interrupts.

0 - before setting chstatus fo #$ff
1 - after sending DISK_WRITE
2 - after the interrupt after the DISK_WRITE
3 - after sending 64 bytes
4 - before we send the DISK_WR_GO

But if I do a read first, I see this: 01

which indicates that we are not getting the interrupt after the disk write.

I just also tried to do consecutive reads and the second one stalls on
the hardware. So there is something I'm not doing at the end of a read
which is preventing the next operation.

## Sun 09 Jul 2023 13:51:46 AEST

I got it working on the hardware. Forgot looping for eight times,
just read the status and keep looping on USB_INT_DISK_READ/WRITE.
Now I need to get this working on Salmi.

## Sun 09 Jul 2023 16:09:26 AEST

Got Salmi working fine. I then moved the CH375 init/read/write subroutines
into ROM so that they can be called from C. The test assembly is now quite
small :-)

So, I wonder if we can get a read block, modify, write working in C?

## Mon 10 Jul 2023 08:49:55 AEST

Yes :-) I can do this:

```
int main() {
  char buf[512];
  int err= ch375init();
  romputs("In: "); prhexword(err); romputc('\n');
  err= readblock( (long)1, buf); romputs("Rd: "); prhexword(err); romputc('\n');
  for (int i=0; i<512; i++) { buf[i] |= 0x20; } romputc('\n');
  err= writeblock( (long)1, buf); romputs("Wr: "); prhexword(err); romputc('\n');
  err= readblock( (long)1, buf); romputs("Rd2: "); prhexword(err); romputc('\n');
  for (int i=0; i<512; i++) { romputc(buf[i]); } romputc('\n');
  return(0);
}
```

On both Salmi and the readl hardware, I see:

```
> g0000
@@In: 0001
@@@@@@@@@Rd: 0001

@@@@@@@@@Wr: 0001
@@@@@@@@@Rd2: 0001
<512 bytes of lowercase>
```

I just checked the blocks on the real USB key and block 1 is definitely now all
lowercase, so the C code did actually write to the key. Fantastic!!!

Now I can work with the filesystem code on Salmi and, now and then, take it over
to the real hardware to check that it works. I also need to modify mkfs.c to
write big-endian fields.

## Mon 10 Jul 2023 09:11:42 AEST

Well, looks like mkfs.c is already set up with little-endian functions and
all fields are converted through these functions. So it's easy to write
substitute functions to do the big-endian conversion.

## Mon 10 Jul 2023 10:01:15 AEST

I'm starting work on the xv6 code. I rewrote blk.c to call the CH375 routines.
I've got a test program that calls bread() and bwrite() and both work on Salmi!
I had to modify Salmi to load a second s19 file as the cmoc binary files are
too big to paste.

## Mon 10 Jul 2023 10:25:58 AEST

More progress but not right. I'm now compiling fs.c to call iinit() which reads
in the superblock. On Salmi, I'm getting:

```
> g0000
@@@@@@@@@@@sb: size 2 nblocks 0 ninodes 0 nlog 1 logstart 0 inodestart 417 bmap start 0
```

which I'm sure isn't right! I just recompiled the working xv6 fs code, and I see:

```
sb: size 1000 nblocks 963 ninodes 200 nlog 8 logstart 2 inodestart 10 bmap start 36
```

Ahah. uint on 6809 is 2 bytes, on Linux it's 4 bytes. I think I'll have to redefine
uint as unsigned long on 6809.

## Tue 11 Jul 2023 11:13:01 AEST

I've done a first pass with the xv6 fs code to make ints 16-bit and to put in 32-bit
types for ino_t, blk_t and off_t. Of course, the usertests don't work yet. I've also
quickly recreated a working xv fs directory where usertests do work. So I can run
gdb on both versions concurrently and see where they differ in behaviour.

Ah, I changed direntries to 12 byte filenames. Now most of the usertests pass
once I rebuilt a filesystem with mkfs.

## Tue 11 Jul 2023 16:16:46 AEST

I now have usertests running with 16-bit (u)ints. Turns out that the filesystem
image has 8 log blocks, but param.h has 6 log blocks. Looks like we need eight.

Next to do: compile the programs like cat.c, catinto.c, mkdir.c and make sure
they work with 16 bit ints. Then go back through the whole code and look at
the Uint/Ints and see if they should be changed to one of the new 32-bit types.

## Tue 11 Jul 2023 16:42:15 AEST

I've gone back through the fs code and changed a few (U)Ints to another
type, all the tests still pass. Yay!

## Wed 12 Jul 2023 08:14:59 AEST

I've copied the 16xvfs code into the XV6FS directory, and used cfm to
compile all the fs routines. I made a few changes as per cfm's warnings.
I also copied them back to 16xvfs and checked that the usertests still
pass. I've rewritten the Makefile in XV6FS to build try.c and all the
routines with cfm. This opens README and prints it out. I now have a
6809 s19 binary! I also compiled mkfs to be big-endian and I've built
a fs.img with 12 byte filenames and big endian. So now to try it under Salmi!

What I get is:

```
> g0000
@@@@@@@@@@@sb: size 0 nblocks 1 ninodes 15660 nlog 202 logstart 203 inodestart 204 bmap start 205
@@@@@@@@@@@@@@@@@@@@@@@@@@@
```

Ah. mkfs.c was making big-endian fields and then using them when the code running
is little endian. Fixed I think. Now I get:

```
> g0000
@@@@@@@@@@@sb: size 1000 nblocks 963 ninodes 200 nlog 8 logstart 2 inodestart 10 bmap start 36
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@CH375 read error offset 8393728
```

so the superblock is OK.

## Wed 12 Jul 2023 16:44:50 AEST

I've added %X to cprintf() so I can print out longs in hex, e.g. block numbers.
I added debug statements to blk.c. On the 16-bit xvfs I see:

```
blkr 00000001
sb: size 1000 nblocks 963 ninodes 200 nlog 8 logstart 2 inodestart 10 bmap start 36
blkr 00000002
blkw 00000002
blkr 0000000A
blkr 00000025
blkr 00000026
blkr 00000027
blkr 00000028
blkr 00000029
blkr 0000002A
```

On the cfm version, I see:

```
blkr 00000001
sb: size 000003E8 nblocks 000003C3 ninodes 000000C8 nlog 00000008
logstart 00000002 inodestart 0000000A bmap start 00000024
blkr 00000002
blkw 00000002
blkr 0000000A
blkr 00000025
blkr 0000400A
CH375 read error offset 8393728
```

All the superblock values are correct. So it's the block read after 0x25.  gdb says:

```
#0  blkrw (b=0x55555555ba60 <bcache+2208>) at blk.c:44
#1  bread (blockno=38) at bio.c:91
#2  readi (ip=0x55555555c7cc <icache+76>, 
    dst=0x55555555b0c0 <buf> "", off=0, n=100) at fs.c:443
#3  fileread (f=0x55555555c540 <ftable>, 
    addr=0x55555555b0c0 <buf> "", n=100) at file.c:91
#4  sys_read (fd=0, p=0x55555555b0c0 <buf> "", n=100) at sysfile.c:70
#5  main () at try.c:27
```

Maybe I need to eyeball the fs.img file? Perhaps another mkfs bug?

## Wed 12 Jul 2023 17:09:44 AEST

I can't spot anything yet. Inode 1  is '/', inode 2 is "README" according to
the dirent entries that I can spot.

Inode 2 is:

```
00001480  00 02 00 00 00 00 00 01  00 00 08 ee 00 00 00 26  |...............&|
00001490  00 00 00 27 00 00 00 28  00 00 00 29 00 00 00 2a  |...'...(...)...*|
000014a0  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00
```

Type 2, size 0x08ee (2286), 1 link, blocks 0x26 to 0x2a. Maybe when we read
inode 2 we get it wrong? Or are we reading the content of / incorrectly?

## Wed 12 Jul 2023 17:37:25 AEST

In the directory lookup, 16xvfs does `README is inum 2`, but XVFS
does `README is inum 00020000`. And a `hd` on fs.img shows:

```
00004a20  00 02 00 00 52 45 41 44  4d 45 00 00 00 00 00 00  |....README......|
```

That should be `00 00 00 02` not `00 02 00 00`! So, a mkfs bug!

## Thu 13 Jul 2023 08:39:35 AEST

I fixed the mkfs bug, it was using xshort() not xint() on the inums in dirents.
So, progress but not there yet. The Salmi version of try is now doing:

```
> g0000
blkr 00000001
sb: size 000003E8 nblocks 000003C3 ninodes 000000C8 nlog 00000008 logstart 00000002 inodestart 0000000A bmap start 00000024
blkr 00000002
blkw 00000002
ilock 00000001 valid 0
  bread 00000001
blkr 0000000A
ino 00000001 addrs:
  00000025
  ...
blkr 00000025
README is inum 00000002
ilock 00000002 valid 0
  bread 00000002
ino 00000002 addrs:
  00000026
  00000027
  00000028
  00000029
  0000002A
  ...
ilock 00000002 valid 1
blkr 00000026
sb: size 000003E8 nblocks 000003C3 ninodes 000000C8 nlog 00000008 logstart 00000002 inodestart 0000000A bmap start 00000024
blkw 00000002
ilock 00000001 valid 1
README is inum 00000002
ilock 00000002 valid 1
ilock 00000002 valid 1
sb: size 000003E8 nblocks 000003C3 ninodes 000000C8 nlog 00000008 logstart 00000002 inodestart 0000000A bmap start 00000024
<repeats>
```

Not sure what's going on here. We do read blk $26, the first block from the file.
But no block reads after this. We seem to get into a loop re-reading the superblock.
The loop seems to be 16 times, not sure if this is a magic number or some xv6
constant.

## Thu 13 Jul 2023 09:21:23 AEST

I had the idea of changing Salmi to output the JSR calls with the called address
and a 16-byte dump of the stack at the time of the call. I also found that I can
get symbol output from `cmoc` with the `-i` option. So I can combine the two
with a Perl script to see the function calls and, by hand, work out the arguments
on the stack at the time. Perhaps even get the Perl script to `know' the arg
types for the functions?

## Thu 13 Jul 2023 10:51:51 AEST

I have modified Salmi and have a script to convert addresses to symbols. I've
got a call/return trace from the above bad run. We go off the rails after the
read of block $26.

## Thu 13 Jul 2023 11:05:14 AEST

Aargh. I found the problem!!! In try.c I was using write(), puts() and
putchar(), which are cmoc library functions not xv6 functions. So I
swapped to romputs() and romputc() and now I see the contents of the
README file! Yay!!!

## Thu 13 Jul 2023 11:14:15 AEST

I removed the '@' printing from the firq handler and the debug code in the xvfs,
and now I see the README file printed with no problems. Wow.

## Thu 13 Jul 2023 11:19:48 AEST

Woohoo, I modified ls.c to take no args and use the ROM functions. In Salmi:

```
sb: size 000003E8 nblocks 000003C3 ninodes 000000C8 nlog 00000008
logstart 00000002 inodestart 0000000A bmap start 00000024
.            1 1 512
..           1 1 512
README       2 2 2286
cat          2 3 16712
echo         2 4 15604
forktest     2 5 9312
grep         2 6 19884
init         2 7 16136
kill         2 8 15660
ln           2 9 15532
ls           2 10 18116
mkdir        2 11 15692
rm           2 12 15668
sh           2 13 31496
stressfs     2 14 16544
usertests    2 15 1912
wc           2 16 17288
zombie       2 17 15244
```

## Thu 13 Jul 2023 11:25:12 AEST

I made a program to mkdir("Foo") which worked (using `ls` to see), but
it took a long time to run. Not sure if there's a problem or not.

```
...
zombie       2 17 15244
Foo          1 18 32
```

I'm not sure what to do next :-) I could try the programs out on the
real hardware, of course.

I should start looking in to:

* writing a simple shell which loads and runs commands,
* getting exit() to reinvoke the shell
* seeing if I can fit the xvfs code into 8K of ROM
* how much RAM I need for xvfs data: buffers etc.
* can I lose the log.c code to make things smaller.

## Thu 13 Jul 2023 11:33:34 AEST

Damn. I updated the ROM on the SBC, loaded the `try` binary into mousepad,
selected all, then used L to load it onto the hardware. Then I did this:

```
> g0000                                                                              
bget: no buffers                                                                     
Warren's Simple 6809 Monitor, $Revision: 1.27 $
```

So, could it be the paste? Or is Salmi not simulating the hardware correctly?
Question, why did we get to bget() before printing the superblock? I pasted
with a 1ms character delay with no change. Sigh, it's going to be hard to
diagnose this one.

## Thu 13 Jul 2023 11:58:34 AEST

Let's park the hardware issue for now. I'm examining the lwobjdump values.
For the actual code, looks like 9175 bytes which won't fit into 8K :-)
In terms of biggest to smallest: fs.c 112C, sysfile.c 07CE, log.c 03D1,
file.c 02D5, cprintf 01C9, bio.c 01A3, blk.c 00AB.

cprintf() can go into RAM, saving 457 bytes. If we lose log.c (977 decimal)
that brings things down to 7741 decimal bytes.

That leaves 451 bytes of ROM. I'm thinking that I won't be able to put a
shell into the ROM. Instead, a "spawn" handler which copies arguments to
the top of the stack, loads an executable and then starts it running?

The next question is the RAM usage. Assume that we lose log.c, how much RAM?
The answer is 6258 bytes, with bio.c using 4725 bytes decimal. So, 1533 for
the rest of the code.

Can we lose log.c, drop the buffers down to 2048 (four 512-byte buffers),
which would take 3581 bytes of RAM overall?

Overall: we can fit the xvfs code into ROM along with boot code, IRQ handlers
and some code to set up argc/argv and load a named executable. We can
probably get away with 4K of RAM for the xvfs data structures. That gives
50K of RAM for programs.

## Thu 13 Jul 2023 12:47:48 AEST

Removing the log.c was easy. Replace log_write() with bwrite(), get rid of
begin_op() and end_op() and log_init(). I tried in the 16xvfs first, then
brought the changes over to XV6FS and checked try.c under Salmi, all OK.

I also found how to get cmoc to start code at a certain point and read-write
data at a certain point. So that opens the ability to move code into ROM.

## Thu 13 Jul 2023 12:59:36 AEST

I can drop NBUFS to 4 now, with no logging. That makes the obj sizes:

```
bio.o
SECTION bss CODE 0A41 bytes
SECTION rodata CODE 0011 bytes
SECTION code CODE 01A3 bytes
blk.o
SECTION rwdata CODE 0002 bytes
SECTION rodata CODE 0052 bytes
SECTION code CODE 00AB bytes
file.o
SECTION bss CODE 00E0 bytes
SECTION rodata CODE 0035 bytes
SECTION code CODE 02C8 bytes
fs.o
SECTION bss CODE 0302 bytes
SECTION rodata CODE 00AB bytes
SECTION code CODE 10E7 bytes
sysfile.o
SECTION bss CODE 0020 bytes
SECTION rodata CODE 0063 bytes
SECTION code CODE 0789 bytes
```

The .map file for try when I now build it has code+rodata from $0000 to $25E2.
rwdata+bss starts at $25E3 to $34A4. In decimal, that's 9699 bytes of code and
3777 bytes of data. Damn! I think I can reduce the rodata by shrinking a lot of
strings.

## Fri 14 Jul 2023 10:54:27 AEST

I worked out why the read() of README failed on the real hardware. I'd
already noticed that the RAM seems to have non-zero contents when I turn
on the SBC. So, I wrote a short routine to zero out all the RAM. Then I
loaded `try` and ran it, and it gave me the contents of README! So I should
add a ROM routine which I can call to clear the RAM, and I can call it on a
reset as well.

## Fri 14 Jul 2023 13:06:08 AEST

I just RCS tagged all the files in XV6FS with the "sbc_v1" tag to identify
the version of the code which now works on the actual hardware. The next task
is to reduce code & data enough to fit it into the 8K of ROM. I'll start with
the strings.

I've trimmed the strings, but the code size is still 9347, 1155 over budget.
The main culprits are:

```
4327 fs.o
1929 sysfile.o
 712 file.o
 419 bio.o
 238 signedDivDWordDWord.void_o
 171 blk.o
 170 try.o
 131 atexit.o
 109 memmove.o
```

We can discount 170 and 131 from try.o and atexit.o. Read-only data is:

```
  49 fs.o
  29 sysfile.o
  20 file.o
  16 blk.o
  14 try.o
   4 bio.o
```

which is negligible. So, the question is, how to lose about 1K of actual code?
The top symbols in size are:

```
 496 writei
 455 readi
 419 balloc
 365 bmap
 364 sys_unlink
 320 create
 314 filewrite
 289 itrunc
 267 bfree
 255 dirlink
 249 sys_open
 239 ilock
 232 sys_link
 231 ialloc
 210 dirlookup
 200 namex
```

## Fri 14 Jul 2023 14:44:34 AEST

I'm discouraged. I tried to build usertests with cmoc and run it on
Salmi. Some tests pass, but most seem to get into an infinite loop. Not
sure what to do. Sigh.

I just checked the pinout of the CH376 and it's compatible with the CH375.
The CH376 can read/write files on a FAT filesystem. So maybe I can ditch
the idea of a Unix filesystem in ROM. Instead, provide a Unix-like API
on top of the underlying FAT filesystem?

Available CH376 commands above the CH375 ones:

* get/set file size
* write request data
* write with offset?
* set filename
* mount disk
* open file
* enumerate files in dir
* create file
* delete file
* close file
* move offset, like lseek?
* byte read/write
* mkdir

And the xv6 API is:

* dup()
* read()
* write()
* close()
* fstat()
* link()
* unlink()
* open()
* mkdir() -> set filename
* chdir()

Argh. I don't think we can open multiple files
concurrently. That sucks.

## Fri 14 Jul 2023 16:58:27 AEST

I went back to the XV6FS code. I tried the native Linux 16 bit version
with 4 buffers, and it also hangs. But 6 works, so now back trying
6 buffers in the cfm version.

## Sat 15 Jul 2023 14:34:27 AEST

Most of the tests pass with 6 buffers. These don't:

```
  // writetest1();	FAIL	error: write big file failed: %i
  // bigfile();		FAIL	write bigfile failed
```

But the rest are OK, so I am heartened again.

I also tried making ino_t and blk_t 16 bits not 32 bits. The main code
size difference is fs.o: 3616 not 4327. That's a 711 byte saving! That
would mean only 64K inodes, and a disk size of 32M maximum. I could live
with that. But there might be savings elsewhere, too.

## Sun 16 Jul 2023 09:19:07 AEST

sizeof(struct dinode) is 64. So I should be able to do shifts and ANDs
instead of / and %, which might help to remove the use of
signedDivDWordDWord.void_o.

I just did an indent of all the C files as I was finding the inconsistent
formatting difficult.

## Mon 17 Jul 2023 08:06:35 AEST

I just symlinked most of XV6FS into 16xvfs, so that I can run usertests on Linux
and also build the code for the 6809, and not worry about keeping it all in sync.
The Makefiles are separate, mainly so I can build little/big-endian mkfs versions,
and hence little/big-endian filesystem images.

Just checked, I can run usertests on both Linux and 6809 using the same code.

## Mon 17 Jul 2023 13:25:50 AEST

With the empty try.c, I see:

```
Code size: 9192, (1000 over budget), data size 4727
```

That's with 131 for atexit.o, 28 for try.o, 26 for exit.o, 11 for \_exit.o.
I added some asserts in usertests.c to confirm that struct(dinode) size is 64.

## Mon 17 Jul 2023 15:41:50 AEST

I removed major/minor and now have Code size: 9114, (922 over budget),
data size 4727. About to try 16-bit xvino_t and xvblk_t. I can compile the
6809 fs code and I get: Code size: 8342, (150 over budget), data size 4659.
I can compile mkfs.c on the Linux side with 16-bit xvino_t and xvblk_t and the
usertests pass. I can't get it to compile with `cfm` yet. Fixed (I hope!).

It's interesting that signedDivDWordDWord.void_o takes 238 bytes of code, but
when I compile the C files down to assembly with `cfm`, there is no call to it.
So it must be used by a library function?

Yes, signedDivOrModOnDWord calls signedDivDWordDWord and fs.c calls the former.

Yay, I got rid of that by replacing `/ BSIZE` and `% BSIZE` with shifts
and ANDs. The usertests pass, and I now have:

```
Code size: 8026, (334 under budget), data size 4659
```

which also takes into account the size of the various exit functions we
don't need. So that might be enough to squeeze it all into ROM!!!

I should try running the usertests on Salmi now with this. That will also
test if the mkfs is working :-)

It fails in ilock(), so I suspect mkfs is wrong.

## Mon 17 Jul 2023 16:53:58 AEST

Comparing the big/little fs.img by eyeballing them, they both look OK. So
I'll have to trace the fs code by hand I think.

## Mon 17 Jul 2023 17:42:07 AEST

For some reason, `cfm` thinks the dinodes are size 58 not 60?! D'oh, it
should be 64 not 60. Now fixed and I can open README and read it with try
on Salmi. Yay!!!

## Tue 18 Jul 2023 09:18:19 AEST

I ran the usertests on Salmi, and now the only one which fails is
writetest1(). I'm going to try it on the hardware now. So far
everything has passed, now waiting for the slow bigdir test ...

While I'm waiting, things to do:

* put stuff from the existing monitor into romfuncs.s, so the only thing I use from the existing monitor are the IRQ/FIRQ/vectors. This will help me determine how much ROM space I have left.
* write an a.out export function for the linker because ...
* I need to write a spawn() function that has to live in ROM and, thus, must be short.

Here's what the spawn() function needs to do:

* Receive an array of strings, i.e argv[].
* Open argv[2] and return error if this doesn't exist.
* Copy the arguments to the top of the stack, create argc.
* Read and load the a.out binary from the disk. If it's binary and not s19, the code to deal with it will be small. Hopefully I can simply load the file in with no buffer copying in the spawn code.
* Deduce the bss area and zero that.
* Set fds 0, 1 and 2 to console I/O?
* Set the initial stack pointer
* JSR to $0000
* On exit() or return, spawn("/bin/sh", NULL);

Yes, given that text, rodata, bss are consecutive, all I need from a.out
is the start and size of the bss so I can clear it. In fact, if I can
build that into crt0.s, then I don't even need to do that. Ditto if I can
set the SP in crt0.s, then less code in ROM.

The bigdir test works on the hardware, so we are winning today!!!

## Tue 18 Jul 2023 11:09:42 AEST

I brought in these ROM routines into romfuncs.s: putc, getputc, puts,
ch375init, readblock, writeblock. We now have 40 bytes spare! Argh.
I need to put the spawn code into ROM, plus the IRQ handlers, the
vectors and the initialisation code. I'll need to do some code trimming.

## Tue 18 Jul 2023 11:31:41 AEST

Symbols s_bss and l_bss hold the start and length of the bss in an executable.
I should be able to use these in crt0.s to zero out the bss, and not have to
do it in ROM.

I also see that cmoc has a --raw option. Just checked, it spits out everything
except bss. So there's no need for an a.out format, I can just load the raw
file at location 0 and jump there. Yayy!!

## Tue 18 Jul 2023 14:41:08 AEST

`cmoc`'s argument `-fomit-frame-pointer` saves about 167 bytes. I'll add it to
.bin/cfm.

I'm trying to build the fs code without the -lninec library. Missing functions
are now: memcpy, memmove, memset, strncmp, strncpy. I wonder/hope I can find
assembly versions of these to save code size.

I found some in the `cmoc` library. The code budget is now -325 vs -186 with the
C versions of the functions.

## Wed 19 Jul 2023 09:25:13 AEST

I brought in the (F)IRQ handlers and the vector table from the monitor,
removed the sbrk code from the crt0.s. We are now 295 bytes under code
budget. I also chose to start the stack at $C400 which is $D800 - $1400.

I realised that the spawn code can be spawn(char *argv[]), and it can rely
on the fact that there is a non-ROM function copyargs() which puts the
arguments just below $C400. So, spawn() can simply open and load the
executable and jump to $0000. That will make the size of spawn() much
smaller.

## Sat 22 Jul 2023 15:12:15 AEST

I've had a bout of dizziness/vertigo which has made doing anything nearly
impossible. Just before, I had to resort to using a different assembler to
create the vector table for the romfuncs.s which is being used by the xv6
code, relocated into ROM. I was able, finally, to move the xv6 code up to
$E000, with the rwdata and bss starting at $C400. Once the vectors were
finally correct, I could build the `try.c` code which prints 'README' to
the console. The exit() code was simply an infinite loop.

So, yes I can get the xv6 code working in ROM. Now I want to write some
code to ask the user for a file name, load that file at $0000 and run it.
To do that I can either set up a bunch of `equ`s for the xv6 sys\_ functions,
or try to fit an SWI jump table in to ROM, which is the proper way.

If I start with this, the spawn() code initially will simply be to set the SP
and jump to $0000. Ah, I'll also need to bring back the sbrk() SWI function.

With the current "print README" try.c I am 154 bytes under budget.

## Sun 23 Jul 2023 08:31:48 AEST

I've gone with no SWI handler, but asm code which is visible to the userland
and which simply jumps to the system calls in ROM. That saves ROM bytes for
now. I'll revisit later when there's no more major ROM changes.

I've also coded up an assembly version of sbrk() which can live completely in
userland. Only tested with the first sbrk(program_end) call from crt0.s.

I've got a userland program which uses romputc() to print out "Hello\\n". I run
the existing 'print README' code in ROM, then break into the Salmi monitor when
it goes into the exit() infinite loop. Then I can run the userland program which
calls its built-in sbrk(), then calls the ROM code via the userland jumps. Works.
Now to try calling some of the XV6 functions.

Cool, yes I can run a userland program that prints out README and it works! Yay.
I wonder if I can get `ls` to work?

Yes. It took some manual compiling with all the required lib functions, but I
got it to work:

```
monitor>g $0000
.              1 1 512
..             1 1 512
README         2 2 2286
cat            2 3 16712
echo           2 4 15604
forktest       2 5 9312
grep           2 6 19884
init           2 7 16136
kill           2 8 15660
ln             2 9 15532
ls             2 10 18116
mkdir          2 11 15692
rm             2 12 15668
sh             2 13 31496
stressfs       2 14 16544
usertests      2 15 1912
wc             2 16 17288
zombie         2 17 15244
```

Now I need to write, for now, some simple code to get one word from the
console, load it into memory, set the SP and run it.


## Sun 23 Jul 2023 11:30:47 AEST

I got the vectors to work by compiling each .c file to .o, then assembling
each .s file to .o, then finally doing a manual lwlink with all the .o files.
Yay!

There was code to prevent reading to a buffer pointing at address zero (NULL).
I've removed it as we need this to load code at address zero. Or, I can change
the load address in the link file?!

## Sun 23 Jul 2023 11:45:27 AEST

I changed the load address to $0002. Woo hoo, I can load and run a binary:
```

$ try
.              1 1 512
..             1 1 512
README         2 2 2286
cat            2 3 16712
echo           2 4 15604
forktest       2 5 9312
grep           2 6 19884
init           2 7 16136
kill           2 8 15660
ln             2 9 15532
ls             2 10 18116
mkdir          2 11 15692
rm             2 12 15668
sh             2 13 31496
stressfs       2 14 16544
try            2 15 1662
usertests      2 16 1912
wc             2 17 17288
zombie         2 18 15244

```

Now to get exit() to just do a reset for now, so I can restart the dumb shell!
Just did that and it works fine!!! Wow, excellent. I'm down to 84 spare bytes
now. But if I can move the shell code out to userland, that will give me more
room. I can also lose getputc() and replace with calls to getc() and putc()
in userland. Time to mark everything with an RCS tag.

OK, I've tagged all the XV6FS code with 'basic_shell:'. This has the XV6 code
in ROM with a vector table, a dumb shell that reads one word, loads it at
$0002, sets the SP and calls the code. On exit, we reset the SBC to get back
to the shell.

## Sun 23 Jul 2023 14:00:09 AEST

In assembly, I've written a spawn(char *file) routine which opens file, loads
it starting at $0002, sets SP and jumps to $0002. I've removed the xv6fs.c
main() function. Instead, `reset` now calls INILIB to zero the bss, sys_init() to
initialise the fs data structures, then calls spawn("try"). And try is a file
on the filesystem with a raw binary that does an ls -l of /. And it all works!
However, exit is doing a reset, so it's a loop of ls output at the moment :-)

I now have about 240 spare ROM bytes!

## Sun 23 Jul 2023 14:15:58 AEST

Amazing. I borrowed the code to read a word in, made it a userland program
that calls spawn(). It's now called `sh` and the program initially executed.
So now I can do:
```

$ ./ROMXV6 			# Start the Salmi simulator
$ try				# Get the dumb shell, type in 'try'
.              1 1 512		# spawn("try") which does an 'ls -l .'
..             1 1 512
README         2 2 2286
cat            2 3 16712
echo           2 4 15604
forktest       2 5 9312
grep           2 6 19884
init           2 7 16136
kill           2 8 15660
ln             2 9 15532
ls             2 10 18116
mkdir          2 11 15692
rm             2 12 15668
sh             2 13 244
stressfs       2 14 16544
try            2 15 1665
usertests      2 16 1912
wc             2 17 17288
zombie         2 18 15244	# This now exit()s which does spawn("sh")
$

```

I have to keep rebuilding the `ninec` library each time I change anything in
ROM as the locations of things keep moving. Maybe I'll have room for a jump
table somewhere?!

## Sun 23 Jul 2023 14:30:57 AEST

I copied the xv6 rom image to the real ROM chip, ditto the fs image to USB key
and tried it on the SBC. Result: panic bl4. I'm hoping that it's RAM which is
not zeroed. I've rearranged the blkinit() code, so now sys_init calls the
ch375init() function directly, and I've changed mrt0.s to zero everything from
the start of rwdata ($C400) up to the I/O space ($D800). I also added -x to
Salmi to randomise the RAM contents.

With all of the above, I can run Salmi -x and things work with no panics.
Haven't tried it on the hardware just yet.

Phew. Yes it works on the hardware. Yay :-) I can say I've booted the xv6
code to a dumb shell, typed in a command, run that command and returned to the
shell. A good milestone!!

## Mon 24 Jul 2023 11:41:19 AEST

I've written the start of a shell that creates an argv[] and puts it at the
top of the stack using memmove(). Right now it runs, strtok()s to separate the
arguments, and then copies them to the top of the stack, then while (1)s.
Here's the result:
```

$ ./ROMXV6
$ newsh
$ Hello there Warren
3
0: Hello
1: there
2: Warren
memsize 29, destbuf d7e2 	<-- 29 below $d7ff (top of stack)
3
0: Hello
1: there
2: Warren
monitor>dump $d7e0 $d7ff
d7e0: 00 00 00 03 d7 ec d7 f2 d7 f8 00 00 48 65 6c 6c ............Hell
d7f0: 6f 00 74 68 65 72 65 00 57 61 72 72 65 6e 00 00 o.there.Warren..

```

`00 03` is argc, `d7ec d7f2 d7f8 0000` are the argv[] pointers including a
NULL, followed by the three strings. Now I need to call spawn(argv), and get
spawn() to set SP below argc (below argv). And I need to get the existing
reset code to call spawn() the new way as well.

## Mon 24 Jul 2023 13:07:35 AEST

I've got reset now building 03 (argc), a 2-element argv (one NULL, the other
the pointer) and the string "sh" on the stack, and spawn() loading the named
argv[0] and jumping to $0002. This works, and I've modified exit() to call
this code, so we get back to sh on exit. One problem: we have to jsr to
spawn() but this puts the return address on the stack. And the cmoc startup
code then calls main() so now there are two return addresses on the stack!
I think I need to increase SP by 2 in spawn() so as to lose one of them.

## Mon 24 Jul 2023 14:17:11 AEST

I also got the argv thing wrong. argv is a pointer to the base of the array
of arguments. So we need: argc, argv pointer to .. array of pointers followed
by the strings themselves. I worked out how to do this, and we now have:
```

$ ./ROMXV6 			<- Start Salmi simulator
$ ls kill ln ls sh		<- Shell tokenises the line
kill           2 9 15660	<- ls(1) processes argv[1] to argv[argc-1]
ln             2 10 15532
ls             2 11 1731
sh             2 15 1086
$ echo hello there how are you?	<- Ditto echo
hello there how are you?
$

```

Yay, so we now have arguments to our processes :-)

I compiled usertests as a real userland program and it's now running. I compiled
rm.c and it works. wc.c compiles but produces no output, I wonder why not?!

I stopped the usertests on Salmi and put it all over to the SBC hardware.
It's all running fine over there. wc not though, no surprise. It's slower
than Salmi, more's the pity.

We still have about 254 bytes of ROM left as well. I'm trying to work out
if I can mark some inodes as console devices, so we can read()/write()
from/to the UART.

## Mon 24 Jul 2023 16:59:31 AEST

In `file.h` we have:
```

struct file {
enum { FD_NONE, FD_PIPE, FD_INODE } type;
...
};

```

of which FD_PIPE is not used anymore, only FD_NONE and FD_INODE. I could
repurpose FD_PIPE to be FD_CONSOLE. I was also thinking that if we open("/tty")
then we can strcmp() and determine we want the console.

If possible, I'd also like character/line at a time reading, and echo/no-echo
reading. Not sure if all of that is possible with the ROM budget.

## Mon 24 Jul 2023 17:40:12 AEST

I can now fd=open("/tty"), write(fd, "Hello world\n", 14) and it works.
No reading as yet.

## Tue 25 Jul 2023 09:29:31 AEST

I've implemented one-char reading with echo and this works. The shell now
closes all fds at startup, and also opens fds 0, 1, 2 to the console. I
compiled cat which writes to fd 1 and that works.

## Tue 25 Jul 2023 10:55:55 AEST

I'm trying to put redirection into the shell. I need to find a way to skip
the arguments that don't need to be passed to the next program. I guess if
the redirections are at the end of the line I just need to lower argc :-)
Yes that works.

I have implemented <, > and 2>. I don't think I can do >> because there is
no O_APPEND implementation in the xv6 code. Pity!

The ROM budget is now 166 bytes left. We're doing well :-)

I've got most of the userland xv6 programs working except grep.c. I wrote my
own shell as the xv6 shell has functionality I don't need. I also need to add
"cd" functionality to my shell.

OK, grep works. I added "cd" but, for some reason, it's not working in Salmi.
On the SBC everything that I've done works. And "cd" sort of works:
```

$ mkdir Fred
$ cd Fred
About to chdir to Fred
$ ls
$ ../ls
.              1 15 32
..             1 1 512
.              1 15 32
..             1 1 512
[repeats etc.]

```

whereas on Salmi, I see:
```

$ mkdir Fred
$ cd Fred
About to chdir to Fred
0028: invalid opcode $01
S  $C3F6 U  $0000 X  $D36E Y  $0427   KEFHINZVC
A  $00   B  $00   DP $00   CC $89     1..1
PC $0029 2002  NextInst: BRA $002D
monitor>

```

## Tue 25 Jul 2023 13:01:52 AEST

I implemented '>>' in the shell by opening the file and seeking to the end. Works.
Now chasing down the chdir problem. We get past the chdir(), the problem occurs
when we call exit().

OK, so if I #if 0 out the call to chdir() and sail past to the exit(), it all
works fine. So, something in the chdir followed by the exit isn't right.

I think I might have it. Once we chdir() to another directory, there is no "sh"
to execute :-) Oops! So I should be running "/sh" not sh. Yes, that fixed it.
Now I need to get the shell to add "/" to the first argument :-)

I made the '/' modification to the shell, then remembered I had to strcmp("/cd")
once the change was made. Now I can mkdir, cd, ls, mkdir again, cd, ls, and I
can also cd .., cd ..

I can also cd /, so that's all fantastic. We now have:
```

$ ls
.              1 1 512
..             1 1 512
README         2 2 2287
cat            2 3 960
echo           2 4 306
grep           2 5 1473
ln             2 6 781
ls             2 7 1731
mkdir          2 8 817
rm             2 9 816
sh             2 10 2686
try            2 11 305
usertests      2 12 13239
wc             2 13 1063

```

and that's about all the xv6 userland programs working. The real xv6 shows:
```

$ ls
.              1 1 512
..             1 1 512
README         2 2 2286
cat            2 3 16712
echo           2 4 15604
forktest       2 5 9312		<- No multitasking on Nine-E
grep           2 6 19884
init           2 7 16136	<- No multitasking on Nine-E
kill           2 8 15660	<- No multitasking on Nine-E
ln             2 9 15532
ls             2 10 18116
mkdir          2 11 15692
rm             2 12 15668
sh             2 13 31496
stressfs       2 14 16544	<- No multitasking on Nine-E
usertests      2 15 67448
wc             2 16 17288
zombie         2 17 15244	<- No multitasking on Nine-E
console        3 18 0		<- No device files on Nine-E

```

I just tagged everything with the 'chdir_ok:' RCS tag.
And ... it all works on the SBC hardware, wonderful!!!
Now I think I need to find out what in the library isn't working, why not,
and try to fix it. Would be nice if I could compile it under Linux with
16-bit ints and 32-bit longs.

## Tue 25 Jul 2023 14:09:48 AEST

On Linux with this program:
```

\#include <stdio.h>
\#define int short
void main() {
int x= 34; long y= 4567;
printf("%d %d, sizes %d %d\\n", x, y, sizeof(x), sizeof(y));
}

```

I can do:
```

$ cc -o fred -g -m32 fred.c
$ ./fred
34 4567, sizes 2 4

```

so that's a good sign. Perhaps I can do this with each of the Fuzix library
functions and test them to see what's going on.

## Wed 26 Jul 2023 10:22:42 AEST

Stocktake time. We now have the XV6 filesystem in 8K of ROM with these
functions: chdir, close, dup, exit, fstat, link, mkdir, open, read, romgetputc,
romputc, spawn, unlink, write. All the non-forking XV6 userland programs work
and we have a basic shell. The Fuxiz libc mostly works except the printf parts
of the stdio.

I asked on COFF for either a 16-bit big-endian Unix-like dev environment (so I
can debug the libc), and/or a different libc to try. I also found:

 - Elks, a 16-bit OS for 8086 with a libc: https://github.com/jbruchon/elks
 - Minix 1.5, of course.
 - One Man's Unix: http://www.pix.net/mirrored/discordia.org.uk/~steve/omu.html
   which is a non-multitasking Unix-like system for the 6809. Could have a libc.
 - NitrOS-9: http://www.nitros9.org/battle.html, which is a Unix-like system
   with multitasking for the 6809.

Clem Cole suggested a few things, including:

 - vbcc: http://www.compilers.de/vbcc.html, which is another C compiler for the
   6809. This might also have a libc. Looks very interesting.

I also came across this:

 - AM9511/AM9512: https://thechipletter.substack.com/p/amds-early-math-innovation,
   a couple of math coprocessors for 8-bit CPUs from AMD. Might be useful to
   have in future designs?! It's a pity there isn't a reasonably-sized 5V FPGA
   still available, as I could implement the MMU and FPU in it.

To start with, I think I'll look at vbcc which comes with its own assembler and
linker.

## Thu 27 Jul 2023 06:35:11 AEST

I had the idea in the middle of the night. Why not put level converters in
front of the 6809 and do the rest of the circuit in 3.3V components, which
makes finding an FPGA much easier. I wonder if I could make a daughterboard
which would plug into the side of the ULX3S?

A quick look at vbcc. The calling convention is different: 1st arg in D,
the rest on the stack. Also, doesn't recognise -S even though the manual
says it does.

Took a while to work out how to call the compiler. This works:
```

vc +s6809 try.c fred.s

```

where `fred.s` is `romcalls.s` but with a different syntax:
```

```
    .text
    .global romgetputc
```

romgetputc:
jmp 0xE294
...
.global sys_open
sys_open:
jmp 0xFAC4

```
    .global sys_read
```

sys_read:
jmp 0xFBD5

```

The resulting binary is Intel hex format.

I can't seem to find the source code for the libc that vbcc uses. I'll need
that. I've emailed the author to ask if it is available.

## Thu 27 Jul 2023 12:52:57 AEST

I like that there is a config file for the compiler/asm/link stages, so
it's easy to change the include path etc. I can compile all the XV6 files
with no changes, so that's good. I'd have to rewrite the .s files, and
work out how to do the final link, in order to compare the size of the code.

I used vbcc -c to compile the userland programs and found a few warnings to
fix. But they all compile to .o, so that's a good sign.

## Thu 27 Jul 2023 13:17:34 AEST

Ah. I started to work on trying the existing Library functions out. I just
found out that there's no lseek() in the xv6 filesystem code. That's a bit
of a problem, as a bunch of stdio functions need it. Sigh. I did implement
it for xv6-freebsd and it's about 40 lines of code. I wonder if it will fit?

Nope, by a few bytes:
```

Section: rodata (strncmp.o) load at FFFD, length 0000
Section: vectors (romfuncs.o) load at FFF6, length 000A

```

I left the code in there but commented out. Maybe the vbcc compiler will
optimise the size down enough to make it work?

## Thu 27 Jul 2023 15:50:54 AEST

No, seems like vbcc makes bigger code. I'm using -O2 for the compiler and,
at the link stage, I get:
```

Fatal error 63: Predefined limits of destination memory region rom for
section .text were exceeded (0x1e029).

```

Ah, it's something to do with the vectors. If I remove them, I get:
```

0000e000 .text  (size 1a9b)
0000fa9b .rodata  (size 7c, allocated 7b)
0000c400 .data  (size 3, allocated 0)
0000c403 .bss  (size 11ff, allocated 0)

```

That's 1381 spare ROM bytes. Now I need to work out how to do the vectors,
and then try to make it all work!!!

## Fri 28 Jul 2023 06:45:25 AEST

I had the idea, which can be used with cmoc too, of starting the ROM with
a table of jump addresses. No "jmp" just the addresses. And in romcalls.s
I think I can do:
```

\_exit:	jmp	[$e004]

```

i.e. load the address from $e004 and jump to it. That way, I can have a table
of syscalls and no need to keep recompiling the library each time I change the
contents of ROM.

## Sat 29 Jul 2023 15:30:45 AEST

Away at Rosewood for the weekend doing working equitation. Yes, the above
works in the cmoc version. I also worked out how to get the vector table
to work with vbcc/vlink. I need to decorate the vector section with
attributes:
```

.section .vectors, "acrx"

```

## Mon 31 Jul 2023 13:21:49 AEST

Back home. I've spent an hour getting it all to compile with `vbcc` and
(hopefully) also changing the ABI (D holds 1st argument). Now to test it
and see what happens.

So I'm getting into spawn() with what appears to be the right arguments
from reset(). However, it's loading the contents of '/' from $0002 onwards.

Fixed. I have a dummy shell that prints out "Hello\n" with romputc() and
while (1)s. That works. Now to try some other stuff.

Ahah. I had to fix crt0.s to preserve the stack. Then I had to fix
cprintf() to understand the new calling convention. Yay, I can print
out 1 and "/sh". OK, `ls` runs. Now I have to work out how to change
the shell to obey the new ABI.

## Mon 31 Jul 2023 15:13:06 AEST

Done. Now I can't work out why I'm not calling exit() correctly.

## Mon 31 Jul 2023 15:27:00 AEST

Getting closer. I removed exit() and atexit() from the library and
added an 'exit' label in `romcalls.s`. `ls` seems to work with arguments.
`cat` goes into an infinite loop. `echo` crashes. Sigh.

## Tue 01 Aug 2023 09:12:27 AEST

I'm suspecting write()s to fd1 (stout). This program works for the
`romputc` but not the `sys_write`:
```

char *str= "Hello world\\n";
char *str2= "Hello WORLD\\n";

int main() {
int ch;

while (1) {
ch= *str; str++;
if (ch==0) break;
romputc(ch);
}
sys_write(1, str2, 12);
exit(0); return(0);
}

```

## Tue 01 Aug 2023 09:21:22 AEST

I'm such an idiot! I had the vector for `sys_write` in the jump table at
$e01e not $e01c. D'oh!. With it in the right spot, everything is working
and I'm running the usertests. Phew!

According to the symbol table for the ROM, I now have 1107 free bytes
and that includes the `sys_lseek` which I need to fix. And the usertests
all ran fine.

Just tried it on the SBC and I got a panic. I ran Salmi with junk in RAM
and it gave the same result. So, I must not be clearing the ROM variables
like I should be.

Ah, I was loading the value at `__BS` (start of bss) not the actual value
of `__BS` :-) Fixed. Now the SBC runs fine and I'm doing the usertests.

## Tue 01 Aug 2023 09:53:21 AEST

I had this crazy idea. I could load the symbol table from both the ROM
and a program in Salmi. Then I could change several monitor commands so
when I specify a non-numeric address, it looks up the symbol table and
uses that, e.g.
```

dis main main+30
dump buffer buffer+100

```

And when I do single-steps, I could print out something like:
```

main: blah .....
monitor> s
main+1: blah ....

```

And I wonder if I could implement the 'n' (next) command which would
jump over JSR/RTS like we have in `gdb`.

It'd be a lot of work but I'd then have a symbolic debugging environment
of sorts.

## Tue 01 Aug 2023 10:54:46 AEST

I can build the libary with vbcc and I've extracted what I think are the
required builtin .o files from the existing libvc.a. But when I do a
userland compile, I get:
```

Error 21: filewTnyoj.o (.text+0x40): Reference to undefined symbol \__moduint16.
Error 21: filewTnyoj.o (.text+0x5c): Reference to undefined symbol \__divuint16.

```

I've also put what I think are the needed .o files explicitly on the command
line and it still happens. Not sure what to do at present.

## Wed 02 Aug 2023 09:04:41 AEST

I finally found the problem. The distributed 6809-sim libvc.a has two div.o
files:

```
$ ar vt libvc.a | grep div.o
rw-rw-r-- 1000/1000    174 Nov 18 05:08 2021 div.o
rw-rw-r-- 1000/1000    172 Nov 18 05:08 2021 div.o
rw-rw-r-- 1000/1000    231 Nov 18 05:08 2021 ldiv.o
```

So I'd extracted the second one and that was causing the problem. I've now
extracted the first one, put it in my own library and I can link against it.

Frank Willie (who wrote the vlink program) also sent me the 6809 assembly
files for the memXXX, strXXX and div/mod/mul library functions which is
excellent. It's a pity that they are under NDA.

## Fri 04 Aug 2023 08:43:59 AEST

I've moved the userland commands into a new directory Cmds/Simple and
I've been able to build them there along with mkfs outside of the
VccXV6FS directory (which has the kernel code). They run but don't
give quite the right results. I checked mkfs and it's OK, so it is
probably how I'm doing the include files. cat is OK, echo. ls not so.

## Fri 04 Aug 2023 11:18:26 AEST

Dammit. I'd copied a cprintf.c into the library from before the vbcc
ABI change, so it was getting bad arguments. Seems to be fixed now.

## Fri 04 Aug 2023 11:38:05 AEST

I've done a bit of a cleanup. We now have:

 - Cmds: userland commands
 - Datasheets: datasheets of components
 - journal.md: this file
 - Kicad: SBC design
 - Library: libc source code and include files. XV6 .h files in xv6/
 - Old: old stuff I don't need any more
 - Salmi: the simulator
 - Verilog: the Verilog version of the SBC
 - XV6FS: the XV6 filesystem code, no header files

## Fri 04 Aug 2023 14:37:15 AEST

Now working on bringing up the libc. There's a lot of stuff I did in xv6-public
which I can re-use here. I've just got stat() to work. I'm trying opendir() and
friends but no luck just yet.

## Fri 04 Aug 2023 15:22:33 AEST

D'oh!. cprintf() can't deal with longs, and inums are longs. That took way too long
to diagnose. opendir() works.

## Sun 06 Aug 2023 10:39:17 AEST

I brought in my ls.c from xv6-public. I had to add %c to cprintf(). It's also using
the stdio puts(). This works except the last entry in the output doesn't have the
trailing newline. I've even added an fflush() but that doesn't help. Sigh. No, I
had to put it in before both exit()s. So that works. It means I'll have to call
fflush() on exit. Maybe I should have an exit() libcall and a sys_exit() syscall.

## Sun 06 Aug 2023 11:00:14 AEST

I'm now trying printf(). I think it's the varags that don't work. I wonder
if `vbcc` has varargs?

## Sun 06 Aug 2023 12:56:15 AEST

I found a `stdarg.h` file for 6809 in the vbcc source distribution. It
seems to work. I can now printf a bunch of things and they work. Yay!!

## Mon 07 Aug 2023 09:33:13 AEST

I addec code to fstat() to set S_IFCHR for the terminal, and isatty()
now uses this. It works.

I've removed the `exit` label from `romcalls.s`, leaving `_exit`. The
`exit()` function now calls `do_exit()`. This allows `atexit()` to work.
I also changed the `startup.s` code to call `exit()` and not `_exit()`.

Now I can change the `startup.s` code to call `stdio_init_vars()` which
will register a function to `fflush()` on exit :-)

## Mon 07 Aug 2023 10:38:37 AEST

I quickly added an `errno`, located at $DFFE, shared between the xv6
code and userland. It works. I just did this:

```
$ try
Set errno to 0
About to open /foobar
fd is -1, errno 22
```

and 22 is EINVAL :-)

So now I feel happy that most of the libc works, including stdio, va_args,
printf etc. Hopefully I can bring in more of the userland.

One bug: if I `mkdir X; cd X; ls -l`, I get the output for `/` not the
new directory.

## Mon 07 Aug 2023 11:08:49 AEST

Hey, I knew I was going to have to fix `mkfs.c` to make a filesystem with
dirs and files in dirs. Guess what, I already did it for xv6-public! Yay!!!

## Tue 08 Aug 2023 09:26:14 AEST

I moved a few things around and redid the Makefiles; there's a Build dir
which is where things are placed for `mkfs` to build the filesystem image.

The `ls` bug doesn't affect `oldls`. But if I run `ls` twice in a row,
the second one ends up back in `/`.

Ah. I `chdir()` into a dir so I can `stat()` without having to prepend
the relative pathname, then `chdir()` back out. That's fine when the
shell stays around while `ls` is running. Not so here. I might have to
borrow the code from `oldls` which prepends the relative path to make
the `stat()` code work. Damn.

## Tue 08 Aug 2023 10:13:16 AEST

Now that I can put dirs in the filesystem image, I'm thinking of making
a `/bin` and putting the executables in there. Done. Yay!

## Tue 08 Aug 2023 11:19:01 AEST

I've imported the `s` editor (a tiny vi-like editor). It compiles but crashes
when runs. It certainly works on Linux. Anyway, something for later.
Also, the xv6-freebsd `grep` doesn't work yet.

## Wed 09 Aug 2023 10:21:25 AEST

I'm debugging the `s` editor. I'm breaking at the start address $0002.
When I dump memory, everything about $3800 is zero, even though there
should be instructions here. And when I do `cat vi > z` I get a write error
and `z` ends up being only size $3600. But I can do a `wc vi` and get the
right size.

So, why isn't the full binary being loaded into memory?
I've added debug code to `romfuncs.s`, so I can see the `read()` writing to:
0002 0402 0802 0C02 1002 1402 1802 1C02 2002 2402 2802 2C02 3002 3402 3802
3C02 4002 4402 4802 4C02 5002 5402 5802 5C02 5FAA. But there's nothing at $4000.

I compiled `od` from `xv6-freebsd` and I did an `od vi`. Did the same on
Linux. The outputs are the same up to $3800 where we get lots of 00s in the
xv6 output.

We have 27 direct blocks in the i-node. 27*512 is $3600 and the first indirect
block is at offset $3800. So I expect that `mkfs` is not writing out big
files properly. Or, the xv6 code isn't reading them back in properly.

It was `mkfs`. The indirect block nums were 32-bit ints, not 16-bit xvblk_ts.
Now fixed. I can run `vi` but I think I need to disable echo from romgetputs().

## Wed 09 Aug 2023 12:20:14 AEST

I think this kernel function: `tcattr(int how, int newval)`:

`tcattr()`: Get and/or set the terminal characteristics. If how is true,
overwrite the current attributes, else do nothing. Always return the
current attributes. For now:

```
#define TC_ECHO  0x01		- Echo every character read from the terminal
```

I should be able to do it in assembly in `romfuncs.s`. Then in the library
I can implement `tcsetattr()` and `tcgetattr()`.

## Thu 10 Aug 2023 08:40:53 AEST

I've added `tcattr()` as a system call and I've tested it. Now to implement
`tcsetattr()` and `tcgetattr()`. Done and they work. Now I can look at the
`s` editor :-)

I'm getting a `b_gets improper line number 1` error, but I can sort of
do things with it. Seems like it's not echoing characters until the next
character comes in. I've added a bunch of `fflush()`es and that seems to
help with echo problem.

## Thu 10 Aug 2023 13:31:33 AEST

Looks like `rewind()` isn't working. Forget that for now. I seem to be
using the wrong instructions in `sbrk()` for checking the memory limits.
I fixed it a bit. Now I can edit a 1-line README and add 100 lines with
no problems. But when I open and edit `roff_manual` which is 6K, I can
append or open a line, but when I do ESC it goes into an infinite loop.

Also, the `s` editor occupies 28K including text, data & bss. 4K of that
is vfnprintf()! I wonder if I could lose the stdio stuff, use read(),
write(), cprintf() and romget/puts() and save some memory?!

But still need to check sbrk() works. Yes it's fine now, I wrote a test
that exercised brk() and sbrk() across all 64K.

The infinite loop seems to be in `fflush()`!! Damn.

I thought of changing the printf()s in the `s` editor to cprintf()s, but there
are `sprintf()`s as well. Maybe I should write a `csprintf()`?

## Thu 10 Aug 2023 15:24:51 AEST

OK, something is overwriting memory!!

```
monitor>dis $031e $032e
031e: JSR $0079       ;bd 00 79 
0321: TFR D,X         ;1f 01 
0323: LEAS 4,S        ;32 64 
0325: CMPX #$0000     ;8c 00 00 
0328: BLE $0336       ;2f 0c 
032a: TFR X,D         ;1f 10 
032c: LEAU D,U        ;33 cb 

... later ...

monitor>dis $031e $032e
031e: LDB #$00        ;c6 00 
0320: ROL $1F01       ;79 1f 01 
0323: LEAS 4,S        ;32 64 
0325: CMPX #$0000     ;8c 00 00 
0328: BLE $0336       ;2f 0c
```

## Thu 10 Aug 2023 17:05:11 AEST

Idea: add watchpoints to Salmi as well as being able to load map files.
Then I can find out who is writing to these addresses. I'm trying to work
out a) how not to compromise the speed, b) have several watch addresses and
c) perhaps have a range to watch (e.g. < $2000, where $2000 is the end of
the `.text` area). Hmm ....

## Fri 11 Aug 2023 09:51:34 AEST

I added the watchpoints to Salmi. It seems that `free()` is writing 2 bytes
at address $031e. Now to find out why.

Hmm, I don't think it's `free()`s fault, as:

```
 JSR to free: D=A9EB, 08 0B AA 0B 6C 40 5E 04 05 69 48 9F 00 00 50 A0 
 JSR to free: D=AA0B, 08 0B AA 2B 6C 40 5E 04 05 69 48 9F 00 00 50 A0 
 JSR to free: D=AA2B, 08 0B A6 7F 6C 40 5E 04 05 69 48 9F 00 00 50 A0 
 JSR to free: D=A67F, 08 0B A6 9F 6C 40 5E 04 05 69 48 9F 00 00 50 A0 
 JSR to free: D=0321, 08 06 5F 19 6C 40 5E 04 05 69 48 9F 00 00 50 A0
```

Someone is calling `free()` with what seems to be a bogus value!
It is `void free_recs()`. No, I'm checking the way `malloc()` is
working. It's calling `sbrk(0)` to get the current brk value. The
highest is $AAA8. Later, someone calls `malloc()` and it returns $D010!!
So there's a bug in `malloc()`.

## Sun 13 Aug 2023 08:58:35 AEST

I've written a malloc()/free() test in `try.c`. It isn't showing anything
out of the ordinary yet. I'm also `memset()`ing the memory, in case it
tromps on some location that it shouldn't. Sigh.

I'm going to import the Minix 1.5 `malloc` code and see how that goes.
Wow. It works, and now the `s` editor can edit `roff_manual` with no
problems. Phew.

## Sun 13 Aug 2023 10:02:10 AEST

I added :wq, ctrl-f and ctrl-b to `s`, so now it feels like what I use in
`vi`.

And ... it all works on the hardware. So I've edited the roff_manual with
the `s` editor and it feels good :-)

Now I feel like I need to add readline() and glob() functionality to the
shell, so I can deal with my typing mistakes, and so I can do * and ?.

## Sun 13 Aug 2023 10:13:35 AEST

Hmm, a regression error on the real hardware. With `usertests` I'm seeing:

```
...
rmdot ok
fourteen test
fourteen ok
bigfile test
bigfile test ok
subdir test
bl3		(a panic)
```

With Salmi, I'm seeing:

```
...
bigfile test
bigfile test ok
subdir test
subdir ok	<- OK
linktest
linktest ok
unlinkread test
unlinkread ok
```

## Mon 14 Aug 2023 09:57:28 AEST

I thought the panic might be because we now have things in a subdir. I
copied all the files to '/' and I still get the same panic. Looks
like I'll have to do some regression testing.

Also looks like I'll have to write a tiny less(1). I've already got the
design in my head. It won't read from a pipe, which means I can read the
file, find the offsets of each line start, close and reopen it.

I've got a tarball of the project on the 4th August. This passes the test
on the SBC with no panic. I just rebuilt the fs.img with the new `mkfs`
and the tests still pass. Now I've made a `bin` subdir and the tests
still pass. So, it's not the `mkfs`. Damn. Let's try the old kernel
and the new `fs.img` ... That's OK too. So I feel that it narrows
things down to changes to the XV6FS code.

## Tue 15 Aug 2023 09:28:00 AEST

I've managed to write a simple less-like pager in about 300 lines of C.
Works under Linux. I'm pleasantly surprised that I wrote very few bugs in
the process!! Now trying to get it to work in the project.

So `lseek()` is only returning an int when it should be returning an off_t.
Fixed, but now I need to fix `romcalls.s`.

## Wed 16 Aug 2023 10:50:04 AEST

Actually, `sys_lseek()` seems to be returning 0 always. Not sure why
at the moment. Ah, I'd forgotten that I'd changed the `argfd()` arguments.
Also, there was a couple of `int`s that should have been `xvoff_t`. So
it now works. But the pager isn't working yet.

## Wed 16 Aug 2023 13:58:47 AEST

OK. So `fgets()/fgetc()` both read in 512 bytes and then read from the
buffer. However, this means that `lseek()/ftell()` are not giving the
file offset of where `fgets()/fgetc()` last read from, but multiples of
512. So I might `fgets()` a 60-char line but the offset is now 512.
I've rewritten the pager to just add on the buffer length to the offset
instead of using `ftell()` and this now works.

Now, `roff roff_manual` works. But `roff roff_manual > foo` goes into
an infinite loop. We get up to 3486 bytes which is 6 blocks of 512.
So could it be the stdio library or the xv6 code?

I changed the buf size in stdio to 256 and now the file gets to 6*256==1754.
So it's definitely a stdio bug :-S. How to debug it?

Ah. `fwrite()` is fast but `fputs()` and `putchar()` are slow when not
going to a tty. I wonder why?

Hmm. If I `fopen()` a file and then `fputs()` to the FILE pointer, it's fast.
However, if I do `FILE *zout= stdout` and `fputs()` to `zout`, it's slow.
But after a certain number of lines (80 the first time, about 120 the
second time) it goes quickly. Weird.

I wonder if the "stdout" structure has initial values which are wrong?

When we do a `>` redirect, the file pointer mode goes from 32 on the first
line, to 160 and then, after about 200 lines, to 544 which is fast. When
I `fopen()` an output file first, we get: 44 on line 1, 172 up to line 199
and then mode 556; all are fast.

These translate to:

 - 32		MODE_WRITE
 - 160		MODE_WRITING | MODE_WRITE
 - 544		MODE_ERR | MODE_WRITE
 - 44		MODE_WRITE | MODE_FREEFIL | MODE_FREEBUF
 - 172		MODE_WRITING | MODE_WRITE | MODE_FREEFIL | MODE_FREEBUF
 - 556		MODE_ERR | MODE_WRITE | MODE_FREEFIL | MODE_FREEBUF

and all are also mode 0: IOFBF, full buffering.
I'm guessing that it's the MODE_FREEBUF that's the issue.

Ah ... also, why the MODE_ERR. That's because we are getting up to the
first indirect block and failing. Sigh. Another bug.

## Thu 17 Aug 2023 10:35:47 AEST

I checked, we are not writing to a weird block number like 0 or a billion.
So the problem is higher up in the xv6 stack.

The calltrace for the last good write of 512 bytes in my latest try program is:

```
 JSR to write (0079): X=0000, D=0004, 03 EE 0F 78 02 00 0C 64 0C 64 00 30 C3 F2 C3 F6 
 JSR to .l1 (F208): X=C3DA, D=0004, F3 53 00 00 C3 DA D2 88 00 04 02 00 00 1A 00 04 
 RTS to F353 X=0008 D=0000
 JSR to filewrite (E536): X=0008, D=D288, F3 B1 0F 78 00 00 02 00 D2 88 00 00 02 00 00 1A 
 JSR to ilock (E975): X=D288, D=D36E, E5 B1 00 00 00 00 00 00 02 00 00 00 02 00 00 1A 
 RTS to E5B1 X=D36E D=0001
 JSR to writei (ED39): X=D36E, D=D36E, E5 CD 0F 78 00 00 34 00 00 00 02 00 00 00 00 00 
 JSR to __asrl (FCCA): X=C3A8, D=0000, ED CF D2 8E D3 78 D3 6E 02 00 36 00 02 00 34 00 
 RTS to EDCF X=C3A8, D=0000
 JSR to .l107 (EA72): X=001A, D=D36E, ED DC 00 1A D3 78 D3 6E 02 00 36 00 00 00 00 1A 
 JSR to .l6 (E6C6): X=D3B0, D=0034, EA A2 D3 B0 D3 6E D2 8E 00 00 ED DC 00 1A D3 78 
 JSR to bread (E374): X=0000, D=0022, E6 EA 00 0B 00 00 CA 2C CA 2C D3 78 00 34 00 00 
 BSR to .l12 (E305): X=0022, D=0022, E3 7A 00 00 E6 EA 00 0B 00 00 CA 2C CA 2C D3 78 
 RTS to E37A X=CE45 D=CE42
 RTS to E6EA X=CE45 D=CE42
```

and the one after that is:

```
JSR to write (0079): X=0000, D=0004, 03 EE 0F 78 02 00 0C 64 0C 64 00 30 C3 F2 C3 F6 
 JSR to .l1 (F208): X=C3DA, D=0004, F3 53 00 00 C3 DA D2 88 00 04 02 00 00 1B 00 04 
 RTS to F353 X=0008 D=0000
 JSR to filewrite (E536): X=0008, D=D288, F3 B1 0F 78 00 00 02 00 D2 88 00 00 02 00 00 1B 
 JSR to ilock (E975): X=D288, D=D36E, E5 B1 00 00 00 00 00 00 02 00 00 00 02 00 00 1B 
 RTS to E5B1 X=D36E D=0001
 JSR to writei (ED39): X=D36E, D=D36E, E5 CD 0F 78 00 00 36 00 00 00 02 00 00 00 00 00 
 RTS to E5CD X=FFFF D=FFFF
 JSR to iunlock (EA30): X=FFFF, D=D36E, E5 F8 00 00 00 00 00 00 02 00 FF FF FF FF 00 1B 
 RTS to E5F8 X=FFFF, D=D36E
 RTS to F3B1 X=FFFF D=FFFF
 RTS to 03EE X=FFFF D=FFFF
```

We are failing this test in `writei()`:

```
if (off + n > MAXFILE * BSIZE)
```

The C preprocessor shows:

```
if ( off + n > ( 27 + ( 512 / sizeof ( xvoff_t ) ) ) * 512 )
```

which is (27 + (512/4)) * 512 or 79360 bytes, the maximim file size. That's because
we only have one indirect block.  Now 79360 is 0x13600 or 65,536 + 13,824.
Looking at the assembly code, we have:

```
        ldx     0,s
        cmpx    #0		<- 0x0000
        bgt     .l202
        bne     .l184
        ldx     2,s
        cmpx    #13824		<- 0x3600
        bls     .l184
```

so it seems we have lost the 0x10000 in the top four bytes. This seems to generate
the right assembly code:

```
if (off + n > (xvoff_t)MAXFILE * (xvoff_t)BSIZE)
```

Yes that fixes the problem.

## Thu 17 Aug 2023 12:00:15 AEST

I'm thinking about the slow `>` problem. It think it's because I'm opening the file
`O_RDWR` when I do the `>` in the shell. I looked at `fopen()` and it's doing
`O_CREAT | O_TRUNC`, so I tried this in the shell.

But when I do this:

```
$ cat roff_manual > jim
cat: write error
$ ls -l jim
-rwxrwxrwx     1 root root      0 jim
```

so, for some reason, `write()`s to this file are failing. Sigh.

## Thu 17 Aug 2023 16:36:05 AEST

It's weird. `stdio0.c` has this:

```
static unsigned char bufout[BUFSIZ];	// Where BUFSIZ is 256

FILE stdoutstruct = {
        bufout, bufout, bufout, bufout, bufout + BUFSIZ,  // latter is bufend
         1, _IOFBF | __MODE_WRITE | __MODE_IOTRAN,
        "       ", (FILE *)0
};
```

And I even try to manually set bufend in the C code:

```
STATIC void __stdio_init_vars(void)
{
	...
        stdout->bufend= bufout + BUFSIZ;
	...
}
```

The C preprocessor output shows:

```
stdout -> bufend = bufout + 256 ;
```

But the assembly has:

```
        ldd     #(bufout)
        std     8,x		// Note, no add 256!
```

Argh!!! Even this doesn't work:

```
        stdout->bufend= &bufout[BUFSIZ];
```

## Thu 17 Aug 2023 16:44:50 AEST

OK, it's a compiler problem. If I define BUFSIZ as 255+1, the code works, even
the static struct assignment at the top. But not with 256! I've sent Volker
an e-mail with some test code to show the problem.

## Thu 17 Aug 2023 17:16:47 AEST

There's a project here:
https://hackaday.io/project/19000-a-4-4ics-z80-homemade-computer-on-breadboard
which uses an Atmega32A as the I/O controller and ROM for a Z80. And a
follow-on project here:
https://hackaday.io/project/159973-z80-mbc2-a-4-ics-homebrew-z80-computer
where the Atmega32A also acts as a primitive MMU to provide bank-switched
RAM to the Z80.

I'm wondering if I could borrow the idea for the next-gen 6809 project
instead of using a CPLD or having to level shift. The Atmega32A runs
at 5V :-)

Actually the ATMEGA64A looks like the go as it has 53 I/O pins, most
are in groups of 8 and there's support for two UARTs and an SDCARD
interface. There's also the ATMEGA128 with 128K of flash for program
storage.

## Fri 18 Aug 2023 09:45:57 AEST

Now that I have `vi`, `roff` and `less` working, I had to bring it up
on the SBC. I edited the roff manual, added a few words, then `roff`d
it and then scrolled back and forwards with my `less`. So nice.

The `usertests` still fail with `bl3`. Damn. So I still have some
regression testing to do.

## Fri 18 Aug 2023 10:19:26 AEST

I took the `errno` stuff out of `sysfile.c` and `file.c` and that fixed
the `bl3` panic. Hmm. Now to narrow down which one :-S

It's not in `file.c`, so it's got to be in `sysfile.c`. We are making
subdirs, so I'll see ...

I took the first 16 out of 32 `errno` uses out of `sysfile.c`, then the
system just hung. I'm wondering if I shouldn't have `errno` just before
the I/O area. Maybe I should move it a bit further down?!

Damn. No that's not the problem. OK, I took out the bottom 15 `errno`
references in `sysfile.c` and that fixed the problem. Now to remove
the last eight :-)

## Fri 18 Aug 2023 11:09:46 AEST

Wow, it's the `errno=0` at the top of `sys_open()`. Crazy!

D'oh!!! I had `errno` mapped at $DFFE which is IN the I/O area. It
should be $D7FE. Stupid me!! I fixed the `vlink.cmd` file in the
current XV6FS and, yes, that fixes the problem. Argh!!!

## Fri 18 Aug 2023 14:16:04 AEST

I added rudimentary `*` and `?` matching, and the shell can also detect if a file
is executable or not, so it won't try to run `bin` or `README :-) Nice.

I was thinking of where to announce this project, once I have it documented enough
and up on Github. https://anycpu.org/forum looks like a good place. The
retrobrew forum seems very quiet, but it's also a place to try:
https://www.retrobrewcomputers.org/forum/index.php

## Sat 19 Aug 2023 14:38:57 AEST

I pushed the current system up to Github and published a short description of
it on AnyCPU. I also heard back from the people writing CoCo C compiler and
mentioned the project to them.

Crazy idea. I was thinking of using the Atmega64 as an MMU/IO device. But
it can access 64K of external SRAM, and there's a heap of GPIO pins. So:

 - Why not port xv6 to the Atmega64? We can still do MMU/IO with the device
   itself. It has timers, interrupts, and we can add on a time-of-day clock.
   We could bring up a full xv6 + userland on it, perhaps?

 - Even more ambitious: put the operating system in the Atmega64 and map it
   into the top 32/64 bytes of the 6809? This gives nearly 64K of memory to
   a 6809 process, and it can call the OS by writing commands/data to specific
   memory locations, a la the CH375. The Atmega64 can still do the memory
   mapping and I/O.

   With a large SRAM, we could have e.g. 4K pages.

   Context switching: the Atmega64 gets a timer interrupt. It sends an NMI
   down to the 6809. The NMI handler starts up, sends a message to the
   Atmega64 (via the command address) that it's ready to context switch.
   It goes into an infinite loop waiting for the response. The Atmega64
   remaps all the RAM pages and then sends the OK response. The NMI handler
   can then RTI and the new process starts up.

   So that means that perhaps we keep the top page mapped as ROM, so
   that it doesn't disappear when we do the context switch. One problem
   would be saving a copy of the stack pointer to the Atmega64. I guess
   the NMI handler can send it to the Atmega64 and get it back with the
   OK response.

Perhaps a port of xv6 to the Atmega64 first :-)

## Sat 19 Aug 2023 15:11:37 AEST

URLs for programming Atmel AVRs from Linux:
https://electronics.stackexchange.com/questions/66145/avr-how-to-program-an-avr-chip-in-linux
http://www.ladyada.net/learn/avr/setup-unix.html
and https://www.ladyada.net/learn/avr/

And here's a schematic on how to add SRAM to a Atmega128:
https://www.avrfreaks.net/s/topic/a5C3l000000Ud6mEAC/t164124

And an Atmega128 development board with associated docs:
https://www.openimpulse.com/blog/products-page/product-category/atmega128-minimal-development-board/

Looking at the board:
https://www.openimpulse.com/blog/wp-content/uploads/2014/03/ATmega128-Minimal-Development-Board-5.jpg
I could make a daughterboard that sits on top with the RAM
and I/O devices, e.g. USB to UART device, time of day clock,
SD card socket. Later, I could add a socket for the 6809 :-)

Atmega simulator: https://github.com/buserror/simavr

## Mon 21 Aug 2023 09:02:03 AEST

I finally found the schematic and source code for the project with the Z80
and the Atmega providing ROM and I/O. The Atmega halts the CPU while doing 
its work, which means that ROM reads are slooow. I can't use this for RAM
as every RAM access would be slow. So I think I'll stick with the idea of
using the ATF1508 CPLD for the next project.

I replaced the BSD `grep` with the Fuzix one. It works. Now I realised
that the shall doesn't have any quotes, single or double :-) I'll need
to add them to use '*' etc. Done. I've imported a few other simple commands
from Minix 1.5, and wrote my own `mv`.

## Mon 21 Aug 2023 10:58:54 AEST

I got readline working in the shell, I had to add '\r' to the readline
source code. No tab completion, though!

But I have this bug:

```
$ echo roff*
roff_manual 				# Expands properly
$ echo roff* > fred			# Doesn't expand when doing redirection
$ ls -l fred
-rwxrwxrwx     1 root root      1 fred
$ cat fred
					# Just a single newline character
$ od -b fred
0000000 012
```

Ah, I see the problem. On a pattern I'm opening the dir only once,
building a list of matches for all patterns, then appending them
to the argv. So when I get to the redirect, I toss everything after
the redirect on the line.

E.g.

```
ls -l *.c > foo				# Becomes
ls -l > foo a.c b.c fred.c
```

OK, I fixed this by opening the dir as many times as there are
patterns on the command line. And it all works fine on the hardware too!!
