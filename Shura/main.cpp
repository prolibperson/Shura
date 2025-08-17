#include "Engine.h"

auto main() -> int
{
	Engine engine_inst;

	if (!engine_inst.init())
		return -1;

	engine_inst.run();

	engine_inst.shutdown();
	return 0;
}