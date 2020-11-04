modded class PlayerBase
{
    bool m_Trader_IsTrader = false;
    ref TraderMenu m_TraderMenu;

    override void Init()
    {
        DayzPlayerItemBehaviorCfg toolsOneHanded = new DayzPlayerItemBehaviorCfg;
        toolsOneHanded.SetToolsOneHanded();   

        GetDayZPlayerType().AddItemInHandsProfileIK("VehicleKeyBase", "dz/anims/workspaces/player/player_main/props/player_main_1h_keys.asi", toolsOneHanded, "dz/anims/anm/player/ik/gear/handcuff_keys.anm");
        super.Init();

        RegisterNetSyncVariableBool("m_Trader_IsTrader");
    }

#ifdef DAYZ_1_09
#ifndef DAYZ_1_10
    override void SetActions()
	{
		super.SetActions();

		AddAction(ActionUnlockVehicle);
        AddAction(ActionLockVehicle);
        AddAction(ActionUnlockVehicleInside);
        AddAction(ActionLockVehicleInside);
        AddAction(ActionTrade);
	}
#endif
#endif
#ifdef DAYZ_1_10
    override void SetActions(out TInputActionMap InputActionMap)
	{
		super.SetActions(InputActionMap);

		AddAction(ActionUnlockVehicle, InputActionMap);
        AddAction(ActionLockVehicle, InputActionMap);
        AddAction(ActionUnlockVehicleInside, InputActionMap);
        AddAction(ActionLockVehicleInside, InputActionMap);
        AddAction(ActionTrade, InputActionMap);
	}
#endif

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