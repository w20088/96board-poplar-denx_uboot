#include <config.h>

.section .image,#alloc

#ifdef CONFIG_ARM64_SUPPORT
.align	3
.globl arm64_boot_entry
arm64_boot_entry:
#ifdef CONFIG_ARCH_HI3798CV2X
.incbin "../arch/arm/cpu/hi3798cv2x/arm64_boot/arm64_boot.bin"
#endif
.globl arm64_boot_end
arm64_boot_end:
#endif
