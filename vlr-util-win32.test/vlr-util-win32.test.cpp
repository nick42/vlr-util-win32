// vlr-util-win32.test.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"

#include <array>
#include <vector>

int main(int argc, char** argv)
{
	testing::InitGoogleTest(&argc, argv);
	auto nResult_GoogleTest = RUN_ALL_TESTS();

	auto oCatchSession = Catch::Session{};

	// Faking command line until we convert...
	auto arrFakeCommandLine = std::vector<const char*>{};
	arrFakeCommandLine.push_back(argv[0]);

	int returnCode = oCatchSession.applyCommandLine((int)arrFakeCommandLine.size(), arrFakeCommandLine.data());
	if (returnCode != 0) // Indicates a command line error
		return returnCode;

	auto nCatchResult = oCatchSession.run();
	// Ignoring result...

	std::cout << "Testing..." << std::endl;

	return nResult_GoogleTest | nCatchResult;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
