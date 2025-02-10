#ifndef LLVM_LIB_CODEGEN_AMILIVEINTERVALSANALYSIS_H
#define LLVM_LIB_CODEGEN_AMILIVEINTERVALSANALYSIS_H

#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/SlotIndexes.h"
#include <vector>

namespace llvm {

class AMiLiveIntervalsAnalysis : public MachineFunctionPass {
public:

  static char ID; // Pass identification

  AMiLiveIntervalsAnalysis();

  bool runOnMachineFunction(MachineFunction &MF) override;

  void getAnalysisUsage(AnalysisUsage &AU) const override;

private:
  MachineRegisterInfo *MRI;
  SlotIndexes *SI;
  LiveIntervals *LIS;

  void printMIRWithSlotIndexes(MachineFunction &MF);

  void AddSegment(MachineInstr &Mi, MachineBasicBlock *MBB);

  void AddSegment(MachineInstr &MI, MachineInstr &MI2);

  bool isRegisterUsedAfterSlotIndex(Register Reg, SlotIndex GivenIndex);

  std::vector<MachineBasicBlock *>
  findUnreachableBlocks(MachineFunction &MF, MachineBasicBlock *StartBB);

  std::vector<MachineBasicBlock *>
  findDisconnectedBlocks(MachineFunction &MF, MachineBasicBlock *StartBB);
  };

} // end namespace llvm

#endif // LLVM_LIB_CODEGEN_AMILIVEINTERVALSANALYSIS_H
