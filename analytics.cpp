#include "analytics.hpp"

inline int sign(int num)
{
  if(num>0) return 1;
  else if(num<0) return -1;
  else return 0;
}


namespace fail
{


  Analytics::Analytics(ParserData* ps):pd(ps)
  {
    //accounting for writes this time
    readsMatrix = new unsigned*[pd->ourVars->size()];
    writesMatrix = new unsigned*[pd->ourVars->size()];
    totalWrites = new int[pd->ourVars->size()];
    totalReads = new int[pd->ourVars->size()];
    errorVector = new int[pd->ourVars->size()];
    for(size_t i=0;i<pd->ourVars->size();++i)
      {
	readsMatrix[i] = new unsigned[pd->functions->size()];
	writesMatrix[i] = new unsigned[pd->functions->size()];
        totalWrites[i]=0;
        totalReads[i]=0;
      }
    int tmp;
    for(size_t i=0;i<pd->ourVars->size();++i)
      for(size_t j=0;j<pd->functions->size();++j)
	{
	  tmp=(pd->functions->at(j))->checkReference
	    (pd->ourVars->at(i));
	  if(tmp==1)
	    {
	      writesMatrix[i][j]=1;
	      readsMatrix[i][j]=0;
              totalWrites[i]++;
	    }
	  else if(tmp==0)
	    {
	      writesMatrix[i][j]=0;
	      readsMatrix[i][j]=1;
              totalReads[i]++;
	    }
	  else
	    {
	      writesMatrix[i][j]=0;
	      readsMatrix[i][j]=0;
	    }
	}
    //Checksums
    for(size_t i=0;i<pd->ourVars->size();++i)
      errorVector[i]=sign(sign(totalWrites[i])*(sign(totalReads[i]+sign(totalWrites[i]-1))));
  }
}
