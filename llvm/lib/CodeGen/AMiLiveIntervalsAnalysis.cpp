#include "AMiLiveIntervalsAnalysis.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/CodeGen/LiveIntervals.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include <queue>
#include <unordered_set>

using namespace llvm;

AMiLiveIntervalsAnalysis::AMiLiveIntervalsAnalysis()
    : MachineFunctionPass(ID) {}

void AMiLiveIntervalsAnalysis::printMIRWithSlotIndexes(MachineFunction &MF) {
  for (MachineBasicBlock &MBB : MF) {
    errs() << "MBB #" << MBB.getNumber() << ":\n";

    for (MachineInstr &MI : MBB) {
      if (SI->hasIndex(MI)) {
        errs() << "  [" << SI->getInstructionIndex(MI) << "] ";
      } else {
        errs() << "  [No SlotIndex] ";
      }
      MI.print(errs());
    }
  }
}

std::vector<MachineBasicBlock *>
AMiLiveIntervalsAnalysis::findUnreachableBlocks(MachineFunction &MF,
                                                MachineBasicBlock *StartBB) {
  std::unordered_set<MachineBasicBlock *> V; // Visited
  std::queue<MachineBasicBlock *> W;         // Worklist
  std::vector<MachineBasicBlock *> R;

  // Start BFS from StartBB
  W.push(StartBB);
  V.insert(StartBB);

  while (!W.empty()) {
    MachineBasicBlock *CurBB = W.front();
    W.pop();

    // Traverse successors
    for (MachineBasicBlock *Succ : CurBB->successors()) {
      if (V.insert(Succ).second) { // Insert and check if newly inserted
        W.push(Succ);
      }
    }
  }

  // Collect the unreachable (i.e., the unvisited) blocks
  for (MachineBasicBlock &MBB : MF) {
    if (!V.count(&MBB)) {
      R.push_back(&MBB);
    }
  }

  return R;
}

std::vector<MachineBasicBlock *>
AMiLiveIntervalsAnalysis::findDisconnectedBlocks(MachineFunction &MF,
                                                 MachineBasicBlock *StartBB) {
  std::vector<MachineBasicBlock *> R;

  for (MachineBasicBlock *MBB : findUnreachableBlocks(MF, StartBB)) {
    std::vector<MachineBasicBlock *> U = findUnreachableBlocks(MF, MBB);
    if (std::find(U.begin(), U.end(), StartBB) != U.end()) {
      R.push_back(MBB);
    }
  }

  return R;
}

void AMiLiveIntervalsAnalysis::AddSegment(MachineInstr &MI,
                                          MachineBasicBlock *MBB) {
  if (MBB->instr_begin() != MBB->instr_end()) {
    assert(MI.getOperand(0).isDef() && "TODO");
    LIS->addSegmentToEndOfBlock(MI.getOperand(0).getReg(), *MBB->instr_begin());
  }
}

void AMiLiveIntervalsAnalysis::AddSegment(MachineInstr &MI, MachineInstr &MI2) {
  VNInfo *VNI;
  assert(MI.getOperand(0).isDef() && "TODO");
  LiveInterval &LI = LIS->getInterval(MI.getOperand(0).getReg());
  VNI = LI.getVNInfoAt(LI.beginIndex()); // TODO: Is this correct?
  SlotIndex I = SI->getInstructionIndex(MI2);
  LiveInterval::Segment S(I, I.getNextIndex(), VNI);
  LI.addSegment(S);
}

/// Function to check if a register is used in a basic block with a SlotIndex >
/// GivenIndex
bool AMiLiveIntervalsAnalysis::isRegisterUsedAfterSlotIndex(
    Register Reg, SlotIndex GivenIndex, MachineFunction &MF) {
  MachineRegisterInfo &MRI = MF.getRegInfo();

  // Iterate over all uses of the register
  for (MachineOperand &MO : MRI.reg_nodbg_operands(Reg)) {
    if (!MO.isUse())
      continue; // We only care about uses

    MachineInstr *MI = MO.getParent();
    SlotIndex UseIndex = SI->getInstructionIndex(*MI);

    // Check if the use occurs after the given SlotIndex
    if (UseIndex > GivenIndex) {
      return true;
    }
  }
  return false;
}

#if 0
  LiveInterval &LI = LIS->getInterval(MI.getOperand(0).getReg());
  SlotIndex startIdx = Indexes.getMBBStartIdx(&MBB);
  LI.liveAt(startIdx)
#endif

bool AMiLiveIntervalsAnalysis::runOnMachineFunction(MachineFunction &MF) {
  errs() << "Running AMiLiveIntervalsAnalysis on function: " << MF.getName()
         << "\n";

  SI = &getAnalysis<SlotIndexesWrapperPass>().getSI();
  LIS = &getAnalysis<LiveIntervalsWrapperPass>().getLIS();

  for (MachineBasicBlock &MBB : MF) {
    for (MachineInstr &MI : MBB) {
      if (MI.getFlag(MI.Persistent)) {
        for (MachineBasicBlock *MBB2 : findDisconnectedBlocks(MF, &MBB)) {

          // RULE 1
          if (SI->getMBBStartIdx(MBB2) > SI->getMBBStartIdx(&MBB)) {
            if (isRegisterUsedAfterSlotIndex(MI.getOperand(0).getReg(), SI->getMBBEndIdx(MBB2), MF)) {
            errs() << MI << ": " << MBB2->getName() << "(" << MBB2->getNumber()
                   << ")" << "\n";
            AddSegment(MI, MBB2);
            errs() << LIS->getInterval(MI.getOperand(0).getReg()) << "\n";
            }
          }
        }
      }
    }
  }

  // printMIRWithSlotIndexes(MF);

  return false;
}

void AMiLiveIntervalsAnalysis::getAnalysisUsage(AnalysisUsage &AU) const {
  MachineFunctionPass::getAnalysisUsage(AU);
  AU.addRequired<LiveIntervalsWrapperPass>();
  AU.addRequired<SlotIndexesWrapperPass>();
  AU.setPreservesAll();
}

char AMiLiveIntervalsAnalysis::ID = 0;

static RegisterPass<AMiLiveIntervalsAnalysis>
    X("ami-liveintervals", "AMi Live Interval Analysis", false, false);
