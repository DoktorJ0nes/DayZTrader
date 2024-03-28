class SafeZoneTrigger : CylinderTrigger
{
	private bool m_RemoveAnimals = false;
	private bool m_RemoveInfected = false;
	private bool m_RemoveEAI = false;
	private int m_ExitTimerInSeconds = 30;

	override void EEInit()
	{
		super.EEInit();		
		#ifndef SERVER
			GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(this.SpawnCylinderShape, 1000, true);
		#endif
	}

	void SpawnCylinderShape()
	{		
		if(GetGame().IsClient())
		{
			PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
			if(player && player.HasReceivedAllTraderData())
			{
				GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).Remove(this.SpawnCylinderShape);
				if(player.m_Trader_SafezoneShowDebugShapes)
				{
					float radius = GetCollisionRadius();
					//float height = 100;		
					array<int> coloursToChoose = {COLOR_RED, COLOR_GREEN, COLOR_BLUE, COLOR_YELLOW, COLOR_RED_A, COLOR_GREEN_A, COLOR_GREEN_A, COLOR_BLUE_A, COLOR_YELLOW_A};
					int color = coloursToChoose.GetRandomElement();	
					vector minMax[2];
					GetCollisionBox(minMax);
					float height = Math.AbsFloat(minMax[0][1]) + Math.AbsFloat(minMax[1][1]);
					vector pos = GetPosition();
					pos[1] = pos[1] + (height * 0.5);
					Debug.DrawCylinder(pos, radius, height, color, ShapeFlags.VISIBLE|ShapeFlags.DOUBLESIDE);
					Print("Got debug shape with radius " + radius );
				}
			}
			
		}
	}

	void InitSafeZone(int exitTimerInSeconds, bool shouldRemoveAnimals, bool shouldRemoveInfected, bool shouldRemoveEAI)
	{
		m_ExitTimerInSeconds = exitTimerInSeconds;
		m_RemoveAnimals = shouldRemoveAnimals;
		m_RemoveInfected = shouldRemoveInfected;
		m_RemoveEAI = shouldRemoveEAI;
	}

	override bool CanAddObjectAsInsider(Object object)
	{
		#ifdef SERVER
		CarScript vehicle = CarScript.Cast(object);
		if(vehicle)
		{
			return true;
		}
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
			if(player)
			{				
				#ifdef ENFUSION_AI_PROJECT
				if(player.IsAI() && m_RemoveEAI)
				{
					return true;
				}
				#endif
				if(player.GetIdentity() != null)
				{
					return !player.IsTrader();
				}
				else
				{
					return false;
				}
			}
		}
		return false;
		#else
		PlayerBase player = PlayerBase.Cast(object);
		return (player && player.IsControlledPlayer() && !player.IsTrader());
		#endif
	}

	override void OnEnterServerEvent( TriggerInsider insider )
	{
		super.OnEnterServerEvent( insider );
		
		if (insider)
		{
			PlayerBase playerInsider = PlayerBase.Cast( insider.GetObject() );			
			if (playerInsider)
			{
				#ifdef ENFUSION_AI_PROJECT		
				if (playerInsider && playerInsider.IsAI())
				{
					return;
				}
				#endif
				playerInsider.AddSafeZoneTrigger();
				return;
			}
			CarScript vehicle = CarScript.Cast(insider.GetObject());
			if(vehicle)
			{
				vehicle.SetIsInSafezone(true);
			}
		}
	}

	override protected void OnEnterClientEvent(TriggerInsider insider) 
	{
		super.OnEnterClientEvent(insider);
		
		PlayerBase player;
		if (!Class.CastTo(player, insider.GetObject()))
			return;
		
		#ifdef ENFUSION_AI_PROJECT
		if (player.IsAI())
			return;
		#endif
		if (player == GetGame().GetPlayer())
			VONManager.GetInstance().SetInSafeZone(true);
	}

	override protected void OnLeaveClientEvent(TriggerInsider insider) 
	{
		super.OnLeaveClientEvent(insider);
		
		PlayerBase player;
		if (!Class.CastTo(player, insider.GetObject()))
			return;
		
		#ifdef ENFUSION_AI_PROJECT
		if (player.IsAI())
			return;
		#endif
		if (player == GetGame().GetPlayer())
			VONManager.GetInstance().SetInSafeZone(false);
	}

	override void OnLeaveServerEvent( TriggerInsider insider )
	{
		super.OnLeaveServerEvent( insider );
		
		if (insider)
		{
			PlayerBase playerInsider = PlayerBase.Cast(insider.GetObject());			
			if (playerInsider)
			{	
				#ifdef ENFUSION_AI_PROJECT
				if (playerInsider.IsAI())
					return;
				#endif
				playerInsider.RemoveSafeZoneTrigger(m_ExitTimerInSeconds);
				return;
			}
			
			CarScript vehicle = CarScript.Cast(insider.GetObject());
			if(vehicle)
			{
				vehicle.SetIsInSafezone(false);
			}
		}
	}

	override void OnStayServerEvent(TriggerInsider insider, float deltaTime) 
	{
		super.OnStayServerEvent( insider, deltaTime);		
		#ifdef ENFUSION_AI_PROJECT
		PlayerBase playerInsider = PlayerBase.Cast(insider.GetObject());			
		if (playerInsider && playerInsider.IsAI())
		{
			playerInsider.SetHealth("","Health", 0);
			playerInsider.Delete();
			return;
		}
		#endif
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
