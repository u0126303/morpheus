# To print the passes that are scheduled / run
#-debug-pass=Arguments
#-debug-pass=Executions

CLANG=../install/bin/clang
LLC=../install/bin/llc

#rm -f test.ll
#rm -f before.mir
#rm -f after.mir

#$CLANG -S -emit-llvm -target riscv32 test.c -o test.ll
#$LLC --stop-before=greedy test.ll -o before.mir

$LLC --run-pass=greedy,virtregrewriter before.mir -o after.mir
