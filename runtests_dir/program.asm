.import print
.import init
.import new
.import delete
lis $4
.word 4
lis $11
.word 1
lis $12
.word init
lis $23
.word print
sw $1, -4($30)
sub $30, $30, $4
sw $2, -4($30)
sub $30, $30, $4
add $2, $0, $0
sw $31, -4($30)
sub $30, $30, $4
jalr $12
add $30, $30, $4
lw $31, -4($30)
sub $29, $30, $4
beq $0, $0, Fwain
Fwain:
; 0 : 0
lis $3
.word 0
sw $3, -4($30)
sub $30, $30, $4
; 0 : -4
lis $3
.word 0
sw $3, -4($30)
sub $30, $30, $4
; 1 : -8
lis $3
.word 1
sw $3, -4($30)
sub $30, $30, $4
Lloop0:
lw $3, 8($29)
sw $3, -4($30)
sub $30, $30, $4
lw $3, 4($29)
sw $3, -4($30)
sub $30, $30, $4
lw $3, -8($29)
add $30, $30, $4
lw $5, -4($30)
mult $3, $5
mflo $3
add $30, $30, $4
lw $5, -4($30)
slt $3, $5, $3
sub $3, $11, $3
beq $3, $0, Lend1
lis $3
.word -8
add $3, $3, $29
sw $3, -4($30)
sub $30, $30, $4
lw $3, 4($29)
sw $3, -4($30)
sub $30, $30, $4
lw $3, -8($29)
add $30, $30, $4
lw $5, -4($30)
mult $3, $5
mflo $3
add $30, $30, $4
lw $5, -4($30)
sw $3, 0($5)
lis $3
.word -4
add $3, $3, $29
sw $3, -4($30)
sub $30, $30, $4
lis $3
.word 1
sw $3, -4($30)
sub $30, $30, $4
lw $3, -4($29)
add $30, $30, $4
lw $5, -4($30)
add $3, $3, $5
add $30, $30, $4
lw $5, -4($30)
sw $3, 0($5)
beq $0, $0, Lloop0
Lend1:
lis $3
.word -8
add $3, $3, $29
sw $3, -4($30)
sub $30, $30, $4
lis $3
.word 1
add $30, $30, $4
lw $5, -4($30)
sw $3, 0($5)
Lloop2:
lis $3
.word 1
sw $3, -4($30)
sub $30, $30, $4
lw $3, -8($29)
add $30, $30, $4
lw $5, -4($30)
add $3, $3, $5
sw $3, -4($30)
sub $30, $30, $4
lw $3, 8($29)
add $30, $30, $4
lw $5, -4($30)
; SLASH
div $3, $5
mflo $3
sw $3, -4($30)
sub $30, $30, $4
lis $3
.word 1
sw $3, -4($30)
sub $30, $30, $4
lw $3, -8($29)
add $30, $30, $4
lw $5, -4($30)
add $3, $3, $5
add $30, $30, $4
lw $5, -4($30)
slt $3, $5, $3
sub $3, $11, $3
beq $3, $0, Lend3
lis $3
.word -8
add $3, $3, $29
sw $3, -4($30)
sub $30, $30, $4
lis $3
.word 1
sw $3, -4($30)
sub $30, $30, $4
lw $3, -8($29)
add $30, $30, $4
lw $5, -4($30)
add $3, $3, $5
add $30, $30, $4
lw $5, -4($30)
sw $3, 0($5)
beq $0, $0, Lloop2
Lend3:
lis $3
.word 0
add $3, $3, $29
sw $3, -4($30)
sub $30, $30, $4
lw $3, -8($29)
add $30, $30, $4
lw $5, -4($30)
sw $3, 0($5)
lw $3, -4($29)
sw $3, -4($30)
sub $30, $30, $4
lw $3, 0($29)
add $30, $30, $4
lw $5, -4($30)
sub $3, $3, $5
jr $31
add $30, $30, $4
add $30, $30, $4
