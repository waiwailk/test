#include <stdio.h>
//#include "Wrapper.h"
#include "InstrumentData.h"
int main()
{
	std::auto_ptr<InstrumentData> data( new InstrumentData() );
	//data.process();
	//data.populate();
	
	//work for issue 1.
	data.get()->process();
	
  return 0;
}