#ifndef LLVM_LIB_CODEGEN_AMILIVEINTERVALSANALYSIS_H
#define LLVM_LIB_CODEGEN_AMILIVEINTERVALSANALYSIS_H

#include "llvm/CodeGen/MachineFunctionPass.h"

namespace llvm {

class AMiLiveIntervalsAnalysis : public MachineFunctionPass {
public:

  static char ID; // Pass identification

  AMiLiveIntervalsAnalysis();

  bool runOnMachineFunction(MachineFunction &MF) override;

  void getAnalysisUsage(AnalysisUsage &AU) const override;
};

} // end namespace llvm

#endif // LLVM_LIB_CODEGEN_AMILIVEINTERVALSANALYSIS_H
