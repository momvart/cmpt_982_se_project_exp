//===- svf-ex.cpp -- A driver example of SVF-------------------------------------//
//
//                     SVF: Static Value-Flow Analysis
//
// Copyright (C) <2013->  <Yulei Sui>
//

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
//===-----------------------------------------------------------------------===//

/*
 // A driver program of SVF including usages of SVF APIs
 //
 // Author: Yulei Sui,
 */

#include "SVF-LLVM/LLVMUtil.h"
#include "Graphs/SVFG.h"
#include "WPA/Andersen.h"
#include "SVF-LLVM/SVFIRBuilder.h"
#include "Util/Options.h"
#include "AAPass.h"

using namespace llvm;
using namespace std;
using namespace SVF;

static llvm::cl::opt<std::string> InputFilename(cl::Positional,
                                                llvm::cl::desc("<input bitcode>"), llvm::cl::init("-"));

/*!
 * An example to query alias results of two LLVM values
 */
SVF::AliasResult aliasQuery(PointerAnalysis *pta, Value *v1, Value *v2)
{
    SVFValue *val1 = LLVMModuleSet::getLLVMModuleSet()->getSVFValue(v1);
    SVFValue *val2 = LLVMModuleSet::getLLVMModuleSet()->getSVFValue(v2);

    return pta->alias(val1, val2);
}

/*!
 * An example to print points-to set of an LLVM value
 */
std::string printPts(PointerAnalysis *pta, Value *val)
{

    std::string str;
    raw_string_ostream rawstr(str);
    SVFValue *svfval = LLVMModuleSet::getLLVMModuleSet()->getSVFValue(val);

    NodeID pNodeId = pta->getPAG()->getValueNode(svfval);
    const PointsTo &pts = pta->getPts(pNodeId);
    for (PointsTo::iterator ii = pts.begin(), ie = pts.end();
         ii != ie; ii++)
    {
        rawstr << " " << *ii << " ";
        PAGNode *targetObj = pta->getPAG()->getGNode(*ii);
        if (targetObj->hasValue())
        {
            rawstr << "(" << targetObj->getValue()->toString() << ")\t ";
        }
    }

    return rawstr.str();
}

/*!
 * An example to query/collect all successor nodes from a ICFGNode (iNode) along control-flow graph (ICFG)
 */
void traverseOnICFG(ICFG *icfg, const Instruction *inst)
{
    SVFInstruction *svfinst = LLVMModuleSet::getLLVMModuleSet()->getSVFInstruction(inst);

    ICFGNode *iNode = icfg->getICFGNode(svfinst);
    FIFOWorkList<const ICFGNode *> worklist;
    Set<const ICFGNode *> visited;
    worklist.push(iNode);

    /// Traverse along VFG
    while (!worklist.empty())
    {
        const ICFGNode *iNode = worklist.pop();
        for (ICFGNode::const_iterator it = iNode->OutEdgeBegin(), eit =
                                                                      iNode->OutEdgeEnd();
             it != eit; ++it)
        {
            ICFGEdge *edge = *it;
            ICFGNode *succNode = edge->getDstNode();
            if (visited.find(succNode) == visited.end())
            {
                visited.insert(succNode);
                worklist.push(succNode);
            }
        }
    }
}

/*!
 * An example to query/collect all the uses of a definition of a value along value-flow graph (VFG)
 */
void traverseOnVFG(const SVFG *vfg, Value *val)
{
    SVFIR *pag = SVFIR::getPAG();
    SVFValue *svfval = LLVMModuleSet::getLLVMModuleSet()->getSVFValue(val);

    PAGNode *pNode = pag->getGNode(pag->getValueNode(svfval));
    const VFGNode *vNode = vfg->getDefSVFGNode(pNode);
    FIFOWorkList<const VFGNode *> worklist;
    Set<const VFGNode *> visited;
    worklist.push(vNode);

    /// Traverse along VFG
    while (!worklist.empty())
    {
        const VFGNode *vNode = worklist.pop();
        for (VFGNode::const_iterator it = vNode->OutEdgeBegin(), eit =
                                                                     vNode->OutEdgeEnd();
             it != eit; ++it)
        {
            VFGEdge *edge = *it;
            VFGNode *succNode = edge->getDstNode();
            if (visited.find(succNode) == visited.end())
            {
                visited.insert(succNode);
                worklist.push(succNode);
            }
        }
    }

    /// Collect all LLVM Values
    for (Set<const VFGNode *>::const_iterator it = visited.begin(), eit = visited.end(); it != eit; ++it)
    {
        const VFGNode *node = *it;
        /// can only query VFGNode involving top-level pointers (starting with % or @ in LLVM IR)
        /// PAGNode* pNode = vfg->getLHSTopLevPtr(node);
        /// Value* val = pNode->getValue();
    }
}

int main(int argc, char **argv)
{
    int arg_num = 0;
    char **arg_value = new char *[argc];
    std::vector<std::string> moduleNameVec;
    LLVMUtil::processArguments(argc, argv, arg_num, arg_value, moduleNameVec);
    cl::ParseCommandLineOptions(arg_num, arg_value,
                                "Whole Program Points-to Analysis\n");

    if (Options::WriteAnder() == "ir_annotator")
    {
        LLVMModuleSet::getLLVMModuleSet()->preProcessBCs(moduleNameVec);
    }

    SVFModule *svfModule = LLVMModuleSet::getLLVMModuleSet()->buildSVFModule(moduleNameVec);

    /// Build Program Assignment Graph (SVFIR)
    SVFIRBuilder builder(svfModule);
    SVFIR *pag = builder.build();

    /// Create Andersen's pointer analysis
    Andersen* ander = AndersenWaveDiff::createAndersenWaveDiff(pag);
    // auto *ander = FlowSensitive::createFSWPA(pag);

    // /// Query aliases
    // /// aliasQuery(ander,value1,value2);

    // Print points-to information
    auto* M = LLVMModuleSet::getLLVMModuleSet()->getMainLLVMModule();
    for (auto F = M->begin(); F != M->end(); F++)
    {
        outs() << F->getName() << "\n";
        for (auto BB = F->begin(); BB != F->end(); BB++)
        {
            for (auto I = BB->begin(); I != BB->end(); I++)
            {
                outs() << *I << "\n";
                outs() << printPts(ander, &*I) << "\n";
                outs() << "Done\n";
            }
        }
    }

    /*
    /// Call Graph
    PTACallGraph* callgraph = ander->getPTACallGraph();

    /// ICFG
    ICFG* icfg = pag->getICFG();

    /// Value-Flow Graph (VFG)
    VFG* vfg = new VFG(callgraph);

    /// Sparse value-flow graph (SVFG)
    SVFGBuilder svfBuilder;
    SVFG* svfg = svfBuilder.buildFullSVFG(ander);

    /// Collect uses of an LLVM Value
    /// traverseOnVFG(svfg, value);

    /// Collect all successor nodes on ICFG
    /// traverseOnICFG(icfg, value);


    // clean up memory
    delete vfg;
    */

    /*
     // auto SVFaa = new SVFDeletingAAPass();
     auto SVFaa = new SVFAAPass(false);
     SVFaa->setPAType(PointerAnalysis::AndersenWaveDiff_WPA);
    //  SVFaa->setPAType(PointerAnalysis::FSSPARSE_WPA);
     SVFaa->runOnModule(*SVF::LLVMModuleSet::getLLVMModuleSet()->getMainLLVMModule());
    */

    // AndersenWaveDiff::releaseAndersenWaveDiff();
    SVFIR::releaseSVFIR();

    LLVMModuleSet::getLLVMModuleSet()->dumpModulesToFile(".svf.bc");
    SVF::LLVMModuleSet::releaseLLVMModuleSet();

    llvm::llvm_shutdown();
    return 0;
}