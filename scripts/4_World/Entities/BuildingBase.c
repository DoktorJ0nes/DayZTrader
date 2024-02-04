modded class BuildingBase
{
	int m_Trader_TraderID = -1;
	void BuildingBase()
	{
		RegisterNetSyncVariableInt("m_Trader_TraderID", -1, 1000);
	}
};