#include <iostream>
#include <winsock2.h>
#pragma comment(lib,"ws2_32.lib")

#include "Trace.h"
#include "TestData.h"

int main(void)
{
	//--------------------------------------------------------
	// Get initialize data
	//--------------------------------------------------------
	TestData  data;
	std::list<int> &rNodeList = data.getNodeList();

	//--------------------------------------------------------
	// Print the original data
	//--------------------------------------------------------
	Trace::out("\n\nClient: Original List\n\n");
	int j(0);
	for (std::list<int>::iterator it=rNodeList.begin(); it!=rNodeList.end(); ++it)
	{
		Trace::out("    i:%d  %d\n",j++,*it);
	}

	//--------------------------------------------------------
	// Add your code and hooks here:
	//--------------------------------------------------------





	//--------------------------------------------------------
	// Print the sorted data
	//--------------------------------------------------------
	Trace::out("\n\nClient: Sorted List from the server\n\n");
	int k(0);
	for (std::list<int>::iterator it=rNodeList.begin(); it!=rNodeList.end(); ++it)
	{
		Trace::out("    i:%d  %d\n",k++,*it);
	}
	Trace::out("Client Done\n\n");
	system("PAUSE");

	return 0;
}