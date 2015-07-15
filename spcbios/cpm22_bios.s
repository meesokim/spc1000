;==============================================================================
;   CPM22 BIOS for SPC-1000
;
;           cpm22_boot.s
;                                   By meeso.kim
;==============================================================================

	.module	bios
	.area	_CODE
	
SDINIT		= 0
SDWRITE		= 1
SDREAD		= 2
SDSEND  	= 3
SDCOPY		= 4
SDFORMAT	= 5
SDSTATUS	= 6
SDDRVSTS	= 7
SDRAMTST	= 8
SDTRANS2	= 9
SDNOACT		= 0xA
SDTRANS1	= 0xB
SDRCVE		= 0xC
SDGO		= 0xD
SDLOAD		= 0xE
SDSAVE		= 0xF
SDLDNGO		= 0x10	
		

