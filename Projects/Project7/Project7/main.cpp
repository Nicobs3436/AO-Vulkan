#include "GEApplication.h"
#include <iostream>
#include <stdexcept>

//
// PROYECTO: Project7
//
// DESCRIPCIÓN: Añade una articulación de tipo balljoint
//              Define una escena con una cruz y dos articulaciones
//
int main()
{
	GEApplication app;

	try
	{
		app.run();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}