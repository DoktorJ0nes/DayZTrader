class TraderSafeZoneArea
{	
	string Name;
	vector Position;
	int Radius;
	int ExitTimerInSeconds;
	bool RemoveAnimals;
	bool RemoveInfected;
};

class TraderSafezonesConfig
{
	[NonSerialized()]
	private static const string m_ConfigName = TraderProfileFolder + "/Safezones.json";
	protected float version;
	ref array<TraderSafeZoneArea> safeZones;

	void Default()
    {
		version = 1.0;
		TraderSafeZoneArea example = new TraderSafeZoneArea;
		example.Name = "Green Mountain";
		example.Position = "3703.35 402.0 5986.36";
		example.Radius = 200;
		example.ExitTimerInSeconds = 30;
		example.RemoveAnimals = true;
		example.RemoveInfected = true;
		safeZones = new array<TraderSafeZoneArea>;
		safeZones.Insert(example);
		Save();
	}

	void Load() 
	{			
		if (!FileExist(m_ConfigName))
		{
		    TM_Print("[" + this.ToString() + "] " + m_ConfigName + "' does NOT exist, creating default config! This will be an example config, please customize.");
		    Default();
		}    
		JsonFileLoader<TraderSafezonesConfig>.JsonLoadFile(m_ConfigName, this);
		foreach (TraderSafeZoneArea safeZoneArea : safeZones)
		{
			SpawnTrigger(safeZoneArea);
		}					
	}

	void Save() 
	{
        JsonFileLoader<TraderSafezonesConfig>.JsonSaveFile(m_ConfigName, this);
	}

	void SpawnTrigger(TraderSafeZoneArea szArea)
	{
		SafeZoneTrigger newTrigger;
		if (Class.CastTo(newTrigger, GetGame().CreateObjectEx("SafeZoneTrigger", szArea.Position, ECE_NONE)))
		{
			newTrigger.SetCollisionCylinder( szArea.Radius, 100 );
			newTrigger.InitSafeZone(szArea.ExitTimerInSeconds, szArea.RemoveAnimals, szArea.RemoveInfected);
			TM_Print("Safezone " + szArea.Name + " spawned at " + szArea.Position);
		}
	}

	bool IsPositionInASafeZone(vector position)
	{
		foreach (TraderSafeZoneArea safeZoneArea : safeZones)
		{
            float distanceToSafezone = vector.Distance(position, safeZoneArea.Position);
			if(distanceToSafezone <= safeZoneArea.Radius)
			{
				return true;
			}
		}	
		return false;
	}

};