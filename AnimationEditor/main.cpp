
#include "importer/FormatHeader.h"
#include "importer/FormatImporter.h"
#include "AnimationEditorApplication.h"

int main()
{
	AnimationEditorApplication app;
	app.LoadSkeletonFilesInDirectory("C:\\Repos\\AnimationEditor\\Assets");
	app.LoadAnimationFilesInDirectory("Assets/");
	return 0;
}