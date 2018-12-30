modded class ActionGetInTransport
{
	Transport m_transportClone;

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
	}
};
