ENTRY(main)

SECTIONS
{
	. = 0x00100000;

	.text : ALIGN(4k)
	{
		*(.text)
		*(.rodata*)
	}

	.data : ALIGN(4k)
	{
		*(.data)

	}

	.bss : ALIGN(4k)
	{
		*(COMMON)
		*(.bss)
	}
}
