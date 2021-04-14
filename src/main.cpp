#include "lightbender.h"
#include "ws.h"

int main(int argc, char ** argv)
{
  if(argc!=3){
    cerr<<"Usage: "<<argv[0]<<" *.r *.obj"<<endl;
    return 1;
  }

  mascgl_workspace ws;
  if(!ws.initialize(argv[1]))
  {
    cerr<<"! Error: Failed to read "<<argv[1]<<endl;
    return 1;
  }

  LightBender lb(ws);
  if(!lb.build())
  {
    cerr<<"! Error: Failed to build light bender"<<endl;
    return false;
  }

  cout<<"- Saving light bender to "<<argv[2]<<endl;
  lb.save2obj(argv[2]);

  return 0;
}
