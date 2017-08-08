ENTRY(main)
OUTPUT_FORMAT(binary)

SECTIONS
{
	. = 0x00100000;

	.text :
	{
		*(.text)
		*(.rodata*)
	}

	.data :
	{
		*(.data)

	}

	.bss :
	{
		*(COMMON)
		*(.bss)
	}
}
