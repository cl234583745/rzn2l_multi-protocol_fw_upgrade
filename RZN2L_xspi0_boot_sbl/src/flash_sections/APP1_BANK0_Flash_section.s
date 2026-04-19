/* This file is used to include the application image into the loader project. */

.section .APP1_BANK0_IMAGE_APP_FLASH_section, "ax", %progbits
.incbin "../../RZN2L_xspi0_app1/Debug/RZN2L_xspi0_app1_with_crc.bin"
