class TD_Trader
{	
    int ID;
	string ClassName;
    string DisplayName;
    string Currency;
	vector Position;
	vector Orientation;
	vector VehiclePosition;
	vector VehicleOrientation;
	bool IsNPC;
    ref array<string> Categories;
    ref array<TraderObjectAttachment> Attachments;
};

class TraderDefinitionsConfig : Managed
{
	[NonSerialized()]
	private string m_ConfigName = TraderProfileFolder + "/Traders.json";    

	protected float version;
	ref array<ref TD_Trader> Traders;

	void Default()
    {
		version = 1.0;
        ref TD_Trader defaultTrader = new TD_Trader;
        defaultTrader.ID = 0;
        defaultTrader.ClassName = "SurvivorF_Irena";
        defaultTrader.DisplayName = "Medical Supplies";
        defaultTrader.Position = "11589.292969 57.922482 14683.179688";
        defaultTrader.Orientation = "-46.066719 0 0";
        defaultTrader.IsNPC = true;
        defaultTrader.Categories = {"1","2","3"};
        defaultTrader.Currency = "Rubles";
		Traders = new array<ref TD_Trader>;
		Traders.Insert(defaultTrader);
		Save();
	}

	void Load() 
	{			
		if (!FileExist(m_ConfigName))
		{
		    TM_Print("[" + this.ToString() + "] " + m_ConfigName + "' does NOT exist, creating default config! This will be an example config, please customize.");
		    Default();
		}    
		JsonFileLoader<TraderDefinitionsConfig>.JsonLoadFile(m_ConfigName, this);
        SpawnTraderObjects();
	}

	void Save() 
	{
        JsonFileLoader<TraderDefinitionsConfig>.JsonSaveFile(m_ConfigName, this);
	}

    void SpawnTraderObjects()
    {
        foreach (TD_Trader trader : Traders)
		{
            if(trader.IsNPC)
            {
                PlayerBase npc = PlayerBase.Cast(SpawnObject(trader));
                if(npc)
                {
                    npc.SetPlayerAsTrader(trader.ID);
                    foreach(TraderObjectAttachment att : trader.Attachments)
                    {
                        att.SpawnAttachment(npc);
                    }
                }
                else
                {
                    TM_Print("TraderObject: " + trader.ClassName + " was set as NPC but it is not a PlayerBase.  Object was spawned at " + trader.Position + " but it won't have attachments spawned on it.");
                    continue;
                }

            }
            else
            {
               BuildingBase building = BuildingBase.Cast(SpawnObject(trader));
               if(building)
               {
                    building.SetBuildingAsTrader(trader.ID);
               }
            }
            //TODO: Check if the currency or category is found in the other files and error otherwise
		}
    }

	EntityAI SpawnObject(TD_Trader traderObj)
	{
		Object newtraderObj = GetGame().CreateObjectEx(traderObj.ClassName, traderObj.Position, ECE_PLACE_ON_SURFACE);
		if (newtraderObj)
		{			
			newtraderObj.SetPosition(traderObj.Position);
			newtraderObj.SetOrientation(traderObj.Orientation);
			TM_Print("TraderObject: " + traderObj.ClassName + " spawned at " + traderObj.Position);
            return newtraderObj;
		}
        TM_Print("TraderObject: " + traderObj.ClassName + "could NOT be spawned at " + traderObj.Position + ". Please check class name is correct.");
        return null;
	}

    TD_Trader GetTraderByID(int ID)
    { 
        foreach (TD_Trader trader : Traders)
		{
            if(trader.ID == ID)
            {
                return trader;
            }
        }
        return null;
    }
};