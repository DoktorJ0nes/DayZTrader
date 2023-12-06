class ActionTrade: ActionInteractBase
{
	int m_TraderID =  -1;
	PlayerBase m_Player;
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

		PlayerBase ntarget = PlayerBase.Cast(target.GetObject());
		bool isTraderNPCCharacter = false;
		if(ntarget)
		{
			if (!ntarget.m_Trader_IsTrader)
				return false;
		}				

        return CanOpenTrader(player, ntarget);
	}
    
    override void OnStartClient(ActionData action_data)
    {
		m_Player = action_data.m_Player;
		if ( g_Game.GetUIManager().GetMenu() == NULL )
		{					
			initializeTraderMenu();
		}
    }

    bool CanOpenTrader(PlayerBase player, PlayerBase NPC)
	{
		m_Player = player;
		vector playerPosition = player.GetPosition();
		m_TraderID = NPC.m_Trader_TraderID;
		if(m_TraderID == -1)
		{
			return false;
		}		
		PluginTraderData traderDataPlugin = PluginTraderData.Cast(GetPlugin(PluginTraderData));
        if(traderDataPlugin)
        {
			string traderName = traderDataPlugin.GetTraderByID(m_TraderID).DisplayName;
			m_Text = "Trade [" + traderName + "]";
		}
		else
		{
			m_Text = "Trade";
		}
		return true;
	}
	void initializeTraderMenu()
	{
		if (GetGame().GetUIManager().GetMenu() == NULL) 
		{                
			m_Player.m_TraderMenu = TraderMenu.Cast(GetGame().GetUIManager().EnterScriptedMenu(TRADERMENU_UI, null));
			m_Player.m_TraderMenu.m_TraderID = m_TraderID;
			m_Player.m_TraderMenu.InitTraderValues();
		}
	}
}