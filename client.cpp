#include "parser.hpp"
#include "analytics.hpp"

#include <stdexcept>
#include <exception>
#include <iostream>
#include <ctime>

int main(int argc,char** argv)
{
  if(argc>1)
  {
    std::cout<<"Prep!\n";
    std::string file(argv[1]);
    fail::ThreadSetup ts;
    ts.threadCreateFunc = "pthread_create";
    ts.threadEntryFunc = "main";
    ts.threadFuncArgNum = 2;
    ts.threadSyncFuncs.push_back("pthread_join");
    clock_t start=clock();
    fail::Parser* p = new fail::Parser(file,&ts);
    fail::ParserData* pd = p->getParserDataPtr();
    fail::Analytics* as = new fail::Analytics(pd);
    clock_t end=clock();
    std::vector<fail::VarDecl> errorenousVars = as->getFaultyVars();
    for(const auto& vd: errorenousVars)
    {
        std::cout<<"Errors in access to "<<vd.name<<"\n";
    }
    std::cout<<"Estimated at "<<(double)(end-start)/CLOCKS_PER_SEC<<"\n";
    delete as;
    delete p;
  }
  return 0;
}
