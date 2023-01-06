// Copyright 1998-2020 Epic Games, Inc. All Rights Reserved.



#pragma once

#include "Modules/ModuleManager.h"

class FWidgetDocumentationPluginModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
