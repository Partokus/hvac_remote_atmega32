#include "sources/remote/remote.h"

int main(void)
{
	CRemote remote;
	
	remote.init();
	remote.setup();
	
    while (1) 
    {
		remote.loop();
    }
}
