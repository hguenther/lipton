/*
 * LiptonPass.h
 *
 *  Created on: Feb 26, 2015
 *      Author: laarman
 */

#ifndef LIPTONBIN_LLVM_LIPTONPASS_H_
#define LIPTONBIN_LLVM_LIPTONPASS_H_

#include <list>
#include <string>
#include <vector>

#include "llvm/ReachPass.h"

#include <llvm/Pass.h>
#include <llvm/IR/Instruction.h>
#include <llvm/Analysis/CallGraph.h>
#include <llvm/Analysis/AliasAnalysis.h>
#include <llvm/Analysis/AliasSetTracker.h>
#include <llvm/ADT/DenseMap.h>
#include <llvm/ADT/DenseSet.h>


using namespace llvm;

namespace VVT {

enum yield_loc_e {
    YIELD_BEFORE,
    YIELD_AFTER
};

enum mover_e {
    RightMover = 0,
    NoneMover,
    LeftMover,
    BothMover,
};

class LiptonPass : public ModulePass {

public:
    static char         ID;
    string              Name;
    bool                verbose;
    int                 Block = 1;

    LiptonPass();
    LiptonPass(ReachPass &RP, string name, bool v);

    struct Processor {
        LiptonPass                 *Pass;
        Function                   *ThreadF = NULL;
        Processor(LiptonPass *L, StringRef action) : Pass(L) {  }
        virtual ~Processor() {}
        virtual Instruction *process (Instruction *I)
                                     { return nullptr; }
        virtual bool yield (CallInst *call) { return false; }
        virtual void thread (Function *F) {}
        virtual bool block (BasicBlock &B) { return false; }
        virtual void deblock (BasicBlock &B) {  }
    };


    DenseMap<AliasSet *, list<Instruction *>>   AS2I;
    DenseMap<Instruction *, pair<mover_e, int>> Movers;
    DenseMap<Function *, AliasSetTracker *>     ThreadAliases;
    DenseMap<Function *, vector<Instruction *>> Instructions;
    Function                       *Yield = nullptr;
    Function                       *Act = nullptr;
    AliasAnalysis                  *AA = nullptr;
    ReachPass                      *Reach = nullptr;
    GlobalVariable                 *Phase = nullptr;

    bool isYieldCall (Instruction *I);
    void insertYield (Instruction *I, yield_loc_e loc);
    mover_e movable(Instruction *I);

private:
    Processor                      *handle = nullptr;

    void doYield (Instruction *I, int b);
    // getAnalysisUsage - This pass requires the CallGraph.
    virtual void getAnalysisUsage(AnalysisUsage &AU) const;
    bool runOnModule (Module &M);

    void walkGraph ( Instruction *I );
    void walkGraph ( BasicBlock &B );
    void walkGraph ( Function &F );
    template <typename ProcessorT>
    void walkGraph ();
};


}

#endif /* LIPTONBIN_LLVM_LIPTONPASS_H_ */
