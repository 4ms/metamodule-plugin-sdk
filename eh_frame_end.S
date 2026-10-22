// MP2 plugins: ends the plugin's .eh_frame with a zero terminator (what crtend.o does in a normal
// link), so the firmware's unwinder knows where the plugin's unwind info ends when the loader
// registers it. Linked last, so it lands at the end of .eh_frame.
	.section .eh_frame,"a",%progbits
	.4byte 0
