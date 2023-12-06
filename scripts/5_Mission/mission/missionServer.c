modded class MissionServer
{
	static const string m_Trader_ConfigFilePath = "$profile:Trader/TraderConfig.txt";
	static const string m_Trader_ObjectsFilePath = "$profile:Trader/TraderObjects.txt";
	static const string m_Trader_VehiclePartsFilePath = "$profile:Trader/TraderVehicleParts.txt";
	static const string m_Trader_VariableFilePath = "$profile:Trader/TraderVariables.txt";
	static const string m_Trader_AdminsFilePath = "$profile:Trader/TraderAdmins.txt";
	bool m_Trader_ReadAllTraderData = false;

	string m_Trader_CurrencyName;
	ref array<string> m_Trader_CurrencyClassnames;
	ref array<int> m_Trader_CurrencyValues;
	
	ref array<string> m_Trader_TraderNames;
	ref array<vector> m_Trader_TraderPositions;
	ref array<int> m_Trader_TraderIDs;
	ref array<vector> m_Trader_TraderVehicleSpawns;
	ref array<vector> m_Trader_TraderVehicleSpawnsOrientation;
	
	ref array<string> m_Trader_Categorys;
	ref array<int> m_Trader_CategorysTraderKey;
	
	ref array<int> m_Trader_ItemsTraderId;
	ref array<int> m_Trader_ItemsCategoryId;
	ref array<string> m_Trader_ItemsClassnames;
	ref array<int> m_Trader_ItemsQuantity;
	ref array<int> m_Trader_ItemsBuyValue;
	ref array<int> m_Trader_ItemsSellValue;	

	ref array<string> m_Trader_Vehicles;
	ref array<string> m_Trader_VehiclesParts;
	ref array<int> m_Trader_VehiclesPartsVehicleId;

	ref array<string> m_Trader_AdminPlayerUIDs;

	float m_Trader_BuySellTimer = 0.3;
	
	override void InvokeOnConnect(PlayerBase player, PlayerIdentity identity)
    {
        super.InvokeOnConnect(player, identity);
        PluginTraderData traderData = PluginTraderData.Cast(GetPlugin(PluginTraderData));
        if(traderData)
        {
            traderData.Client_RequestData(identity);
			TM_Print("Sent trader data to: " + identity.GetName());
        }
	}

	override void OnInit()
	{		
		super.OnInit();
	}

	override void HandleBody(PlayerBase player)
    {
		if (player.IsUnconscious() || player.IsRestrained())
            player.SetAllowDamage(true);

        super.HandleBody(player);
    }

	void SetPlayerVehicleIsInSafezone( PlayerBase player, bool isInSafezone )
	{
		CarScript car = CarScript.Cast(player.GetParent());

		if (car)
		{
			car.m_Trader_IsInSafezone = isInSafezone;
			car.SynchronizeValues();
			car.SetAllowDamage(!isInSafezone);
		}
	}
}