#include <MemoryModel/PointerAnalysis.h>
#include <WPA/Andersen.h>
#include <WPA/FlowSensitive.h>
#include "Util/ExtAPI.h"
#include "llvm/ADT/SparseBitVector.h"
#include "SVF-LLVM/SVFIRBuilder.h"

#include <fstream>

#include "AAPass.h"

using namespace llvm;
using namespace SVF;

char SVFAAPass::ID = 0;

SVFAAPass::~SVFAAPass() {
    delete _pta;
}

bool SVFAAPass::isModelingConstants() {
    return modelConstantsIndividually;
}

bool SVFAAPass::runOnModule(llvm::Module& module) {
    runPointerAnalysis(module, type);
    return false;
}

void SVFAAPass::runPointerAnalysis(llvm::Module& module, u32_t kind) {
    // auto svfModule = LLVMModuleSet::getLLVMModuleSet()->buildSVFModule(module);
    auto svfModule = LLVMModuleSet::getLLVMModuleSet()->getSVFModule();
    SVFIRBuilder builder(svfModule);
    SVFIR* _pag = builder.build();

    switch (kind) {
    // case PointerAnalysis::Andersen_WPA:
    //     _pta = new Andersen();
    //     break;
    // case PointerAnalysis::AndersenLCD_WPA:
    //     _pta = new AndersenLCD();
    //     break;
    // case PointerAnalysis::AndersenWave_WPA:
    //     _pta = new AndersenWave();
    //    break;
    case PointerAnalysis::AndersenWaveDiff_WPA:
        _pta = AndersenWaveDiff::createAndersenWaveDiff(_pag);
        break;
    // case PointerAnalysis::AndersenWaveDiffWithType_WPA:
    //     _pta = new AndersenWaveDiffWithType();
    //      break;
    case PointerAnalysis::FSSPARSE_WPA:
        _pta = FlowSensitive::createFSWPA(_pag);
        break;
    default:
        llvm::errs() << "This pointer analysis has not been implemented yet.\n";
        break;
    }


//    _pta->quiet = true;
//    _pta->modelConstants = modelConstantsIndividually;

    //need to keep this around becauase otherwise module will be deleted
    // _pta->analyze();

    PAG* pag = _pta->getPAG();
    // PointsTo memObjects;
    // for(auto& idToType : *pag) {
    //     if(isa<ObjVar>(idToType.second) && !pag->getObject(idToType.first)->isFunction()  )
    //         memObjects.set(idToType.first);
    // }

    // errs() << "All mem objects: ";
    // SVF::dump(memObjects.toNodeBS(), SVFUtil::errs());

    for(auto& idToType : *pag) {
        // if(ObjVar* opn = dyn_cast<ObjVar>(idToType.second)) {
        // if(ObjVar::classof(idToType.second)){
        if(ObjVar* opn = dyn_cast<ObjVar>(idToType.second)) {
          // ObjVar* opn = (ObjVar*)idToType.second;
          // auto* opn = idToType.second;
          unsigned nodeId = idToType.first;
          auto ptsToOrIsPointedTo = _pta->getPts(nodeId); 
//          ptsToOrIsPointedTo |= _pta->getRevPts(nodeId);
//          ptsToOrIsPointedTo &= memObjects;
          if(!ptsToOrIsPointedTo.empty()) {
           errs() << "Mem object " << nodeId << " " << opn->getValueName() << " " << pag->getObject(nodeId)->getValue()->toString();
           SVF::dump(ptsToOrIsPointedTo.toNodeBS(), SVFUtil::errs());
           errs() << "\n";
//            ptsToOrIsPointedTo.set(nodeId);
            auto foundElem = std::find_if(disjointObjects.begin(), disjointObjects.end(),
              [&ptsToOrIsPointedTo](const PointsTo& e)  
                  {return e.intersects(ptsToOrIsPointedTo);});
            if( foundElem == disjointObjects.end()) {
                disjointObjects.push_front(ptsToOrIsPointedTo);
            } else {
                *foundElem |= ptsToOrIsPointedTo;
            }
          }
          else {
            // errs() << "Empty\n";
          }
        }
    }
    
    //make all uniqueIds;
    for(auto dob = disjointObjects.begin();  dob != disjointObjects.end(); dob++) {
      for(auto nid : *dob) {
          if(pag->getObject(nid) != nullptr) {
              dob->set(pag->getObject(nid)->getId());
          }
      }
    }
    int changes = 0;
    do {
      changes = 0;
      for(auto it = disjointObjects.begin(); it != disjointObjects.end(); it++) {
            PointsTo &dob = *it;
            for(auto it1  = disjointObjects.begin(); it1 != disjointObjects.end(); it1++) {
                if(it != it1 && it->intersects(*it1)) {
                    dob |= *it1;
                    it1 = disjointObjects.erase(it1);
                    changes++;
                }
            }
            if(changes > 0) break;
      }
      errs() << "Completed loop with " << changes << " changes\n";
    } while(changes > 0);

    //don't need this if I don't filter by memory objects
    //for(auto& dob : disjointObjects) {
    //    memObjects = memObjects - dob;
    //}
    //for(auto nid: memObjects) {
    //    PointsTo *pt = new PointsTo();
    //    pt->set(nid);
    //    disjointObjects.push_front(*pt);
    //}
#define PRINT_OBJS
    auto numObjs = 0;
    for(auto& dob : disjointObjects) {
      SVF::dump(dob.toNodeBS(), SVFUtil::errs());
      numObjs += dob.count();
#ifdef PRINT_OBJS      
      for(auto nid : dob) {
          if(nid == 0 || pag->getObject(nid) == nullptr ||  pag->getObject(nid)->getValue() == nullptr) continue;
        errs() << nid << " -> "  << pag->getObject(nid)->getValue() << " tainted: " << pag->getObject(nid)->getId() << " is FIObjVar: " << isa<FIObjVar>(pag->getGNode(nid)) << " is: ";
        auto* llvmVallue = LLVMModuleSet::getLLVMModuleSet()->getLLVMValue(pag->getObject(nid)->getValue());
        if(auto* I = dyn_cast<Instruction>(llvmVallue)) {
          errs() << " in " << (I->getFunction()->getName());
        }
        // if(!isa<SVFFunction>(*pag->getObject(nid)->getValue()))
        errs() << pag->getObject(nid)->getValue()->toString() << "\n";
        // else errs() << "\n";
      }
#endif
    }
    errs() << "Number of dijoint objects: " << disjointObjects.size() << "\n";
    errs() << "Number of objects: " << numObjs << "\n";


}

int SVFAAPass::getMaxGroupedObjects() {
    return disjointObjects.size();
}
void SVFAAPass::printsPtsTo(const llvm::Value* V) {
  assert(V && "Can't print null ptrs");
  PAG* pag = _pta->getPAG();
  SVFValue* svfval = LLVMModuleSet::getLLVMModuleSet()->getSVFValue(V);
  NodeID node = pag->getValueNode(svfval);
  auto ptsTo = _pta->getPts(node);
//  ptsTo |= _pta->getRevPts(node);
  for(auto nid: ptsTo) {
    if(nid == 0 || pag->getObject(nid) == nullptr ||  pag->getObject(nid)->getValue() == nullptr) continue;
    errs() << "node: " << nid  << " " << pag->getObject(nid)->getValue() << " -> ";
//    pag->getObject(nid)->getRefVal()->dump();
  }
  errs() << "node: " << node << " -> ";
//  llvm::dump(ptsTo, errs());
}

std::string kindTostring(GenericNode<PAGNode, PAGEdge>::GNodeK kind) {
  switch(kind) {
    case PAGNode::PNODEK::ValNode: return "ValNode";
    case PAGNode::PNODEK::ObjNode: return "ObjNode";
    case PAGNode::PNODEK::RetNode: return "RetNode";
    case PAGNode::PNODEK::VarargNode: return "VarargNode";
    case PAGNode::PNODEK::GepValNode: return "GepValNode";
    case PAGNode::PNODEK::GepObjNode: return "GepObjNode";
    case PAGNode::PNODEK::FIObjNode: return "FIOBJNode";
    case PAGNode::PNODEK::DummyValNode: return "DummyValNode";
    case PAGNode::PNODEK::DummyObjNode: return "DummyObjNode";
    default: return " UNKOWN!!! ";
  }
}