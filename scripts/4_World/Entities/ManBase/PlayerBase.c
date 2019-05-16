/*modded class PlayerBase
{
    ref ActionBase actionUnlockVehicle;
    ref ActionBase actionLockVehicle;
    ref ActionBase actionUnlockVehicleInside;
    ref ActionBase actionLockVehicleInside;

    override void Init()
    {
        DayzPlayerItemBehaviorCfg toolsOneHanded = new DayzPlayerItemBehaviorCfg;
        toolsOneHanded.SetToolsOneHanded();   

        GetDayZPlayerType().AddItemInHandsProfileIK("VehicleKeyBase", "dz/anims/workspaces/player/player_main/props/player_main_1h_keys.asi", toolsOneHanded, "dz/anims/anm/player/ik/gear/handcuff_keys.anm");
        super.Init();
    }

    override void OnSelectPlayer()
    {
        super.OnSelectPlayer();

        actionUnlockVehicle = new ActionUnlockVehicle();
        actionLockVehicle = new ActionLockVehicle();
        actionUnlockVehicleInside = new ActionUnlockVehicleInside();
        actionLockVehicleInside = new ActionLockVehicleInside();

        actionUnlockVehicle.CreateConditionComponents();
        actionLockVehicle.CreateConditionComponents();
        actionUnlockVehicleInside.CreateConditionComponents();
        actionLockVehicleInside.CreateConditionComponents();

        m_ActionManager.m_ActionsMap.Set(3555, actionUnlockVehicle);
        m_ActionManager.m_ActionsMap.Set(3556, actionLockVehicle);
        m_ActionManager.m_ActionsMap.Set(3557, actionUnlockVehicleInside);
        m_ActionManager.m_ActionsMap.Set(3558, actionLockVehicleInside);
    }

    override void GetContinuousActions(out TIntArray actions)
	{
        actions.Insert(3555);
        actions.Insert(3556);
        actions.Insert(3557);
        actions.Insert(3558);

        super.GetContinuousActions(actions);
    }
}*/

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