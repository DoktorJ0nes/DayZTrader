modded class BuildingBase
{
	int m_Trader_TraderIndex = -1;
	void BuildingBase()
	{
		RegisterNetSyncVariableInt("m_Trader_TraderIndex", -1, 1000);
	}
};