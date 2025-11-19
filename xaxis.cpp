#include <stdio.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include "XPLMPlugin.h"
#include "XPLMDataAccess.h"
#include "XPLMUtilities.h"
#include "XPLMProcessing.h"	
#include "XPLMMenus.h"

struct config  {
	int leftTriggerAxis;
	int rightTriggerAxis;
	int pollSize;
};

static XPLMDataRef		joystickDataRef = NULL;
static XPLMDataRef      yokeHeadingRef = NULL;
static config         	pluginConfig = {-1, -1, -1};
static std::string 		configPath = "";

static float FlightLoopCallback(
	float inElapsedSinceLastCall,
	float inElapsedTimeSinceLastFlightLoop,
	int inCounter,
	void *inRefcon
);

static float LoadConfigCallback(
	float inElapsedSinceLastCall,
	float inElapsedTimeSinceLastFlightLoop,
	int inCounter,
	void *inRefcon
);

static float PollCallback(
	float inElapsedSinceLastCall,
	float inElapsedTimeSinceLastFlightLoop,
	int inCounter,
	void *inRefcon
);

static void MenuCallback(
	void *inMenuRef,
	void *inItemRef
);

static config GetConfig();

PLUGIN_API int XPluginStart(
						char *		outName,
						char *		outSig,
						char *		outDesc)
{
	int xaxisRef;
	XPLMMenuID xaxisMenu;

	strcpy(outName, "xaxis");
	strcpy(outSig, "steveiliop56.plugins.xaxis");
	strcpy(outDesc, "A plugin that merges the triggers of an Xbox controller into a single axis.");

	char path[2048]; // please don't have paths longer than 2048 characters
	XPLMGetSystemPath(path);

	char dirSep[2];
    strcpy(dirSep, XPLMGetDirectorySeparator());

	configPath = std::string(path) + std::string(dirSep) + "Resources" + std::string(dirSep) + "plugins" + std::string(dirSep) + "xaxis" + std::string(dirSep) + "config.txt";

	joystickDataRef = XPLMFindDataRef("sim/joystick/joystick_axis_values");

	if (joystickDataRef == NULL) {
		XPLMDebugString("XAxis: failed to find dataref sim/joystick/joystick_axis_values\n");
		return 0;
	}

	yokeHeadingRef = XPLMFindDataRef("sim/cockpit2/controls/yoke_heading_ratio");

	if (yokeHeadingRef == NULL) {
		XPLMDebugString("XAxis: failed to find dataref sim/cockpit2/controls/yoke_heading_ratio\n");
		return 0;
	}

	xaxisRef = XPLMAppendMenuItem(
						XPLMFindPluginsMenu(),
						"XAxis",
						0,						
						1);

	xaxisMenu = XPLMCreateMenu(
						"XAxis",
						XPLMFindPluginsMenu(),
						xaxisRef,
						MenuCallback,
						0);
					
	XPLMAppendMenuItem(xaxisMenu, "Reload config", (void *) 1, 1);	
	XPLMAppendMenuSeparator(xaxisMenu);
	XPLMAppendMenuItem(xaxisMenu, "Start polling", (void *) 2, 1);
	XPLMAppendMenuItem(xaxisMenu, "Stop polling", (void *) 3, 1);
	XPLMAppendMenuSeparator(xaxisMenu);
	XPLMAppendMenuItem(xaxisMenu, "Start overriding axis", (void *) 4, 1);
	XPLMAppendMenuItem(xaxisMenu, "Stop overriding axis", (void *) 5, 1);

	XPLMRegisterFlightLoopCallback(
		FlightLoopCallback,
		-1,
		NULL
	);

	XPLMRegisterFlightLoopCallback(
		LoadConfigCallback,
		-1,
		NULL
	);

	return 1;
}

PLUGIN_API void XPluginStop(void)
{
	XPLMUnregisterFlightLoopCallback(
		FlightLoopCallback,
		NULL
	);
	XPLMUnregisterFlightLoopCallback(
		LoadConfigCallback,
		NULL
	);
	XPLMUnregisterFlightLoopCallback(
		PollCallback,
		NULL
	);
}

PLUGIN_API void XPluginDisable(void)
{
	XPLMUnregisterFlightLoopCallback(
		FlightLoopCallback,
		NULL
	);
	XPLMUnregisterFlightLoopCallback(
		LoadConfigCallback,
		NULL
	);
	XPLMUnregisterFlightLoopCallback(
		PollCallback,
		NULL
	);
}

PLUGIN_API int XPluginEnable(void)
{
	XPLMRegisterFlightLoopCallback(
		FlightLoopCallback,
		-1,
		NULL
	);
	return 1;
}

PLUGIN_API void XPluginReceiveMessage(void)
{
}

float FlightLoopCallback(
	float inElapsedSinceLastCall,
	float inElapsedTimeSinceLastFlightLoop,
	int inCounter,
	void *inRefcon
)
{
	float axisValues[pluginConfig.pollSize];
    int numValues = XPLMGetDatavf(joystickDataRef, axisValues, 0, pluginConfig.pollSize);    
	float combinedTriggerValue = axisValues[pluginConfig.rightTriggerAxis] - axisValues[pluginConfig.leftTriggerAxis];
	XPLMSetDataf(yokeHeadingRef, combinedTriggerValue);
	return -1;
}

float LoadConfigCallback(
	float inElapsedSinceLastCall,
	float inElapsedTimeSinceLastFlightLoop,
	int inCounter,
	void *inRefcon
)
{
	pluginConfig = GetConfig();
	if (pluginConfig.leftTriggerAxis == -1 || pluginConfig.rightTriggerAxis == -1 || pluginConfig.pollSize == -1) {
		XPLMDebugString("XAxis: Failed to read configuration file\n");
	};
	return 0;
}

float PollCallback(
	float inElapsedSinceLastCall,
	float inElapsedTimeSinceLastFlightLoop,
	int inCounter,
	void *inRefcon
)
{
	if (pluginConfig.pollSize <= 0) {
		XPLMDebugString("XAxis: PollCallback called before configuration was loaded\n");
		return 0;
	}
	char debugBuffer[256];
	float axisValues[pluginConfig.pollSize];
    int numValues = XPLMGetDatavf(joystickDataRef, axisValues, 0, pluginConfig.pollSize);    
	for (int i = 0; i < numValues; i++) {
		if (axisValues[i] == -0.01f || axisValues[i] == 0.0f || axisValues[i] == 0.01f) {
			continue;
		}
		sprintf(debugBuffer, "XAxis: Axis %d value: %f\n", i, axisValues[i]);
		XPLMDebugString(debugBuffer);
	}
	return 1;
}

config GetConfig() {
	char debugBuffer[256];
	try {
		// Get configuration from file
		std::string cfgLine;
		std::ifstream cfgReader(configPath);
		std::getline(cfgReader, cfgLine);
		cfgReader.close();

		// Parse configuration
		char *cCfgLine = cfgLine.data();

		if (cCfgLine == NULL || strlen(cCfgLine) == 0) {
			XPLMDebugString("XAxis: Configuration file is empty\n");
			return {-1, -1, -1};
		}

		sprintf(debugBuffer, "XAxis: Reading config line: %s\n", cCfgLine);
		XPLMDebugString(debugBuffer);

		int values[3];
		char* token = strtok(cCfgLine, " , ");
		int i = 0;

		while (token != NULL && i < 3) {
			values[i++] = std::stoi(token);
			token = strtok(NULL, " , ");
		}

		// Return configuration struct
		sprintf(debugBuffer, "XAxis: Loaded config - Left Trigger Axis: %d, Right Trigger Axis: %d, Poll Size: %d\n",
			values[0], values[1], values[2]);
		XPLMDebugString(debugBuffer);
		return {values[0], values[1], values[2]};
	}
	catch (int e) {
		sprintf(debugBuffer, "XAxis: Exception %d occurred while reading config file\n", e);
		XPLMDebugString(debugBuffer);
		return {-1, -1, -1};
	}
}

static void MenuCallback(
	void *inMenuRef,
	void *inItemRef
)
{
	if (inItemRef == (void*)1) {
		XPLMDebugString("XAxis: Reloading configuration file\n");
		pluginConfig = GetConfig();
		return;
	}
	if (inItemRef == (void*)2) {
		XPLMDebugString("XAxis: Starting polling\n");
		XPLMRegisterFlightLoopCallback(
			PollCallback,
			-1,
			NULL
		);
		return;
	}
	if (inItemRef == (void*)3) {
		XPLMDebugString("XAxis: Stopping polling\n");
		XPLMUnregisterFlightLoopCallback(
			PollCallback,
			NULL
		);
		return;
	}
	if (inItemRef == (void*)4) {
		XPLMDebugString("XAxis: Starting axis override\n");
		XPLMUnregisterFlightLoopCallback(
			FlightLoopCallback,
			NULL
		);
		XPLMRegisterFlightLoopCallback(
			FlightLoopCallback,
			-1,
			NULL
		);
		return;
	}
	if (inItemRef == (void*)5) {
		XPLMDebugString("XAxis: Stopping axis override\n");
		XPLMUnregisterFlightLoopCallback(
			FlightLoopCallback,
			NULL
		);
		return;
	}
	XPLMDebugString("XAxis: Unknown menu item selected\n");
}