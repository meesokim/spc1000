.align
.equ  locked,   1
.equ  unlocked, 0
.global _lock_mutex_mmu
_lock_mutex_mmu:
  LDR     r1, =locked
1:
  LDREX   r2, [r0]
  CMP     r2, r1        // Test if mutex is locked or unlocked
  BEQ     2f
  STREXNE r2, r1, [r0]  // Not locked, attempt to lock it
  CMPNE   r2, #1        // Check if Store-Exclusive failed
  BEQ     1b           // Failed - retry from 1
  // Lock acquired
  //DMB                   // Required before accessing protected resource
  BX      lr
2:
// Take appropriate action while waiting for mutex to become unlocked
  //wfi
  nop
  B       1b           // Retry from 1


// BUG: unlock_mutex
// Declare for use from C as extern void unlock_mutex(void * mutex);
.global _unlock_mutex_mmu
_unlock_mutex_mmu:
    LDR     r1, =unlocked
    //DMB                   // Required before releasing protected resource
    STR     r1, [r0]      // Unlock mutex
    // SIGNAL_UPDATE: none
    BX      lr