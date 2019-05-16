/*class ActionUnlockVehicleOLD: ActionInteractBase
{
	string m_AnimSource = "";
	
	void ActionLockVehicle()
	{
		m_MessageSuccess = "";
		//m_CommandUID = DayZPlayerConstants.CMD_ACTIONMOD_OPENDOORFW;
		m_StanceMask = DayZPlayerConstants.STANCEMASK_ALL;
		//m_StanceMask = DayZPlayerConstants.STANCEMASK_ERECT | DayZPlayerConstants.STANCEMASK_CROUCH;
		m_HUDCursorIcon = CursorIcons.OpenDoors;
	}

	override void CreateConditionComponents()
	{
		m_ConditionItem = new CCINonRuined;
		m_ConditionTarget = new CCTParent(10);
        //m_ConditionTarget = new CCTSelf;
	}

	override int GetType()
	{
		return 3556;
	}

	override string GetText()
	{
		return "Unlock";
	}

	override bool ActionCondition( PlayerBase player, ActionTarget target, ItemBase item )
	{	
		// if( !target ) return false;
		// //if( IsDamageDestroyed(action_data.m_Target) ) return false;
		// //if( !IsTransport(action_data.m_Target) ) return false;
		// if( !IsInReach(player, target, UAMaxDistances.DEFAULT) ) return false;

		// CarScript car;
		// if ( Class.CastTo(car, target.GetParent()) )
		// {
		// 	array<string> selections = new array<string>();
			
		// 	CarDoor carDoor = CarDoor.Cast(target.GetObject());
		// 	if (carDoor)
		// 	{
		// 		carDoor.GetActionComponentNameList(target.GetComponentIndex(), selections);
				
		// 		for (int i = 0; i < selections.Count(); i++)
		// 		{
		// 			m_AnimSource = car.GetAnimSourceFromSelection( selections[i]);
		// 			if ( m_AnimSource != "" )
		// 			{
		// 				//! if player is in car and cannot reach doors
		// 				if (player.IsInVehicle() && !car.CanReachDoorsFromSeat(m_AnimSource, car.CrewMemberIndex( player )) )
		// 					return false;

		// 				//! is in reach, should open the door
		// 				if ( car.GetAnimationPhase( m_AnimSource ) <= 0.5 )
		// 					return true;
		// 			}
		// 		}
		// 	}
		// }
		// return false;

        return true;
	}
	
	override void OnStartServer( ActionData action_data )
	{
		// Car car = Car.Cast(action_data.m_Target.GetParent());
		// if( car )
		// {
		// 	car.SetAnimationPhase( m_AnimSource, 1.0);
		// 	if ( !GetGame().IsMultiplayer() || GetGame().IsClient() )
		// 		SEffectManager.PlaySound("offroad_door_open_SoundSet", car.GetPosition() );
		// }
	}

	override void OnStartClient( ActionData action_data )
	{
		// Car car = Car.Cast(action_data.m_Target.GetParent());
		// if( car )
		// {
		// 	car.SetAnimationPhase( m_AnimSource, 1.0);
		// }
	}
	
	override bool CanBeUsedInVehicle()
	{
		return true;
	}
}


//_----------------------------------------------------------------------------------------------------------------------------------------------------------------


class ActionLockUnlockVehicleOLDD: ActionInteractBase
{	
	void ActionLockUnlockVehicle()
	{
		m_MessageSuccess = "";
		//m_StanceMask = DayZPlayerConstants.STANCEMASK_ALL;
		m_StanceMask = DayZPlayerConstants.STANCEMASK_ERECT | DayZPlayerConstants.STANCEMASK_CROUCH;
		m_HUDCursorIcon = CursorIcons.OpenDoors;
	}

	override void CreateConditionComponents()
	{
		m_ConditionItem = new CCINone;
		m_ConditionTarget = new CCTParent(10);
	}
	
	override bool CanBeUsedInVehicle()
	{
		return true;
	}

	override bool IsInstant()
	{
		return true;
	}

	// override bool UseAcknowledgment()
	// {
	// 	return false;
	// }

	protected bool playerHasVehicleKeyInHands(PlayerBase player, int vehicleKeyHash)
    {
        VehicleKey vehicleKey = VehicleKey.Cast(player.GetHumanInventory().GetEntityInHands());

        if(vehicleKey)
        {
            if(vehicleKey.hash == vehicleKeyHash)
                return true;
        }

        return false;
    }

	override void OnUpdate(ActionData action_data)
	{
		super.OnUpdate(action_data);

		Print("[LOCKUNLOCK] Update::LockUnlock State " + action_data.m_State);

		if(action_data.m_State == UA_START)
		{
			Print("[LOCKUNLOCK] End");
			End(action_data);
		}
	}
}


class ActionUnlockVehicleOLDD: ActionLockUnlockVehicleOLDD
{	
	void ActionUnlockVehicle()
	{
		ActionLockUnlockVehicle();
	}

	override int GetType()
	{
		return 3555;
	}

	override string GetText()
	{
		return "Unlock";
	}

	override bool ActionCondition( PlayerBase player, ActionTarget target, ItemBase item )
	{	
		if( !target ) return false;
		if( !IsInReach(player, target, UAMaxDistances.DEFAULT) ) return false;

		CarScript carScript;
		if ( Class.CastTo(carScript, target.GetParent()) )
		{
			if (carScript.m_Trader_Locked && playerHasVehicleKeyInHands(player, carScript.m_Trader_VehicleKeyHash))
				return true;
		}

        return false;
	}
	
	override void OnStartServer( ActionData action_data )
	{
		Print("[LOCKUNLOCK] OnStartServer::Unlock");

		CarScript carScript = CarScript.Cast(action_data.m_Target.GetParent());
		if( carScript )
		{
			carScript.m_Trader_Locked = false;
			carScript.SynchronizeValues();

			Print("[LOCKUNLOCK] Unlocked Vehicle");
		}
		else
		{
			Print("[LOCKUNLOCK] Can not unlock Vehicle");
		}
	}

	override void OnStartClient( ActionData action_data )
	{
		Print("[LOCKUNLOCK] OnStartClient::Unlock");

		// Car car = Car.Cast(action_data.m_Target.GetParent());
		// if( car )
		// {
		// 	car.SetAnimationPhase( m_AnimSource, 1.0);
		// }
	}

	override void Start( ActionData action_data )
	{
		Print("[LOCKUNLOCK] Start::Unlock");

		super.Start(action_data);

		// if (!GetGame().IsServer())
		// 	return;

		// Print("[LOCKUNLOCK] Start::Unlock");

		// CarScript carScript = CarScript.Cast(action_data.m_Target.GetParent());
		// if( carScript )
		// {
		// 	carScript.m_Trader_Locked = false;
		// 	carScript.SynchronizeValues();

		// 	Print("[LOCKUNLOCK] Unlocked Vehicle");
		// }
		// else
		// {
		// 	Print("[LOCKUNLOCK] Can not unlock Vehicle");
		// }

	}

	override void OnExecuteServer( ActionData action_data )
	{
		Print("[LOCKUNLOCK] OnExecuteServer::Unlock");

		CarScript carScript = CarScript.Cast(action_data.m_Target.GetParent());
		if( carScript )
		{
			carScript.m_Trader_Locked = false;
			carScript.SynchronizeValues();

			Print("[LOCKUNLOCK] Unlocked Vehicle");
		}
		else
		{
			Print("[LOCKUNLOCK] Can not unlock Vehicle");
		}
	}

	// override void OnFinishProgressServer( ActionData action_data )
    // {
    //     Print("[LOCKUNLOCK] OnFinishProgressServer::Unlock");

	// 	CarScript carScript = CarScript.Cast(action_data.m_Target.GetParent());
	// 	if( carScript )
	// 	{
	// 		carScript.m_Trader_Locked = false;
	// 		carScript.SynchronizeValues();

	// 		Print("[LOCKUNLOCK] Unlocked Vehicle");
	// 	}
	// 	else
	// 	{
	// 		Print("[LOCKUNLOCK] Can not unlock Vehicle");
	// 	}
    // }
}

class ActionLockVehicleOLDD: ActionLockUnlockVehicleOLDD
{	
	void ActionLockVehicle()
	{
		ActionLockUnlockVehicle();
	}

	override int GetType()
	{
		return 3556;
	}

	override string GetText()
	{
		return "Lock";
	}

	override bool ActionCondition( PlayerBase player, ActionTarget target, ItemBase item )
	{	
		if( !target ) return false;
		if( !IsInReach(player, target, UAMaxDistances.DEFAULT) ) return false;

		CarScript carScript;
		if ( Class.CastTo(carScript, target.GetParent()) )
		{
			if (!carScript.m_Trader_Locked && playerHasVehicleKeyInHands(player, carScript.m_Trader_VehicleKeyHash))
				return true;
		}

        return false;
	}
	
	override void OnStartServer( ActionData action_data )
	{
		CarScript carScript = CarScript.Cast(action_data.m_Target.GetParent());
		if( carScript )
		{
			carScript.m_Trader_Locked = true;
			carScript.SynchronizeValues();
		}
	}

	override void OnStartClient( ActionData action_data )
	{
		// Car car = Car.Cast(action_data.m_Target.GetParent());
		// if( car )
		// {
		// 	car.SetAnimationPhase( m_AnimSource, 1.0);
		// }
	}
}*/

/*
class ActionTestDebugCB : ActionContinuousBaseCB
{
	override void CreateActionComponent()
	{
		m_ActionData.m_ActionComponent = new CAContinuousTime(UATimeSpent.BANDAGE);
	}
};

class ActionTestDebug: ActionContinuousBase
{	
	void ActionTestDebug()
	{
		m_CallbackClass = ActionTestDebugCB;
		m_CommandUID = DayZPlayerConstants.CMD_ACTIONFB_BANDAGE;
		m_FullBody = true;
		m_StanceMask = DayZPlayerConstants.STANCEMASK_ALL; //STANCEMASK_CROUCH;
		m_MessageStartFail = "There's nothing to bandage.";
		m_MessageStart = "I have started bandaging myself";
		m_MessageSuccess = "I have bandaged myself.";
		m_MessageFail = "I have moved and bandaging was canceled.";
		m_MessageCancel = "I stopped bandaging.";
		m_SpecialtyWeight = UASoftSkillsWeight.PRECISE_LOW;
	}

	override void CreateConditionComponents()  
	{		
		m_ConditionItem = new CCINone;
		m_ConditionTarget = new CCTNone;
	}
		
	override int GetType()
	{
		return 3555;
	}
		
	override string GetText()
	{
		return "TEST";
	}

	override bool ActionCondition( PlayerBase player, ActionTarget target, ItemBase item )
	{
		return true;
	}
	
	override void OnFinishProgressServer( ActionData action_data )
	{	
		Print("[LockUnlockVehicle] OnFinishProgressServer");
	}
};
*/

/*
class ActionTestDebugCB : ActionContinuousBaseCB
{
	override void CreateActionComponent()
	{
		m_ActionData.m_ActionComponent = new CAContinuousTime(1.0);
	}
};

class ActionTestDebug: ActionContinuousBase
{	
	void ActionTestDebug()
	{
		m_CallbackClass = ActionTestDebugCB;
		m_CommandUID = DayZPlayerConstants.CMD_ACTIONFB_INTERACT;
		m_FullBody = true;
		m_StanceMask = DayZPlayerConstants.STANCEMASK_ERECT | DayZPlayerConstants.STANCEMASK_CROUCH;
		m_MessageSuccess = "";
	}

	override void CreateConditionComponents()  
	{		
		m_ConditionItem = new CCINonRuined;
		m_ConditionTarget = new CCTNonRuined( UAMaxDistances.DEFAULT );
	}
		
	override int GetType()
	{
		return 3555;
	}
		
	override string GetText()
	{
		return "#unlock";
	}

	override bool CanBeUsedInVehicle()
	{
		return true;
	}

	override bool ActionCondition( PlayerBase player, ActionTarget target, ItemBase item )
	{
		if(!target) 
			return false;

		if (!target.GetObject().IsInherited(CarDoor))
			return false;

		CarScript carScript;
		if (Class.CastTo(carScript, target.GetParent()))
		{
			if (carScript.m_Trader_Locked && playerHasVehicleKeyInHands(player, carScript.m_Trader_VehicleKeyHash))
				return true;
		}

        return false;
	}
	
	override void OnFinishProgressServer( ActionData action_data )
	{	
		Print("[LockUnlockVehicle] OnFinishProgressServer");

		CarScript carScript = CarScript.Cast(action_data.m_Target.GetParent());
		if( carScript )
		{
			carScript.m_Trader_Locked = false;
			carScript.SynchronizeValues();
		}
	}

	protected bool playerHasVehicleKeyInHands(PlayerBase player, int vehicleKeyHash)
    {
        VehicleKey vehicleKey = VehicleKey.Cast(player.GetHumanInventory().GetEntityInHands());

        if(vehicleKey)
        {
            if(vehicleKey.hash == vehicleKeyHash)
                return true;
        }

        return false;
    }
};
*/