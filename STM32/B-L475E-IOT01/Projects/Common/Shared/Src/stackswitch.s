
        #define SHT_PROGBITS 0x1

        PUBLIC savestack
        PUBLIC stack_switch
        PUBLIC stack_restore
        ;; Forward declaration of sections.
        SECTION CSTACK:DATA:NOROOT(3)
      
    

        SECTION `.bss`:DATA:REORDER:NOROOT(2)
        DATA
savestack:
        DS8 4

        SECTION `.text`:CODE:NOROOT(1)
        THUMB
stack_switch:
        MOV      R0,SP
        LDR.N    R1,??DataTable1
        STR      R0,[R1, #+0]
        LDR.N    R0,??DataTable1_1
        MOV      SP,R0
        BX       LR               ;; return

        SECTION `.text`:CODE:NOROOT(1)
        THUMB
stack_restore:
        LDR.N    R0,??DataTable1
        LDR      SP,[R0, #+0]
        BX       LR               ;; return

        SECTION `.text`:CODE:NOROOT(2)
        SECTION_TYPE SHT_PROGBITS, 0
        DATA
??DataTable1:
        DC32     savestack

        SECTION `.text`:CODE:NOROOT(2)
        SECTION_TYPE SHT_PROGBITS, 0
        DATA
??DataTable1_1:
        DCD     sfe(CSTACK)

        SECTION `.iar_vfe_header`:DATA:NOALLOC:NOROOT(2)
        SECTION_TYPE SHT_PROGBITS, 0
        DATA
        DC32 0

        END
// 
// 1 028 bytes in section .bss
//    28 bytes in section .text
// 
//    28 bytes of CODE memory
// 1 028 bytes of DATA memory
//
//Errors: none
//Warnings: none
