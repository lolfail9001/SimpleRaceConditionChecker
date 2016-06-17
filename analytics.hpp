#ifndef _ANALYTICS_HPP
#define _ANALYTICS_HPP

#include "commons.hpp"

namespace fail
{
  struct ErrorData
  {
    VarDecl vd;
    int totalWrites;
    int totalReads;
    int errorCheck;
    ErrorData(const VarDecl& vardecl,
              int tw,int tr,int ec):vd(vardecl),
                                    totalWrites(tw),
                                    totalReads(tr),
                                    errorCheck(ec)
    {}
    ErrorData(const ErrorData& ed):vd(ed.vd),
                                   totalWrites(ed.totalWrites),
                                   totalReads(ed.totalReads),
                                   errorCheck(ed.errorCheck)
    {}
  };


  class Analytics
  {
  private:
    ParserData* pd;
    unsigned** readsMatrix;
    unsigned** writesMatrix;
    int* totalWrites;
    int* totalReads;
    int* errorVector;
  public:
    Analytics(ParserData* ps);
    ~Analytics()
    {
      delete[] errorVector;
      delete[] totalReads;
      delete[] totalWrites;
      for(size_t i=0;i<pd->ourVars->size();++i)
      {
        delete[] readsMatrix[i];
        delete[] writesMatrix[i];
      }
      delete[] readsMatrix;
      delete[] writesMatrix;
    }
    inline std::vector<ErrorData> getErrorData() const
    {
      std::vector<ErrorData> result;
      for(size_t i=0;i<pd->ourVars->size();++i)
      {
        result.push_back(ErrorData(pd->ourVars->at(i),totalWrites[i],
                                   totalReads[i],errorVector[i]));
      }
      return result;
    }
    
    inline std::vector<VarDecl> getFaultyVars()
    {
      std::vector<VarDecl> result;
      for(size_t i=0;i<pd->ourVars->size();++i)
        if(errorVector[i])
          result.push_back(pd->ourVars->at(i));
      return result;
    }
  };
}


#endif
