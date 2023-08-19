MEMORY
{
  ram :  org = 0x0002, len = 0xD7FC
  err :  org = 0xDFFE, len = 0x0002
}

SECTIONS {
	.text: { *(.text) } > ram
	.dtors : { *(.dtors) } > ram
	.ctors : { *(.ctors) } > ram
	.rodata: { *(.rodata) } > ram
	.dpage: { *(.dpage) } > ram
	.data: { *(.data) } > ram AT> ram
	.bss (NOLOAD) : { *(.bss) } > ram
	.errno (NOLOAD) : { *(.errno) } > err

	__BS = ADDR(.bss);
	__BL = SIZEOF(.bss);
	__ENDBSS = ADDR(.bss) + SIZEOF(.bss);
	__DS = ADDR(.data);
	__DC = LOADADDR(.data);
	__DL = SIZEOF(.data);

	__STACK = 0xC3F6;
}
