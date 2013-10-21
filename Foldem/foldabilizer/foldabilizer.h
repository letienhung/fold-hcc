#pragma once

#include "interfaces/ModePluginDockWidget.h"
#include "SurfaceMeshPlugins.h"

class foldem_widget;

class foldabilizer : public SurfaceMeshModePlugin
{
	Q_OBJECT
	Q_INTERFACES(ModePlugin)

	// Plugin interfaces
	QIcon icon(){ return QIcon("images/icon.png"); }
	void create();
	void destroy();
	void decorate();

public:
    foldabilizer();

private:
	foldem_widget *widget;
};


