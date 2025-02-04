#include "AMiLiveIntervalsAnalysis.h"
#include "llvm/CodeGen/LiveIntervals.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/SlotIndexes.h"
#include "llvm/Pass.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

AMiLiveIntervalsAnalysis::AMiLiveIntervalsAnalysis()
    : MachineFunctionPass(ID) {}

bool AMiLiveIntervalsAnalysis::runOnMachineFunction(MachineFunction &MF) {
  errs() << "Running AMiLiveIntervalsAnalysis on function: " << MF.getName()
         << "\n";

  SlotIndexes &SI = getAnalysis<SlotIndexesWrapperPass>().getSI();
  LiveIntervals &LIS = getAnalysis<LiveIntervalsWrapperPass>().getLIS();

  SmallVector<MachineInstr *> V;

  // Example: Iterate over MachineBasicBlocks and MachineInstructions
  for (MachineBasicBlock &MBB : MF) {
    for (MachineInstr &MI : MBB) {
      if (MI.getFlag(MI.Persistent)) {
        V.push_back(&MI);
      }
    }
  }

  LiveInterval *LI;
  VNInfo *VNI;
  for (auto MI : V) {
    if (MI == V[0]) {
      LI = &LIS.getInterval(MI->getOperand(0).getReg());
      VNI = LI->getVNInfoAt(LI->beginIndex());
    }
    else {
      SlotIndex I = SI.getInstructionIndex(*MI);
      LiveInterval::Segment S(I, I.getNextIndex(), VNI);
      LI->addSegment(S);
    }
  }

  return false; // Return true if the pass modifies the MachineFunction
}

// Optional: Provide analysis usage information
void AMiLiveIntervalsAnalysis::getAnalysisUsage(AnalysisUsage &AU) const {
  MachineFunctionPass::getAnalysisUsage(AU);
  AU.addRequired<LiveIntervalsWrapperPass>();
  AU.addRequired<SlotIndexesWrapperPass>();
  AU.setPreservesAll();
}

char AMiLiveIntervalsAnalysis::ID = 0;

// Register the pass
static RegisterPass<AMiLiveIntervalsAnalysis>
    X("ami-liveintervals", "AMi Live Interval Analysis", false, false);
