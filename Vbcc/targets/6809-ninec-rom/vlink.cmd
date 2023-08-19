MEMORY
{
  ram :  org = 0xC400, len = 0x1400
  rom :  org = 0xE000, len = 0x1FF5
  vect :  org = 0xFFF6, len = 0x000A
}

SECTIONS {
	.text: { *(.text) } > rom
	.dtors : { *(.dtors) } > rom
	.ctors : { *(.ctors) } > rom
	.rodata: { *(.rodata) } > rom
	.vectors: { *(.vectors) } > vect
	.dpage: { *(.dpage) } > ram
	.data: { *(.data) } > ram
	.bss (NOLOAD) : { *(.bss) } > ram

	__BS = ADDR(.bss);
	__BL = SIZEOF(.bss);
	__DS = ADDR(.data);
	__DC = LOADADDR(.data);
	__DL = SIZEOF(.data);

	__STACK = 0xC3F3;
}
