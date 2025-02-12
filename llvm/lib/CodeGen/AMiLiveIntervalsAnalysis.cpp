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

#define DEBUG_TYPE "ami-liveintervals"

static cl::opt<bool> FlagDisable("ami-disable-add-segment", cl::init(false),
                                 cl::Hidden);
static cl::opt<bool> FlagCFG("ami-view-cfg", cl::init(false), cl::Hidden);

AMiLiveIntervalsAnalysis::AMiLiveIntervalsAnalysis()
    : MachineFunctionPass(ID) {}

void AMiLiveIntervalsAnalysis::printMIRWithSlotIndexes(MachineFunction &MF) {
  for (MachineBasicBlock &MBB : MF) {
    LLVM_DEBUG(dbgs() << "MBB #" << MBB.getNumber() << ":\n");

    for (MachineInstr &MI : MBB) {
      if (SI->hasIndex(MI)) {
        LLVM_DEBUG(dbgs() << "  [" << SI->getInstructionIndex(MI) << "] ");
      } else {
        LLVM_DEBUG(dbgs() << "  [???] ");
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
                                          MachineBasicBlock *Range) {
  if (!FlagDisable) {
    if (Range->instr_begin() != Range->instr_end()) {

      assert(MI.getOperand(0).isDef() && "TODO");
      Register R = MI.getOperand(0).getReg();
      LiveInterval &LI = LIS->getInterval(R);
      VNInfo *VNI = LI.getVNInfoAt(SI->getMBBStartIdx(Range));

      LLVM_DEBUG(dbgs() << "Adding segment to %" << R.virtReg2Index(R) << "\n");
      LLVM_DEBUG(dbgs() << " BEFORE: \n");
      LLVM_DEBUG(dbgs() << "  " << LI << "\n");
      // if (!LI.liveAt(Start))
      LI.addSegment(LiveRange::Segment(SI->getMBBStartIdx(Range),
                                       SI->getMBBEndIdx(Range), VNI));
      // LIS->addSegmentToEndOfBlock(R, *Range->instr_begin());
      LLVM_DEBUG(dbgs() << " AFTER: \n");
      LLVM_DEBUG(dbgs() << "  " << LI << "\n");
    }
  }
}

void AMiLiveIntervalsAnalysis::AddSegment(MachineInstr &MI,
                                          MachineInstr &Range) {
  if (!FlagDisable) {
    VNInfo *VNI;
    assert(MI.getOperand(0).isDef() && "TODO");
    Register R = MI.getOperand(0).getReg();
    LiveInterval &LI = LIS->getInterval(R);
    VNI = LI.getVNInfoAt(LI.beginIndex()); // TODO: Is this correct?
    SlotIndex I = SI->getInstructionIndex(Range);
    LiveInterval::Segment S(I, I.getNextIndex(), VNI);
    LLVM_DEBUG(dbgs() << "Adding segment to %" << R.virtReg2Index(R) << "\n");
    LLVM_DEBUG(dbgs() << " BEFORE: \n");
    LLVM_DEBUG(dbgs() << "  " << LI << "\n");
    LI.addSegment(S);
    LLVM_DEBUG(dbgs() << " AFTER: \n");
    LLVM_DEBUG(dbgs() << "  " << LI << "\n");
  }
}

/// Function to check if a register is used in a basic block with a SlotIndex >
/// GivenIndex
bool AMiLiveIntervalsAnalysis::isRegisterUsedAfterSlotIndex(
    Register Reg, SlotIndex GivenIndex) {

  // Iterate over all uses of the register
  for (MachineOperand &MO : MRI->reg_nodbg_operands(Reg)) {
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

bool AMiLiveIntervalsAnalysis::runOnMachineFunction(MachineFunction &MF) {
  LLVM_DEBUG(
      dbgs() << "-------------------------------------------------------\n");
  LLVM_DEBUG(dbgs() << "Running AMiLiveIntervalsAnalysis on function: "
                    << MF.getName() << "\n");
  LLVM_DEBUG(
      dbgs() << "--------------------------------------------------------\n");

  MRI = &MF.getRegInfo();
  SI = &getAnalysis<SlotIndexesWrapperPass>().getSI();
  LIS = &getAnalysis<LiveIntervalsWrapperPass>().getLIS();

  printMIRWithSlotIndexes(MF);

  if (FlagCFG)
    MF.viewCFG();

  LLVM_DEBUG(
      dbgs() << "-------------------------------------------------------\n");

  for (MachineBasicBlock &MBB : MF) {
    for (MachineInstr &pMI : MBB) {
      if (pMI.getFlag(pMI.Persistent)) {
        for (MachineBasicBlock *MBB2 : findDisconnectedBlocks(MF, &MBB)) {

          // CASE 1
          if (SI->getMBBStartIdx(MBB2) > SI->getMBBStartIdx(&MBB)) {

            // CASE 1.1
            if (isRegisterUsedAfterSlotIndex(pMI.getOperand(0).getReg(),
                                             SI->getMBBEndIdx(MBB2))) {
              // TODO: Is this a bug?
              //       A persistent instruction should not be live after any
              //       join point with MBB2 as this would violate program
              //       correctness? It would override the assigned value in the
              //       other path, unless the assignment comes after the
              //       persistent instruction? A new temporary should have been
              //       created by the transformation pass to deal with this
              //       case?
              LLVM_DEBUG(dbgs()
                         << "CASE 2.1: "
                         << "p:" << SI->getInstructionIndex(pMI) << " --> "
                         << "bb:" << MBB2->getNumber() << "\n");
            }

            // CASE 1.2
            for (MachineInstr &MI : *MBB2) {
              for (MachineOperand &MO : MI.all_uses()) {
                // errs() << "Use operand: " << MO.getReg() << "\n";

                Register usedReg = MO.getReg();

                // Check if the register is valid and not a special register
                if (usedReg.isVirtual()) {
                  // Get the instruction that defines the virtual register
                  MachineInstr *definingInst = MRI->getVRegDef(usedReg);
                  if (definingInst) {

                    assert(SI->hasIndex(*definingInst) && "TODO");
                    SlotIndex definingIndex =
                        SI->getInstructionIndex(*definingInst);

                    if (definingIndex < SI->getInstructionIndex(pMI)) {
                      LLVM_DEBUG(
                          dbgs()
                          << "CASE 1.2: "
                          << "def:" << SI->getInstructionIndex(*definingInst)
                          << " --> "
                          << "p:" << SI->getInstructionIndex(pMI) << " --> "
                          << "use:" << SI->getInstructionIndex(MI) << "\n");
                      AddSegment(*definingInst, pMI);
                      // errs() << LIS->getInterval(pMI.getOperand(0).getReg())
                      // << "\n";/a
                    }
                  } else {
                    // TODO
                  }
                } else if (usedReg.isPhysical()) {
                  // Physical registers are trickier because they may be
                  // defined by multiple instructions, especially in low-level
                  // code generation. If you are dealing with physical
                  // registers, you typically need to track modifications
                  // across instructions, which may involve target-specific
                  // logic. For physical registers, we may need to track
                  // through the definition This depends on the target's
                  // constraints, but we can still check if it's a use.

#if 0
                    errs() << MI2;
                    // TODO
                    assert(false && "TODO");
#endif

                  // You can further check for instructions modifying this
                  // physical register.
                }
              }
            }
          }

          // CASE 2
          if (SI->getMBBStartIdx(MBB2) < SI->getMBBStartIdx(&MBB)) {

            // CASE 2.1
            for (MachineInstr &MI : *MBB2) {
              // TODO: generalize "for (MachineOperand &MO : MI.all_defs())"
              //        for simplicity, now assume there is only one def at most
              if (MI.getOperand(0).isReg()) {
                if (MI.getOperand(0).isDef()) {
                  if (isRegisterUsedAfterSlotIndex(MI.getOperand(0).getReg(),
                                                   SI->getMBBEndIdx(&MBB))) {
                    LLVM_DEBUG(dbgs()
                               << "CASE 2.1: "
                               << "def:" << SI->getInstructionIndex(MI)
                               << " --> "
                               << "p:" << SI->getInstructionIndex(pMI) << "\n");

                    {
                      // TODO: No segment to add?
                      LiveInterval &LI =
                          LIS->getInterval(MI.getOperand(0).getReg());
                      SlotIndex Idx = SI->getMBBEndIdx(&MBB);
                      assert(LI.liveAt(Idx) && "TODO");
                    }
                  }
                }
              }
            }

            // CASE 2.2
            // TODO: Refactor: This is mostly copy-paste from CASE 1.2
            for (MachineOperand &MO : pMI.all_uses()) {

              Register usedReg = MO.getReg();

              if (usedReg.isVirtual()) {
                MachineInstr *definingInst = MRI->getVRegDef(usedReg);
                if (definingInst) {
                  assert(SI->hasIndex(*definingInst) && "TODO");
                  SlotIndex definingIndex =
                      SI->getInstructionIndex(*definingInst);

                  if (definingIndex < SI->getMBBStartIdx(MBB2)) {
                    LLVM_DEBUG(dbgs() << "CASE 2.2: "
                                      << "def:"
                                      << SI->getInstructionIndex(*definingInst)
                                      << " --> "
                                      << "bb:" << MBB2->getNumber() << " --> "
                                      << "use: " << SI->getInstructionIndex(pMI)
                                      << "\n");
                    AddSegment(*definingInst, MBB2);
                  } else {
                    // TODO
                  }
                } else if (usedReg.isPhysical()) {
                  // TODO
                }
              }
            }
          }
        }
      }
    }
  }

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
    X(DEBUG_TYPE, "AMi Live Interval Analysis", false, false);
