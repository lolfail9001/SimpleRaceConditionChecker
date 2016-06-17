#ifndef _COMMONS_HPP
#define _COMMONS_HPP

#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Rewrite/Core/Rewriter.h>
#include <clang/AST/AST.h>
#include <clang/AST/ASTConsumer.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Frontend/ASTConsumers.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Frontend/CompilerInstance.h>

#include <vector>
#include <algorithm>
#include <string>

namespace fail
{

static llvm::cl::OptionCategory AsyncErrorToolingCategory("Race condition detection");


struct ThreadSetup
{
  std::string threadCreateFunc;
  std::string threadEntryFunc;
  unsigned threadFuncArgNum;
  std::vector<std::string> threadSyncFuncs;
};

struct FullSourceLoc
{
  unsigned line,col;
  FullSourceLoc(clang::FullSourceLoc fsl):line(fsl.getSpellingLineNumber()),
                                          col(fsl.getSpellingColumnNumber())
  {}
  FullSourceLoc(const FullSourceLoc& fsl):line(fsl.line),
                                          col(fsl.col)

  {}

  FullSourceLoc& operator=(const FullSourceLoc& fsl)
  {
    line=fsl.line;
    col=fsl.col;
    return *this;
  }

  bool operator==(const FullSourceLoc& fsl) const
  {
    return line==fsl.line && col==fsl.col;
  }
};

class FunctionDecl
{
 public:
  std::string name;
  std::string type;
  FullSourceLoc fslStart,fslEnd;
  FunctionDecl(clang::FunctionDecl* fd,clang::ASTContext* c):name(fd->getName().str()),type(fd->getType().getCanonicalType().getAsString()),fslStart(FullSourceLoc(c->getFullLoc(fd->getLocStart()))),fslEnd(FullSourceLoc(c->getFullLoc(fd->getLocEnd())))
  {}
  FunctionDecl(const FunctionDecl& other):name(other.name),
                                          type(other.type),
                                          fslStart(other.fslStart),
                                          fslEnd(other.fslEnd)
  {}

  FunctionDecl& operator=(const FunctionDecl& other)
  {
    name=other.name;
    type=other.type;
    fslStart=other.fslStart;
    fslEnd=other.fslEnd;
    return *this;
  }

  inline bool operator==(const FunctionDecl& other) const {return fslStart==other.fslStart&&fslEnd==other.fslEnd;}

};

class VarDecl
{
 public:
  std::string name;
  std::string type;
  FullSourceLoc fslStart,fslEnd;
  VarDecl(clang::VarDecl* vd,clang::ASTContext* c):name(vd->getName().str()),type(vd->getType().getCanonicalType().getAsString()),fslStart(FullSourceLoc(c->getFullLoc(vd->getLocStart()))),fslEnd(FullSourceLoc(c->getFullLoc(vd->getLocEnd())))
  {}
  VarDecl(const VarDecl& other):name(other.name),
                                type(other.type),
                                fslStart(other.fslStart),
                                fslEnd(other.fslEnd){}
  VarDecl& operator=(const VarDecl& other)
  {
    name=other.name;
    type=other.type;
    fslStart=other.fslStart;
    fslEnd=other.fslEnd;
    return *this;
  }
  inline bool operator==(const VarDecl& other) const{return fslStart==other.fslStart&&fslEnd==other.fslEnd;}
};


class VarRef
{
 public:
  enum RefType
  {
    READ,
    WRITE
  };
 public:
  RefType type;
  VarDecl vd;
  FullSourceLoc fslStart,fslEnd;
 public:
  VarRef(clang::DeclRefExpr* dre,RefType rt,clang::ASTContext* c) : type(rt), vd(clang::cast<clang::VarDecl>(dre->getFoundDecl()),c),fslStart(FullSourceLoc(c->getFullLoc(dre->getLocStart()))),fslEnd(FullSourceLoc(c->getFullLoc(dre->getLocEnd())))
  {}
  VarRef(const VarRef& vr): type(vr.type),
                            vd(vr.vd),
                            fslStart(vr.fslStart),
                            fslEnd(vr.fslEnd)
  {}

  VarRef& operator=(const VarRef& other)
  {
    type=other.type;
    vd=other.vd;
    fslStart=other.fslStart;
    fslEnd=other.fslEnd;
    return *this;
  }

  bool operator==(const VarRef& other) const
  {
    return type==other.type && vd==other.vd;
  }
};

struct RefTable
{
  FunctionDecl func;
  VarRef ref;
  RefTable(const FunctionDecl& f,
           const VarRef& vr):func(f),ref(vr)
  {}

  RefTable(const RefTable& rt):func(rt.func),
                               ref(rt.ref)
  {}
};

class FuncSeq
{
 public:
  std::list<VarRef> refs;
  FunctionDecl func;
  std::vector<FunctionDecl> calls;
  bool thread;
  bool tomultiply;
  unsigned depth;
 public:
  FuncSeq(clang::FunctionDecl* fd,clang::ASTContext* c,unsigned d=4):func(fd,c), thread(false), tomultiply(false),depth(d)
  {}

  FuncSeq(const FuncSeq& fs):refs(fs.refs),
                             func(fs.func),
                             calls(fs.calls),
                             thread(fs.thread),
                             tomultiply(false),
                             depth(fs.depth)
  {}

  FuncSeq& operator=(const FuncSeq& other)
  {
    func = other.func;
    calls.clear();
    refs.clear();
    refs.insert(refs.begin(),other.refs.begin(),other.refs.end());
    calls.insert(calls.begin(),other.calls.begin(),other.calls.end());
    thread = other.thread;
    depth = other.depth;
    return *this;
  }

  inline void addCall(clang::FunctionDecl* call,clang::ASTContext* c)
  {
    calls.push_back(FunctionDecl(call,c));
  }
  inline void addCall(const FunctionDecl& fd)
  {
    calls.push_back(fd);
  }

  
  void addRef(const VarRef& vr)
  {
    bool noWrite;
    switch(vr.type)
    {
      case VarRef::READ:
        //Check that we have no write vr
        noWrite=true;
        for(const auto& ref : refs)
        {
          if(ref.type==VarRef::WRITE && ref.vd==vr.vd)
          {noWrite=false; break;}
        }
        if(noWrite)
          refs.push_back(vr);
        break;
      case VarRef::WRITE:
        //Removing all read accesses
        for(const auto& ref: refs)
        {
          if(ref.type==VarRef::READ && ref.vd==vr.vd)
          {
            refs.remove(ref);
            break;
          }
        }
        refs.push_back(vr);
        break;
    }
  }

  inline bool operator==(const FuncSeq& other) const
  {
    return other.func==func;
  }
  inline bool operator==(const FunctionDecl& other) const
  {
    return func==other;
  }

  void mergeWithCalls(const std::vector<FuncSeq*>& funcs)
  {
    mergeWithCalls(funcs,depth);
  }
  
  void mergeWithCalls(const std::vector<FuncSeq*>& funcs,unsigned d)
  {
    std::vector<FunctionDecl>::iterator it;
    if(d==0) return;
    for(const auto f : funcs)
    {
      //Find it in calls
      if((it=std::find(calls.begin(),calls.end(),f->func))!=calls.end())
      {
        //Found!
        calls.erase(it);
        for(const auto& r : f->refs)
          addRef(r);
        for(const auto& c : f->calls)
          calls.push_back(c);
      }
    }
    //May as well
    if(calls.size()>0)
        mergeWithCalls(funcs,d-1);
  }

  //Returns 0 for reads
  //Returns 1 for writes
  //Returns -1 for not found
  int checkReference(const VarDecl& var) const
  {
    for(const auto& vr : refs)
    {
      if(vr.vd==var)
      {
        switch(vr.type)
        {
          case VarRef::READ: return 0;
          case VarRef::WRITE: return 1; 
        }
      }
    }
    return -1;
  }
  
};

struct ParserData
{
  ThreadSetup* threadSetup;
  std::vector<VarDecl>* ourVars;
  std::vector<FuncSeq* >* functions;
  unsigned mergeDepth;
};


}
#endif
