modded class MissionGameplay
{	
	float traderModIsLoadedReplicationTimer = 0.1;
	
	override void TickScheduler(float timeslice)
	{
		super.TickScheduler(timeslice);
			
		
		TickSchedulerTrader(timeslice);
	}

	void TickSchedulerTrader(float timeslice)
	{
		PlayerBase player = PlayerBase.Cast( GetGame().GetPlayer() );

		if (!player)
			return;

		updateTraderNotifications(timeslice);
		handleTraderModIsLoadedReplication(timeslice);
	}

	void updateTraderNotifications(float timeslice)
	{
		PlayerBase player = PlayerBase.Cast( GetGame().GetPlayer() );

		if (player.m_Trader_TraderNotifications)
				player.m_Trader_TraderNotifications.Update(timeslice);
	}

	void handleTraderModIsLoadedReplication(float timeslice)
	{
		PlayerBase player = PlayerBase.Cast( GetGame().GetPlayer() );

		traderModIsLoadedReplicationTimer -= timeslice;			
		if (traderModIsLoadedReplicationTimer > 0)
			return;			
		traderModIsLoadedReplicationTimer = 0.5;
		
		if (!GetGame().IsServer() && !player.m_Trader_TraderModIsLoadedHandled)
		{
			GetGame().RPCSingleParam(GetGame().GetPlayer(), TRPCs.RPC_TRADER_MOD_IS_LOADED, new Param1<PlayerBase>(PlayerBase.Cast(GetGame().GetPlayer())), true);
		}
	}
	
	override void OnKeyRelease(int key)
	{
		super.OnKeyRelease(key);		

		if ( key == KeyCode.KC_ESCAPE )
		{	
			TraderMenu traderMenu = TraderMenu.Cast(GetGame().GetUIManager().GetMenu());
			if(traderMenu)
		  		traderMenu.Close();
		}
	}
}