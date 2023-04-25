#include "MemoryModel/PointerAnalysis.h"
#include "SVF-LLVM/LLVMUtil.h"

using namespace SVF;

class AAPass {
protected:
  enum PassType {
      Dummy,
      SVF,
      SVFDelete,
      PreRun,
      Manual
    };
private:
  const PassType Kind;
public:
  AAPass(PassType pt): Kind(pt) {}
  PassType getKind() const { return Kind; }
  virtual int getMaxGroupedObjects() = 0;
  virtual void printsPtsTo(const llvm::Value* V) = 0;
  virtual bool isModelingConstants() = 0;
  virtual bool runOnModule(llvm::Module& module) = 0;
  virtual ~AAPass() {};
};

class SVFAAPass : public llvm::ModulePass,  public AAPass {

public:
  static char ID;

  enum AliasCheckRule {
    Conservative, ///< return MayAlias if any pta says alias
    Veto,         ///< return NoAlias if any pta says no alias
    Precise       ///< return alias result by the most precise pta
  };

  SVFAAPass(): SVFAAPass(false) {}

  SVFAAPass(bool modelConstants)
      : llvm::ModulePass(ID), AAPass(SVF), modelConstantsIndividually(modelConstants),
        type(PointerAnalysis::Default_PTA), _pta(0) {}

  ~SVFAAPass();


  static bool classof(const AAPass* aa) {return aa->getKind() == SVF;}
  virtual inline void *getAdjustedAnalysisPointer(llvm::AnalysisID id) {
    return this;
  }

  virtual bool runOnModule(llvm::Module &module);

  virtual inline llvm::StringRef getPassName() const { return "AAPass"; }

  void setPAType(PointerAnalysis::PTATY type) { this->type = type; }

  BVDataPTAImpl *getPTA() { return _pta; }
  //void getPointsTo(const llvm::Value* V);
  int getMaxGroupedObjects();
  void printsPtsTo(const llvm::Value* V);
  bool isModelingConstants();

private:
  bool modelConstantsIndividually;
  void runPointerAnalysis(llvm::Module &module, u32_t kind);
  std::list<PointsTo> disjointObjects;

  PointerAnalysis::PTATY type;
  BVDataPTAImpl *_pta;
};

class SVFDeletingAAPass : public llvm::ModulePass,  public AAPass {

public:
  static char ID;

  SVFDeletingAAPass(): SVFDeletingAAPass(false) {}

  SVFDeletingAAPass(bool modelConstants)
      : llvm::ModulePass(ID), AAPass(SVFDelete), modelConstantsIndividually(modelConstants),
        type(PointerAnalysis::Default_PTA), _pta(0) {}

  ~SVFDeletingAAPass() {}


  static bool classof(const AAPass* aa) {return aa->getKind() == SVFDelete;}
  virtual inline void *getAdjustedAnalysisPointer(llvm::AnalysisID id) {
    return this;
  }
  virtual bool runOnModule(llvm::Module &module);
  virtual inline llvm::StringRef getPassName() const { return "AAPass cleans up SVF"; }
  void setPAType(PointerAnalysis::PTATY type) { this->type = type; }

  int getMaxGroupedObjects();
  void printsPtsTo(const llvm::Value* V);
  bool isModelingConstants();

private:
  int valToGroup(const llvm::Value* V);
  bool modelConstantsIndividually;
  void runPointerAnalysis(llvm::Module &module, u32_t kind);
  std::list<PointsTo> disjointObjects;
  std::unordered_map<const llvm::Value*, int> valueToGroup;

  PointerAnalysis::PTATY type;
  BVDataPTAImpl *_pta;
};
