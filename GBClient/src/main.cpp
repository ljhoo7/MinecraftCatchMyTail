#include "TestClient.h"

int main()
{
	TestClient pTestClient;

	pTestClient.Start();

	while (true)
	{
		Sleep(10);
	}

	return 0;
}