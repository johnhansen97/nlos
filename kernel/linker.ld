/* The bootloader will look at this image and start execution at the symbol
   designated as the entry point. */
ENTRY(_loader)

/* Tell where the various sections of the object files will be put in the final
   kernel image. */
SECTIONS
{
	_upper_half = 0xC0000000;
	_kernel_physical_start = 0x00100000;
	_kernel_virtual_start = _upper_half + _kernel_physical_start;
	
	/* Begin putting sections at 1 MiB, a conventional place for kernels to be
	   loaded at by the bootloader. */
	. = _kernel_virtual_start;

	/* First put the multiboot header, as it is required to be put very early
	   early in the image or the bootloader won't recognize the file format.
	   Next we'll put the .text section. */
	.text ALIGN(4k) : AT(ADDR(.text) - 0xC0000000)
	{
		*(.multiboot)
		*(.text)
		*(.rodata*)
	}

	/* Read-write data (initialized) */
	.data ALIGN(4K) : AT(ADDR(.data) - 0xC0000000)
	{
		*(.data)
	}

	/* Read-write data (uninitialized) and stack */
	.bss ALIGN(4k) : AT(ADDR(.bss) - 0xC0000000)
	{
		*(COMMON)
		*(.bss)
	}

	_kernel_virtual_end = ALIGN(4m);
	_kernel_physical_end = ALIGN(4m) - _upper_half;
	/* The compiler may produce other sections, by default it will put them in
	   a segment with the same name. Simply add stuff here as needed. */
}
