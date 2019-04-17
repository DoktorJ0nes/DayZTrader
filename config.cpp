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
class CfgModels
{
	class ruble{
		sections[]={"WholeObject"};
	};

	class keyLada{
		sections[]={"WholeObject"};
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
		model="TM\Trader\ruble.p3d";
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
		displayName="5 Ruble Note";
		hiddenSelectionsTextures[]={"TM\Trader\data\ruble5_co.paa"};
	};
	
	class MoneyRuble10: MoneyRuble1
	{
		displayName="10 Ruble Note";
		hiddenSelectionsTextures[]={"TM\Trader\data\ruble10_co.paa"};
	};
	
	class MoneyRuble25: MoneyRuble1
	{
		displayName="25 Ruble Note";
		hiddenSelectionsTextures[]={"TM\Trader\data\ruble25_co.paa"};
	};
	
	class MoneyRuble50: MoneyRuble1
	{
		displayName="50 Ruble Note";
		hiddenSelectionsTextures[]={"TM\Trader\data\ruble50_co.paa"};
	};
	
	class MoneyRuble100: MoneyRuble1
	{
		displayName="100 Ruble Note";
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

	class VehicleKeyBase: Inventory_Base
	{
		scope=2;
		//ContinuousActions[] = {3555,3556};
		displayName="Vehicle Key";
		descriptionShort="This is just a normal Key to lock and unlock your Vehicle.";
		model="TM\Trader\keyLada.p3d";
		rotationFlags = 17;
		lootCategory="Materials";
		lootTag[]=
		{
			"Civilian",
			"Work"
		};
		weight=4;
		itemSize[]={1,1};
		fragility = 0.01;
		hiddenSelections[]={"WholeObject"};
		class DamageSystem
		{
			class GlobalHealth
			{
				class Health
				{
					hitpoints = 100;
					healthLevels[] = {{1.0,{"TM\Trader\data\keyLada.rvmat"}},{0.7,{"TM\Trader\data\keyLada.rvmat"}},{0.5,{"TM\Trader\data\keyLada_damage.rvmat"}},{0.3,{"TM\Trader\data\keyLada_damage.rvmat"}},{0.0,{"TM\Trader\data\keyLada_destruct.rvmat"}}};
				};
			};
		};
	};

	class VehicleKeyRed: VehicleKeyBase
	{
		hiddenSelectionsTextures[]={"TM\Trader\data\keyLadaRed_co.paa"};
	};

	class VehicleKeyBlack: VehicleKeyBase
	{
		hiddenSelectionsTextures[]={"TM\Trader\data\keyLadaBlack_co.paa"};
	};

	class VehicleKeyGrayCyan: VehicleKeyBase
	{
		hiddenSelectionsTextures[]={"TM\Trader\data\keyLadaGrayCyan_co.paa"};
	};

	class VehicleKeyYellow: VehicleKeyBase
	{
		hiddenSelectionsTextures[]={"TM\Trader\data\keyLadaYellow_co.paa"};
	};

	class VehicleKeyPurple: VehicleKeyBase
	{
		hiddenSelectionsTextures[]={"TM\Trader\data\keyLadaPurple_co.paa"};
	};
};