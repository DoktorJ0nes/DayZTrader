////////////////////////////////////////////////////////////////////
//DeRap: Trader\config.bin
//Produced from mikero's Dos Tools Dll version 7.03
//https://armaservices.maverick-applications.com/Products/MikerosDosTools/default
//'now' is Thu May 16 22:05:14 2019 : 'file' last modified on Wed May 15 17:36:14 2019
////////////////////////////////////////////////////////////////////

#define _ARMA_

class CfgMods
{
	class Trader
	{
		dir = "Trader";
		picture = "";
		action = "";
		hideName = 1;
		hidePicture = 1;
		name = "";
		credits = "";
		author = "Dr_J0nes";
		authorID = "0";
		version = "1.0";
		extra = 0;
		type = "mod";
		dependencies[] = {"Game","World","Mission"};
		class defs
		{
			class gameScriptModule
			{
				value = "";
				files[] = {"TM/Trader/scripts/3_Game"};
			};
			class worldScriptModule
			{
				value = "";
				files[] = {"TM/Trader/scripts/4_World"};
			};
			class missionScriptModule
			{
				value = "";
				files[] = {"TM/Trader/scripts/5_Mission"};
			};
		};
	};
};
class CfgPatches
{
	class trader
	{
		units[] = {"MoneyRuble1","MoneyRuble5","MoneyRuble10","MoneyRuble25","MoneyRuble50","MoneyRuble100","Land_RoadCone","Hoodie_GraffitiTiles","Hoodie_DrJ0nes","VehicleKey"};
		weapons[] = {};
		requiredVersion = 0.1;
		requiredAddons[] = {"DZ_Data"};
	};
};
class CfgModels
{
	class ruble{
		sections[]={"WholeObject"};
	};
};
class CfgVehicles
{
	class Inventory_Base;
	class MoneyRuble1: Inventory_Base
	{
		scope=2;
		displayName=$STR_tm_1_ruble_note;
		descriptionShort="$STR_tm_ruble_description";
		model="TM\Trader\ruble.p3d";
		canBeSplit=1;
		rotationFlags = 16;
		lootCategory="Materials";
		lootTag[]=
		{
			"Civilian",
			"Work"
		};
		itemSize[]={1,2};
		weight=1;
		varQuantityInit=1;
		varQuantityMin=0;
		varQuantityMax=500;
		varQuantityDestroyOnMin=1;
		destroyOnEmpty=1;
		absorbency=1;
		hiddenSelections[]={"WholeObject"};
		hiddenSelectionsTextures[]={"TM\Trader\data\ruble1_co.paa"};
		class DamageSystem
		{
			class GlobalHealth
			{
				class Health
				{
					hitpoints = 100;
					healthLevels[] = {{1.0,{"TM\Trader\data\ruble.rvmat"}},{0.7,{"TM\Trader\data\ruble.rvmat"}},{0.5,{"TM\Trader\data\ruble_damage.rvmat"}},{0.3,{"TM\Trader\data\ruble_damage.rvmat"}},{0.0,{"TM\Trader\data\ruble_destruct.rvmat"}}};
				};
			};
		};
	};
	
	class MoneyRuble5: MoneyRuble1
	{
		displayName=$STR_tm_5_ruble_note;
		hiddenSelectionsTextures[]={"TM\Trader\data\ruble5_co.paa"};
	};
	
	class MoneyRuble10: MoneyRuble1
	{
		displayName=$STR_tm_10_ruble_note;
		hiddenSelectionsTextures[]={"TM\Trader\data\ruble10_co.paa"};
	};
	
	class MoneyRuble25: MoneyRuble1
	{
		displayName=$STR_tm_25_ruble_note;
		hiddenSelectionsTextures[]={"TM\Trader\data\ruble25_co.paa"};
	};
	
	class MoneyRuble50: MoneyRuble1
	{
		displayName=$STR_tm_50_ruble_note;
		hiddenSelectionsTextures[]={"TM\Trader\data\ruble50_co.paa"};
	};
	
	class MoneyRuble100: MoneyRuble1
	{
		displayName=$STR_tm_100_ruble_note;
		hiddenSelectionsTextures[]={"TM\Trader\data\ruble100_co.paa"};
	};

	class HouseNoDestruct;
	class Land_RoadCone: HouseNoDestruct
	{
		scope = 2;
		model = "TM\Trader\Road_Cone.p3d";
	};
	class Hoodie_ColorBase;
	class Hoodie_GraffitiTiles: Hoodie_ColorBase
	{
		scope = 2;
		visibilityModifier = 0.95;
		hiddenSelectionsTextures[] = {"","","","","TM\Trader\data\hoodie_graffiti_co.paa","TM\Trader\data\hoodie_graffiti_co.paa","TM\Trader\data\hoodie_graffiti_co.paa"};
	};
	class Hoodie_DrJ0nes: Hoodie_ColorBase
	{
		scope = 2;
		visibilityModifier = 0.95;
		hiddenSelectionsTextures[] = {"","","","","TM\Trader\data\hoodie_drj0nes_co.paa","TM\Trader\data\hoodie_drj0nes_co.paa","TM\Trader\data\hoodie_drj0nes_co.paa"};
	};
	class VehicleKey: Inventory_Base
	{
		scope = 2;
		displayName = $STR_tm_vehicle_key;
		descriptionShort = "$STR_tm_vehicle_key_description";
		model = "TM\Trader\Money_Ruble1.p3d";
		canBeSplit = 0;
		rotationFlags = 16;
		lootCategory = "Materials";
		lootTag[] = {"Civilian","Work"};
		itemSize[] = {1,1};
		weight = 1;
		varQuantityInit = 1;
		varQuantityMin = 0;
		varQuantityMax = 1;
		varQuantityDestroyOnMin = 1;
		destroyOnEmpty = 1;
		absorbency = 1;
		class DamageSystem
		{
			class GlobalHealth
			{
				class Health
				{
					hitpoints = 100;
					healthLabels[] = {1.0,0.7,0.5,0.3,0.0};
					healthLevels[] = {{1.0,{}},{0.5,{}},{0.0,{}}};
				};
			};
		};
	};
};
