
static const string TraderProfileFolder = "$profile:/Trader";

class PluginTraderServerConfig : PluginBase
{
	ref TraderSafezonesConfig safeZonesConfig;
	//ref TraderObjectFiles traderObjectsConfig;
	override void OnInit()
	{     
		if (GetGame().IsServer())
		{
            Print("Initializing plugin: " + this.ToString());
            if (!FileExist(TraderProfileFolder))
            {            
                Print("[" + this.ToString() + "] " + TraderProfileFolder + "' does NOT exist, creating directory!");
                MakeDirectory(TraderProfileFolder);	
            }
            GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.LoadConfigs, 1000, false);
        }
    }

    void LoadConfigs()
    {
        SpawnSafeZones();
        SpawnTraderObjects();
    }

    void SpawnSafeZones()
    {        
        safeZonesConfig = new TraderSafezonesConfig;    
        safeZonesConfig.Load();  
    }

    bool IsPositionInSafeZone(vector position)
    {
        return safeZonesConfig.IsPositionInASafeZone(position);
    }

    void SpawnTraderObjects()
    {
        TraderObjectFiles traderObjectsConfig = new TraderObjectFiles;
        traderObjectsConfig.Load();          
    }

}