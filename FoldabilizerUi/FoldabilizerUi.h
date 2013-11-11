#pragma once

#include "interfaces/ModePluginDockWidget.h"
#include "SurfaceMeshPlugins.h"

class FuiWidget;

class FoldabilizerUi : public SurfaceMeshModePlugin
{
	Q_OBJECT
	Q_INTERFACES(ModePlugin)

public:
    FoldabilizerUi();

	// Plugin interfaces
	QIcon icon(){ return QIcon("image/icon.png"); }
	void create();
	void destroy();
	void decorate();

private:
	FuiWidget *widget;

public slots:
	void sayHi();
};


