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
			GetGame().RPCSingleParam(GetGame().GetPlayer(), TRPCs.RPC_TRADER_MOD_IS_LOADED, new Param1<PlayerBase>( GetGame().GetPlayer() ), true);
		}
	}
	
	override void OnKeyRelease(int key)
	{
		super.OnKeyRelease(key);
		

		if ( key == KeyCode.KC_B )
		{			
			handleTraderMenuKeyPress();
		}

		if ( key == KeyCode.KC_ESCAPE )
		{	
			if (isTraderMenuOpened())
				closeTraderMenu();
		}
	}

	void handleTraderMenuKeyPress()
	{
		PlayerBase player = GetGame().GetPlayer();
		vector playerPosition = player.GetPosition();

		if (player.m_Trader_RecievedAllData == false)
		{			
			player.MessageStatus("[Trader] MISSING TRADER DATA FROM SERVER!");				
			return;
		}

		// only call these after we made sure the client has all trader data loaded!
		int traderUID = getNearbyTraderUID(playerPosition);
		bool canOpenTraderMenu = getCanOpenTraderMenu(playerPosition, traderUID);

		if (!canOpenTraderMenu)
			return;

		if (isTraderMenuOpened())
			closeTraderMenu();
		else
			openTraderMenu(traderUID);
	}

	bool getCanOpenTraderMenu(vector position, int traderUID)
	{		
		PlayerBase player = GetGame().GetPlayer();
		bool playerIsInSafezoneRange = getIsInSafezoneRange(position)
		
		if (traderUID == -1 && playerIsInSafezoneRange)
		{
			TraderMessage.PlayerWhite("#tm_no_trader_nearby", player, 5);
			return false;
		}

		if (!playerIsInSafezoneRange)
			return false;	

		return true;
	}

	int getNearbyTraderUID(vector position)
	{
		PlayerBase player = GetGame().GetPlayer();

		for ( int traderUID = 0; traderUID < player.m_Trader_TraderPositions.Count(); traderUID++ )
		{
			if (getIsTraderNearby(position, traderUID))
				return traderUID;
		}

		return -1;
	}

	bool getIsInSafezoneRange(vector position)
	{
		PlayerBase player = GetGame().GetPlayer();

		for ( int traderUID = 0; traderUID < player.m_Trader_TraderPositions.Count(); traderUID++ )
		{
			if (getDistanceToTrader(position, traderUID) <= player.m_Trader_TraderSafezones.Get(traderUID) || getIsTraderNearby(position, traderUID))
				return true;
		}

		return false;
	}

	int getTraderID(int traderUID)
	{
		PlayerBase player = GetGame().GetPlayer();

		return player.m_Trader_TraderIDs.Get(traderUID);
	}

	float getDistanceToTrader(vector position, int traderUID)
	{
		PlayerBase player = GetGame().GetPlayer();

		return vector.Distance(position, player.m_Trader_TraderPositions.Get(traderUID));
	}

	bool getIsTraderNearby(vector position, int traderUID)
	{
		return getDistanceToTrader(position, traderUID) <= 1.7;
	}

	vector getTraderVehicleSpawnPosition(int traderUID)
	{
		PlayerBase player = GetGame().GetPlayer();

		return player.m_Trader_TraderVehicleSpawns.Get(traderUID);
	}

	vector getTraderVehicleSpawnOrientation(int traderUID)
	{
		PlayerBase player = GetGame().GetPlayer();

		return player.m_Trader_TraderVehicleSpawnsOrientation.Get(traderUID);
	}

	void initializeTraderMenu(int traderUID)
	{
		PlayerBase player = GetGame().GetPlayer();

		m_TraderMenu = new TraderMenu;
		m_TraderMenu.m_TraderID = getTraderID(traderUID);
		m_TraderMenu.m_TraderUID = traderUID;
		m_TraderMenu.m_TraderVehicleSpawn = getTraderVehicleSpawnPosition(traderUID);
		m_TraderMenu.m_TraderVehicleSpawnOrientation = getTraderVehicleSpawnOrientation(traderUID);
		m_TraderMenu.m_buySellTime = player.m_Trader_BuySellTimer;
		m_TraderMenu.Init();
	}

	void openTraderMenu(int traderUID)
	{
		if ( g_Game.GetUIManager().GetMenu() == NULL )
		{					
			initializeTraderMenu(traderUID);

			GetGame().GetUIManager().ShowScriptedMenu( m_TraderMenu, NULL );
		}
	}

	bool isTraderMenuOpened()
	{
		if (m_TraderMenu)
			return m_TraderMenu.m_active;

		return false;
	}

	void closeTraderMenu()
	{
		m_TraderMenu.m_active = false;
	}
}