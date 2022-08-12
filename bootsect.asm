.code16
.org 0x7c00

_start:
	cli
	movw %cs, %ax #запись адреса сегмента 
	movw %ax, %ds #запись адреса сегмена в качестве значения, указывающего на начало сегмента данных
	movw %ax, %ss #сохранение в качестве сегмета стека
	movw $_start, %sp #сохранение стека в качестве адреса первой инструкции код, после чего стэк будет расти вверх.
	sti
	
	movw $0x0003, %ax #очистка экрана
	int $0x10
	movw $0x1000, %ax #addres of kernel
	movw %ax, %es
	movw $0x0000, %bx
	movb $0x25, %al #сколько секторов считать
	movb $0x01, %cl #номер сегмента
	movb $0x00, %ch #номер цилиндра
	movb $0x00, %dh #номер головки
	movb $0x01, %dl #номер диска
	
	movb $0x02, %ah 
	int $0x13
	
	cli
	lgdt gdt_info
	inb $0x92, %al
	orb $2, %al
	outb %al, $0x92
	movl %cr0, %eax  
	orb $1, %al 
	movl%eax, %cr0
	ljmp $0x8, $protected_mode

.code32

protected_mode:
	movw $0x10, %ax
	movw %ax, %es
	movw %ax, %ds
	movw %ax, %ss
	call 0x10000 #вызвать ядро

gdt_info:
	.word gdt_info -gdt
	.word gdt, 0

gdt:
	.byte 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	.byte 0xff, 0xff, 0x00, 0x00, 0x00, 0x9A, 0xCF, 0x00
	.byte 0xff, 0xff, 0x00, 0x00, 0x00, 0x92, 0xCF, 0x00

.zero (512 -(. - _start) - 2)
.byte 0x55, 0xAA
