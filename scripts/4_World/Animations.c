modded class ModItemRegisterCallbacks
{
    override void RegisterOneHanded( DayZPlayerType pType, DayzPlayerItemBehaviorCfg pBehavior )
    {
        super.RegisterOneHanded( pType, pBehavior );

        pType.AddItemInHandsProfileIK("VehicleKeyBase", "dz/anims/workspaces/player/player_main/props/player_main_1h_keys.asi", pBehavior, "dz/anims/anm/player/ik/gear/handcuff_keys.anm");
	}
}