modded class MissionGameplay
{	
	float traderModIsLoadedReplicationTimer = 0.1;
	
	ref TraderMenu m_TraderMenu;

	override void OnInit()
  	{
		g_Game.SetProfileOption( EDayZProfilesOptions.GAME_MESSAGES, 0 );
		g_Game.SetProfileOption( EDayZProfilesOptions.ADMIN_MESSAGES, 0 );
		g_Game.SetProfileOption( EDayZProfilesOptions.PLAYER_MESSAGES, 0 );

		super.OnInit();
	}
	
	override void TickScheduler(float timeslice)
	{
		PlayerBase player = PlayerBase.Cast( GetGame().GetPlayer() );

		super.TickScheduler(timeslice);
			
		
		if ( player )
		{
			if (player.m_Trader_TraderNotifications)
				player.m_Trader_TraderNotifications.Update(timeslice);

			traderModIsLoadedReplicationTimer -= timeslice;			
			if (traderModIsLoadedReplicationTimer > 0)
				return;			
			traderModIsLoadedReplicationTimer = 0.5;
			
			if (!GetGame().IsServer() && !player.m_Trader_TraderModIsLoadedHandled)
			{
				GetGame().RPCSingleParam(GetGame().GetPlayer(), TRPCs.RPC_TRADER_MOD_IS_LOADED, new Param1<PlayerBase>( GetGame().GetPlayer() ), true);
			}
		}
	}

	/*void DEBUG_PrintVehiclesNearbyHash(vector positionAt)
	{
		PlayerBase player = GetGame().GetPlayer();

		vector size = "3 5 9";
		array<Object> excluded_objects = new array<Object>;
		array<Object> nearby_objects = new array<Object>;

		if (GetGame().IsBoxColliding( positionAt, "0 0 0", size, excluded_objects, nearby_objects))
		{
			for (int i = 0; i < nearby_objects.Count(); i++)
			{					
				Transport transport;
				Class.CastTo(transport, nearby_objects.Get(i));
				if (!transport)
					continue;

				CarScript carScript = CarScript.Cast(nearby_objects.Get(i));
				if(!carScript)
					continue;

				TraderMessage.PlayerWhite("[" + nearby_objects.Get(i).GetType() + "]\nHasKey: " + carScript.m_Trader_HasKey + "\nLocked: " + carScript.m_Trader_Locked + "\nHash: " + carScript.m_Trader_VehicleKeyHash, player);
			}
		}
	}

	void DEBUG_PrintVehicleKeyInHandsHash()
    {
		PlayerBase player = GetGame().GetPlayer();

        VehicleKeyBase vehicleKey = VehicleKeyBase.Cast(player.GetHumanInventory().GetEntityInHands());

        if(vehicleKey)
			TraderMessage.PlayerWhite("[" + vehicleKey.GetType() + "]\nHash: " + vehicleKey.GetHash(), player);
    }*/
	
	override void OnKeyRelease(int key)
	{
		super.OnKeyRelease(key);
		
		
		PlayerBase player = GetGame().GetPlayer();

		/*if ( key == KeyCode.KC_O )
		{
			DEBUG_PrintVehiclesNearbyHash(player.GetPosition());
			DEBUG_PrintVehicleKeyInHandsHash();
		}*/

		if ( key == KeyCode.KC_B )
		{			
			bool traderNearby = false;
			bool playerIsInSafezoneRange = false;
			int traderID = -1;
			int traderUID = -1;
			vector traderVehicleSpawn = "0 0 0";
			vector traderVehicleSpawnOrientation = "0 0 0";
			
			if (player.m_Trader_RecievedAllData == false)
			{			
				player.MessageStatus("[Trader] MISSING TRADER DATA FROM SERVER!");				
				return;
			}
			
			for ( int i = 0; i < player.m_Trader_TraderPositions.Count(); i++ )
			{	
				float playerDistanceToTrader = vector.Distance(player.GetPosition(), player.m_Trader_TraderPositions.Get(i));

				if (playerDistanceToTrader <= player.m_Trader_TraderSafezones.Get(i))
					playerIsInSafezoneRange = true;

				if (playerDistanceToTrader <= 1.7)
				{
					traderNearby = true;
					playerIsInSafezoneRange = true;
					traderID = player.m_Trader_TraderIDs.Get(i);
					traderUID = i;
					traderVehicleSpawn = player.m_Trader_TraderVehicleSpawns.Get(i);
					traderVehicleSpawnOrientation = player.m_Trader_TraderVehicleSpawnsOrientation.Get(i);
				}
			}
			
			if (!traderNearby && playerIsInSafezoneRange)
			{
				TraderMessage.PlayerWhite("#tm_no_trader_nearby", player, 5);
				return;
			}

			if (!playerIsInSafezoneRange)
				return;		

			if (m_TraderMenu)
				m_TraderMenu.m_active = false;		
			
			if ( g_Game.GetUIManager().GetMenu() == NULL )
			{					
				m_TraderMenu = new TraderMenu;
				m_TraderMenu.m_TraderID = traderID;
				m_TraderMenu.m_TraderUID = traderUID;
				m_TraderMenu.m_TraderVehicleSpawn = traderVehicleSpawn;
				m_TraderMenu.m_TraderVehicleSpawnOrientation = traderVehicleSpawnOrientation;
				m_TraderMenu.m_buySellTime = player.m_Trader_BuySellTimer;
				m_TraderMenu.Init();
				GetGame().GetUIManager().ShowScriptedMenu( m_TraderMenu, NULL );
			}
		}

		if ( key == KeyCode.KC_ESCAPE )
		{	
			if (m_TraderMenu)
				m_TraderMenu.m_active = false;
		}
	}
}