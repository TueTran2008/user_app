	.file	"bdf_rle.c"
	.text
.Ltext0:
	.type	bg_err, @function
bg_err:
.LFB6:
	.file 1 "bdf_rle.c"
	.loc 1 276 1
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$16, %rsp
	movq	%rdi, -8(%rbp)
	.loc 1 277 3
	movq	-8(%rbp), %rax
	movq	%rax, %rdi
	call	puts@PLT
	.loc 1 278 1
	nop
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE6:
	.size	bg_err, .-bg_err
	.type	bg_init_rle, @function
bg_init_rle:
.LFB7:
	.loc 1 281 1
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$16, %rsp
	movq	%rdi, -8(%rbp)
	movl	%esi, -12(%rbp)
	movl	%edx, -16(%rbp)
	.loc 1 282 18
	movq	-8(%rbp), %rax
	movl	$0, 136(%rax)
	.loc 1 283 20
	movq	-8(%rbp), %rax
	movl	$1, 132(%rax)
	.loc 1 284 22
	movq	-8(%rbp), %rax
	movl	-12(%rbp), %edx
	movl	%edx, 124(%rax)
	.loc 1 285 22
	movq	-8(%rbp), %rax
	movl	-16(%rbp), %edx
	movl	%edx, 128(%rax)
	.loc 1 286 18
	movq	-8(%rbp), %rax
	movl	$0, 140(%rax)
	.loc 1 287 18
	movq	-8(%rbp), %rax
	movl	$1, 144(%rax)
	.loc 1 288 3
	movq	-8(%rbp), %rax
	movq	%rax, %rdi
	call	bg_ClearTargetData@PLT
	.loc 1 289 1
	nop
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE7:
	.size	bg_init_rle, .-bg_init_rle
	.section	.rodata
.LC0:
	.string	"error in bg_01_rle 1 0"
.LC1:
	.string	"error in bg_01_rle 1 a"
.LC2:
	.string	"error in bg_01_rle 1 b"
	.text
	.type	bg_01_rle, @function
bg_01_rle:
.LFB8:
	.loc 1 292 1
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$16, %rsp
	movq	%rdi, -8(%rbp)
	movl	%esi, -12(%rbp)
	movl	%edx, -16(%rbp)
	.loc 1 293 10
	movq	-8(%rbp), %rax
	movl	132(%rax), %eax
	.loc 1 293 6
	testl	%eax, %eax
	jne	.L4
	.loc 1 293 35 discriminator 1
	movq	-8(%rbp), %rax
	movl	140(%rax), %eax
	.loc 1 293 30 discriminator 1
	cmpl	%eax, -12(%rbp)
	jne	.L4
	.loc 1 293 58 discriminator 2
	movq	-8(%rbp), %rax
	movl	144(%rax), %eax
	.loc 1 293 53 discriminator 2
	cmpl	%eax, -16(%rbp)
	jne	.L4
	.loc 1 295 7
	movq	-8(%rbp), %rax
	movl	136(%rax), %eax
	.loc 1 295 19
	leal	1(%rax), %edx
	movq	-8(%rbp), %rax
	movl	%edx, 136(%rax)
	.loc 1 296 10
	movq	-8(%rbp), %rax
	movl	$1, %edx
	movl	$1, %esi
	movq	%rax, %rdi
	call	bg_AddTargetBits@PLT
	.loc 1 296 8
	testl	%eax, %eax
	jne	.L7
	.loc 1 297 14
	leaq	.LC0(%rip), %rdi
	call	bg_err
	.loc 1 297 46
	movl	$0, %eax
	jmp	.L6
.L4:
	.loc 1 301 12
	movq	-8(%rbp), %rax
	movl	132(%rax), %eax
	.loc 1 301 8
	testl	%eax, %eax
	jne	.L8
	.loc 1 303 12
	movq	-8(%rbp), %rax
	movl	$0, %edx
	movl	$1, %esi
	movq	%rax, %rdi
	call	bg_AddTargetBits@PLT
	.loc 1 303 10
	testl	%eax, %eax
	jne	.L9
	.loc 1 304 9
	leaq	.LC0(%rip), %rdi
	call	bg_err
	.loc 1 304 41
	movl	$0, %eax
	jmp	.L6
.L9:
	.loc 1 305 9
	movq	-8(%rbp), %rax
	movl	136(%rax), %eax
	.loc 1 305 21
	leal	1(%rax), %edx
	movq	-8(%rbp), %rax
	movl	%edx, 136(%rax)
.L8:
	.loc 1 307 10
	movq	-8(%rbp), %rax
	movl	124(%rax), %ecx
	movl	-12(%rbp), %edx
	movq	-8(%rbp), %rax
	movl	%ecx, %esi
	movq	%rax, %rdi
	call	bg_AddTargetBits@PLT
	.loc 1 307 8
	testl	%eax, %eax
	jne	.L10
	.loc 1 308 14
	leaq	.LC1(%rip), %rdi
	call	bg_err
	.loc 1 308 46
	movl	$0, %eax
	jmp	.L6
.L10:
	.loc 1 309 10
	movq	-8(%rbp), %rax
	movl	128(%rax), %ecx
	movl	-16(%rbp), %edx
	movq	-8(%rbp), %rax
	movl	%ecx, %esi
	movq	%rax, %rdi
	call	bg_AddTargetBits@PLT
	.loc 1 309 8
	testl	%eax, %eax
	jne	.L11
	.loc 1 310 14
	leaq	.LC2(%rip), %rdi
	call	bg_err
	.loc 1 310 46
	movl	$0, %eax
	jmp	.L6
.L11:
	.loc 1 321 22
	movq	-8(%rbp), %rax
	movl	$0, 132(%rax)
	.loc 1 322 20
	movq	-8(%rbp), %rax
	movl	136(%rax), %edx
	.loc 1 322 24
	movq	-8(%rbp), %rax
	movl	124(%rax), %eax
	.loc 1 322 20
	addl	%eax, %edx
	movq	-8(%rbp), %rax
	movl	%edx, 136(%rax)
	.loc 1 323 20
	movq	-8(%rbp), %rax
	movl	136(%rax), %edx
	.loc 1 323 24
	movq	-8(%rbp), %rax
	movl	128(%rax), %eax
	.loc 1 323 20
	addl	%eax, %edx
	movq	-8(%rbp), %rax
	movl	%edx, 136(%rax)
	.loc 1 324 20
	movq	-8(%rbp), %rax
	movl	-12(%rbp), %edx
	movl	%edx, 140(%rax)
	.loc 1 325 20
	movq	-8(%rbp), %rax
	movl	-16(%rbp), %edx
	movl	%edx, 144(%rax)
.L7:
	.loc 1 327 10
	movl	$1, %eax
.L6:
	.loc 1 328 1
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE8:
	.size	bg_01_rle, .-bg_01_rle
	.type	bg_prepare_01_rle, @function
bg_prepare_01_rle:
.LFB9:
	.loc 1 340 1
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$16, %rsp
	movq	%rdi, -8(%rbp)
	movl	%esi, -12(%rbp)
	movl	%edx, -16(%rbp)
	.loc 1 342 8
	jmp	.L13
.L16:
	.loc 1 344 30
	movq	-8(%rbp), %rax
	movl	124(%rax), %eax
	.loc 1 344 26
	movl	$1, %edx
	movl	%eax, %ecx
	sall	%cl, %edx
	movl	%edx, %eax
	.loc 1 344 48
	subl	$1, %eax
	.loc 1 344 10
	movl	%eax, %ecx
	movq	-8(%rbp), %rax
	movl	$0, %edx
	movl	%ecx, %esi
	movq	%rax, %rdi
	call	bg_01_rle
	.loc 1 344 8
	testl	%eax, %eax
	jne	.L14
	.loc 1 345 14
	movl	$0, %eax
	jmp	.L15
.L14:
	.loc 1 346 16
	movq	-8(%rbp), %rax
	movl	124(%rax), %eax
	.loc 1 346 12
	movl	$1, %edx
	movl	%eax, %ecx
	sall	%cl, %edx
	movl	%edx, %eax
	.loc 1 346 7
	movl	%eax, %edx
	movl	-12(%rbp), %eax
	subl	%edx, %eax
	addl	$1, %eax
	movl	%eax, -12(%rbp)
.L13:
	.loc 1 342 21
	movq	-8(%rbp), %rax
	movl	124(%rax), %eax
	.loc 1 342 17
	movl	$1, %edx
	movl	%eax, %ecx
	sall	%cl, %edx
	movl	%edx, %eax
	.loc 1 342 39
	subl	$1, %eax
	.loc 1 342 8
	cmpl	%eax, -12(%rbp)
	jnb	.L16
	.loc 1 348 8
	jmp	.L17
.L19:
	.loc 1 350 33
	movq	-8(%rbp), %rax
	movl	128(%rax), %eax
	.loc 1 350 29
	movl	$1, %edx
	movl	%eax, %ecx
	sall	%cl, %edx
	movl	%edx, %eax
	.loc 1 350 51
	subl	$1, %eax
	.loc 1 350 10
	movl	%eax, %edx
	movl	-12(%rbp), %ecx
	movq	-8(%rbp), %rax
	movl	%ecx, %esi
	movq	%rax, %rdi
	call	bg_01_rle
	.loc 1 350 8
	testl	%eax, %eax
	jne	.L18
	.loc 1 351 14
	movl	$0, %eax
	jmp	.L15
.L18:
	.loc 1 352 7
	movl	$0, -12(%rbp)
	.loc 1 353 16
	movq	-8(%rbp), %rax
	movl	128(%rax), %eax
	.loc 1 353 12
	movl	$1, %edx
	movl	%eax, %ecx
	sall	%cl, %edx
	movl	%edx, %eax
	.loc 1 353 7
	movl	%eax, %edx
	movl	-16(%rbp), %eax
	subl	%edx, %eax
	addl	$1, %eax
	movl	%eax, -16(%rbp)
.L17:
	.loc 1 348 21
	movq	-8(%rbp), %rax
	movl	128(%rax), %eax
	.loc 1 348 17
	movl	$1, %edx
	movl	%eax, %ecx
	sall	%cl, %edx
	movl	%edx, %eax
	.loc 1 348 39
	subl	$1, %eax
	.loc 1 348 8
	cmpl	%eax, -16(%rbp)
	jnb	.L19
	.loc 1 355 6
	cmpl	$0, -12(%rbp)
	jne	.L20
	.loc 1 355 15 discriminator 1
	cmpl	$0, -16(%rbp)
	je	.L21
.L20:
	.loc 1 356 10
	movl	-16(%rbp), %edx
	movl	-12(%rbp), %ecx
	movq	-8(%rbp), %rax
	movl	%ecx, %esi
	movq	%rax, %rdi
	call	bg_01_rle
	.loc 1 356 8
	testl	%eax, %eax
	jne	.L21
	.loc 1 357 14
	movl	$0, %eax
	jmp	.L15
.L21:
	.loc 1 358 10
	movl	$1, %eax
.L15:
	.loc 1 359 1
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE9:
	.size	bg_prepare_01_rle, .-bg_prepare_01_rle
	.section	.rodata
.LC3:
	.string	"error in bg_rle_compress"
.LC4:
	.string	"bdf_rle.c"
.LC5:
	.string	"bd_is_one == 0"
	.text
	.globl	bg_rle_compress
	.type	bg_rle_compress, @function
bg_rle_compress:
.LFB10:
	.loc 1 363 1
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$64, %rsp
	movq	%rdi, -40(%rbp)
	movq	%rsi, -48(%rbp)
	movl	%edx, -52(%rbp)
	movl	%ecx, -56(%rbp)
	movl	%r8d, -60(%rbp)
	.loc 1 374 6
	cmpq	$0, -48(%rbp)
	jne	.L23
	.loc 1 375 9
	movq	-40(%rbp), %rax
	addq	$40, %rax
	movq	%rax, -48(%rbp)
.L23:
	.loc 1 377 3
	movl	-56(%rbp), %edx
	movl	-52(%rbp), %ecx
	movq	-40(%rbp), %rax
	movl	%ecx, %esi
	movq	%rax, %rdi
	call	bg_init_rle
	.loc 1 382 10
	movq	-40(%rbp), %rax
	movq	16(%rax), %rax
	.loc 1 382 6
	cmpq	$255, %rax
	jg	.L24
	.loc 1 384 33
	movq	-40(%rbp), %rax
	movq	16(%rax), %rax
	.loc 1 384 10
	movzbl	%al, %edx
	movq	-40(%rbp), %rax
	movl	%edx, %esi
	movq	%rax, %rdi
	call	bg_AddTargetData@PLT
	.loc 1 384 8
	testl	%eax, %eax
	jns	.L25
	.loc 1 385 14
	leaq	.LC3(%rip), %rdi
	call	bg_err
	.loc 1 385 48
	movl	$0, %eax
	jmp	.L26
.L24:
	.loc 1 389 33
	movq	-40(%rbp), %rax
	movq	16(%rax), %rax
	.loc 1 389 42
	sarq	$8, %rax
	.loc 1 389 10
	movzbl	%al, %edx
	movq	-40(%rbp), %rax
	movl	%edx, %esi
	movq	%rax, %rdi
	call	bg_AddTargetData@PLT
	.loc 1 389 8
	testl	%eax, %eax
	jns	.L27
	.loc 1 390 14
	leaq	.LC3(%rip), %rdi
	call	bg_err
	.loc 1 390 48
	movl	$0, %eax
	jmp	.L26
.L27:
	.loc 1 391 33
	movq	-40(%rbp), %rax
	movq	16(%rax), %rax
	.loc 1 391 10
	movzbl	%al, %edx
	movq	-40(%rbp), %rax
	movl	%edx, %esi
	movq	%rax, %rdi
	call	bg_AddTargetData@PLT
	.loc 1 391 8
	testl	%eax, %eax
	jns	.L25
	.loc 1 392 14
	leaq	.LC3(%rip), %rdi
	call	bg_err
	.loc 1 392 48
	movl	$0, %eax
	jmp	.L26
.L25:
	.loc 1 395 8
	movq	-40(%rbp), %rax
	movl	$0, %esi
	movq	%rax, %rdi
	call	bg_AddTargetData@PLT
	.loc 1 395 6
	testl	%eax, %eax
	jns	.L28
	.loc 1 396 12
	leaq	.LC3(%rip), %rdi
	call	bg_err
	.loc 1 396 46
	movl	$0, %eax
	jmp	.L26
.L28:
	.loc 1 402 60
	movq	-48(%rbp), %rax
	movq	(%rax), %rax
	.loc 1 402 8
	movl	%eax, %edx
	.loc 1 402 31
	movq	-40(%rbp), %rax
	movq	(%rax), %rax
	.loc 1 402 35
	movl	1272(%rax), %eax
	.loc 1 402 8
	movl	%eax, %ecx
	movq	-40(%rbp), %rax
	movl	%ecx, %esi
	movq	%rax, %rdi
	call	bg_AddTargetBits@PLT
	.loc 1 402 6
	testl	%eax, %eax
	jne	.L29
	.loc 1 403 12
	leaq	.LC3(%rip), %rdi
	call	bg_err
	.loc 1 403 46
	movl	$0, %eax
	jmp	.L26
.L29:
	.loc 1 404 60
	movq	-48(%rbp), %rax
	movq	8(%rax), %rax
	.loc 1 404 8
	movl	%eax, %edx
	.loc 1 404 31
	movq	-40(%rbp), %rax
	movq	(%rax), %rax
	.loc 1 404 35
	movl	1276(%rax), %eax
	.loc 1 404 8
	movl	%eax, %ecx
	movq	-40(%rbp), %rax
	movl	%ecx, %esi
	movq	%rax, %rdi
	call	bg_AddTargetBits@PLT
	.loc 1 404 6
	testl	%eax, %eax
	jne	.L30
	.loc 1 405 12
	leaq	.LC3(%rip), %rdi
	call	bg_err
	.loc 1 405 46
	movl	$0, %eax
	jmp	.L26
.L30:
	.loc 1 406 60
	movq	-48(%rbp), %rax
	movq	16(%rax), %rax
	.loc 1 406 8
	movl	%eax, %edx
	.loc 1 406 73
	movq	-40(%rbp), %rax
	movq	(%rax), %rax
	.loc 1 406 77
	movl	1264(%rax), %eax
	.loc 1 406 97
	subl	$1, %eax
	.loc 1 406 68
	movl	$1, %esi
	movl	%eax, %ecx
	sall	%cl, %esi
	movl	%esi, %eax
	.loc 1 406 8
	addl	%eax, %edx
	.loc 1 406 31
	movq	-40(%rbp), %rax
	movq	(%rax), %rax
	.loc 1 406 35
	movl	1264(%rax), %eax
	.loc 1 406 8
	movl	%eax, %ecx
	movq	-40(%rbp), %rax
	movl	%ecx, %esi
	movq	%rax, %rdi
	call	bg_AddTargetBits@PLT
	.loc 1 406 6
	testl	%eax, %eax
	jne	.L31
	.loc 1 407 12
	leaq	.LC3(%rip), %rdi
	call	bg_err
	.loc 1 407 46
	movl	$0, %eax
	jmp	.L26
.L31:
	.loc 1 408 60
	movq	-48(%rbp), %rax
	movq	24(%rax), %rax
	.loc 1 408 8
	movl	%eax, %edx
	.loc 1 408 73
	movq	-40(%rbp), %rax
	movq	(%rax), %rax
	.loc 1 408 77
	movl	1268(%rax), %eax
	.loc 1 408 97
	subl	$1, %eax
	.loc 1 408 68
	movl	$1, %esi
	movl	%eax, %ecx
	sall	%cl, %esi
	movl	%esi, %eax
	.loc 1 408 8
	addl	%eax, %edx
	.loc 1 408 31
	movq	-40(%rbp), %rax
	movq	(%rax), %rax
	.loc 1 408 35
	movl	1268(%rax), %eax
	.loc 1 408 8
	movl	%eax, %ecx
	movq	-40(%rbp), %rax
	movl	%ecx, %esi
	movq	%rax, %rdi
	call	bg_AddTargetBits@PLT
	.loc 1 408 6
	testl	%eax, %eax
	jne	.L32
	.loc 1 409 12
	leaq	.LC3(%rip), %rdi
	call	bg_err
	.loc 1 409 46
	movl	$0, %eax
	jmp	.L26
.L32:
	.loc 1 411 10
	movq	-40(%rbp), %rax
	movq	(%rax), %rax
	.loc 1 411 14
	movl	1160(%rax), %eax
	.loc 1 411 6
	testl	%eax, %eax
	jne	.L33
	.loc 1 413 58
	movq	-40(%rbp), %rax
	movq	24(%rax), %rax
	.loc 1 413 10
	movl	%eax, %edx
	.loc 1 413 78
	movq	-40(%rbp), %rax
	movq	(%rax), %rax
	.loc 1 413 82
	movl	1280(%rax), %eax
	.loc 1 413 99
	subl	$1, %eax
	.loc 1 413 73
	movl	$1, %esi
	movl	%eax, %ecx
	sall	%cl, %esi
	movl	%esi, %eax
	.loc 1 413 10
	addl	%eax, %edx
	.loc 1 413 33
	movq	-40(%rbp), %rax
	movq	(%rax), %rax
	.loc 1 413 37
	movl	1280(%rax), %eax
	.loc 1 413 10
	movl	%eax, %ecx
	movq	-40(%rbp), %rax
	movl	%ecx, %esi
	movq	%rax, %rdi
	call	bg_AddTargetBits@PLT
	.loc 1 413 8
	testl	%eax, %eax
	jne	.L34
	.loc 1 414 14
	leaq	.LC3(%rip), %rdi
	call	bg_err
	.loc 1 414 48
	movl	$0, %eax
	jmp	.L26
.L33:
	.loc 1 416 15
	movq	-40(%rbp), %rax
	movq	(%rax), %rax
	.loc 1 416 19
	movl	1160(%rax), %eax
	.loc 1 416 11
	cmpl	$2, %eax
	jne	.L35
	.loc 1 418 59
	movq	-48(%rbp), %rax
	movq	(%rax), %rax
	.loc 1 418 10
	movl	%eax, %edx
	.loc 1 418 71
	movq	-40(%rbp), %rax
	movq	(%rax), %rax
	.loc 1 418 75
	movl	1280(%rax), %eax
	.loc 1 418 92
	subl	$1, %eax
	.loc 1 418 66
	movl	$1, %esi
	movl	%eax, %ecx
	sall	%cl, %esi
	movl	%esi, %eax
	.loc 1 418 10
	addl	%eax, %edx
	.loc 1 418 33
	movq	-40(%rbp), %rax
	movq	(%rax), %rax
	.loc 1 418 37
	movl	1280(%rax), %eax
	.loc 1 418 10
	movl	%eax, %ecx
	movq	-40(%rbp), %rax
	movl	%ecx, %esi
	movq	%rax, %rdi
	call	bg_AddTargetBits@PLT
	.loc 1 418 8
	testl	%eax, %eax
	jne	.L34
	.loc 1 419 14
	leaq	.LC3(%rip), %rdi
	call	bg_err
	.loc 1 419 48
	movl	$0, %eax
	jmp	.L26
.L35:
	.loc 1 423 59
	movq	-48(%rbp), %rax
	movq	(%rax), %rax
	.loc 1 423 10
	movl	%eax, %edx
	.loc 1 423 71
	movq	-40(%rbp), %rax
	movq	(%rax), %rax
	.loc 1 423 75
	movl	1280(%rax), %eax
	.loc 1 423 92
	subl	$1, %eax
	.loc 1 423 66
	movl	$1, %esi
	movl	%eax, %ecx
	sall	%cl, %esi
	movl	%esi, %eax
	.loc 1 423 10
	addl	%eax, %edx
	.loc 1 423 33
	movq	-40(%rbp), %rax
	movq	(%rax), %rax
	.loc 1 423 37
	movl	1280(%rax), %eax
	.loc 1 423 10
	movl	%eax, %ecx
	movq	-40(%rbp), %rax
	movl	%ecx, %esi
	movq	%rax, %rdi
	call	bg_AddTargetBits@PLT
	.loc 1 423 8
	testl	%eax, %eax
	jne	.L34
	.loc 1 424 14
	leaq	.LC3(%rip), %rdi
	call	bg_err
	.loc 1 424 48
	movl	$0, %eax
	jmp	.L26
.L34:
	.loc 1 427 13
	movl	$0, -16(%rbp)
	.loc 1 428 15
	movl	$0, -20(%rbp)
	.loc 1 429 14
	movl	$0, -24(%rbp)
	.loc 1 430 14
	movl	$0, -28(%rbp)
	.loc 1 434 15
	movq	-48(%rbp), %rax
	movq	24(%rax), %rax
	.loc 1 434 18
	movl	%eax, %edx
	.loc 1 434 22
	movq	-48(%rbp), %rax
	movq	8(%rax), %rax
	.loc 1 434 18
	addl	%edx, %eax
	.loc 1 434 25
	subl	$1, %eax
	.loc 1 434 10
	movl	%eax, -8(%rbp)
	.loc 1 434 3
	jmp	.L36
.L44:
	.loc 1 436 17
	movq	-48(%rbp), %rax
	movq	16(%rax), %rax
	.loc 1 436 12
	movl	%eax, -4(%rbp)
	.loc 1 436 5
	jmp	.L37
.L43:
	.loc 1 438 12
	movl	-8(%rbp), %edx
	movl	-4(%rbp), %ecx
	movq	-40(%rbp), %rax
	movl	%ecx, %esi
	movq	%rax, %rdi
	call	bg_GetBBXPixel@PLT
	.loc 1 438 10
	testl	%eax, %eax
	jne	.L38
	.loc 1 440 5
	cmpl	$0, -16(%rbp)
	je	.L39
	.loc 1 442 24
	movl	-28(%rbp), %eax
	cltq
	leaq	0(,%rax,4), %rcx
	leaq	bd_list.3193(%rip), %rdx
	movl	-20(%rbp), %eax
	movl	%eax, (%rcx,%rdx)
	.loc 1 443 14
	movl	$0, -16(%rbp)
	.loc 1 444 14
	addl	$1, -28(%rbp)
	.loc 1 445 16
	movl	$0, -20(%rbp)
.L39:
	.loc 1 447 13
	addl	$1, -20(%rbp)
	jmp	.L40
.L38:
	.loc 1 451 5
	cmpl	$0, -16(%rbp)
	jne	.L41
	.loc 1 453 24
	movl	-28(%rbp), %eax
	cltq
	leaq	0(,%rax,4), %rcx
	leaq	bd_list.3193(%rip), %rdx
	movl	-20(%rbp), %eax
	movl	%eax, (%rcx,%rdx)
	.loc 1 454 14
	movl	$1, -16(%rbp)
	.loc 1 455 14
	addl	$1, -28(%rbp)
	.loc 1 456 16
	movl	$0, -20(%rbp)
.L41:
	.loc 1 458 13
	addl	$1, -20(%rbp)
.L40:
	.loc 1 461 10
	movl	-24(%rbp), %eax
	cmpl	-20(%rbp), %eax
	jge	.L42
	.loc 1 462 13
	movl	-20(%rbp), %eax
	movl	%eax, -24(%rbp)
.L42:
	.loc 1 436 44 discriminator 2
	addl	$1, -4(%rbp)
.L37:
	.loc 1 436 24 discriminator 1
	movl	-4(%rbp), %eax
	movslq	%eax, %rdx
	.loc 1 436 29 discriminator 1
	movq	-48(%rbp), %rax
	movq	16(%rax), %rcx
	.loc 1 436 38 discriminator 1
	movq	-48(%rbp), %rax
	movq	(%rax), %rax
	.loc 1 436 33 discriminator 1
	addq	%rcx, %rax
	.loc 1 436 5 discriminator 1
	cmpq	%rax, %rdx
	jl	.L43
	.loc 1 434 43 discriminator 2
	subl	$1, -8(%rbp)
.L36:
	.loc 1 434 31 discriminator 1
	movl	-8(%rbp), %eax
	movslq	%eax, %rdx
	.loc 1 434 37 discriminator 1
	movq	-48(%rbp), %rax
	movq	24(%rax), %rax
	.loc 1 434 3 discriminator 1
	cmpq	%rax, %rdx
	jge	.L44
	.loc 1 467 23
	movl	-28(%rbp), %eax
	cltq
	leaq	0(,%rax,4), %rcx
	leaq	bd_list.3193(%rip), %rdx
	movl	-20(%rbp), %eax
	movl	%eax, (%rcx,%rdx)
	.loc 1 468 13
	addl	$1, -28(%rbp)
	.loc 1 470 20
	movl	-28(%rbp), %eax
	andl	$1, %eax
	.loc 1 470 6
	testl	%eax, %eax
	je	.L45
	.loc 1 472 5
	cmpl	$0, -16(%rbp)
	je	.L46
	.loc 1 472 5 is_stmt 0 discriminator 1
	leaq	__PRETTY_FUNCTION__.3200(%rip), %rcx
	movl	$472, %edx
	leaq	.LC4(%rip), %rsi
	leaq	.LC5(%rip), %rdi
	call	__assert_fail@PLT
.L46:
	.loc 1 473 25 is_stmt 1
	movl	-28(%rbp), %eax
	cltq
	leaq	0(,%rax,4), %rdx
	leaq	bd_list.3193(%rip), %rax
	movl	$0, (%rdx,%rax)
	.loc 1 474 15
	addl	$1, -28(%rbp)
.L45:
	.loc 1 484 10
	movl	$0, -12(%rbp)
	.loc 1 484 3
	jmp	.L47
.L49:
	.loc 1 489 53
	movl	-12(%rbp), %eax
	addl	$1, %eax
	.loc 1 489 51
	cltq
	leaq	0(,%rax,4), %rdx
	leaq	bd_list.3193(%rip), %rax
	movl	(%rdx,%rax), %eax
	.loc 1 489 10
	movl	%eax, %esi
	.loc 1 489 39
	movl	-12(%rbp), %eax
	cltq
	leaq	0(,%rax,4), %rdx
	leaq	bd_list.3193(%rip), %rax
	movl	(%rdx,%rax), %eax
	.loc 1 489 10
	movl	%eax, %ecx
	movq	-40(%rbp), %rax
	movl	%esi, %edx
	movl	%ecx, %esi
	movq	%rax, %rdi
	call	bg_prepare_01_rle
	.loc 1 489 8
	testl	%eax, %eax
	jne	.L48
	.loc 1 490 14
	movl	$0, %eax
	jmp	.L26
.L48:
	.loc 1 484 32 discriminator 2
	addl	$2, -12(%rbp)
.L47:
	.loc 1 484 3 discriminator 1
	movl	-12(%rbp), %eax
	cmpl	-28(%rbp), %eax
	jl	.L49
	.loc 1 497 8
	movq	-40(%rbp), %rax
	movl	$0, %edx
	movl	$1, %esi
	movq	%rax, %rdi
	call	bg_AddTargetBits@PLT
	.loc 1 497 6
	testl	%eax, %eax
	jne	.L50
	.loc 1 498 12
	movl	$0, %eax
	jmp	.L26
.L50:
	.loc 1 500 8
	movq	-40(%rbp), %rax
	movq	%rax, %rdi
	call	bg_FlushTargetBits@PLT
	.loc 1 500 6
	testl	%eax, %eax
	jne	.L51
	.loc 1 501 12
	movl	$0, %eax
	jmp	.L26
.L51:
	.loc 1 504 10
	movq	-40(%rbp), %rax
	movq	16(%rax), %rax
	.loc 1 504 6
	cmpq	$255, %rax
	jg	.L52
	.loc 1 506 28
	movq	-40(%rbp), %rax
	movl	116(%rax), %edx
	.loc 1 506 7
	movq	-40(%rbp), %rax
	movq	104(%rax), %rax
	.loc 1 506 20
	addq	$1, %rax
	.loc 1 506 24
	movb	%dl, (%rax)
	jmp	.L53
.L52:
	.loc 1 510 28
	movq	-40(%rbp), %rax
	movl	116(%rax), %edx
	.loc 1 510 7
	movq	-40(%rbp), %rax
	movq	104(%rax), %rax
	.loc 1 510 20
	addq	$2, %rax
	.loc 1 510 24
	movb	%dl, (%rax)
.L53:
	.loc 1 526 10
	movl	$1, %eax
.L26:
	.loc 1 527 1
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE10:
	.size	bg_rle_compress, .-bg_rle_compress
	.section	.rodata
	.align 8
.LC6:
	.string	"RLE Compress: Encoding %ld bits %u/%u"
	.text
	.globl	bf_RLECompressAllGlyphsWithFieldSize
	.type	bf_RLECompressAllGlyphsWithFieldSize, @function
bf_RLECompressAllGlyphsWithFieldSize:
.LFB11:
	.loc 1 530 1
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$96, %rsp
	movq	%rdi, -72(%rbp)
	movl	%esi, -76(%rbp)
	movl	%edx, -80(%rbp)
	movl	%ecx, -84(%rbp)
	.loc 1 533 17
	movq	$0, -16(%rbp)
	.loc 1 536 10
	movl	$0, -4(%rbp)
	.loc 1 536 3
	jmp	.L55
.L57:
	.loc 1 538 12
	movq	-72(%rbp), %rax
	movq	8(%rax), %rax
	.loc 1 538 24
	movl	-4(%rbp), %edx
	movslq	%edx, %rdx
	salq	$3, %rdx
	addq	%rdx, %rax
	.loc 1 538 8
	movq	(%rax), %rax
	movq	%rax, -24(%rbp)
	.loc 1 539 12
	movq	-24(%rbp), %rax
	movq	16(%rax), %rax
	.loc 1 539 8
	testq	%rax, %rax
	js	.L56
	.loc 1 541 7
	movq	-24(%rbp), %rdx
	leaq	-64(%rbp), %rcx
	movq	-72(%rbp), %rax
	movq	%rcx, %rsi
	movq	%rax, %rdi
	call	bf_copy_bbx_and_update_shift@PLT
	.loc 1 597 7
	movl	-80(%rbp), %ecx
	movl	-76(%rbp), %edx
	movl	-84(%rbp), %edi
	leaq	-64(%rbp), %rsi
	movq	-24(%rbp), %rax
	movl	%edi, %r8d
	movq	%rax, %rdi
	call	bg_rle_compress
	.loc 1 598 23
	movq	-24(%rbp), %rax
	movl	116(%rax), %eax
	.loc 1 598 35
	leal	0(,%rax,8), %edx
	.loc 1 598 40
	movq	-24(%rbp), %rax
	movl	120(%rax), %eax
	.loc 1 598 37
	addl	%edx, %eax
	cltq
	.loc 1 598 18
	addq	%rax, -16(%rbp)
	.loc 1 599 10
	cmpl	$0, -84(%rbp)
	je	.L56
	.loc 1 601 86
	movq	-24(%rbp), %rax
	movl	116(%rax), %eax
	.loc 1 601 98
	leal	0(,%rax,8), %edx
	.loc 1 601 103
	movq	-24(%rbp), %rax
	movl	120(%rax), %eax
	.loc 1 601 2
	leal	(%rdx,%rax), %esi
	movq	-24(%rbp), %rax
	movl	136(%rax), %ecx
	movq	-24(%rbp), %rax
	movq	8(%rax), %rdx
	movq	-72(%rbp), %rax
	movl	%esi, %r8d
	leaq	.LC6(%rip), %rsi
	movq	%rax, %rdi
	movl	$0, %eax
	call	bf_Log@PLT
.L56:
	.loc 1 536 35 discriminator 2
	addl	$1, -4(%rbp)
.L55:
	.loc 1 536 21 discriminator 1
	movq	-72(%rbp), %rax
	movl	16(%rax), %eax
	.loc 1 536 3 discriminator 1
	cmpl	%eax, -4(%rbp)
	jl	.L57
	.loc 1 606 10
	movq	-16(%rbp), %rax
	.loc 1 607 1
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE11:
	.size	bf_RLECompressAllGlyphsWithFieldSize, .-bf_RLECompressAllGlyphsWithFieldSize
	.globl	bf_RLE_get_glyph_data
	.type	bf_RLE_get_glyph_data, @function
bf_RLE_get_glyph_data:
.LFB12:
	.loc 1 611 1
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movq	%rdi, -24(%rbp)
	movl	%esi, %eax
	movb	%al, -28(%rbp)
	.loc 1 612 12
	movq	-24(%rbp), %rax
	movq	1288(%rax), %rax
	movq	%rax, -8(%rbp)
	.loc 1 613 8
	addq	$23, -8(%rbp)
.L64:
	.loc 1 616 14
	movq	-8(%rbp), %rax
	addq	$1, %rax
	movzbl	(%rax), %eax
	.loc 1 616 8
	testb	%al, %al
	je	.L66
	.loc 1 618 14
	movq	-8(%rbp), %rax
	movzbl	(%rax), %eax
	.loc 1 618 8
	cmpb	%al, -28(%rbp)
	jne	.L62
	.loc 1 620 22
	movq	-24(%rbp), %rax
	movq	1288(%rax), %rax
	.loc 1 620 19
	movq	-8(%rbp), %rdx
	subq	%rax, %rdx
	movq	%rdx, %rax
	.loc 1 620 36
	subl	$23, %eax
	jmp	.L63
.L62:
	.loc 1 622 17
	movq	-8(%rbp), %rax
	addq	$1, %rax
	movzbl	(%rax), %eax
	movzbl	%al, %eax
	.loc 1 622 10
	addq	%rax, -8(%rbp)
	.loc 1 616 8
	jmp	.L64
.L66:
	.loc 1 617 7
	nop
	.loc 1 624 10
	movl	$0, %eax
.L63:
	.loc 1 625 1
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE12:
	.size	bf_RLE_get_glyph_data, .-bf_RLE_get_glyph_data
	.section	.rodata
	.align 8
.LC7:
	.string	"RLE Compress: best zero bits %d, one bits %d, total bit size %lu"
	.align 8
.LC8:
	.string	"RLE Compress: Font code generation, selected glyphs=%d, total glyphs=%d"
	.align 8
.LC9:
	.string	"RLE Compress: Error, glyph too large, encoding=%ld cnt=%d"
	.align 8
.LC10:
	.string	"RLE Compress: ASCII gylphs=%d, Unicode glyphs=%d"
	.align 8
.LC11:
	.string	"RLE Compress: Glyphs per unicode lookup table entry=%d"
	.align 8
.LC12:
	.string	"RLE Compress: Error, glyph too large, encoding=%ld"
	.align 8
.LC13:
	.string	"RLE Compress: Unicode lookup table len=%d, written entries=%d"
	.align 8
.LC14:
	.string	"RLE Compress: Unicode lookup table first entry: delta=%d, encoding=%d"
	.align 8
.LC15:
	.string	"RLE Compress: Unicode lookup table last entry: delta=%d, encoding=%d"
	.align 8
.LC16:
	.string	"unicode_lookup_table_len == unicode_lookup_table_pos"
	.align 8
.LC17:
	.string	"RLE Compress: 'A' pos = %u, 'a' pos = %u"
.LC18:
	.string	"RLE Compress: Font size %d"
	.text
	.globl	bf_RLECompressAllGlyphs
	.type	bf_RLECompressAllGlyphs, @function
bf_RLECompressAllGlyphs:
.LFB13:
	.loc 1 629 1
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	pushq	%rbx
	subq	$152, %rsp
	.cfi_offset 3, -24
	movq	%rdi, -152(%rbp)
	.loc 1 634 7
	movl	$0, -36(%rbp)
	.loc 1 634 21
	movl	$0, -40(%rbp)
	.loc 1 635 17
	movq	$0, -96(%rbp)
	.loc 1 636 17
	movl	$4294967295, %eax
	movq	%rax, -48(%rbp)
	.loc 1 658 20
	movl	$0, -52(%rbp)
	.loc 1 659 15
	movq	-152(%rbp), %rax
	movl	$65, %esi
	movq	%rax, %rdi
	call	bf_GetIndexByEncoding@PLT
	movl	%eax, -100(%rbp)
	.loc 1 660 6
	cmpl	$0, -100(%rbp)
	js	.L68
	.loc 1 662 26
	movq	-152(%rbp), %rax
	movq	8(%rax), %rax
	.loc 1 662 38
	movl	-100(%rbp), %edx
	movslq	%edx, %rdx
	salq	$3, %rdx
	addq	%rdx, %rax
	movq	(%rax), %rax
	.loc 1 662 54
	movq	48(%rax), %rax
	.loc 1 662 56
	movl	%eax, %ecx
	.loc 1 662 59
	movq	-152(%rbp), %rax
	movq	8(%rax), %rax
	.loc 1 662 71
	movl	-100(%rbp), %edx
	movslq	%edx, %rdx
	salq	$3, %rdx
	addq	%rdx, %rax
	movq	(%rax), %rax
	.loc 1 662 87
	movq	64(%rax), %rax
	.loc 1 662 56
	addl	%ecx, %eax
	.loc 1 662 22
	movl	%eax, -52(%rbp)
.L68:
	.loc 1 665 16
	movl	$0, -56(%rbp)
	.loc 1 666 11
	movq	-152(%rbp), %rax
	movl	$49, %esi
	movq	%rax, %rdi
	call	bf_GetIndexByEncoding@PLT
	movl	%eax, -104(%rbp)
	.loc 1 667 6
	cmpl	$0, -104(%rbp)
	js	.L69
	.loc 1 669 22
	movq	-152(%rbp), %rax
	movq	8(%rax), %rax
	.loc 1 669 34
	movl	-104(%rbp), %edx
	movslq	%edx, %rdx
	salq	$3, %rdx
	addq	%rdx, %rax
	movq	(%rax), %rax
	.loc 1 669 46
	movq	48(%rax), %rax
	.loc 1 669 48
	movl	%eax, %ecx
	.loc 1 669 51
	movq	-152(%rbp), %rax
	movq	8(%rax), %rax
	.loc 1 669 63
	movl	-104(%rbp), %edx
	movslq	%edx, %rdx
	salq	$3, %rdx
	addq	%rdx, %rax
	movq	(%rax), %rax
	.loc 1 669 75
	movq	64(%rax), %rax
	.loc 1 669 48
	addl	%ecx, %eax
	.loc 1 669 18
	movl	%eax, -56(%rbp)
.L69:
	.loc 1 672 17
	movl	$0, -60(%rbp)
	.loc 1 673 11
	movq	-152(%rbp), %rax
	movl	$103, %esi
	movq	%rax, %rdi
	call	bf_GetIndexByEncoding@PLT
	movl	%eax, -108(%rbp)
	.loc 1 674 6
	cmpl	$0, -108(%rbp)
	js	.L70
	.loc 1 676 23
	movq	-152(%rbp), %rax
	movq	8(%rax), %rax
	.loc 1 676 35
	movl	-108(%rbp), %edx
	movslq	%edx, %rdx
	salq	$3, %rdx
	addq	%rdx, %rax
	movq	(%rax), %rax
	.loc 1 676 47
	movq	64(%rax), %rax
	.loc 1 676 19
	movl	%eax, -60(%rbp)
.L70:
	.loc 1 680 19
	movl	$0, -64(%rbp)
	.loc 1 681 14
	movq	-152(%rbp), %rax
	movl	$40, %esi
	movq	%rax, %rdi
	call	bf_GetIndexByEncoding@PLT
	movl	%eax, -112(%rbp)
	.loc 1 682 6
	cmpl	$0, -112(%rbp)
	js	.L71
	.loc 1 684 25
	movq	-152(%rbp), %rax
	movq	8(%rax), %rax
	.loc 1 684 37
	movl	-112(%rbp), %edx
	movslq	%edx, %rdx
	salq	$3, %rdx
	addq	%rdx, %rax
	movq	(%rax), %rax
	.loc 1 684 52
	movq	48(%rax), %rax
	.loc 1 684 54
	movl	%eax, %ecx
	.loc 1 684 57
	movq	-152(%rbp), %rax
	movq	8(%rax), %rax
	.loc 1 684 69
	movl	-112(%rbp), %edx
	movslq	%edx, %rdx
	salq	$3, %rdx
	addq	%rdx, %rax
	movq	(%rax), %rax
	.loc 1 684 84
	movq	64(%rax), %rax
	.loc 1 684 54
	addl	%ecx, %eax
	.loc 1 684 21
	movl	%eax, -64(%rbp)
	.loc 1 685 26
	movq	-152(%rbp), %rax
	movq	8(%rax), %rax
	.loc 1 685 38
	movl	-112(%rbp), %edx
	movslq	%edx, %rdx
	salq	$3, %rdx
	addq	%rdx, %rax
	movq	(%rax), %rax
	.loc 1 685 53
	movq	64(%rax), %rax
	.loc 1 685 22
	movl	%eax, -68(%rbp)
	jmp	.L72
.L71:
	.loc 1 689 21
	movl	-52(%rbp), %eax
	movl	%eax, -64(%rbp)
	.loc 1 690 8
	cmpl	$0, -64(%rbp)
	jne	.L73
	.loc 1 691 23
	movl	-56(%rbp), %eax
	movl	%eax, -64(%rbp)
.L73:
	.loc 1 693 22
	movl	-60(%rbp), %eax
	movl	%eax, -68(%rbp)
.L72:
	.loc 1 697 14
	movl	$2, -28(%rbp)
	.loc 1 697 3
	jmp	.L74
.L78:
	.loc 1 699 16
	movl	$2, -32(%rbp)
	.loc 1 699 5
	jmp	.L75
.L77:
	.loc 1 701 20
	movl	-32(%rbp), %edx
	movl	-28(%rbp), %esi
	movq	-152(%rbp), %rax
	movl	$0, %ecx
	movq	%rax, %rdi
	call	bf_RLECompressAllGlyphsWithFieldSize
	movq	%rax, -96(%rbp)
	.loc 1 702 10
	movq	-48(%rbp), %rax
	cmpq	-96(%rbp), %rax
	jbe	.L76
	.loc 1 704 17
	movq	-96(%rbp), %rax
	movq	%rax, -48(%rbp)
	.loc 1 705 13
	movl	-28(%rbp), %eax
	movl	%eax, -36(%rbp)
	.loc 1 706 13
	movl	-32(%rbp), %eax
	movl	%eax, -40(%rbp)
.L76:
	.loc 1 699 37 discriminator 2
	addl	$1, -32(%rbp)
.L75:
	.loc 1 699 5 discriminator 1
	cmpl	$6, -32(%rbp)
	jle	.L77
	.loc 1 697 35 discriminator 2
	addl	$1, -28(%rbp)
.L74:
	.loc 1 697 3 discriminator 1
	cmpl	$8, -28(%rbp)
	jle	.L78
	.loc 1 711 3
	movq	-48(%rbp), %rsi
	movl	-40(%rbp), %ecx
	movl	-36(%rbp), %edx
	movq	-152(%rbp), %rax
	movq	%rsi, %r8
	leaq	.LC7(%rip), %rsi
	movq	%rax, %rdi
	movl	$0, %eax
	call	bf_Log@PLT
	.loc 1 712 3
	movl	-40(%rbp), %edx
	movl	-36(%rbp), %esi
	movq	-152(%rbp), %rax
	movl	$0, %ecx
	movq	%rax, %rdi
	call	bf_RLECompressAllGlyphsWithFieldSize
	.loc 1 715 3
	movq	-152(%rbp), %rax
	movq	%rax, %rdi
	call	bf_ClearTargetData@PLT
	.loc 1 728 3
	movq	-152(%rbp), %rax
	movl	16(%rax), %ecx
	movq	-152(%rbp), %rax
	movq	1152(%rax), %rdx
	movq	-152(%rbp), %rax
	leaq	.LC8(%rip), %rsi
	movq	%rax, %rdi
	movl	$0, %eax
	call	bf_Log@PLT
	.loc 1 731 26
	movq	-152(%rbp), %rax
	movq	1152(%rax), %rax
	.loc 1 731 3
	movzbl	%al, %edx
	movq	-152(%rbp), %rax
	movl	%edx, %esi
	movq	%rax, %rdi
	call	bf_AddTargetData@PLT
	.loc 1 732 26
	movq	-152(%rbp), %rax
	movl	1160(%rax), %eax
	.loc 1 732 3
	movzbl	%al, %edx
	movq	-152(%rbp), %rax
	movl	%edx, %esi
	movq	%rax, %rdi
	call	bf_AddTargetData@PLT
	.loc 1 733 3
	movl	-36(%rbp), %eax
	movzbl	%al, %edx
	movq	-152(%rbp), %rax
	movl	%edx, %esi
	movq	%rax, %rdi
	call	bf_AddTargetData@PLT
	.loc 1 734 3
	movl	-40(%rbp), %eax
	movzbl	%al, %edx
	movq	-152(%rbp), %rax
	movl	%edx, %esi
	movq	%rax, %rdi
	call	bf_AddTargetData@PLT
	.loc 1 737 26
	movq	-152(%rbp), %rax
	movl	1272(%rax), %eax
	.loc 1 737 3
	movzbl	%al, %edx
	movq	-152(%rbp), %rax
	movl	%edx, %esi
	movq	%rax, %rdi
	call	bf_AddTargetData@PLT
	.loc 1 738 26
	movq	-152(%rbp), %rax
	movl	1276(%rax), %eax
	.loc 1 738 3
	movzbl	%al, %edx
	movq	-152(%rbp), %rax
	movl	%edx, %esi
	movq	%rax, %rdi
	call	bf_AddTargetData@PLT
	.loc 1 739 26
	movq	-152(%rbp), %rax
	movl	1264(%rax), %eax
	.loc 1 739 3
	movzbl	%al, %edx
	movq	-152(%rbp), %rax
	movl	%edx, %esi
	movq	%rax, %rdi
	call	bf_AddTargetData@PLT
	.loc 1 740 26
	movq	-152(%rbp), %rax
	movl	1268(%rax), %eax
	.loc 1 740 3
	movzbl	%al, %edx
	movq	-152(%rbp), %rax
	movl	%edx, %esi
	movq	%rax, %rdi
	call	bf_AddTargetData@PLT
	.loc 1 741 26
	movq	-152(%rbp), %rax
	movl	1280(%rax), %eax
	.loc 1 741 3
	movzbl	%al, %edx
	movq	-152(%rbp), %rax
	movl	%edx, %esi
	movq	%rax, %rdi
	call	bf_AddTargetData@PLT
	.loc 1 744 31
	movq	-152(%rbp), %rax
	movq	1168(%rax), %rax
	.loc 1 744 3
	movzbl	%al, %edx
	movq	-152(%rbp), %rax
	movl	%edx, %esi
	movq	%rax, %rdi
	call	bf_AddTargetData@PLT
	.loc 1 745 31
	movq	-152(%rbp), %rax
	movq	1176(%rax), %rax
	.loc 1 745 3
	movzbl	%al, %edx
	movq	-152(%rbp), %rax
	movl	%edx, %esi
	movq	%rax, %rdi
	call	bf_AddTargetData@PLT
	.loc 1 746 31
	movq	-152(%rbp), %rax
	movq	1184(%rax), %rax
	.loc 1 746 3
	movzbl	%al, %edx
	movq	-152(%rbp), %rax
	movl	%edx, %esi
	movq	%rax, %rdi
	call	bf_AddTargetData@PLT
	.loc 1 747 31
	movq	-152(%rbp), %rax
	movq	1192(%rax), %rax
	.loc 1 747 3
	movzbl	%al, %edx
	movq	-152(%rbp), %rax
	movl	%edx, %esi
	movq	%rax, %rdi
	call	bf_AddTargetData@PLT
	.loc 1 750 6
	cmpl	$0, -52(%rbp)
	jle	.L79
	.loc 1 751 5
	movl	-52(%rbp), %eax
	movzbl	%al, %edx
	movq	-152(%rbp), %rax
	movl	%edx, %esi
	movq	%rax, %rdi
	call	bf_AddTargetData@PLT
	jmp	.L80
.L79:
	.loc 1 753 5
	movl	-56(%rbp), %eax
	movzbl	%al, %edx
	movq	-152(%rbp), %rax
	movl	%edx, %esi
	movq	%rax, %rdi
	call	bf_AddTargetData@PLT
.L80:
	.loc 1 754 3
	movl	-60(%rbp), %eax
	movzbl	%al, %edx
	movq	-152(%rbp), %rax
	movl	%edx, %esi
	movq	%rax, %rdi
	call	bf_AddTargetData@PLT
	.loc 1 757 3
	movl	-64(%rbp), %eax
	movzbl	%al, %edx
	movq	-152(%rbp), %rax
	movl	%edx, %esi
	movq	%rax, %rdi
	call	bf_AddTargetData@PLT
	.loc 1 758 3
	movl	-68(%rbp), %eax
	movzbl	%al, %edx
	movq	-152(%rbp), %rax
	movl	%edx, %esi
	movq	%rax, %rdi
	call	bf_AddTargetData@PLT
	.loc 1 761 3
	movq	-152(%rbp), %rax
	movl	$0, %esi
	movq	%rax, %rdi
	call	bf_AddTargetData@PLT
	.loc 1 762 3
	movq	-152(%rbp), %rax
	movl	$0, %esi
	movq	%rax, %rdi
	call	bf_AddTargetData@PLT
	.loc 1 765 3
	movq	-152(%rbp), %rax
	movl	$0, %esi
	movq	%rax, %rdi
	call	bf_AddTargetData@PLT
	.loc 1 766 3
	movq	-152(%rbp), %rax
	movl	$0, %esi
	movq	%rax, %rdi
	call	bf_AddTargetData@PLT
	.loc 1 769 3
	movq	-152(%rbp), %rax
	movl	$0, %esi
	movq	%rax, %rdi
	call	bf_AddTargetData@PLT
	.loc 1 770 3
	movq	-152(%rbp), %rax
	movl	$0, %esi
	movq	%rax, %rdi
	call	bf_AddTargetData@PLT
	.loc 1 774 16
	movl	$0, -72(%rbp)
	.loc 1 775 10
	movl	$0, -20(%rbp)
	.loc 1 775 3
	jmp	.L81
.L86:
	.loc 1 777 12
	movq	-152(%rbp), %rax
	movq	8(%rax), %rax
	.loc 1 777 24
	movl	-20(%rbp), %edx
	movslq	%edx, %rdx
	salq	$3, %rdx
	addq	%rdx, %rax
	.loc 1 777 8
	movq	(%rax), %rax
	movq	%rax, -120(%rbp)
	.loc 1 778 12
	movq	-120(%rbp), %rax
	movq	16(%rax), %rax
	.loc 1 778 8
	testq	%rax, %rax
	js	.L82
	.loc 1 778 31 discriminator 1
	movq	-120(%rbp), %rax
	movq	16(%rax), %rax
	.loc 1 778 26 discriminator 1
	cmpq	$255, %rax
	jg	.L82
	.loc 1 780 14
	movq	-120(%rbp), %rax
	movq	104(%rax), %rax
	.loc 1 780 10
	testq	%rax, %rax
	je	.L82
	.loc 1 783 9
	movq	-120(%rbp), %rax
	movl	116(%rax), %eax
	.loc 1 783 5
	cmpl	$254, %eax
	jle	.L83
	.loc 1 785 4
	movq	-120(%rbp), %rax
	movl	116(%rax), %ecx
	movq	-120(%rbp), %rax
	movq	8(%rax), %rdx
	movq	-152(%rbp), %rax
	leaq	.LC9(%rip), %rsi
	movq	%rax, %rdi
	movl	$0, %eax
	call	bf_Error@PLT
	.loc 1 786 4
	movl	$1, %edi
	call	exit@PLT
.L83:
	.loc 1 789 9
	movl	$0, -24(%rbp)
	.loc 1 789 2
	jmp	.L84
.L85:
	.loc 1 791 27 discriminator 3
	movq	-120(%rbp), %rax
	movq	104(%rax), %rdx
	.loc 1 791 40 discriminator 3
	movl	-24(%rbp), %eax
	cltq
	addq	%rdx, %rax
	movzbl	(%rax), %eax
	.loc 1 791 4 discriminator 3
	movzbl	%al, %edx
	movq	-152(%rbp), %rax
	movl	%edx, %esi
	movq	%rax, %rdi
	call	bf_AddTargetData@PLT
	.loc 1 789 35 discriminator 3
	addl	$1, -24(%rbp)
.L84:
	.loc 1 789 20 discriminator 1
	movq	-120(%rbp), %rax
	movl	116(%rax), %eax
	.loc 1 789 2 discriminator 1
	cmpl	%eax, -24(%rbp)
	jl	.L85
	.loc 1 793 14
	addl	$1, -72(%rbp)
.L82:
	.loc 1 775 35 discriminator 2
	addl	$1, -20(%rbp)
.L81:
	.loc 1 775 21 discriminator 1
	movq	-152(%rbp), %rax
	movl	16(%rax), %eax
	.loc 1 775 3 discriminator 1
	cmpl	%eax, -20(%rbp)
	jl	.L86
	.loc 1 798 3
	movq	-152(%rbp), %rax
	movl	$0, %esi
	movq	%rax, %rdi
	call	bf_AddTargetData@PLT
	.loc 1 799 3
	movq	-152(%rbp), %rax
	movl	$0, %esi
	movq	%rax, %rdi
	call	bf_AddTargetData@PLT
	.loc 1 801 25
	movq	-152(%rbp), %rax
	movl	1300(%rax), %eax
	.loc 1 801 37
	subl	$23, %eax
	.loc 1 801 21
	movl	%eax, -124(%rbp)
	.loc 1 805 82
	movq	-152(%rbp), %rax
	movq	1152(%rax), %rdx
	.loc 1 805 3
	movl	-72(%rbp), %eax
	movq	%rdx, %rcx
	subq	%rax, %rcx
	movl	-72(%rbp), %edx
	movq	-152(%rbp), %rax
	leaq	.LC10(%rip), %rsi
	movq	%rax, %rdi
	movl	$0, %eax
	call	bf_Log@PLT
	.loc 1 806 33
	movq	-152(%rbp), %rax
	movq	1152(%rax), %rdx
	.loc 1 806 50
	movl	-72(%rbp), %eax
	movq	%rdx, %rcx
	subq	%rax, %rcx
	.loc 1 806 65
	movabsq	$-6640827866535438581, %rdx
	movq	%rcx, %rax
	imulq	%rdx
	leaq	(%rdx,%rcx), %rax
	sarq	$6, %rax
	movq	%rax, %rdx
	movq	%rcx, %rax
	sarq	$63, %rax
	subq	%rax, %rdx
	movq	%rdx, %rax
	.loc 1 806 28
	movl	%eax, -128(%rbp)
	.loc 1 809 3
	movq	-152(%rbp), %rax
	movl	$100, %edx
	leaq	.LC11(%rip), %rsi
	movq	%rax, %rdi
	movl	$0, %eax
	call	bf_Log@PLT
	.loc 1 810 34
	movq	-152(%rbp), %rax
	movl	1300(%rax), %eax
	.loc 1 810 30
	movl	%eax, -132(%rbp)
	.loc 1 813 10
	movl	$1, -20(%rbp)
	.loc 1 813 3
	jmp	.L87
.L88:
	.loc 1 815 5 discriminator 3
	movq	-152(%rbp), %rax
	movl	$0, %esi
	movq	%rax, %rdi
	call	bf_AddTargetData@PLT
	.loc 1 816 5 discriminator 3
	movq	-152(%rbp), %rax
	movl	$0, %esi
	movq	%rax, %rdi
	call	bf_AddTargetData@PLT
	.loc 1 817 5 discriminator 3
	movq	-152(%rbp), %rax
	movl	$0, %esi
	movq	%rax, %rdi
	call	bf_AddTargetData@PLT
	.loc 1 818 5 discriminator 3
	movq	-152(%rbp), %rax
	movl	$0, %esi
	movq	%rax, %rdi
	call	bf_AddTargetData@PLT
	.loc 1 813 46 discriminator 3
	addl	$1, -20(%rbp)
.L87:
	.loc 1 813 17 discriminator 1
	movl	-20(%rbp), %eax
	.loc 1 813 3 discriminator 1
	cmpl	%eax, -128(%rbp)
	ja	.L88
	.loc 1 821 3
	movq	-152(%rbp), %rax
	movl	$0, %esi
	movq	%rax, %rdi
	call	bf_AddTargetData@PLT
	.loc 1 822 3
	movq	-152(%rbp), %rax
	movl	$4, %esi
	movq	%rax, %rdi
	call	bf_AddTargetData@PLT
	.loc 1 823 3
	movq	-152(%rbp), %rax
	movl	$255, %esi
	movq	%rax, %rdi
	call	bf_AddTargetData@PLT
	.loc 1 824 3
	movq	-152(%rbp), %rax
	movl	$255, %esi
	movq	%rax, %rdi
	call	bf_AddTargetData@PLT
	.loc 1 828 28
	movl	$0, -84(%rbp)
	.loc 1 829 34
	movl	$0, -88(%rbp)
	.loc 1 830 26
	movq	-152(%rbp), %rax
	movl	1300(%rax), %eax
	.loc 1 830 22
	subl	-132(%rbp), %eax
	movl	%eax, -76(%rbp)
	.loc 1 831 31
	movq	-152(%rbp), %rax
	movl	1300(%rax), %eax
	.loc 1 831 27
	movl	%eax, -80(%rbp)
	.loc 1 834 10
	movl	$0, -20(%rbp)
	.loc 1 834 3
	jmp	.L89
.L94:
	.loc 1 836 12
	movq	-152(%rbp), %rax
	movq	8(%rax), %rax
	.loc 1 836 24
	movl	-20(%rbp), %edx
	movslq	%edx, %rdx
	salq	$3, %rdx
	addq	%rdx, %rax
	.loc 1 836 8
	movq	(%rax), %rax
	movq	%rax, -120(%rbp)
	.loc 1 837 12
	movq	-120(%rbp), %rax
	movq	16(%rax), %rax
	.loc 1 837 8
	cmpq	$255, %rax
	jle	.L90
	.loc 1 839 14
	movq	-120(%rbp), %rax
	movq	104(%rax), %rax
	.loc 1 839 10
	testq	%rax, %rax
	je	.L90
	.loc 1 842 9
	movq	-120(%rbp), %rax
	movl	116(%rax), %eax
	.loc 1 842 5
	cmpl	$254, %eax
	jle	.L91
	.loc 1 844 4
	movq	-120(%rbp), %rax
	movq	8(%rax), %rdx
	movq	-152(%rbp), %rax
	leaq	.LC12(%rip), %rsi
	movq	%rax, %rdi
	movl	$0, %eax
	call	bf_Error@PLT
	.loc 1 845 4
	movl	$1, %edi
	call	exit@PLT
.L91:
	.loc 1 848 9
	movl	$0, -24(%rbp)
	.loc 1 848 2
	jmp	.L92
.L93:
	.loc 1 850 27 discriminator 3
	movq	-120(%rbp), %rax
	movq	104(%rax), %rdx
	.loc 1 850 40 discriminator 3
	movl	-24(%rbp), %eax
	cltq
	addq	%rdx, %rax
	movzbl	(%rax), %eax
	.loc 1 850 4 discriminator 3
	movzbl	%al, %edx
	movq	-152(%rbp), %rax
	movl	%edx, %esi
	movq	%rax, %rdi
	call	bf_AddTargetData@PLT
	.loc 1 848 35 discriminator 3
	addl	$1, -24(%rbp)
.L92:
	.loc 1 848 20 discriminator 1
	movq	-120(%rbp), %rax
	movl	116(%rax), %eax
	.loc 1 848 2 discriminator 1
	cmpl	%eax, -24(%rbp)
	jl	.L93
	.loc 1 854 32
	addl	$1, -88(%rbp)
	.loc 1 855 6
	cmpl	$100, -88(%rbp)
	jbe	.L90
	.loc 1 858 7
	movl	-84(%rbp), %eax
	cmpl	-128(%rbp), %eax
	jnb	.L90
	.loc 1 860 99
	movl	-76(%rbp), %eax
	shrl	$8, %eax
	movl	%eax, %esi
	.loc 1 860 8
	movq	-152(%rbp), %rax
	movq	1288(%rax), %rax
	.loc 1 860 73
	movl	-84(%rbp), %edx
	leal	0(,%rdx,4), %ecx
	.loc 1 860 75
	movl	-132(%rbp), %edx
	addl	%ecx, %edx
	movl	%edx, %edx
	.loc 1 860 21
	addq	%rdx, %rax
	.loc 1 860 79
	movl	%esi, %edx
	movb	%dl, (%rax)
	.loc 1 861 8
	movq	-152(%rbp), %rax
	movq	1288(%rax), %rax
	.loc 1 861 73
	movl	-84(%rbp), %edx
	leal	0(,%rdx,4), %ecx
	.loc 1 861 48
	movl	-132(%rbp), %edx
	addl	%ecx, %edx
	.loc 1 861 75
	addl	$1, %edx
	movl	%edx, %edx
	.loc 1 861 21
	addq	%rdx, %rax
	.loc 1 861 79
	movl	-76(%rbp), %edx
	movb	%dl, (%rax)
	.loc 1 862 8
	movq	-152(%rbp), %rax
	movq	1288(%rax), %rax
	.loc 1 862 79
	movl	-84(%rbp), %edx
	leal	0(,%rdx,4), %ecx
	movl	-132(%rbp), %edx
	addl	%ecx, %edx
	addl	$2, %edx
	movl	%edx, %edx
	addq	%rdx, %rax
	movzbl	(%rax), %eax
	movl	%eax, %edx
	.loc 1 862 84
	movq	-120(%rbp), %rax
	movq	8(%rax), %rax
	.loc 1 862 94
	sarq	$8, %rax
	.loc 1 862 79
	movl	%edx, %esi
	orl	%eax, %esi
	.loc 1 862 8
	movq	-152(%rbp), %rax
	movq	1288(%rax), %rax
	.loc 1 862 79
	movl	-84(%rbp), %edx
	leal	0(,%rdx,4), %ecx
	movl	-132(%rbp), %edx
	addl	%ecx, %edx
	addl	$2, %edx
	movl	%edx, %edx
	addq	%rdx, %rax
	movl	%esi, %edx
	movb	%dl, (%rax)
	.loc 1 863 8
	movq	-152(%rbp), %rax
	movq	1288(%rax), %rax
	.loc 1 863 79
	movl	-84(%rbp), %edx
	leal	0(,%rdx,4), %ecx
	movl	-132(%rbp), %edx
	addl	%ecx, %edx
	addl	$3, %edx
	movl	%edx, %edx
	addq	%rdx, %rax
	movzbl	(%rax), %eax
	movl	%eax, %edx
	.loc 1 863 84
	movq	-120(%rbp), %rax
	movq	8(%rax), %rax
	.loc 1 863 79
	movl	%edx, %esi
	orl	%eax, %esi
	.loc 1 863 8
	movq	-152(%rbp), %rax
	movq	1288(%rax), %rax
	.loc 1 863 79
	movl	-84(%rbp), %edx
	leal	0(,%rdx,4), %ecx
	movl	-132(%rbp), %edx
	addl	%ecx, %edx
	addl	$3, %edx
	movl	%edx, %edx
	addq	%rdx, %rax
	movl	%esi, %edx
	movb	%dl, (%rax)
	.loc 1 865 30
	addl	$1, -84(%rbp)
	.loc 1 866 37
	movl	$0, -88(%rbp)
	.loc 1 867 29
	movq	-152(%rbp), %rax
	movl	1300(%rax), %eax
	.loc 1 867 25
	subl	-80(%rbp), %eax
	movl	%eax, -76(%rbp)
	.loc 1 868 34
	movq	-152(%rbp), %rax
	movl	1300(%rax), %eax
	.loc 1 868 30
	movl	%eax, -80(%rbp)
.L90:
	.loc 1 834 35 discriminator 2
	addl	$1, -20(%rbp)
.L89:
	.loc 1 834 21 discriminator 1
	movq	-152(%rbp), %rax
	movl	16(%rax), %eax
	.loc 1 834 3 discriminator 1
	cmpl	%eax, -20(%rbp)
	jl	.L94
	.loc 1 876 6
	movl	-84(%rbp), %eax
	cmpl	-128(%rbp), %eax
	jnb	.L95
	.loc 1 878 98
	movl	-76(%rbp), %eax
	shrl	$8, %eax
	movl	%eax, %esi
	.loc 1 878 7
	movq	-152(%rbp), %rax
	movq	1288(%rax), %rax
	.loc 1 878 72
	movl	-84(%rbp), %edx
	leal	0(,%rdx,4), %ecx
	.loc 1 878 74
	movl	-132(%rbp), %edx
	addl	%ecx, %edx
	movl	%edx, %edx
	.loc 1 878 20
	addq	%rdx, %rax
	.loc 1 878 78
	movl	%esi, %edx
	movb	%dl, (%rax)
	.loc 1 879 7
	movq	-152(%rbp), %rax
	movq	1288(%rax), %rax
	.loc 1 879 72
	movl	-84(%rbp), %edx
	leal	0(,%rdx,4), %ecx
	.loc 1 879 47
	movl	-132(%rbp), %edx
	addl	%ecx, %edx
	.loc 1 879 74
	addl	$1, %edx
	movl	%edx, %edx
	.loc 1 879 20
	addq	%rdx, %rax
	.loc 1 879 78
	movl	-76(%rbp), %edx
	movb	%dl, (%rax)
	.loc 1 880 7
	movq	-152(%rbp), %rax
	movq	1288(%rax), %rax
	.loc 1 880 72
	movl	-84(%rbp), %edx
	leal	0(,%rdx,4), %ecx
	.loc 1 880 47
	movl	-132(%rbp), %edx
	addl	%ecx, %edx
	.loc 1 880 74
	addl	$2, %edx
	movl	%edx, %edx
	.loc 1 880 20
	addq	%rdx, %rax
	.loc 1 880 78
	movb	$-1, (%rax)
	.loc 1 881 7
	movq	-152(%rbp), %rax
	movq	1288(%rax), %rax
	.loc 1 881 72
	movl	-84(%rbp), %edx
	leal	0(,%rdx,4), %ecx
	.loc 1 881 47
	movl	-132(%rbp), %edx
	addl	%ecx, %edx
	.loc 1 881 74
	addl	$3, %edx
	movl	%edx, %edx
	.loc 1 881 20
	addq	%rdx, %rax
	.loc 1 881 78
	movb	$-1, (%rax)
	.loc 1 882 29
	addl	$1, -84(%rbp)
.L95:
	.loc 1 886 3
	movq	-152(%rbp), %rax
	movl	$0, %esi
	movq	%rax, %rdi
	call	bf_AddTargetData@PLT
	.loc 1 887 3
	movq	-152(%rbp), %rax
	movl	$0, %esi
	movq	%rax, %rdi
	call	bf_AddTargetData@PLT
	.loc 1 889 3
	movl	-84(%rbp), %ecx
	movl	-128(%rbp), %edx
	movq	-152(%rbp), %rax
	leaq	.LC13(%rip), %rsi
	movq	%rax, %rdi
	movl	$0, %eax
	call	bf_Log@PLT
	.loc 1 892 7
	movq	-152(%rbp), %rax
	movq	1288(%rax), %rax
	.loc 1 892 47
	movl	-132(%rbp), %edx
	addl	$2, %edx
	movl	%edx, %edx
	.loc 1 892 20
	addq	%rdx, %rax
	movzbl	(%rax), %eax
	movzbl	%al, %eax
	.loc 1 892 50
	sall	$8, %eax
	movl	%eax, %ecx
	.loc 1 892 57
	movq	-152(%rbp), %rax
	movq	1288(%rax), %rax
	.loc 1 892 97
	movl	-132(%rbp), %edx
	addl	$3, %edx
	movl	%edx, %edx
	.loc 1 892 70
	addq	%rdx, %rax
	movzbl	(%rax), %eax
	movzbl	%al, %eax
	.loc 1 890 3
	addl	%eax, %ecx
	.loc 1 891 7
	movq	-152(%rbp), %rax
	movq	1288(%rax), %rdx
	.loc 1 891 47
	movl	-132(%rbp), %eax
	.loc 1 891 20
	addq	%rdx, %rax
	movzbl	(%rax), %eax
	movzbl	%al, %eax
	.loc 1 891 50
	sall	$8, %eax
	movl	%eax, %esi
	.loc 1 891 57
	movq	-152(%rbp), %rax
	movq	1288(%rax), %rax
	.loc 1 891 97
	movl	-132(%rbp), %edx
	addl	$1, %edx
	movl	%edx, %edx
	.loc 1 891 70
	addq	%rdx, %rax
	movzbl	(%rax), %eax
	movzbl	%al, %eax
	.loc 1 890 3
	leal	(%rsi,%rax), %edx
	movq	-152(%rbp), %rax
	leaq	.LC14(%rip), %rsi
	movq	%rax, %rdi
	movl	$0, %eax
	call	bf_Log@PLT
	.loc 1 896 7
	movq	-152(%rbp), %rax
	movq	1288(%rax), %rax
	.loc 1 896 72
	movl	-84(%rbp), %edx
	leal	0(,%rdx,4), %ecx
	.loc 1 896 47
	movl	-132(%rbp), %edx
	addl	%ecx, %edx
	.loc 1 896 76
	subl	$2, %edx
	movl	%edx, %edx
	.loc 1 896 20
	addq	%rdx, %rax
	movzbl	(%rax), %eax
	movzbl	%al, %eax
	.loc 1 896 79
	sall	$8, %eax
	movl	%eax, %ecx
	.loc 1 896 86
	movq	-152(%rbp), %rax
	movq	1288(%rax), %rax
	.loc 1 896 151
	movl	-84(%rbp), %edx
	leal	0(,%rdx,4), %esi
	.loc 1 896 126
	movl	-132(%rbp), %edx
	addl	%esi, %edx
	.loc 1 896 155
	subl	$1, %edx
	movl	%edx, %edx
	.loc 1 896 99
	addq	%rdx, %rax
	movzbl	(%rax), %eax
	movzbl	%al, %eax
	.loc 1 894 3
	addl	%eax, %ecx
	.loc 1 895 7
	movq	-152(%rbp), %rax
	movq	1288(%rax), %rax
	.loc 1 895 72
	movl	-84(%rbp), %edx
	leal	0(,%rdx,4), %esi
	.loc 1 895 47
	movl	-132(%rbp), %edx
	addl	%esi, %edx
	.loc 1 895 76
	subl	$4, %edx
	movl	%edx, %edx
	.loc 1 895 20
	addq	%rdx, %rax
	movzbl	(%rax), %eax
	movzbl	%al, %eax
	.loc 1 895 79
	sall	$8, %eax
	movl	%eax, %esi
	.loc 1 895 86
	movq	-152(%rbp), %rax
	movq	1288(%rax), %rax
	.loc 1 895 151
	movl	-84(%rbp), %edx
	leal	0(,%rdx,4), %edi
	.loc 1 895 126
	movl	-132(%rbp), %edx
	addl	%edi, %edx
	.loc 1 895 155
	subl	$3, %edx
	movl	%edx, %edx
	.loc 1 895 99
	addq	%rdx, %rax
	movzbl	(%rax), %eax
	movzbl	%al, %eax
	.loc 1 894 3
	leal	(%rsi,%rax), %edx
	movq	-152(%rbp), %rax
	leaq	.LC15(%rip), %rsi
	movq	%rax, %rdi
	movl	$0, %eax
	call	bf_Log@PLT
	.loc 1 898 3
	movl	-128(%rbp), %eax
	cmpl	-84(%rbp), %eax
	je	.L96
	.loc 1 898 3 is_stmt 0 discriminator 1
	leaq	__PRETTY_FUNCTION__.3275(%rip), %rcx
	movl	$898, %edx
	leaq	.LC4(%rip), %rsi
	leaq	.LC16(%rip), %rdi
	call	__assert_fail@PLT
.L96:
	.loc 1 900 9 is_stmt 1
	movq	-152(%rbp), %rax
	movl	$65, %esi
	movq	%rax, %rdi
	call	bf_RLE_get_glyph_data
	movl	%eax, -136(%rbp)
	.loc 1 901 29
	movl	-136(%rbp), %eax
	shrl	$8, %eax
	movl	%eax, %edx
	.loc 1 901 5
	movq	-152(%rbp), %rax
	movq	1288(%rax), %rax
	.loc 1 901 18
	addq	$17, %rax
	.loc 1 901 23
	movb	%dl, (%rax)
	.loc 1 902 5
	movq	-152(%rbp), %rax
	movq	1288(%rax), %rax
	.loc 1 902 18
	addq	$18, %rax
	.loc 1 902 23
	movl	-136(%rbp), %edx
	movb	%dl, (%rax)
	.loc 1 904 9
	movq	-152(%rbp), %rax
	movl	$97, %esi
	movq	%rax, %rdi
	call	bf_RLE_get_glyph_data
	movl	%eax, -136(%rbp)
	.loc 1 905 29
	movl	-136(%rbp), %eax
	shrl	$8, %eax
	movl	%eax, %edx
	.loc 1 905 5
	movq	-152(%rbp), %rax
	movq	1288(%rax), %rax
	.loc 1 905 18
	addq	$19, %rax
	.loc 1 905 23
	movb	%dl, (%rax)
	.loc 1 906 5
	movq	-152(%rbp), %rax
	movq	1288(%rax), %rax
	.loc 1 906 18
	addq	$20, %rax
	.loc 1 906 23
	movl	-136(%rbp), %edx
	movb	%dl, (%rax)
	.loc 1 908 43
	movl	-124(%rbp), %eax
	shrl	$8, %eax
	movl	%eax, %edx
	.loc 1 908 5
	movq	-152(%rbp), %rax
	movq	1288(%rax), %rax
	.loc 1 908 18
	addq	$21, %rax
	.loc 1 908 23
	movb	%dl, (%rax)
	.loc 1 909 5
	movq	-152(%rbp), %rax
	movq	1288(%rax), %rax
	.loc 1 909 18
	addq	$22, %rax
	.loc 1 909 23
	movl	-124(%rbp), %edx
	movb	%dl, (%rax)
	.loc 1 911 3
	movq	-152(%rbp), %rax
	movl	$97, %esi
	movq	%rax, %rdi
	call	bf_RLE_get_glyph_data
	movl	%eax, %ebx
	movq	-152(%rbp), %rax
	movl	$65, %esi
	movq	%rax, %rdi
	call	bf_RLE_get_glyph_data
	movl	%eax, %edx
	movq	-152(%rbp), %rax
	movl	%ebx, %ecx
	leaq	.LC17(%rip), %rsi
	movq	%rax, %rdi
	movl	$0, %eax
	call	bf_Log@PLT
	.loc 1 913 3
	movq	-152(%rbp), %rax
	movl	1300(%rax), %edx
	movq	-152(%rbp), %rax
	leaq	.LC18(%rip), %rsi
	movq	%rax, %rdi
	movl	$0, %eax
	call	bf_Log@PLT
	.loc 1 915 1
	nop
	addq	$152, %rsp
	popq	%rbx
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE13:
	.size	bf_RLECompressAllGlyphs, .-bf_RLECompressAllGlyphs
	.local	bd_list.3193
	.comm	bd_list.3193,8192,32
	.section	.rodata
	.align 16
	.type	__PRETTY_FUNCTION__.3200, @object
	.size	__PRETTY_FUNCTION__.3200, 16
__PRETTY_FUNCTION__.3200:
	.string	"bg_rle_compress"
	.align 16
	.type	__PRETTY_FUNCTION__.3275, @object
	.size	__PRETTY_FUNCTION__.3275, 24
__PRETTY_FUNCTION__.3275:
	.string	"bf_RLECompressAllGlyphs"
	.text
.Letext0:
	.file 2 "/usr/lib/gcc/x86_64-linux-gnu/8/include/stddef.h"
	.file 3 "/usr/include/x86_64-linux-gnu/bits/types.h"
	.file 4 "/usr/include/x86_64-linux-gnu/bits/types/struct_FILE.h"
	.file 5 "/usr/include/x86_64-linux-gnu/bits/types/FILE.h"
	.file 6 "/usr/include/stdio.h"
	.file 7 "/usr/include/x86_64-linux-gnu/bits/sys_errlist.h"
	.file 8 "/usr/include/x86_64-linux-gnu/bits/stdint-uintn.h"
	.file 9 "bdf_glyph.h"
	.file 10 "bdf_font.h"
	.section	.debug_info,"",@progbits
.Ldebug_info0:
	.long	0xc79
	.value	0x4
	.long	.Ldebug_abbrev0
	.byte	0x8
	.uleb128 0x1
	.long	.LASF153
	.byte	0xc
	.long	.LASF154
	.long	.LASF155
	.quad	.Ltext0
	.quad	.Letext0-.Ltext0
	.long	.Ldebug_line0
	.uleb128 0x2
	.long	.LASF5
	.byte	0x2
	.byte	0xd8
	.byte	0x17
	.long	0x39
	.uleb128 0x3
	.byte	0x8
	.byte	0x7
	.long	.LASF0
	.uleb128 0x3
	.byte	0x4
	.byte	0x7
	.long	.LASF1
	.uleb128 0x4
	.byte	0x8
	.uleb128 0x3
	.byte	0x1
	.byte	0x8
	.long	.LASF2
	.uleb128 0x3
	.byte	0x2
	.byte	0x7
	.long	.LASF3
	.uleb128 0x3
	.byte	0x1
	.byte	0x6
	.long	.LASF4
	.uleb128 0x2
	.long	.LASF6
	.byte	0x3
	.byte	0x25
	.byte	0x17
	.long	0x49
	.uleb128 0x3
	.byte	0x2
	.byte	0x5
	.long	.LASF7
	.uleb128 0x5
	.byte	0x4
	.byte	0x5
	.string	"int"
	.uleb128 0x2
	.long	.LASF8
	.byte	0x3
	.byte	0x29
	.byte	0x16
	.long	0x40
	.uleb128 0x3
	.byte	0x8
	.byte	0x5
	.long	.LASF9
	.uleb128 0x2
	.long	.LASF10
	.byte	0x3
	.byte	0x96
	.byte	0x19
	.long	0x84
	.uleb128 0x2
	.long	.LASF11
	.byte	0x3
	.byte	0x97
	.byte	0x1b
	.long	0x84
	.uleb128 0x6
	.byte	0x8
	.long	0xa9
	.uleb128 0x3
	.byte	0x1
	.byte	0x6
	.long	.LASF12
	.uleb128 0x7
	.long	0xa9
	.uleb128 0x8
	.long	.LASF57
	.byte	0xd8
	.byte	0x4
	.byte	0x31
	.byte	0x8
	.long	0x23c
	.uleb128 0x9
	.long	.LASF13
	.byte	0x4
	.byte	0x33
	.byte	0x7
	.long	0x71
	.byte	0
	.uleb128 0x9
	.long	.LASF14
	.byte	0x4
	.byte	0x36
	.byte	0x9
	.long	0xa3
	.byte	0x8
	.uleb128 0x9
	.long	.LASF15
	.byte	0x4
	.byte	0x37
	.byte	0x9
	.long	0xa3
	.byte	0x10
	.uleb128 0x9
	.long	.LASF16
	.byte	0x4
	.byte	0x38
	.byte	0x9
	.long	0xa3
	.byte	0x18
	.uleb128 0x9
	.long	.LASF17
	.byte	0x4
	.byte	0x39
	.byte	0x9
	.long	0xa3
	.byte	0x20
	.uleb128 0x9
	.long	.LASF18
	.byte	0x4
	.byte	0x3a
	.byte	0x9
	.long	0xa3
	.byte	0x28
	.uleb128 0x9
	.long	.LASF19
	.byte	0x4
	.byte	0x3b
	.byte	0x9
	.long	0xa3
	.byte	0x30
	.uleb128 0x9
	.long	.LASF20
	.byte	0x4
	.byte	0x3c
	.byte	0x9
	.long	0xa3
	.byte	0x38
	.uleb128 0x9
	.long	.LASF21
	.byte	0x4
	.byte	0x3d
	.byte	0x9
	.long	0xa3
	.byte	0x40
	.uleb128 0x9
	.long	.LASF22
	.byte	0x4
	.byte	0x40
	.byte	0x9
	.long	0xa3
	.byte	0x48
	.uleb128 0x9
	.long	.LASF23
	.byte	0x4
	.byte	0x41
	.byte	0x9
	.long	0xa3
	.byte	0x50
	.uleb128 0x9
	.long	.LASF24
	.byte	0x4
	.byte	0x42
	.byte	0x9
	.long	0xa3
	.byte	0x58
	.uleb128 0x9
	.long	.LASF25
	.byte	0x4
	.byte	0x44
	.byte	0x16
	.long	0x255
	.byte	0x60
	.uleb128 0x9
	.long	.LASF26
	.byte	0x4
	.byte	0x46
	.byte	0x14
	.long	0x25b
	.byte	0x68
	.uleb128 0x9
	.long	.LASF27
	.byte	0x4
	.byte	0x48
	.byte	0x7
	.long	0x71
	.byte	0x70
	.uleb128 0x9
	.long	.LASF28
	.byte	0x4
	.byte	0x49
	.byte	0x7
	.long	0x71
	.byte	0x74
	.uleb128 0x9
	.long	.LASF29
	.byte	0x4
	.byte	0x4a
	.byte	0xb
	.long	0x8b
	.byte	0x78
	.uleb128 0x9
	.long	.LASF30
	.byte	0x4
	.byte	0x4d
	.byte	0x12
	.long	0x50
	.byte	0x80
	.uleb128 0x9
	.long	.LASF31
	.byte	0x4
	.byte	0x4e
	.byte	0xf
	.long	0x57
	.byte	0x82
	.uleb128 0x9
	.long	.LASF32
	.byte	0x4
	.byte	0x4f
	.byte	0x8
	.long	0x261
	.byte	0x83
	.uleb128 0x9
	.long	.LASF33
	.byte	0x4
	.byte	0x51
	.byte	0xf
	.long	0x271
	.byte	0x88
	.uleb128 0x9
	.long	.LASF34
	.byte	0x4
	.byte	0x59
	.byte	0xd
	.long	0x97
	.byte	0x90
	.uleb128 0x9
	.long	.LASF35
	.byte	0x4
	.byte	0x5b
	.byte	0x17
	.long	0x27c
	.byte	0x98
	.uleb128 0x9
	.long	.LASF36
	.byte	0x4
	.byte	0x5c
	.byte	0x19
	.long	0x287
	.byte	0xa0
	.uleb128 0x9
	.long	.LASF37
	.byte	0x4
	.byte	0x5d
	.byte	0x14
	.long	0x25b
	.byte	0xa8
	.uleb128 0x9
	.long	.LASF38
	.byte	0x4
	.byte	0x5e
	.byte	0x9
	.long	0x47
	.byte	0xb0
	.uleb128 0x9
	.long	.LASF39
	.byte	0x4
	.byte	0x5f
	.byte	0xa
	.long	0x2d
	.byte	0xb8
	.uleb128 0x9
	.long	.LASF40
	.byte	0x4
	.byte	0x60
	.byte	0x7
	.long	0x71
	.byte	0xc0
	.uleb128 0x9
	.long	.LASF41
	.byte	0x4
	.byte	0x62
	.byte	0x8
	.long	0x28d
	.byte	0xc4
	.byte	0
	.uleb128 0x2
	.long	.LASF42
	.byte	0x5
	.byte	0x7
	.byte	0x19
	.long	0xb5
	.uleb128 0xa
	.long	.LASF156
	.byte	0x4
	.byte	0x2b
	.byte	0xe
	.uleb128 0xb
	.long	.LASF43
	.uleb128 0x6
	.byte	0x8
	.long	0x250
	.uleb128 0x6
	.byte	0x8
	.long	0xb5
	.uleb128 0xc
	.long	0xa9
	.long	0x271
	.uleb128 0xd
	.long	0x39
	.byte	0
	.byte	0
	.uleb128 0x6
	.byte	0x8
	.long	0x248
	.uleb128 0xb
	.long	.LASF44
	.uleb128 0x6
	.byte	0x8
	.long	0x277
	.uleb128 0xb
	.long	.LASF45
	.uleb128 0x6
	.byte	0x8
	.long	0x282
	.uleb128 0xc
	.long	0xa9
	.long	0x29d
	.uleb128 0xd
	.long	0x39
	.byte	0x13
	.byte	0
	.uleb128 0xe
	.long	.LASF46
	.byte	0x6
	.byte	0x89
	.byte	0xe
	.long	0x2a9
	.uleb128 0x6
	.byte	0x8
	.long	0x23c
	.uleb128 0xe
	.long	.LASF47
	.byte	0x6
	.byte	0x8a
	.byte	0xe
	.long	0x2a9
	.uleb128 0xe
	.long	.LASF48
	.byte	0x6
	.byte	0x8b
	.byte	0xe
	.long	0x2a9
	.uleb128 0xe
	.long	.LASF49
	.byte	0x7
	.byte	0x1a
	.byte	0xc
	.long	0x71
	.uleb128 0xc
	.long	0x2e9
	.long	0x2de
	.uleb128 0xf
	.byte	0
	.uleb128 0x7
	.long	0x2d3
	.uleb128 0x6
	.byte	0x8
	.long	0xb0
	.uleb128 0x7
	.long	0x2e3
	.uleb128 0xe
	.long	.LASF50
	.byte	0x7
	.byte	0x1b
	.byte	0x1a
	.long	0x2de
	.uleb128 0x3
	.byte	0x8
	.byte	0x5
	.long	.LASF51
	.uleb128 0x3
	.byte	0x8
	.byte	0x7
	.long	.LASF52
	.uleb128 0x2
	.long	.LASF53
	.byte	0x8
	.byte	0x18
	.byte	0x13
	.long	0x5e
	.uleb128 0x2
	.long	.LASF54
	.byte	0x8
	.byte	0x1a
	.byte	0x14
	.long	0x78
	.uleb128 0x3
	.byte	0x10
	.byte	0x4
	.long	.LASF55
	.uleb128 0x2
	.long	.LASF56
	.byte	0x9
	.byte	0x9
	.byte	0x21
	.long	0x333
	.uleb128 0x10
	.long	.LASF58
	.value	0x520
	.byte	0xa
	.byte	0x15
	.byte	0x8
	.long	0x57a
	.uleb128 0x9
	.long	.LASF59
	.byte	0xa
	.byte	0x17
	.byte	0x7
	.long	0x71
	.byte	0
	.uleb128 0x9
	.long	.LASF60
	.byte	0xa
	.byte	0x19
	.byte	0xa
	.long	0x703
	.byte	0x8
	.uleb128 0x9
	.long	.LASF61
	.byte	0xa
	.byte	0x1a
	.byte	0x7
	.long	0x71
	.byte	0x10
	.uleb128 0x9
	.long	.LASF62
	.byte	0xa
	.byte	0x1b
	.byte	0x7
	.long	0x71
	.byte	0x14
	.uleb128 0x9
	.long	.LASF63
	.byte	0xa
	.byte	0x1e
	.byte	0x8
	.long	0x70f
	.byte	0x18
	.uleb128 0x11
	.long	.LASF64
	.byte	0xa
	.byte	0x1f
	.byte	0x7
	.long	0x71
	.value	0x418
	.uleb128 0x11
	.long	.LASF65
	.byte	0xa
	.byte	0x20
	.byte	0x7
	.long	0x71
	.value	0x41c
	.uleb128 0x11
	.long	.LASF66
	.byte	0xa
	.byte	0x22
	.byte	0x8
	.long	0x84
	.value	0x420
	.uleb128 0x11
	.long	.LASF67
	.byte	0xa
	.byte	0x24
	.byte	0x8
	.long	0x84
	.value	0x428
	.uleb128 0x11
	.long	.LASF68
	.byte	0xa
	.byte	0x25
	.byte	0x8
	.long	0x84
	.value	0x430
	.uleb128 0x11
	.long	.LASF69
	.byte	0xa
	.byte	0x27
	.byte	0x8
	.long	0x84
	.value	0x438
	.uleb128 0x11
	.long	.LASF70
	.byte	0xa
	.byte	0x28
	.byte	0x8
	.long	0x84
	.value	0x440
	.uleb128 0x11
	.long	.LASF71
	.byte	0xa
	.byte	0x29
	.byte	0x8
	.long	0x84
	.value	0x448
	.uleb128 0x11
	.long	.LASF72
	.byte	0xa
	.byte	0x2a
	.byte	0x8
	.long	0x84
	.value	0x450
	.uleb128 0x11
	.long	.LASF73
	.byte	0xa
	.byte	0x2c
	.byte	0x7
	.long	0x71
	.value	0x458
	.uleb128 0x11
	.long	.LASF74
	.byte	0xa
	.byte	0x2d
	.byte	0x7
	.long	0x71
	.value	0x45c
	.uleb128 0x11
	.long	.LASF75
	.byte	0xa
	.byte	0x2e
	.byte	0x7
	.long	0x71
	.value	0x460
	.uleb128 0x12
	.string	"fp"
	.byte	0xa
	.byte	0x2f
	.byte	0x9
	.long	0x2a9
	.value	0x468
	.uleb128 0x11
	.long	.LASF76
	.byte	0xa
	.byte	0x31
	.byte	0x9
	.long	0xa3
	.value	0x470
	.uleb128 0x11
	.long	.LASF77
	.byte	0xa
	.byte	0x32
	.byte	0x9
	.long	0xa3
	.value	0x478
	.uleb128 0x11
	.long	.LASF78
	.byte	0xa
	.byte	0x34
	.byte	0x8
	.long	0x84
	.value	0x480
	.uleb128 0x11
	.long	.LASF79
	.byte	0xa
	.byte	0x36
	.byte	0x7
	.long	0x71
	.value	0x488
	.uleb128 0x12
	.string	"max"
	.byte	0xa
	.byte	0x38
	.byte	0x9
	.long	0x5b4
	.value	0x490
	.uleb128 0x11
	.long	.LASF80
	.byte	0xa
	.byte	0x3b
	.byte	0x8
	.long	0x84
	.value	0x4b0
	.uleb128 0x11
	.long	.LASF81
	.byte	0xa
	.byte	0x3c
	.byte	0x8
	.long	0x84
	.value	0x4b8
	.uleb128 0x11
	.long	.LASF82
	.byte	0xa
	.byte	0x3d
	.byte	0x8
	.long	0x84
	.value	0x4c0
	.uleb128 0x11
	.long	.LASF83
	.byte	0xa
	.byte	0x3e
	.byte	0x8
	.long	0x84
	.value	0x4c8
	.uleb128 0x11
	.long	.LASF84
	.byte	0xa
	.byte	0x41
	.byte	0x8
	.long	0x84
	.value	0x4d0
	.uleb128 0x11
	.long	.LASF85
	.byte	0xa
	.byte	0x42
	.byte	0x8
	.long	0x84
	.value	0x4d8
	.uleb128 0x11
	.long	.LASF86
	.byte	0xa
	.byte	0x45
	.byte	0x8
	.long	0x84
	.value	0x4e0
	.uleb128 0x11
	.long	.LASF87
	.byte	0xa
	.byte	0x46
	.byte	0x8
	.long	0x84
	.value	0x4e8
	.uleb128 0x11
	.long	.LASF88
	.byte	0xa
	.byte	0x49
	.byte	0x7
	.long	0x71
	.value	0x4f0
	.uleb128 0x11
	.long	.LASF89
	.byte	0xa
	.byte	0x4a
	.byte	0x7
	.long	0x71
	.value	0x4f4
	.uleb128 0x11
	.long	.LASF90
	.byte	0xa
	.byte	0x4b
	.byte	0x7
	.long	0x71
	.value	0x4f8
	.uleb128 0x11
	.long	.LASF91
	.byte	0xa
	.byte	0x4c
	.byte	0x7
	.long	0x71
	.value	0x4fc
	.uleb128 0x11
	.long	.LASF92
	.byte	0xa
	.byte	0x4d
	.byte	0x7
	.long	0x71
	.value	0x500
	.uleb128 0x11
	.long	.LASF93
	.byte	0xa
	.byte	0x50
	.byte	0xc
	.long	0x6f1
	.value	0x508
	.uleb128 0x11
	.long	.LASF94
	.byte	0xa
	.byte	0x51
	.byte	0x7
	.long	0x71
	.value	0x510
	.uleb128 0x11
	.long	.LASF95
	.byte	0xa
	.byte	0x52
	.byte	0x7
	.long	0x71
	.value	0x514
	.uleb128 0x11
	.long	.LASF96
	.byte	0xa
	.byte	0x55
	.byte	0x7
	.long	0x71
	.value	0x518
	.uleb128 0x11
	.long	.LASF97
	.byte	0xa
	.byte	0x56
	.byte	0x7
	.long	0x71
	.value	0x51c
	.byte	0
	.uleb128 0x8
	.long	.LASF98
	.byte	0x20
	.byte	0x9
	.byte	0xc
	.byte	0x8
	.long	0x5b4
	.uleb128 0x13
	.string	"w"
	.byte	0x9
	.byte	0xe
	.byte	0x8
	.long	0x84
	.byte	0
	.uleb128 0x13
	.string	"h"
	.byte	0x9
	.byte	0xf
	.byte	0x8
	.long	0x84
	.byte	0x8
	.uleb128 0x13
	.string	"x"
	.byte	0x9
	.byte	0x10
	.byte	0x8
	.long	0x84
	.byte	0x10
	.uleb128 0x13
	.string	"y"
	.byte	0x9
	.byte	0x11
	.byte	0x8
	.long	0x84
	.byte	0x18
	.byte	0
	.uleb128 0x2
	.long	.LASF99
	.byte	0x9
	.byte	0x13
	.byte	0x1c
	.long	0x57a
	.uleb128 0x8
	.long	.LASF100
	.byte	0xa0
	.byte	0x9
	.byte	0x16
	.byte	0x8
	.long	0x6eb
	.uleb128 0x13
	.string	"bf"
	.byte	0x9
	.byte	0x18
	.byte	0x9
	.long	0x6eb
	.byte	0
	.uleb128 0x9
	.long	.LASF66
	.byte	0x9
	.byte	0x1a
	.byte	0x8
	.long	0x84
	.byte	0x8
	.uleb128 0x9
	.long	.LASF101
	.byte	0x9
	.byte	0x1b
	.byte	0x8
	.long	0x84
	.byte	0x10
	.uleb128 0x9
	.long	.LASF67
	.byte	0x9
	.byte	0x1d
	.byte	0x8
	.long	0x84
	.byte	0x18
	.uleb128 0x9
	.long	.LASF68
	.byte	0x9
	.byte	0x1e
	.byte	0x8
	.long	0x84
	.byte	0x20
	.uleb128 0x13
	.string	"bbx"
	.byte	0x9
	.byte	0x20
	.byte	0x9
	.long	0x5b4
	.byte	0x28
	.uleb128 0x9
	.long	.LASF102
	.byte	0x9
	.byte	0x21
	.byte	0x8
	.long	0x84
	.byte	0x48
	.uleb128 0x9
	.long	.LASF103
	.byte	0x9
	.byte	0x22
	.byte	0x7
	.long	0x71
	.byte	0x50
	.uleb128 0x9
	.long	.LASF104
	.byte	0x9
	.byte	0x24
	.byte	0xc
	.long	0x6f1
	.byte	0x58
	.uleb128 0x9
	.long	.LASF105
	.byte	0x9
	.byte	0x25
	.byte	0x7
	.long	0x71
	.byte	0x60
	.uleb128 0x9
	.long	.LASF106
	.byte	0x9
	.byte	0x26
	.byte	0x7
	.long	0x71
	.byte	0x64
	.uleb128 0x9
	.long	.LASF93
	.byte	0x9
	.byte	0x2b
	.byte	0xc
	.long	0x6f1
	.byte	0x68
	.uleb128 0x9
	.long	.LASF94
	.byte	0x9
	.byte	0x2c
	.byte	0x7
	.long	0x71
	.byte	0x70
	.uleb128 0x9
	.long	.LASF95
	.byte	0x9
	.byte	0x2d
	.byte	0x7
	.long	0x71
	.byte	0x74
	.uleb128 0x9
	.long	.LASF107
	.byte	0x9
	.byte	0x2e
	.byte	0x7
	.long	0x71
	.byte	0x78
	.uleb128 0x9
	.long	.LASF108
	.byte	0x9
	.byte	0x32
	.byte	0xc
	.long	0x40
	.byte	0x7c
	.uleb128 0x9
	.long	.LASF109
	.byte	0x9
	.byte	0x33
	.byte	0xc
	.long	0x40
	.byte	0x80
	.uleb128 0x9
	.long	.LASF110
	.byte	0x9
	.byte	0x35
	.byte	0x7
	.long	0x71
	.byte	0x84
	.uleb128 0x9
	.long	.LASF111
	.byte	0x9
	.byte	0x36
	.byte	0xc
	.long	0x40
	.byte	0x88
	.uleb128 0x9
	.long	.LASF112
	.byte	0x9
	.byte	0x37
	.byte	0xc
	.long	0x40
	.byte	0x8c
	.uleb128 0x9
	.long	.LASF113
	.byte	0x9
	.byte	0x38
	.byte	0xc
	.long	0x40
	.byte	0x90
	.uleb128 0x9
	.long	.LASF114
	.byte	0x9
	.byte	0x3a
	.byte	0x8
	.long	0x84
	.byte	0x98
	.byte	0
	.uleb128 0x6
	.byte	0x8
	.long	0x327
	.uleb128 0x6
	.byte	0x8
	.long	0x308
	.uleb128 0x2
	.long	.LASF115
	.byte	0x9
	.byte	0x3d
	.byte	0x22
	.long	0x5c0
	.uleb128 0x6
	.byte	0x8
	.long	0x709
	.uleb128 0x6
	.byte	0x8
	.long	0x6f7
	.uleb128 0xc
	.long	0xa9
	.long	0x720
	.uleb128 0x14
	.long	0x39
	.value	0x3ff
	.byte	0
	.uleb128 0x15
	.long	.LASF157
	.byte	0x1
	.value	0x274
	.byte	0x6
	.quad	.LFB13
	.quad	.LFE13-.LFB13
	.uleb128 0x1
	.byte	0x9c
	.long	0x922
	.uleb128 0x16
	.string	"bf"
	.byte	0x1
	.value	0x274
	.byte	0x24
	.long	0x6eb
	.uleb128 0x3
	.byte	0x91
	.sleb128 -168
	.uleb128 0x17
	.string	"i"
	.byte	0x1
	.value	0x276
	.byte	0x7
	.long	0x71
	.uleb128 0x2
	.byte	0x91
	.sleb128 -36
	.uleb128 0x17
	.string	"j"
	.byte	0x1
	.value	0x276
	.byte	0xa
	.long	0x71
	.uleb128 0x2
	.byte	0x91
	.sleb128 -40
	.uleb128 0x17
	.string	"bg"
	.byte	0x1
	.value	0x277
	.byte	0x9
	.long	0x709
	.uleb128 0x3
	.byte	0x91
	.sleb128 -136
	.uleb128 0x18
	.long	.LASF116
	.byte	0x1
	.value	0x279
	.byte	0x7
	.long	0x71
	.uleb128 0x2
	.byte	0x91
	.sleb128 -44
	.uleb128 0x18
	.long	.LASF117
	.byte	0x1
	.value	0x279
	.byte	0xe
	.long	0x71
	.uleb128 0x2
	.byte	0x91
	.sleb128 -48
	.uleb128 0x18
	.long	.LASF118
	.byte	0x1
	.value	0x27a
	.byte	0x7
	.long	0x71
	.uleb128 0x2
	.byte	0x91
	.sleb128 -52
	.uleb128 0x18
	.long	.LASF119
	.byte	0x1
	.value	0x27a
	.byte	0x15
	.long	0x71
	.uleb128 0x2
	.byte	0x91
	.sleb128 -56
	.uleb128 0x18
	.long	.LASF120
	.byte	0x1
	.value	0x27b
	.byte	0x11
	.long	0x39
	.uleb128 0x3
	.byte	0x91
	.sleb128 -112
	.uleb128 0x18
	.long	.LASF121
	.byte	0x1
	.value	0x27c
	.byte	0x11
	.long	0x39
	.uleb128 0x2
	.byte	0x91
	.sleb128 -64
	.uleb128 0x18
	.long	.LASF122
	.byte	0x1
	.value	0x27e
	.byte	0x7
	.long	0x71
	.uleb128 0x3
	.byte	0x91
	.sleb128 -116
	.uleb128 0x18
	.long	.LASF123
	.byte	0x1
	.value	0x27f
	.byte	0x7
	.long	0x71
	.uleb128 0x3
	.byte	0x91
	.sleb128 -68
	.uleb128 0x18
	.long	.LASF124
	.byte	0x1
	.value	0x280
	.byte	0x7
	.long	0x71
	.uleb128 0x3
	.byte	0x91
	.sleb128 -120
	.uleb128 0x18
	.long	.LASF125
	.byte	0x1
	.value	0x281
	.byte	0x7
	.long	0x71
	.uleb128 0x3
	.byte	0x91
	.sleb128 -72
	.uleb128 0x18
	.long	.LASF126
	.byte	0x1
	.value	0x282
	.byte	0x7
	.long	0x71
	.uleb128 0x3
	.byte	0x91
	.sleb128 -124
	.uleb128 0x18
	.long	.LASF127
	.byte	0x1
	.value	0x283
	.byte	0x7
	.long	0x71
	.uleb128 0x3
	.byte	0x91
	.sleb128 -76
	.uleb128 0x18
	.long	.LASF128
	.byte	0x1
	.value	0x284
	.byte	0x7
	.long	0x71
	.uleb128 0x3
	.byte	0x91
	.sleb128 -128
	.uleb128 0x18
	.long	.LASF129
	.byte	0x1
	.value	0x285
	.byte	0x7
	.long	0x71
	.uleb128 0x3
	.byte	0x91
	.sleb128 -80
	.uleb128 0x18
	.long	.LASF130
	.byte	0x1
	.value	0x286
	.byte	0x7
	.long	0x71
	.uleb128 0x3
	.byte	0x91
	.sleb128 -84
	.uleb128 0x17
	.string	"pos"
	.byte	0x1
	.value	0x288
	.byte	0xc
	.long	0x40
	.uleb128 0x3
	.byte	0x91
	.sleb128 -152
	.uleb128 0x18
	.long	.LASF131
	.byte	0x1
	.value	0x289
	.byte	0xc
	.long	0x40
	.uleb128 0x3
	.byte	0x91
	.sleb128 -88
	.uleb128 0x18
	.long	.LASF132
	.byte	0x1
	.value	0x28a
	.byte	0xc
	.long	0x40
	.uleb128 0x3
	.byte	0x91
	.sleb128 -140
	.uleb128 0x18
	.long	.LASF133
	.byte	0x1
	.value	0x28b
	.byte	0xc
	.long	0x40
	.uleb128 0x3
	.byte	0x91
	.sleb128 -144
	.uleb128 0x18
	.long	.LASF134
	.byte	0x1
	.value	0x28c
	.byte	0xc
	.long	0x314
	.uleb128 0x3
	.byte	0x91
	.sleb128 -148
	.uleb128 0x18
	.long	.LASF135
	.byte	0x1
	.value	0x28d
	.byte	0xc
	.long	0x314
	.uleb128 0x3
	.byte	0x91
	.sleb128 -92
	.uleb128 0x18
	.long	.LASF136
	.byte	0x1
	.value	0x28e
	.byte	0xc
	.long	0x314
	.uleb128 0x3
	.byte	0x91
	.sleb128 -96
	.uleb128 0x18
	.long	.LASF137
	.byte	0x1
	.value	0x28f
	.byte	0xc
	.long	0x40
	.uleb128 0x3
	.byte	0x91
	.sleb128 -100
	.uleb128 0x18
	.long	.LASF138
	.byte	0x1
	.value	0x290
	.byte	0xc
	.long	0x40
	.uleb128 0x3
	.byte	0x91
	.sleb128 -104
	.uleb128 0x19
	.long	.LASF150
	.long	0x932
	.uleb128 0x9
	.byte	0x3
	.quad	__PRETTY_FUNCTION__.3275
	.byte	0
	.uleb128 0xc
	.long	0xb0
	.long	0x932
	.uleb128 0xd
	.long	0x39
	.byte	0x17
	.byte	0
	.uleb128 0x7
	.long	0x922
	.uleb128 0x1a
	.long	.LASF140
	.byte	0x1
	.value	0x262
	.byte	0xa
	.long	0x40
	.quad	.LFB12
	.quad	.LFE12-.LFB12
	.uleb128 0x1
	.byte	0x9c
	.long	0x98a
	.uleb128 0x16
	.string	"bf"
	.byte	0x1
	.value	0x262
	.byte	0x26
	.long	0x6eb
	.uleb128 0x2
	.byte	0x91
	.sleb128 -40
	.uleb128 0x1b
	.long	.LASF66
	.byte	0x1
	.value	0x262
	.byte	0x32
	.long	0x308
	.uleb128 0x2
	.byte	0x91
	.sleb128 -44
	.uleb128 0x18
	.long	.LASF139
	.byte	0x1
	.value	0x264
	.byte	0xc
	.long	0x6f1
	.uleb128 0x2
	.byte	0x91
	.sleb128 -24
	.byte	0
	.uleb128 0x1c
	.long	.LASF141
	.byte	0x1
	.value	0x211
	.byte	0xf
	.long	0x39
	.quad	.LFB11
	.quad	.LFE11-.LFB11
	.uleb128 0x1
	.byte	0x9c
	.long	0xa2f
	.uleb128 0x16
	.string	"bf"
	.byte	0x1
	.value	0x211
	.byte	0x3a
	.long	0x6eb
	.uleb128 0x3
	.byte	0x91
	.sleb128 -88
	.uleb128 0x1b
	.long	.LASF116
	.byte	0x1
	.value	0x211
	.byte	0x42
	.long	0x71
	.uleb128 0x3
	.byte	0x91
	.sleb128 -92
	.uleb128 0x1b
	.long	.LASF117
	.byte	0x1
	.value	0x211
	.byte	0x4d
	.long	0x71
	.uleb128 0x3
	.byte	0x91
	.sleb128 -96
	.uleb128 0x1b
	.long	.LASF142
	.byte	0x1
	.value	0x211
	.byte	0x58
	.long	0x71
	.uleb128 0x3
	.byte	0x91
	.sleb128 -100
	.uleb128 0x17
	.string	"i"
	.byte	0x1
	.value	0x213
	.byte	0x7
	.long	0x71
	.uleb128 0x2
	.byte	0x91
	.sleb128 -20
	.uleb128 0x17
	.string	"bg"
	.byte	0x1
	.value	0x214
	.byte	0x9
	.long	0x709
	.uleb128 0x2
	.byte	0x91
	.sleb128 -40
	.uleb128 0x18
	.long	.LASF120
	.byte	0x1
	.value	0x215
	.byte	0x11
	.long	0x39
	.uleb128 0x2
	.byte	0x91
	.sleb128 -32
	.uleb128 0x18
	.long	.LASF143
	.byte	0x1
	.value	0x216
	.byte	0x9
	.long	0x5b4
	.uleb128 0x3
	.byte	0x91
	.sleb128 -80
	.byte	0
	.uleb128 0x1c
	.long	.LASF144
	.byte	0x1
	.value	0x16a
	.byte	0x5
	.long	0x71
	.quad	.LFB10
	.quad	.LFE10-.LFB10
	.uleb128 0x1
	.byte	0x9c
	.long	0xb39
	.uleb128 0x16
	.string	"bg"
	.byte	0x1
	.value	0x16a
	.byte	0x1b
	.long	0x709
	.uleb128 0x2
	.byte	0x91
	.sleb128 -56
	.uleb128 0x16
	.string	"bbx"
	.byte	0x1
	.value	0x16a
	.byte	0x26
	.long	0xb39
	.uleb128 0x2
	.byte	0x91
	.sleb128 -64
	.uleb128 0x1b
	.long	.LASF108
	.byte	0x1
	.value	0x16a
	.byte	0x34
	.long	0x40
	.uleb128 0x3
	.byte	0x91
	.sleb128 -68
	.uleb128 0x1b
	.long	.LASF109
	.byte	0x1
	.value	0x16a
	.byte	0x4d
	.long	0x40
	.uleb128 0x3
	.byte	0x91
	.sleb128 -72
	.uleb128 0x1b
	.long	.LASF142
	.byte	0x1
	.value	0x16a
	.byte	0x61
	.long	0x71
	.uleb128 0x3
	.byte	0x91
	.sleb128 -76
	.uleb128 0x17
	.string	"x"
	.byte	0x1
	.value	0x16c
	.byte	0x7
	.long	0x71
	.uleb128 0x2
	.byte	0x91
	.sleb128 -20
	.uleb128 0x17
	.string	"y"
	.byte	0x1
	.value	0x16d
	.byte	0x7
	.long	0x71
	.uleb128 0x2
	.byte	0x91
	.sleb128 -24
	.uleb128 0x17
	.string	"i"
	.byte	0x1
	.value	0x16e
	.byte	0x7
	.long	0x71
	.uleb128 0x2
	.byte	0x91
	.sleb128 -28
	.uleb128 0x18
	.long	.LASF145
	.byte	0x1
	.value	0x16f
	.byte	0x7
	.long	0x71
	.uleb128 0x2
	.byte	0x91
	.sleb128 -32
	.uleb128 0x18
	.long	.LASF146
	.byte	0x1
	.value	0x170
	.byte	0x7
	.long	0x71
	.uleb128 0x2
	.byte	0x91
	.sleb128 -36
	.uleb128 0x18
	.long	.LASF147
	.byte	0x1
	.value	0x171
	.byte	0x7
	.long	0x71
	.uleb128 0x2
	.byte	0x91
	.sleb128 -40
	.uleb128 0x18
	.long	.LASF148
	.byte	0x1
	.value	0x172
	.byte	0x7
	.long	0x71
	.uleb128 0x2
	.byte	0x91
	.sleb128 -44
	.uleb128 0x18
	.long	.LASF149
	.byte	0x1
	.value	0x174
	.byte	0xe
	.long	0xb3f
	.uleb128 0x9
	.byte	0x3
	.quad	bd_list.3193
	.uleb128 0x19
	.long	.LASF150
	.long	0xb60
	.uleb128 0x9
	.byte	0x3
	.quad	__PRETTY_FUNCTION__.3200
	.byte	0
	.uleb128 0x6
	.byte	0x8
	.long	0x5b4
	.uleb128 0xc
	.long	0x71
	.long	0xb50
	.uleb128 0x14
	.long	0x39
	.value	0x7ff
	.byte	0
	.uleb128 0xc
	.long	0xb0
	.long	0xb60
	.uleb128 0xd
	.long	0x39
	.byte	0xf
	.byte	0
	.uleb128 0x7
	.long	0xb50
	.uleb128 0x1d
	.long	.LASF151
	.byte	0x1
	.value	0x153
	.byte	0xc
	.long	0x71
	.quad	.LFB9
	.quad	.LFE9-.LFB9
	.uleb128 0x1
	.byte	0x9c
	.long	0xbb4
	.uleb128 0x16
	.string	"bg"
	.byte	0x1
	.value	0x153
	.byte	0x24
	.long	0x709
	.uleb128 0x2
	.byte	0x91
	.sleb128 -24
	.uleb128 0x16
	.string	"a"
	.byte	0x1
	.value	0x153
	.byte	0x31
	.long	0x40
	.uleb128 0x2
	.byte	0x91
	.sleb128 -28
	.uleb128 0x16
	.string	"b"
	.byte	0x1
	.value	0x153
	.byte	0x3d
	.long	0x40
	.uleb128 0x2
	.byte	0x91
	.sleb128 -32
	.byte	0
	.uleb128 0x1d
	.long	.LASF152
	.byte	0x1
	.value	0x123
	.byte	0xc
	.long	0x71
	.quad	.LFB8
	.quad	.LFE8-.LFB8
	.uleb128 0x1
	.byte	0x9c
	.long	0xc03
	.uleb128 0x16
	.string	"bg"
	.byte	0x1
	.value	0x123
	.byte	0x1c
	.long	0x709
	.uleb128 0x2
	.byte	0x91
	.sleb128 -24
	.uleb128 0x16
	.string	"a"
	.byte	0x1
	.value	0x123
	.byte	0x29
	.long	0x40
	.uleb128 0x2
	.byte	0x91
	.sleb128 -28
	.uleb128 0x16
	.string	"b"
	.byte	0x1
	.value	0x123
	.byte	0x35
	.long	0x40
	.uleb128 0x2
	.byte	0x91
	.sleb128 -32
	.byte	0
	.uleb128 0x1e
	.long	.LASF158
	.byte	0x1
	.value	0x118
	.byte	0xd
	.quad	.LFB7
	.quad	.LFE7-.LFB7
	.uleb128 0x1
	.byte	0x9c
	.long	0xc52
	.uleb128 0x16
	.string	"bg"
	.byte	0x1
	.value	0x118
	.byte	0x1f
	.long	0x709
	.uleb128 0x2
	.byte	0x91
	.sleb128 -24
	.uleb128 0x1b
	.long	.LASF108
	.byte	0x1
	.value	0x118
	.byte	0x2c
	.long	0x40
	.uleb128 0x2
	.byte	0x91
	.sleb128 -28
	.uleb128 0x1b
	.long	.LASF109
	.byte	0x1
	.value	0x118
	.byte	0x45
	.long	0x40
	.uleb128 0x2
	.byte	0x91
	.sleb128 -32
	.byte	0
	.uleb128 0x1f
	.long	.LASF159
	.byte	0x1
	.value	0x113
	.byte	0xd
	.quad	.LFB6
	.quad	.LFE6-.LFB6
	.uleb128 0x1
	.byte	0x9c
	.uleb128 0x16
	.string	"s"
	.byte	0x1
	.value	0x113
	.byte	0x20
	.long	0x2e3
	.uleb128 0x2
	.byte	0x91
	.sleb128 -24
	.byte	0
	.byte	0
	.section	.debug_abbrev,"",@progbits
.Ldebug_abbrev0:
	.uleb128 0x1
	.uleb128 0x11
	.byte	0x1
	.uleb128 0x25
	.uleb128 0xe
	.uleb128 0x13
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x1b
	.uleb128 0xe
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x7
	.uleb128 0x10
	.uleb128 0x17
	.byte	0
	.byte	0
	.uleb128 0x2
	.uleb128 0x16
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x3
	.uleb128 0x24
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3e
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0xe
	.byte	0
	.byte	0
	.uleb128 0x4
	.uleb128 0xf
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
	.byte	0
	.byte	0
	.uleb128 0x5
	.uleb128 0x24
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3e
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0x8
	.byte	0
	.byte	0
	.uleb128 0x6
	.uleb128 0xf
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x7
	.uleb128 0x26
	.byte	0
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x8
	.uleb128 0x13
	.byte	0x1
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x9
	.uleb128 0xd
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x38
	.uleb128 0xb
	.byte	0
	.byte	0
	.uleb128 0xa
	.uleb128 0x16
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0xb
	.byte	0
	.byte	0
	.uleb128 0xb
	.uleb128 0x13
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3c
	.uleb128 0x19
	.byte	0
	.byte	0
	.uleb128 0xc
	.uleb128 0x1
	.byte	0x1
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0xd
	.uleb128 0x21
	.byte	0
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2f
	.uleb128 0xb
	.byte	0
	.byte	0
	.uleb128 0xe
	.uleb128 0x34
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x3f
	.uleb128 0x19
	.uleb128 0x3c
	.uleb128 0x19
	.byte	0
	.byte	0
	.uleb128 0xf
	.uleb128 0x21
	.byte	0
	.byte	0
	.byte	0
	.uleb128 0x10
	.uleb128 0x13
	.byte	0x1
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0xb
	.uleb128 0x5
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x11
	.uleb128 0xd
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x38
	.uleb128 0x5
	.byte	0
	.byte	0
	.uleb128 0x12
	.uleb128 0xd
	.byte	0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x38
	.uleb128 0x5
	.byte	0
	.byte	0
	.uleb128 0x13
	.uleb128 0xd
	.byte	0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x38
	.uleb128 0xb
	.byte	0
	.byte	0
	.uleb128 0x14
	.uleb128 0x21
	.byte	0
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2f
	.uleb128 0x5
	.byte	0
	.byte	0
	.uleb128 0x15
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x3f
	.uleb128 0x19
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x27
	.uleb128 0x19
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x7
	.uleb128 0x40
	.uleb128 0x18
	.uleb128 0x2116
	.uleb128 0x19
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x16
	.uleb128 0x5
	.byte	0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0x18
	.byte	0
	.byte	0
	.uleb128 0x17
	.uleb128 0x34
	.byte	0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0x18
	.byte	0
	.byte	0
	.uleb128 0x18
	.uleb128 0x34
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0x18
	.byte	0
	.byte	0
	.uleb128 0x19
	.uleb128 0x34
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x34
	.uleb128 0x19
	.uleb128 0x2
	.uleb128 0x18
	.byte	0
	.byte	0
	.uleb128 0x1a
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x3f
	.uleb128 0x19
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x27
	.uleb128 0x19
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x7
	.uleb128 0x40
	.uleb128 0x18
	.uleb128 0x2117
	.uleb128 0x19
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x1b
	.uleb128 0x5
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0x18
	.byte	0
	.byte	0
	.uleb128 0x1c
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x3f
	.uleb128 0x19
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x27
	.uleb128 0x19
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x7
	.uleb128 0x40
	.uleb128 0x18
	.uleb128 0x2116
	.uleb128 0x19
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x1d
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x27
	.uleb128 0x19
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x7
	.uleb128 0x40
	.uleb128 0x18
	.uleb128 0x2116
	.uleb128 0x19
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x1e
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x27
	.uleb128 0x19
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x7
	.uleb128 0x40
	.uleb128 0x18
	.uleb128 0x2116
	.uleb128 0x19
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x1f
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x27
	.uleb128 0x19
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x7
	.uleb128 0x40
	.uleb128 0x18
	.uleb128 0x2116
	.uleb128 0x19
	.byte	0
	.byte	0
	.byte	0
	.section	.debug_aranges,"",@progbits
	.long	0x2c
	.value	0x2
	.long	.Ldebug_info0
	.byte	0x8
	.byte	0
	.value	0
	.value	0
	.quad	.Ltext0
	.quad	.Letext0-.Ltext0
	.quad	0
	.quad	0
	.section	.debug_line,"",@progbits
.Ldebug_line0:
	.section	.debug_str,"MS",@progbits,1
.LASF59:
	.string	"is_verbose"
.LASF32:
	.string	"_shortbuf"
.LASF76:
	.string	"str_font"
.LASF156:
	.string	"_IO_lock_t"
.LASF48:
	.string	"stderr"
.LASF21:
	.string	"_IO_buf_end"
.LASF100:
	.string	"_bdf_glyph_struct"
.LASF123:
	.string	"idx_cap_a_ascent"
.LASF86:
	.string	"x_min"
.LASF58:
	.string	"_bdf_font_struct"
.LASF101:
	.string	"map_to"
.LASF19:
	.string	"_IO_write_end"
.LASF1:
	.string	"unsigned int"
.LASF122:
	.string	"idx_cap_a"
.LASF37:
	.string	"_freeres_list"
.LASF13:
	.string	"_flags"
.LASF141:
	.string	"bf_RLECompressAllGlyphsWithFieldSize"
.LASF145:
	.string	"bd_is_one"
.LASF63:
	.string	"line_buf"
.LASF25:
	.string	"_markers"
.LASF144:
	.string	"bg_rle_compress"
.LASF93:
	.string	"target_data"
.LASF65:
	.string	"is_bitmap_parsing"
.LASF151:
	.string	"bg_prepare_01_rle"
.LASF112:
	.string	"rle_last_0"
.LASF113:
	.string	"rle_last_1"
.LASF102:
	.string	"shift_x"
.LASF111:
	.string	"rle_bitcnt"
.LASF96:
	.string	"tile_h_size"
.LASF121:
	.string	"min_total_bits"
.LASF130:
	.string	"idx_para_descent"
.LASF54:
	.string	"uint32_t"
.LASF98:
	.string	"_bbx_struct"
.LASF159:
	.string	"bg_err"
.LASF47:
	.string	"stdout"
.LASF24:
	.string	"_IO_save_end"
.LASF44:
	.string	"_IO_codecvt"
.LASF149:
	.string	"bd_list"
.LASF104:
	.string	"bitmap_data"
.LASF61:
	.string	"glyph_cnt"
.LASF52:
	.string	"long long unsigned int"
.LASF70:
	.string	"bbx_h"
.LASF50:
	.string	"sys_errlist"
.LASF23:
	.string	"_IO_backup_base"
.LASF34:
	.string	"_offset"
.LASF99:
	.string	"bbx_t"
.LASF119:
	.string	"best_rle_1"
.LASF69:
	.string	"bbx_w"
.LASF71:
	.string	"bbx_x"
.LASF72:
	.string	"bbx_y"
.LASF49:
	.string	"sys_nerr"
.LASF84:
	.string	"dx_min"
.LASF27:
	.string	"_fileno"
.LASF115:
	.string	"bg_t"
.LASF125:
	.string	"idx_1_ascent"
.LASF5:
	.string	"size_t"
.LASF16:
	.string	"_IO_read_base"
.LASF80:
	.string	"enc_w"
.LASF67:
	.string	"dwidth_x"
.LASF68:
	.string	"dwidth_y"
.LASF148:
	.string	"bd_chg_cnt"
.LASF46:
	.string	"stdin"
.LASF62:
	.string	"glyph_max"
.LASF110:
	.string	"rle_is_first"
.LASF12:
	.string	"char"
.LASF118:
	.string	"best_rle_0"
.LASF40:
	.string	"_mode"
.LASF66:
	.string	"encoding"
.LASF138:
	.string	"unicode_lookup_table_glyph_cnt"
.LASF43:
	.string	"_IO_marker"
.LASF137:
	.string	"unicode_lookup_table_pos"
.LASF14:
	.string	"_IO_read_ptr"
.LASF142:
	.string	"is_output"
.LASF140:
	.string	"bf_RLE_get_glyph_data"
.LASF53:
	.string	"uint8_t"
.LASF92:
	.string	"dx_max_bit_size"
.LASF75:
	.string	"glyph_pos"
.LASF135:
	.string	"unicode_last_delta"
.LASF127:
	.string	"idx_g_descent"
.LASF17:
	.string	"_IO_write_base"
.LASF51:
	.string	"long long int"
.LASF22:
	.string	"_IO_save_base"
.LASF95:
	.string	"target_cnt"
.LASF128:
	.string	"idx_para"
.LASF87:
	.string	"x_max"
.LASF60:
	.string	"glyph_list"
.LASF139:
	.string	"font"
.LASF38:
	.string	"_freeres_buf"
.LASF154:
	.string	"bdf_rle.c"
.LASF39:
	.string	"__pad5"
.LASF74:
	.string	"bitmap_y"
.LASF89:
	.string	"bbx_y_max_bit_size"
.LASF107:
	.string	"target_bit_pos"
.LASF108:
	.string	"rle_bits_per_0"
.LASF109:
	.string	"rle_bits_per_1"
.LASF31:
	.string	"_vtable_offset"
.LASF129:
	.string	"idx_para_ascent"
.LASF88:
	.string	"bbx_x_max_bit_size"
.LASF55:
	.string	"long double"
.LASF94:
	.string	"target_max"
.LASF103:
	.string	"is_excluded_from_kerning"
.LASF132:
	.string	"unicode_start_pos"
.LASF136:
	.string	"unicode_last_target_cnt"
.LASF155:
	.string	"/mnt/e/Project/S450_Front_Panel/Project/u8g2/tools/font/bdfconv"
.LASF77:
	.string	"str_copyright"
.LASF134:
	.string	"unicode_lookup_table_start"
.LASF90:
	.string	"bbx_w_max_bit_size"
.LASF146:
	.string	"bd_curr_len"
.LASF15:
	.string	"_IO_read_end"
.LASF73:
	.string	"bitmap_x"
.LASF7:
	.string	"short int"
.LASF9:
	.string	"long int"
.LASF97:
	.string	"tile_v_size"
.LASF45:
	.string	"_IO_wide_data"
.LASF114:
	.string	"width_deviation"
.LASF78:
	.string	"selected_glyphs"
.LASF6:
	.string	"__uint8_t"
.LASF116:
	.string	"rle_0"
.LASF117:
	.string	"rle_1"
.LASF85:
	.string	"dx_max"
.LASF36:
	.string	"_wide_data"
.LASF147:
	.string	"bd_max_len"
.LASF33:
	.string	"_lock"
.LASF0:
	.string	"long unsigned int"
.LASF29:
	.string	"_old_offset"
.LASF57:
	.string	"_IO_FILE"
.LASF79:
	.string	"bbx_mode"
.LASF133:
	.string	"unicode_lookup_table_len"
.LASF152:
	.string	"bg_01_rle"
.LASF81:
	.string	"enc_h"
.LASF158:
	.string	"bg_init_rle"
.LASF64:
	.string	"line_pos"
.LASF2:
	.string	"unsigned char"
.LASF82:
	.string	"enc_x"
.LASF83:
	.string	"enc_y"
.LASF8:
	.string	"__uint32_t"
.LASF105:
	.string	"bitmap_width"
.LASF18:
	.string	"_IO_write_ptr"
.LASF157:
	.string	"bf_RLECompressAllGlyphs"
.LASF143:
	.string	"local_bbx"
.LASF153:
	.string	"GNU C17 8.3.0 -mtune=generic -march=x86-64 -g"
.LASF35:
	.string	"_codecvt"
.LASF91:
	.string	"bbx_h_max_bit_size"
.LASF10:
	.string	"__off_t"
.LASF4:
	.string	"signed char"
.LASF56:
	.string	"bf_t"
.LASF131:
	.string	"ascii_glyphs"
.LASF3:
	.string	"short unsigned int"
.LASF124:
	.string	"idx_1"
.LASF150:
	.string	"__PRETTY_FUNCTION__"
.LASF26:
	.string	"_chain"
.LASF42:
	.string	"FILE"
.LASF28:
	.string	"_flags2"
.LASF106:
	.string	"bitmap_height"
.LASF30:
	.string	"_cur_column"
.LASF120:
	.string	"total_bits"
.LASF126:
	.string	"idx_g"
.LASF11:
	.string	"__off64_t"
.LASF41:
	.string	"_unused2"
.LASF20:
	.string	"_IO_buf_base"
	.ident	"GCC: (Debian 8.3.0-6) 8.3.0"
	.section	.note.GNU-stack,"",@progbits
