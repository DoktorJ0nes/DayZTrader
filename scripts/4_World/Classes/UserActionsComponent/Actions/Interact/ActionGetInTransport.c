modded class ActionGetInTransport
{
	/*override void Start( ActionData action_data )
	{
		super.Start( action_data );

		if (GetGame().IsServer())
		{
			CarScript carScript = CarScript.Cast(action_data.m_Target.GetObject());

			if(!carScript)
				return;

			carScript.m_Trader_LastDriverId = action_data.m_Player.GetIdentity().GetId();
		}		
	}*/

	/*Transport m_transportClone;

	override bool ActionCondition( PlayerBase player, ActionTarget target, ItemBase item )
	{
		if (GetGame().IsServer())
		{
			Class.CastTo(m_transportClone, target.GetObject());

			CarScript carScript;
			if ( Class.CastTo(carScript, target.GetObject()) )
			{
				if (player.GetIdentity().GetId() != carScript.m_Trader_OwnerPlayerUID && carScript.m_Trader_OwnerPlayerUID != "NO_OWNER")
					return false;
			}
		}
		
		return super.ActionCondition(player, target, item);
	}

	override void Start( ActionData action_data )
	{
		if (GetGame().IsServer())
		{
			CarScript carScript;
			if ( Class.CastTo(carScript, m_transportClone) )
				carScript.m_Trader_OwnerPlayerUID = "NO_OWNER";
		}

		super.Start( action_data );
	}*/

	/*
	Car car = Car.Cast(action_data.m_Target.GetParent());
	if( car )
	{
		car.SetAnimationPhase( m_AnimSource, 1.0);
		if ( !GetGame().IsMultiplayer() || GetGame().IsClient() )
			SEffectManager.PlaySound("offroad_door_open_SoundSet", car.GetPosition() );
	}
	*/
};
