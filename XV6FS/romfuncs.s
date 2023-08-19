; Assembly code to initialise the hardware and do
; basic I/O on the UART and the CH375 block device.
; Also, a jump table for the XV6 filesystem calls
; and the vector table for IRQs and reset.

; UART read & write addresses
	.set uartrd, 0xd800
	.set uartwr, 0xd800

; Addresses to read & write data and commands to the CH375 device
	.set chdatard, 0xdc00
	.set chdatawr, 0xdc00
	.set chcmdrd,  0xdc01
	.set chcmdwr,  0xdc01

; CH375 commands
	.set CMD_RESET_ALL,    0x05
	.set CMD_SET_USB_MODE, 0x15
	.set CMD_GET_STATUS,   0x22
	.set CMD_RD_USB_DATA,  0x28
	.set CMD_WR_USB_DATA,  0x2B
	.set CMD_DISK_INIT,    0x51
	.set CMD_DISK_SIZE,    0x53
	.set CMD_DISK_READ,    0x54
	.set CMD_DISK_RD_GO,   0x55
	.set CMD_DISK_WRITE,   0x56
	.set CMD_DISK_WR_GO,   0x57
	.set CMD_DISK_READY,   0x59

; CH375 status results
	.set USB_INT_SUCCESS,	 0x14
	.set USB_INT_CONNECT,	 0x15
	.set USB_INT_DISK_READ,	 0x1D
	.set USB_INT_DISK_WRITE, 0x1E

; Terminal characteristics
	.set TC_ECHO,		0x01	; Echo input characters

; Top of stack - below the xv6 variables
	.set stacktop, 0xC3F2

; Initialised variables (done in reset)
	.data
chstatus:	.byte 0			; CH375 status after an FIRQ
uartflg:	.byte 0			; Flag indicating if char in uartch,
					; initially zero (false)
uartch:		.byte 0			; UART character available to read
					; if uartflg==1
termattr:	.word TC_ECHO		; Terminal characteristics

; Uninitialised variables
	.bss
_spawnfd:	.zero 2
_spawncnt:	.zero 2
_spawnptr:	.zero 2

; The location of errno, which is shared between the kernel and
; the userland
	.section .errno, "acrx"
errno:
	.global	errno
	.zero 2

; The jump table of vectors to the XV6 filesystem calls in the ROM.
	.section .jumptable, "acrx"
	.word exit
	.word romgetputc
	.word romputc
	.word spawn
	.word sys_chdir
	.word sys_close
	.word sys_dup
	.word sys_fstat
	.word sys_link
	.word sys_lseek
	.word sys_mkdir
	.word sys_open
	.word sys_read
	.word sys_unlink
	.word sys_write
	.word tcattr


; ROM routines
	.text

; Get and/or set the current terminal characteristics. D holds the
; command: 1 means set. The new value is on the stack. Always return
; the current value.
tcattr:
	.global tcattr
	cmpd	#0x01		; Shall we set the value?
	bne	tcattr01
	ldd	2,S		; Get the new value
	std	termattr	; and store it
tcattr01:
	ldd	termattr	; Load the current value
	rts			; and return it

; Get a character from the UART into D with the top byte zero
getc:	lda	uartflg		; See if there is any UART data to read
	beq	getc		; Loop until there is
	clra			; Reset the status flag
	sta	uartflg
	ldb	uartch		; Get the available character
	rts			; and return

; Send the character in D to the UART
romputc:
	.global	romputc
	stb	uartwr
	rts

; Print the string which is the argument to the UART, followed by a
; newline. Then loop indefinitely.
panic:
	.global	panic
	tfr	D,X		; Get the pointer
panic1: ldb	,x+		; Get next char
	beq	panic2		; End when we hit NUL
	stb	uartwr		; Send to the UART
	bra	panic1		; and loop back
panic2:	ldb	#0x0A		; Send a \n to the UART
	stb	uartwr
panic3:	jmp	panic3

; Get a character from the UART into D, echoing it as well.
; \r characters get converted to \n.
;
romgetputc:
	.global romgetputc
	ldd	termattr
	andb	#TC_ECHO	; Do we need to echo the character?
	bne	getpc1		; Yes
	jmp	getc		; No, get a character and return it
getpc1:	jsr	getc		; Get a character
	cmpb	#0x0d		; If we got an \r
	bne	L1		; send a \n after the \r
	stb	uartwr
	ldb	#0x0a
L1:	stb	uartwr
	clra
	rts

; Print out the A register as two hex digits. Borrowed from the ASSIST09 code
prhex:          pshs    d               ; Save - do not reread
                LDB     #16             ; Shift by 4 bits
                MUL                     ; with multiply
                jsr     prnibble        ; Print the upper nibble
                puls    d               ; Restore bytes
                anda    #0x0f           ; Isolate the lower nibble

prnibble:       adda    #0x90           ; Prepare A-F adjust
                daa                     ; Adjust
                adca    #0x40           ; Prepare character bits
                daa                     ; Adjust
		tfr	a,b
                jsr     romputc
                rts

; Initialise the CH375 device. Return D=1 if OK, D=0 on error.
ch375init:
	.global ch375init
	lda	#CMD_RESET_ALL		; Do a reset
	sta	chcmdwr

	ldb	#0xff			 ; and delay loop for reset to work
L4:	sta	0xFFFF
	sta	0xFFFF
	sta	0xFFFF
	sta	0xFFFF
	sta	0xFFFF
	sta	0xFFFF
	sta	0xFFFF
	sta	0xFFFF
	decb
	bne	L4

	lda	#0xff
	sta	chstatus		; Store dummy value in chstatus
	lda	#CMD_SET_USB_MODE	; Set the USB mode to 6
	ldb	#0x06
	sta	chcmdwr
	stb	chdatawr

L5:	lda	chstatus		; Loop until we get USB_INT_CONNECT
	cmpa	#USB_INT_CONNECT
	bne	L5

	lda	#0xff
	sta	chstatus		; Store dummy value in chstatus
	lda	#CMD_DISK_INIT		; Try to init the disk
	sta	chcmdwr

L6:	lda	chstatus		; Loop until no dummy value
	cmpa	#0xff
	beq	L6
	cmpa	#USB_INT_SUCCESS	; Did we get USB_INT_SUCCESS?
	bne	L7			; No, error
	ldd	#1
	rts
L7:	ldd	#0
	rts

; Given a 2-byte buffer pointer in D and a 4-byte LBA in big-endian format
; on the stack, read the 512-byte block at that LBA into the given buffer.
; Returns D=1 if success, D=0 if failure.
readblock:
	.global readblock
	tfr	D,X			; Get the buffer's start address
	lda	#0xff
	sta	chstatus		; Store dummy value in chstatus
	lda	#CMD_DISK_READ
	sta	chcmdwr
	lda	5,S			; Send the LBA little-endian
	sta	chdatawr
	lda	4,S
	sta	chdatawr
	lda	3,S
	sta	chdatawr
	lda	2,S
	sta	chdatawr
	lda	#1			; and read one block
	sta	chdatawr

L8:	lda	chstatus		; Get a real status after an interrupt
	cmpa	#0xff
	beq	L8
	cmpa	#USB_INT_DISK_READ	; Break loop if not USB_INT_DISK_READ
	bne	L10

	lda	#CMD_RD_USB_DATA	; Now read the data
	sta	chcmdwr
	ldb	chdatard		; Get the buffer size
L9:	lda	chdatard		; Read a data byte from the CH375
	sta	,X+			; Store the byte in the buffer
	decb
	bne	L9			; Loop until the 64 bytes are read

	lda	#0xff
	sta	chstatus		; Store dummy value in chstatus
	lda	#CMD_DISK_RD_GO		; Tell the CH375 to repeat
	sta	chcmdwr
	jmp	L8

L10:	cmpa	#USB_INT_SUCCESS	; Set D=1 if we have USB_INT_SUCCESS
	beq	L11
	ldd	#0			; Otherwise set D=0.
	rts
L11:	ldd	#1
	rts

; Given a 2-byte buffer pointer in D and a 4-byte LBA in big-endian format
; on the stack, write the 512-byte block from the buffer to that LBA.
; Returns D=1 if success, D=0 if failure.
writeblock:
	.global writeblock
	tfr	D,X			; Get the buffer's start address
	lda	#0xff
	sta	chstatus		; Store dummy value in chstatus
	lda	#CMD_DISK_WRITE
	sta	chcmdwr
	lda	5,S			; Send the LBA little-endian
	sta	chdatawr
	lda	4,S
	sta	chdatawr
	lda	3,S
	sta	chdatawr
	lda	2,S
	sta	chdatawr
	lda	#1			; and write one block
	sta	chdatawr

L12:	lda	chstatus		; Get a real status after an interrupt
	cmpa	#0xff
	beq	L12
	cmpa	#USB_INT_DISK_WRITE	; Break loop if not USB_INT_DISK_WRITE
	bne	L14

	lda	#CMD_WR_USB_DATA	; Now write the data
	sta	chcmdwr
	ldb	#0x40			; 64 bytes at a time
	stb	chdatawr		; Send the buffer size
L13:	lda	,X+			; Read the byte from the buffer
	sta	chdatawr		; and send to the CH375
	decb
	bne	L13			; Loop until all 64 bytes are sent

	lda	#0xff
	sta	chstatus		; Store dummy value in chstatus
	lda	#CMD_DISK_WR_GO		; Tell the CH375 to repeat
	sta	chcmdwr
	jmp	L12

L14:	cmpa	#USB_INT_SUCCESS	; Set D=1 if we have USB_INT_SUCCESS
	beq	L15
	ldd	#0			; Otherwise set D=0.
	rts
L15:	ldd	#1
	rts

; UART IRQ Handler

uartirq:
	.global uartirq
	lda	uartrd		; Get the character from the UART
	sta	uartch		; and save it in the 1-char buffer
	lda	#0x01		; Set the status flag
	sta	uartflg
	rti

; CH375 FIRQ Handler

; When we get a fast IRQ, send CMD_GET_STATUS to the
; CH375 to stop the interrupt. Get the current status
; and store it in chstatus. Push/pop A to ensure it's intact.
ch375firq:
	.global ch375firq
	pshs	a
	lda	#0x22		 ; CMD_GET_STATUS
	sta	chcmdwr
	lda	chdatard	; Get the result back
	sta	chstatus
	puls	a
	rti

; The code which is performed on a reset
reset:
	.global	reset
	lds	#stacktop	; Set up the stack pointer
	andcc	#0xaf		; Enable interrupts
	clra			; Reset the UART status flag
	sta	uartflg
	ldd	#TC_ECHO	; Set echo mode on the terminal
	std	termattr

	clra			; Now clear the bss segment
	clrb
	ldx	#__BS		; Get the bss start address
reset1:	std	,X++		; Zero that location
	cmpx	#uartrd		; up to the start of the I/O space
	blt	reset1

	jsr	sys_init	; Initialise the filesystem structures

exit:				; exit() now just runs the shell
	.global exit
	lds	#stacktop	; Set up SP again (for an exit call).
				; Put on the stack a pointer to
				; the argument list, then the list
				; (one pointer then NULL) then the
				; string "/bin/sh" with a trailing NUL.
	ldx	#stacktop
	ldd	#0xc3f4		; At $c3f2, argv pointer to arg list
	std	,X++
	ldd	#0xc3f8		; At $c3f4, 1st list element: ptr to string
	std	,X++
	ldd	#0x0000		; At $c3f6, NULL at end of argument list
	std	,X++
	ldd	#0x2f62		; At $c3f8, "/b"
	std	,X++
	ldd	#0x696e		; At $c3fa, "in"
	std	,X++
	ldd	#0x2f73		; At $c3fc, "/s"
	std	,X++
	ldd	#0x6800		; At $c3fe "h<NUL>"
	std	,X++
		
	ldd	#1		; Set argc to 1
	jsr	spawn		; and call spawn

; spawn is called with D=argc, the SP pointing at argv followed by a pointer
; to an array of argument pointers. It loads the file named by argv[0]
; into memory at $0002 and jumps to the code at $0002. It returns if the
; file does not exist or if the read fails.
spawn:
	.global spawn
	pshs	D		; Save the argc on the stack for now
	clra			; Set D to 0, i.e. O_RDONLY
	clrb
	pshs	B,A		; Push O_RDONLY as the second argument
	ldd	[6,S]		; Get the ptr to file's name in D as 1st arg
	jsr	sys_open	; Call sys_open()
	leas	2,S		; Pop the argument
	std	_spawnfd	; Save the file descriptor
	cmpd	#0xFFFF		; Was it -1, if so branch to the RTS
	beq	L00012
	clra
	ldb	#0x02		; Set the load address to $0002
	std	_spawnptr
L00016:
	ldd	#0x0400		; Try to read 1024 bytes (arg 3)
	pshs	B,A		
	ldd	_spawnptr	; Set spawnptr as arg 2
	pshs	B,A		
	ldd	_spawnfd	; Set spawnfd in D as arg 1
	jsr	sys_read	; Call sys_read()
	leas	4,S		; Pop the 2nd and 3rd arguments
	std	_spawncnt	; Save the result in spawncnt
	beq	L00018		; Leave the loop if nothing was read
	blt	L00012		; Result < 0, error, so return
	ldd	_spawncnt	; spawnptr= spawnptr + spawncnt
	addd	_spawnptr
	std	_spawnptr
	bra	L00016		; Loop back to read more of the file
L00018:
	ldd	_spawnfd	; Close the open file
	jsr	sys_close
	puls	D		; Get argc back from the stack.
	leas	2,S		; We pop the return address. This allows
				; main() to be called by the C startup code
				; and gets the stack layout correct.
	jmp	0x0002		; Now jump to the executable's start address
L00012:
	ldd	_spawnfd	; Close the open file
	jsr	sys_close
	rts			; Return on an error

; The vector table for SWI, NMI, IRQ, FIRQ and reset.
	.section .vectors, "acrx"
	.word	ch375firq
	.word	uartirq
	.word	0x1234
	.word	0x5678
	.word	reset
