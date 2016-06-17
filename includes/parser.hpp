#ifndef _PARSER_HPP
#define _PARSER_HPP


#include "commons.hpp"

namespace fail
{

class ThreadASTVisitor : public clang::RecursiveASTVisitor<ThreadASTVisitor> {
 private:
  clang::ASTContext* cont;
  ParserData* pd;
  clang::FunctionDecl* cf;
  std::vector<FunctionDecl>* threads;
  std::vector<bool>* multiples;
  bool ignore;
  inline bool isWriteOperator(clang::BinaryOperator bo) {
    
    if(bo.isAssignmentOp() ||
       bo.isCompoundAssignmentOp() ||
       bo.isShiftAssignOp())
      return true;
    else
      return false;
  }
 public:
  ThreadASTVisitor(clang::ASTContext* c,
                   ParserData* ps): cont(c),
                                    pd(ps),
                                    cf(nullptr),
                                    threads(new std::vector<FunctionDecl>()),
                                    multiples(new std::vector<bool>())
  {}
  virtual ~ThreadASTVisitor()
  {
    delete threads;
    delete multiples;
    pd=nullptr;
    cf=nullptr;
    cont=nullptr;
    #ifndef NDEBUG
    llvm::outs()<<"Cleaned up visitor\n";
    #endif
  }
  bool VisitFunctionDecl(clang::FunctionDecl* fd);
  bool VisitVarDecl(clang::VarDecl* vd);
  bool VisitDeclRefExpr(clang::DeclRefExpr* de);
  bool VisitCallExpr(clang::CallExpr* ce);
  std::vector<FunctionDecl>* getThreadFuncs() const
  {
    return threads;
  }
  std::vector<bool>* getMultipleThreads() const
  {
    return multiples;
  }
};

class ThreadASTConsumer : public clang::ASTConsumer {
 private:
  ParserData* pd;
  ThreadASTVisitor tastVisitor;
 public:
  ThreadASTConsumer(clang::ASTContext* c,
                    ParserData* ps):pd(ps),
                                    tastVisitor(c,ps)
  {
    #ifndef NDEBUG
    llvm::outs()<<"\nCreating consumer\n";
    #endif
  }
                                    
  virtual void HandleTranslationUnit(clang::ASTContext& ctx);

  virtual ~ThreadASTConsumer()
  {
    #ifndef NDEBUG
    llvm::outs()<<"Cleaning up parser\n";
    #endif
    pd=nullptr;
    clang::ASTConsumer::~ASTConsumer();
    #ifndef NDEBUG
    llvm::outs()<<"Cleaned up!\n";
    #endif
  }
};

class ThreadFrontendAction : public clang::ASTFrontendAction {
 public:
  ThreadFrontendAction(ParserData* ps):pd(ps)
  {}
  std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
      clang::CompilerInstance& CI,llvm::StringRef inFile) {
    #ifndef NDEBUG
    llvm::outs()<<"Creating consumer\n";
    #endif
    return std::unique_ptr<clang::ASTConsumer>
        (new ThreadASTConsumer(&CI.getASTContext(),
                               pd));
  }
  virtual ~ThreadFrontendAction()
  {
    #ifndef NDEBUG
    llvm::outs()<<"Clearing frontend action\n";
    #endif
    pd=nullptr;
    clang::ASTFrontendAction::~ASTFrontendAction();
  }
 private:
  ParserData* pd;
};


class Parser
{
 private:
  ParserData data;
 public:
  Parser(const std::string& file,ThreadSetup* ts,unsigned mergeDepth=4);
  virtual ~Parser();
  inline ParserData* getParserDataPtr() {return &data;}
};

}

#endif
