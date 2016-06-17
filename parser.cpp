#include "parser.hpp"

#include <stdexcept>
#include <exception>

namespace fail
{
bool ThreadASTVisitor::VisitFunctionDecl(clang::FunctionDecl* fd)
{
  if(fd->hasBody())
  {
#ifndef NDEBUG
    llvm::outs()<<"Entering function"<<fd->getNameInfo().getAsString()<<"\n";
#endif
    pd->functions->push_back(new FuncSeq(fd,cont,pd->mergeDepth));
    cf=fd;
    ignore=false;
    std::string funcName=fd->getName().str();
    if(funcName==pd->threadSetup->threadEntryFunc)
    {
      threads->push_back(FunctionDecl(fd,cont));
      multiples->push_back(false);
    }
  }
  return true;
}

bool ThreadASTVisitor::VisitVarDecl(clang::VarDecl* vd)
{
  if(!vd->isLocalVarDeclOrParm())
  {
#ifndef NDEBUG
    llvm::outs()<<"Entering var "<<vd->getNameAsString()<<" declaration\n";
#endif
    pd->ourVars->push_back(VarDecl(vd,cont));
  }
  return true;
}

bool ThreadASTVisitor::VisitDeclRefExpr(clang::DeclRefExpr* de)
{
  //Oh boy
  //We will only check for var references over here, okay?
  if(!ignore&&clang::isa<clang::VarDecl>(de->getFoundDecl()))
  {
#ifndef NDEBUG
      llvm::outs()<<"Entering "<<de->getFoundDecl()->getNameAsString()<<" reference\n";
#endif
#ifdef OPTIMIZE
      if(clang::cast<clang::VarDecl>(de->getFoundDecl())->hasGlobalStorage()) {
#endif //OPTIMIZE
      //Going through parents to figure out whether read or write
      auto parents = cont->getParents(*de);
      bool read=!de->isLValue();
      for(auto p=parents.begin();p!=parents.end();++p)
      {
        if(clang::isa<clang::Expr>(*p->get<clang::Stmt>()))
        {
          if(clang::cast<clang::Expr>(*p->get<clang::Stmt>()).isLValue())
            read=false;
          if(clang::isa<clang::BinaryOperator>(*p->get<clang::Stmt>()) && isWriteOperator(clang::cast<clang::BinaryOperator>
                               (*p->get<clang::Stmt>())))
          {
            break;
          }
          read=true;
        }
      }
      //Figuring out the function now
      for(auto it=pd->functions->begin();it!=pd->functions->end();++it)
      {
        if((*it)->func==FunctionDecl(cf,cont))
        {
          (*it)->addRef(VarRef(de,read?VarRef::READ:VarRef::WRITE,cont));
          break;
        }
      }
#ifdef OPTIMIZE
    }          
#endif // OPTIMIZE
  }
  return true;
}

bool ThreadASTVisitor::VisitCallExpr(clang::CallExpr* ce)
{
  //Jeee...zy
  //First and foremost, check for builtins
  clang::FunctionDecl* callee = ce->getDirectCallee();
  std::string calleeName=callee->getNameInfo().getAsString();
#ifndef NDEBUG
  llvm::outs()<<"Entering "<<calleeName<<" call\n";
#endif
  if(calleeName==pd->threadSetup->threadCreateFunc)
  {
    //So, we are dealing with thread call
    //First and foremost, we are figuring out FunctionDecl of what we're calling
    clang::FunctionDecl* tfd = clang::cast<clang::FunctionDecl>
                               (clang::cast<clang::DeclRefExpr>
                                (ce->getArg
                                 (pd->threadSetup->threadFuncArgNum)->IgnoreImplicit())
                                        ->getFoundDecl());
    threads->push_back(FunctionDecl(tfd,cont));
    bool multiple=false;
    //Digging up
    auto parents = cont->getParents(*ce);
    for(auto p=parents.begin();p!=parents.end();++p)
    {
      if(clang::isa<clang::ForStmt>(*p->get<clang::Stmt>())||
         clang::isa<clang::DoStmt>(*p->get<clang::Stmt>())||
         clang::isa<clang::WhileStmt>(*p->get<clang::Stmt>()))
      {
        multiple=true;
        break;
      }
    }
    multiples->push_back(multiple);
  }
  else if(std::find(pd->threadSetup->threadSyncFuncs.begin(),pd->threadSetup->threadSyncFuncs.end(),calleeName)!=pd->threadSetup->threadSyncFuncs.end())
  {
    ignore=true;
  }
  else {
    //Figuring out the function now
      for(auto it=pd->functions->begin();it!=pd->functions->end();++it)
      {
        if((*it)->func==FunctionDecl(cf,cont))
        {
          (*it)->addCall(callee,cont);
          break;
        }
      }
      
  }
  return true;
}

void ThreadASTConsumer::HandleTranslationUnit(clang::ASTContext& ctx)
{
  #ifndef NDEBUG
  llvm::outs()<<"Entering AST\n";
  #endif
  tastVisitor.TraverseDecl(ctx.getTranslationUnitDecl());
  #ifndef NDEBUG
  llvm::outs()<<"Done with AST\n";
  #endif
  //After that we can now get down to post-process
  std::vector<FunctionDecl>* threads=tastVisitor.getThreadFuncs();
  std::vector<bool>* multiples = tastVisitor.getMultipleThreads();
  //Stage 1
  #ifndef NDEBUG
  llvm::outs()<<"Stage 1 post AST parsing\n";
  #endif
  auto threadsIt=threads->begin();
  auto multiplesIt=multiples->begin();
  while(threadsIt!=threads->end() &&
        multiplesIt!=multiples->end())
  {
    auto funcIt = pd->functions->begin();
    while(funcIt != pd->functions->end())
    {
      if(*(*funcIt)==*threadsIt)
        break;
      ++funcIt;
    }
    if(funcIt!=pd->functions->end())
    {
      (*funcIt)->thread=true;
      (*funcIt)->mergeWithCalls(*(pd->functions));
      (*funcIt)->tomultiply=*multiplesIt;
    }
    ++threadsIt;
    ++multiplesIt;
  }
  //Stage 2, we remove all non-threads
  #ifndef NDEBUG
  llvm::outs()<<"Stage 2 post AST parsing\n";
  #endif
  for(auto it=pd->functions->begin();it!=pd->functions->end();++it)
  {
    if(!(*it)->thread)
    {
      delete (*it);
      (*it)=nullptr;
    }
  }
  pd->functions->erase(std::remove(pd->functions->begin(),pd->functions->end(),nullptr),pd->functions->end());
  //Stage 3, where we fill up doubled threads
  #ifndef NDEBUG
  llvm::outs()<<"Stage 3 post AST parsing\n";
  #endif
  //Rewriting for no iterators because this shit is !@#$!
  for(auto it=0;it<pd->functions->size();++it)
  {
    if((pd->functions->at(it))!=nullptr && (pd->functions->at(it))->tomultiply)
    {
      pd->functions->push_back(new FuncSeq(*pd->functions->at(it)));
      (pd->functions->at(it))->tomultiply=false;
    }
  }
  #ifndef NDEBUG
  llvm::outs()<<"Done with parsing\n";
  #endif
}


Parser::Parser(const std::string &file,ThreadSetup* ts,unsigned mergeD)
{
  //Run a tool invocation and set up a damn parser
  data.threadSetup=ts;
  data.ourVars = new std::vector<VarDecl >();
  data.functions = new std::vector<FuncSeq*>();
  data.mergeDepth = mergeD;
  #ifndef NDEBUG
  llvm::outs()<<"Setup data\n";
  #endif
  std::vector<std::string> cmd {"clang","-fsyntax-only","-I/usr/lib64/clang/3.7.0/include","-v"};
  cmd.push_back(file);
  #ifndef NDEBUG
  llvm::outs()<<"Setup cmd\n";
  #endif
  for(const auto& s: cmd)
    llvm::outs()<<s<<" ";
  clang::tooling::ToolInvocation tool
      (cmd,new ThreadFrontendAction(&data),
       new clang::FileManager(clang::FileSystemOptions()));
  #ifndef NDEBUG
  llvm::outs()<<"\nSetup tool\n";
  #endif
  tool.run();
}



Parser::~Parser()
{
  delete data.ourVars;
  delete data.functions;
}
}
