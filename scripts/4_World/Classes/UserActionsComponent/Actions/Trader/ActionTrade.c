class ActionTrade: ActionInteractBase
{
	private int m_traderID =  -1;
	private int m_traderIndex =  -1;
	private PlayerBase m_Player;
	private float m_Trader_AllowedTradeDistance = 3.0;

	void ActionTrade()
	{
		m_CommandUID = DayZPlayerConstants.CMD_ACTIONMOD_INTERACTONCE;
		m_StanceMask = DayZPlayerConstants.STANCEMASK_ERECT | DayZPlayerConstants.STANCEMASK_CROUCH;
		m_HUDCursorIcon = CursorIcons.CloseHood;
		m_Text = "Trade";
	}

    override void CreateConditionComponents()  
	{
		m_ConditionTarget = new CCTObject(10);//CCTMan(10);
		m_ConditionItem = new CCINone;
	}

	override string GetText()
	{
		return m_Text;
	}

	override bool ActionCondition( PlayerBase player, ActionTarget target, ItemBase item )
	{
        if (GetGame().IsServer())
            return true;

		if(!target || !target.GetObject() || !player)
			return false;		

		if (player && !player.HasReceivedAllTraderData())
		{						
			return false;
		}
		float distance = vector.Distance(player.GetPosition(), target.GetObject().GetPosition());
		if(distance > player.m_Trader_TradingDistance)
		{
			return false;
		}
		bool isTraderNPCObject = false;
		if(player.m_Trader_NPCDummyClasses)
		{	
			for ( int i = 0; i < player.m_Trader_NPCDummyClasses.Count(); i++ )
			{
				string targetName = target.GetObject().GetType();
				string dummyName = player.m_Trader_NPCDummyClasses.Get(i);
				targetName.ToLower();
				dummyName.ToLower();
				if (targetName == dummyName)
				{	
					isTraderNPCObject = true;
				}
			}
		}
		PlayerBase ntarget = PlayerBase.Cast(target.GetObject());
		bool isTraderNPCCharacter = false;
		if(ntarget)
			isTraderNPCCharacter = ntarget.IsTrader();
					
		if (!isTraderNPCCharacter && !isTraderNPCObject)
			return false;

        return CanOpenTrader(player, target.GetObject());
	}
    
    override void OnStartClient(ActionData action_data)
    {
		m_Player = action_data.m_Player;
		if ( g_Game.GetUIManager().GetMenu() == NULL )
		{					
			initializeTraderMenu();
		}
    }

    bool CanOpenTrader(PlayerBase player, Object target)
	{
		m_Player = player;
		vector playerPosition = player.GetPosition();
		m_Trader_AllowedTradeDistance = player.m_Trader_TradingDistance;
		BuildingBase buildingItem = BuildingBase.Cast(target);
		m_traderIndex = getNearbyTraderUID(playerPosition);
		if(buildingItem && buildingItem.m_Trader_TraderIndex > -1)
		{			
			m_traderIndex = buildingItem.m_Trader_TraderIndex;
		}
		PlayerBase playerTrader = PlayerBase.Cast(target);
		if(playerTrader&& playerTrader.m_Trader_TraderIndex > -1)
		{
			m_traderIndex = playerTrader.m_Trader_TraderIndex;
		}
		m_traderID = getTraderID();
		bool canOpenTraderMenu = getCanOpenTraderMenu(playerPosition);
		if (canOpenTraderMenu)
		{			
			if(player.m_Trader_TraderNames)
			{
				string traderName = player.m_Trader_TraderNames.Get(getTraderID());
				m_Text = "Trade [" + traderName + "]";
			}
			return true;
		}
		return false;
	}

    bool getCanOpenTraderMenu(vector position)
	{		
		bool playerIsInSafezoneRange = getIsInSafezoneRange(position);
		
		if (m_traderIndex == -1 && playerIsInSafezoneRange)
		{
			return false;
		}

		if (!playerIsInSafezoneRange)
			return false;	

		return true;
	}

    int getNearbyTraderUID(vector position)
	{
		for ( int traderIndex = 0; traderIndex < m_Player.m_Trader_TraderPositions.Count(); traderIndex++ )
		{
			if (getIsTraderNearby(position, traderIndex))
				return traderIndex;
		}

		return -1;
	}

    bool getIsInSafezoneRange(vector position)
	{
		for ( int traderIndex = 0; traderIndex < m_Player.m_Trader_TraderPositions.Count(); traderIndex++ )
		{
			if (getDistanceToTrader(position, traderIndex) <= m_Player.m_Trader_TraderSafezones.Get(traderIndex) || getIsTraderNearby(position, traderIndex))
				return true;
		}

		return false;
	}

    int getTraderID()
	{
		return m_Player.m_Trader_TraderIDs.Get(m_traderIndex);
	}

    float getDistanceToTrader(vector position, int traderIndex)
	{
		return vector.Distance(position, m_Player.m_Trader_TraderPositions.Get(traderIndex));
	}

    bool getIsTraderNearby(vector position, int traderIndex)
	{
		return getDistanceToTrader(position, traderIndex) <= m_Trader_AllowedTradeDistance;
	}

	void initializeTraderMenu()
	{
		if (GetGame().GetUIManager().GetMenu() == NULL) 
		{                
			m_Player.m_TraderMenu = TraderMenu.Cast(GetGame().GetUIManager().EnterScriptedMenu(TRADERMENU_UI, null));
			m_Player.m_TraderMenu.m_TraderID = m_traderID;
			m_Player.m_TraderMenu.m_traderIndex = m_traderIndex;
			m_Player.m_TraderMenu.m_TraderVehicleSpawn = m_Player.m_Trader_TraderVehicleSpawns.Get(m_traderIndex);
			m_Player.m_TraderMenu.m_TraderVehicleSpawnOrientation = m_Player.m_Trader_TraderVehicleSpawnsOrientation.Get(m_traderIndex);
			m_Player.m_TraderMenu.m_buySellTime = m_Player.m_Trader_BuySellTimer;
			m_Player.m_TraderMenu.InitTraderValues();
		}
	}
}