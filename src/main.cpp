#include "core/Application.h"


int main()
{
	ApplicationProperties appProps;
	appProps.Name = "App Name";
	appProps.WindowProps.Title = "Engine Core";
	appProps.WindowProps.MonitorSelected = 1; // 0 is the main monitor
	appProps.WindowProps.VSyncEnabled = false;

	Application app(appProps);
	app.Run();
}
