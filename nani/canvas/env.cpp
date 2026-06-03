#include "env.h"
#include "internal/env_p.h"
namespace nani::canvas
{
	Env::Env(int argc, char** argv)
	{
		internal::EnvPrivate::Instance()->Initialize();
	}

	Env::~Env()
	{
		internal::EnvPrivate::Instance()->Terminate();
	}

	int Env::WaitForQuit()
	{
		return internal::EnvPrivate::Instance()->WaitForQuit();
	}
}
