#include "gpch.h"
#include "Engine/Application.h"
#include "Game/FNAF-1/FiveNightsAtFreddys.h"

int main() {
	ApplicationSpecifications appSpecs{};
	appSpecs.name = "Five Nights At Freddy";
	appSpecs.width = 800;
	appSpecs.height = 800;
	appSpecs.appVersion = 0.5;
	Application app(appSpecs);
	MEM::Ref<FiveNightsAtFreddys> fnaf = MEM::CreateRef<FiveNightsAtFreddys>(FiveNightsAtFreddys::NIGHT1);
	app.SetGame(fnaf);
	app.Run();
	return 0;
}