	.text

        .set jexit,	  0xe000
        .set jromgetputc, 0xe002
        .set jromputc,	  0xe004
        .set jspawn,	  0xe006
        .set jsys_chdir,  0xe008
        .set jsys_close,  0xe00a
        .set jsys_dup,	  0xe00c
        .set jsys_fstat,  0xe00e
        .set jsys_link,	  0xe010
        .set jsys_lseek,  0xe012
        .set jsys_mkdir,  0xe014
        .set jsys_open,	  0xe016
        .set jsys_read,	  0xe018
        .set jsys_unlink, 0xe01a
        .set jsys_write,  0xe01c
        .set jtcattr,     0xe01e

_exit:
	.global exit
	.global _exit
	jmp [jexit]


romgetputc:
	.global romgetputc
	jmp [jromgetputc]

romputc:
	.global romputc
	jmp [jromputc]

spawn:
	.global spawn
	jmp [jspawn]

chdir:
sys_chdir:
	.global chdir
	.global sys_chdir
	jmp [jsys_chdir]

close:
sys_close:
	.global close
	.global sys_close
	jmp [jsys_close]

dup:
sys_dup:
	.global dup
	.global sys_dup
	jmp [jsys_dup]

sys_fstat:
	.global sys_fstat
	jmp [jsys_fstat]

link:
sys_link:
	.global link
	.global sys_link
	jmp [jsys_link]

lseek:
sys_lseek:
	.global lseek
	.global sys_lseek
	jmp [jsys_lseek]

mkdir:
sys_mkdir:
	.global mkdir
	.global sys_mkdir
	jmp [jsys_mkdir]

open:
sys_open:
	.global open
	.global sys_open
	jmp [jsys_open]

read:
sys_read:
	.global read
	.global sys_read
	jmp [jsys_read]

unlink:
sys_unlink:
	.global unlink
	.global sys_unlink
	jmp [jsys_unlink]

write:
sys_write:
	.global write
	.global sys_write
	jmp [jsys_write]

_tcattr:
	.global _tcattr
	jmp [jtcattr]
