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
		
		dependencies[] = {"Game", "World", "Mission"};
		
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
		units[]={};
		weapons[]={};
		requiredVersion=0.1; 
		requiredAddons[]=
		{
			"DZ_Data"
		};
	};
};
class CfgVehicles
{
	class Inventory_Base;
	class MoneyRuble1: Inventory_Base
	{
		scope=2;
		displayName="1 Ruble Note";
		descriptionShort="This Currency was used in Chernarus before the Disease. It is now only used for trading.";
		model="TM\Trader\Money_Ruble1.p3d";
		canBeSplit=1;
		rotationFlags = 16;
		lootCategory="Materials"; // Should be changed to Money?
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
	
	class MoneyRuble5: MoneyRuble1
	{
		displayName="5 Ruble Note";
		model="TM\Trader\Money_Ruble5.p3d";
	};
	
	class MoneyRuble10: MoneyRuble1
	{
		displayName="10 Ruble Note";
		model="TM\Trader\Money_Ruble10.p3d";
	};
	
	class MoneyRuble25: MoneyRuble1
	{
		displayName="25 Ruble Note";
		model="TM\Trader\Money_Ruble25.p3d";
	};
	
	class MoneyRuble50: MoneyRuble1
	{
		displayName="50 Ruble Note";
		model="TM\Trader\Money_Ruble50.p3d";
	};
	
	class MoneyRuble100: MoneyRuble1
	{
		displayName="100 Ruble Note";
		model="TM\Trader\Money_Ruble100.p3d";
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
};