modded class MissionServer
{		
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