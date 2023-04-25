#include <MemoryModel/PointerAnalysis.h>
#include <WPA/Andersen.h>
#include <WPA/FlowSensitive.h>
#include "Util/ExtAPI.h"
#include "llvm/ADT/SparseBitVector.h"
#include "SVF-LLVM/SVFIRBuilder.h"

#include "AAPass.h"

#include <fstream>

using namespace llvm;

char SVFDeletingAAPass::ID = 0;


bool SVFDeletingAAPass::isModelingConstants() {
    return modelConstantsIndividually;
}

bool SVFDeletingAAPass::runOnModule(llvm::Module& module) {
    runPointerAnalysis(module, type);
    
    for(const auto& F : module) {
        for(const auto& BB : F) {
            for(const auto& I : BB) {
                auto CI = dyn_cast<llvm::CallInst>(&I);
                llvm::StringRef callName = "";
                if(CI && CI->getCalledFunction())
                    callName = CI->getCalledFunction()->getName();
                
                if(   callName == "malloc"
                   || callName == "realloc"
                   || callName == "calloc"
                   || isa<llvm::AllocaInst>(I)  
                    ) {
                    if(int group = valToGroup(&I))
                      valueToGroup[&I] = group;
                }
            }
        }
    }
    auto pag = _pta->getPAG();
//    delete _pta->getPAG();
    errs() << "Deleting pta\n";
    delete _pta;
    errs() << "Deleting PAG\n";
    errs()  << pag << "\n";
    // PAG::releasePAG();
    PAG::releaseSVFIR();
    return false;
}

void SVFDeletingAAPass::runPointerAnalysis(llvm::Module& module, u32_t kind) {
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
        // break;
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
    PointsTo memObjects;
    for(auto& idToType : *pag) {
        if(isa<ObjVar>(idToType.second) && !pag->getObject(idToType.first)->isFunction()  )
            memObjects.set(idToType.first);
    }
    //errs() << "All mem objects: ";
    //llvm::dump(memObjects, errs());

    for(auto& idToType : *pag) {
        if(ObjVar* opn = dyn_cast<ObjVar>(idToType.second)) {
          unsigned nodeId = idToType.first;
          auto ptsToOrIsPointedTo = _pta->getPts(nodeId); 
//          ptsToOrIsPointedTo |= _pta->getRevPts(nodeId);
//          ptsToOrIsPointedTo &= memObjects;
          if(!ptsToOrIsPointedTo.empty()) {
//            errs() << "Mem object " << nodeId << " " << opn->getValueName() << " same group as ";
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
//#define PRINT_OBJS
    auto numObjs = 0;
    for(auto& dob : disjointObjects) {
    //   llvm::dump(dob, errs());
      numObjs += dob.count();
#ifdef PRINT_OBJS      
      for(auto nid : dob) {
          if(nid == 0 || pag->getObject(nid) == nullptr ||  pag->getObject(nid)->getValue() == nullptr) continue;
        errs() << nid << " -> "  << pag->getObject(nid)->getValue() << " tainted: " << pag->getObject(nid)->getId() << " is FIObjVar: " << isa<FIObjVar>(pag->getGNode(nid)) << " is: ";
        if(!isa<SVFFunction>(pag->getObject(nid)->getValue()))
        errs() << pag->getObject(nid)->getValue()->toString() << "\n";
        else errs() << "\n";
      }
#endif
    }
    errs() << "Number of dijoint objects: " << disjointObjects.size() << "\n";
    errs() << "Number of objects: " << numObjs << "\n";


}

int SVFDeletingAAPass::getMaxGroupedObjects() {
    return disjointObjects.size();
}
void SVFDeletingAAPass::printsPtsTo(const llvm::Value* V) {
    errs() << "Can't print pts to in SVFAADeleting Pass\n";
}

int SVFDeletingAAPass::valToGroup(const llvm::Value* V) {

    if(V == nullptr) return 0;
    SVFValue* svfval = LLVMModuleSet::getLLVMModuleSet()->getSVFValue(V);
    PAG* pag = _pta->getPAG();
    if(!pag->hasValueNode(svfval)) return 0;
    NodeID node = pag->getValueNode(svfval);
    auto ptsTo = _pta->getPts(node);
    int resultingGroup = 1;

    for(auto& pts : disjointObjects) {
        assert(pts.contains(ptsTo) == pts.intersects(ptsTo) && "There shouldn't be more elements in ptsTo");
        if(pts.contains(ptsTo)) return resultingGroup;
        resultingGroup++;
        
    }

    return 0;
}
