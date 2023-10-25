class SafeZoneTrigger : CylinderTrigger
{
	void SafeZoneTrigger()
	{
	}

	override bool CanAddObjectAsInsider(Object object)
	{
		PlayerBase player = PlayerBase.Cast(object);
		return player != null;
	}

	override void OnEnterServerEvent( TriggerInsider insider )
	{
		super.OnEnterServerEvent( insider );
		
		if (insider)
		{
			PlayerBase playerInsider = PlayerBase.Cast( insider.GetObject() );
			
			if (playerInsider)
				playerInsider.AddSafeZoneTrigger();
		}
	}

	override protected void OnEnterClientEvent(TriggerInsider insider) 
	{
		super.OnEnterClientEvent(insider);
		
		PlayerBase player;
		if (!Class.CastTo(player, insider.GetObject()))
			return;
		
		if (player == GetGame().GetPlayer())
			VONManager.GetInstance().SetInSafeZone(true);
	}

	override protected void OnLeaveClientEvent(TriggerInsider insider) 
	{
		super.OnLeaveClientEvent(insider);
		
		PlayerBase player;
		if (!Class.CastTo(player, insider.GetObject()))
			return;
		
		if (player == GetGame().GetPlayer())
			VONManager.GetInstance().SetInSafeZone(false);
	}

	override void OnLeaveServerEvent( TriggerInsider insider )
	{
		super.OnLeaveServerEvent( insider );
		
		if ( insider )
		{
			PlayerBase playerInsider = PlayerBase.Cast( insider.GetObject() );
			
			if ( playerInsider )
				playerInsider.RemoveSafeZoneTrigger();
		}
	}

	override bool ShouldRemoveInsider( TriggerInsider insider )
	{
		return !insider.GetObject().IsAlive();
	}
}

class SafeZone extends House
{
	CylinderTrigger m_Trigger;

	float m_Radius = 150;
	float m_PositiveHeight = 25;
	float m_NegativeHeight = 10;

	string m_TriggerType = "SafeZoneTrigger";

	vector m_Position;

	void SafeZone()
	{
		
	}

	void ~SafeZone()
	{
		
	}

	override void EEInit()
	{
		m_Position = GetWorldPosition();

		if (GetGame().IsServer())
			CreateTrigger( m_Position, m_Radius );
	}

	void CreateTrigger( vector pos, int radius )
	{
		pos[1] = pos[1] - m_NegativeHeight;
		
		if ( Class.CastTo( m_Trigger, GetGame().CreateObjectEx( m_TriggerType, pos, ECE_NONE ) ) )
		{
			m_Trigger.SetCollisionCylinder( radius, ( m_NegativeHeight + m_PositiveHeight ) );
		}
	}

	override void EEDelete( EntityAI parent )
	{
		if ( m_Trigger )
			GetGame().ObjectDelete( m_Trigger );
		
		super.EEDelete( parent );
	}
}