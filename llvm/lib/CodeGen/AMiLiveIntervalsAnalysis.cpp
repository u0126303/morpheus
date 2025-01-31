#include "AMiLiveIntervalsAnalysis.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

AMiLiveIntervalsAnalysis::AMiLiveIntervalsAnalysis() : MachineFunctionPass(ID) {}

bool AMiLiveIntervalsAnalysis::runOnMachineFunction(MachineFunction &MF) {
      errs() << "Running AMiLiveIntervalsAnalysis on function: " 
             << MF.getName() << "\n";

      // Example: Iterate over MachineBasicBlocks and MachineInstructions
      for (MachineBasicBlock &MBB : MF) {
        for (MachineInstr &MI : MBB) {
          //errs() << "  Instruction: " << MI << "\n";
        }
      }
      return false; // Return true if the pass modifies the MachineFunction
    }

    // Optional: Provide analysis usage information
void AMiLiveIntervalsAnalysis::getAnalysisUsage(AnalysisUsage &AU) const {
      MachineFunctionPass::getAnalysisUsage(AU);
      AU.setPreservesAll();
}

char AMiLiveIntervalsAnalysis::ID = 0;

// Register the pass
static RegisterPass<AMiLiveIntervalsAnalysis> X("ami-liveintervals", "AMi Live Interval Analysis", false, false);
