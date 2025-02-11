# To print the passes that are scheduled / run
#-debug-pass=Arguments
#-debug-pass=Executions

CLANG=../llvm/build/bin/clang
LLC=../llvm/build/bin/llc

#rm -f case.1.2.ll
#rm -f case.1.2.before.mir
#rm -f case.1.2.after.mir

#$CLANG -S -O1 -emit-llvm -target riscv32 case.1.2.c -o case.1.2.ll
#$LLC --stop-before=greedy case.1.2.ll -o case.1.2.before.mir

$LLC --run-pass=greedy,virtregrewriter -mtriple=riscv32 case.1.2.before.mir -o case.1.2.after.mir
