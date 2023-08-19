MEMORY
{
  ram  :  org = 0xC400, len = 0x13FE
  err  :  org = 0xD7FE, len = 0x0002
  jump :  org = 0xE000, len = 0x0020
  rom  :  org = 0xE020, len = 0x1FD5
  vect :  org = 0xFFF6, len = 0x000A
}

SECTIONS {
	.jumptable: { *(.jumptable) } > jump
	.text: { *(.text) } > rom
	.dtors : { *(.dtors) } > rom
	.ctors : { *(.ctors) } > rom
	.rodata: { *(.rodata) } > rom
	.vectors: { *(.vectors) } > vect
	.dpage: { *(.dpage) } > ram
	.data: { *(.data) } > ram
	.bss (NOLOAD) : { *(.bss) } > ram
	.errno (NOLOAD) : { *(.errno) } > err

	__BS = ADDR(.bss);
	__BL = SIZEOF(.bss);
	__DS = ADDR(.data);
	__DC = LOADADDR(.data);
	__DL = SIZEOF(.data);

	__STACK = 0xC3F3;
}
