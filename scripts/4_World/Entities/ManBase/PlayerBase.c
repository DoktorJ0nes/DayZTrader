modded class PlayerBase
{
    override void Init()
    {
        DayzPlayerItemBehaviorCfg toolsOneHanded = new DayzPlayerItemBehaviorCfg;
        toolsOneHanded.SetToolsOneHanded();   

        GetDayZPlayerType().AddItemInHandsProfileIK("VehicleKeyBase", "dz/anims/workspaces/player/player_main/props/player_main_1h_keys.asi", toolsOneHanded, "dz/anims/anm/player/ik/gear/handcuff_keys.anm");
        super.Init();
    }

    override void SetActions()
	{
		super.SetActions();

		AddAction(ActionUnlockVehicle);
        AddAction(ActionLockVehicle);
        AddAction(ActionUnlockVehicleInside);
        AddAction(ActionLockVehicleInside);
	}

    bool Trader_IsAdmin()
    {
        for (int i = 0; i < m_Trader_AdminPlayerUIDs.Count(); i++)
        {
            if (m_Trader_AdminPlayerUIDs.Get(i) == m_Trader_PlayerUID)
                return true;
        }

        return false;
    }
}