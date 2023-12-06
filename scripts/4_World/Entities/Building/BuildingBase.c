modded class BuildingBase : Building
{
    bool m_Trader_IsTrader = false;
	int	m_Trader_TraderID = -1;

	void BuildingBase()
	{
        RegisterNetSyncVariableBool("m_Trader_IsTrader");
		RegisterNetSyncVariableInt("m_Trader_TraderID");
	}

	void SetBuildingAsTrader(int id)
	{
		m_Trader_IsTrader = true;
		m_Trader_TraderID = id;
		SetAllowDamage(false);
		SetSynchDirty();
	}

	override void SetActions()
	{
		super.SetActions();
        AddAction(ActionTrade);
	}
};