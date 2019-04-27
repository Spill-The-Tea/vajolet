#include "gtest/gtest.h"

#include <iostream>
#include "libchess.h"

using ::testing::Environment;

class EnvironmentInvocationCatcher : public Environment {
protected:
	virtual void SetUp()
	{
		libChessInit();
	}

	virtual void TearDown()
	{
	}
};


int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  ::testing::AddGlobalTestEnvironment( new EnvironmentInvocationCatcher);
  return RUN_ALL_TESTS();
}