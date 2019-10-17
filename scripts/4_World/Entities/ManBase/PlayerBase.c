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
}