class SafeZoneTrigger : CylinderTrigger
{
	private bool m_RemoveAnimals = false;
	private bool m_RemoveInfected = false;
	private int m_ExitTimerInSeconds = 30;

	void SafeZoneTrigger()
	{
	}

	void InitSafeZone(int exitTimerInSeconds, bool shouldRemoveAnimals, bool shouldRemoveInfected)
	{
		m_ExitTimerInSeconds = exitTimerInSeconds;
		m_RemoveAnimals = shouldRemoveAnimals;
		m_RemoveInfected = shouldRemoveInfected;
	}

	override bool CanAddObjectAsInsider(Object object)
	{
		#ifdef SERVER
		DayZCreatureAI creature = DayZCreatureAI.Cast( object );
		if(creature)
		{
			if(creature.IsInherited(ZombieBase) && m_RemoveInfected)
			{
				return true;
			}
			return creature.IsInherited(AnimalBase) && m_RemoveAnimals;
		}
		else
		{
			PlayerBase player = PlayerBase.Cast(object);
			return player != null;
		}
		#else
		PlayerBase player = PlayerBase.Cast(object);
		return (player && player.IsControlledPlayer());
		#endif
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
				playerInsider.RemoveSafeZoneTrigger(m_ExitTimerInSeconds);
		}
	}

	override void OnStayServerEvent(TriggerInsider insider, float deltaTime) 
	{
		super.OnStayServerEvent( insider, deltaTime);
		DayZCreatureAI creature = DayZCreatureAI.Cast( insider.GetObject() );
		if (creature)
		{
			if(creature.IsInherited(ZombieBase) && m_RemoveInfected)
			{
				creature.SetHealth("","Health", 0);
				creature.Delete();
			}
			if(creature.IsInherited(AnimalBase) && m_RemoveAnimals)
			{
				creature.SetHealth("","Health", 0);
				creature.Delete();
			}
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