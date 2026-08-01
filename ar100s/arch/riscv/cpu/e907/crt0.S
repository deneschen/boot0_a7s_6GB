/* Copyright (c) 2017-2018  SiFive Inc. All rights reserved.

   This copyrighted material is made available to anyone wishing to use,
   modify, copy, or redistribute it subject to the terms and conditions
   of the FreeBSD License.   This program is distributed in the hope that
   it will be useful, but WITHOUT ANY WARRANTY expressed or implied,
   including the implied warranties of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  A copy of this license is available at
   http://www.opensource.org/licenses.
*/

/* crt0.S: Entry point for RISC-V METAL programs. */
/*IMPORT hadle_trap*/
.section .text.libgloss.start
.global _start
.type   _start, @function

  /* _start is defined by the METAL to have been called with the following
   * arguments:
   *   a0: the hart ID of the currently executing hart.  Harts can start at
   *       any arbitrary point, it's the C library's job to ensure the code is
   *       safe.
   *   a1: a pointer to a description of the machine on which this code is
   *       currently executing.  This is probably 0 on an embedded system
   *       because they tend to not be dynamically portable.  As such, newlib
   *       ignores this argument.
   *   a2: a pointer to a function that must be run after the envirnoment has
   *       been initialized, but before user code can be expected to be run.
   *       If this is 0 then there is no function to be run. */
_start:
.cfi_startproc
.cfi_undefined ra

# PART 1: initialize all registers
  li  x1, 0
  li  x2, 0
  li  x3, 0
  li  x4, 0
  li  x5, 0
  li  x6, 0
  li  x7, 0
  li  x8, 0
  li  x9, 0
  li  x10,0
  li  x11,0
  li  x12,0
  li  x13,0
  li  x14,0
  li  x15,0

  la gp, __global_pointer

  .global cpu_0_sp
cpu_0_sp:
 la x2, metal_segment_stack_end
# init stack
 li a3, 0
 la a4, metal_segment_stack_begin
init_sp:
 sw a3,0(a4)
 addi a4,a4,4
 bne a4,x2,init_sp

# PART 3:initialize mtvec value
  la    a3,__trap_handler
  csrw  mtvec,a3
  csrw  0x307,a3   /* mtvt (T-Head/CLIC CSR) */

  # enable mie
  li   a3, 0x8
  csrs mstatus,a3

  /*Zero bss*/
  jal	clean_bss

  jal	startup_entry

  /* Call exit to handle libc's cleanup routines.  Under normal contains this
   * shouldn't even get called, but I'm still not using a tail call here
   * because returning to the METAL is the right thing to do in pathological
   * situations. */
  call exit

  /* And here's where we return.  Again, it's a bit odd but the METAL defines
   * this as a bad idea (ie, as opposed to leaving it undefined) and at this
   * point it's really the only thing left to do. */
  mv ra, s0
  ret

.cfi_endproc

/* RISC-V systems always use __libc_{init,fini}_array, but for compatibility we
 * define _{init,fini} to do nothing. */
.global _init
.type   _init, @function
.global _fini
.type   _fini, @function
_init:
_fini:
  ret
.size _init, .-_init
.size _fini, .-_fini


  .global __exit
__exit:
  addi x10,x0,0x0
  addi x1,x0,0x5a
  addi x2,x0,0x6b
  addi x3,x0,0x7c
  li   x3, 0x44333222
  add x4,x0,x3

  .global __fail
__fail:
  addi x10,x0,0x0
  addi x1,x0,0x2c
  addi x2,x0,0x3b
  li x3,0x82348720

	.global __undefine_exception
.align 6
__trap_handler:
j __synchronous_exception

__synchronous_exception:
  #push

  addi sp,sp,-16*8
  sw   x1,0*8(sp)
  sw   x5,1*8(sp)
  sw   x6,2*8(sp)
  sw   x7,3*8(sp)
  sw   a0,4*8(sp)
  sw   a1,5*8(sp)
  sw   a2,6*8(sp)
  sw   a3,7*8(sp)
  sw   a4,8*8(sp)
  sw   a5,9*8(sp)
  csrr a0,mcause
  csrr a1,mepc
  call hadle_trap
  csrw mepc,a0

  lw   x1,0*8(sp)
  lw   x5,1*8(sp)
  lw   x6,2*8(sp)
  lw   x7,3*8(sp)
  lw   a0,4*8(sp)
  lw   a1,5*8(sp)
  lw   a2,6*8(sp)
  lw   a3,7*8(sp)
  lw   a4,8*8(sp)
  lw   a5,9*8(sp)

  addi sp,sp,16*8
  mret


  .global __dummy
__dummy:

__undefine_exception:
	j __undefine_exception


  /* This shim allows main() to be passed a set of arguments that can satisfy the
 * requirements of the C API. */
.section .rodata.libgloss.start
argv:
.dc.a name
envp:
.dc.a 0
name:
.asciz "libgloss"


