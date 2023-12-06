class TraderObjectAttachment
{
    string ClassName;
    ref array<TraderObjectAttachment> Attachments;    

    EntityAI SpawnAttachment(EntityAI target)
    {
        if(!target)
        {
            //error?
            return null;
        }
        EntityAI attachment = EntityAI.Cast(target.GetInventory().CreateAttachment(ClassName));
        if(attachment && Attachments && Attachments.Count() > 0)
        {
            foreach(TraderObjectAttachment attSecond : Attachments)
            {
                attSecond.SpawnAttachment(attachment);
            }
        }
        if(!attachment)
        {
            attachment = EntityAI.Cast(target.GetInventory().CreateInInventory(ClassName));
        }
        return attachment;
    }
};

class TraderObject
{	
	string ClassName;
	vector Position;
	vector Orientation;
    bool IsPersistentItem;
	bool IsNPC;
    ref array<TraderObjectAttachment> Attachments;
};

class TraderObjectsConfig : Managed
{
	[NonSerialized()]
	private string m_ConfigName = TraderProfileFolder;    
	[NonSerialized()]
    private static const string m_FireBarrelName = "BarrelHoles_"; 

	protected float version;
	ref array<TraderObject> traderObjects;
    
    void SetConfigPath(string fileName)
    {
        m_ConfigName = TraderProfileFolder + "/TraderObjects/" + fileName + ".json";
    }

	void Default()
    {
		version = 1.0;
        //Example NPC with clothing
		TraderObject exampleNPC = new TraderObject;
		exampleNPC.ClassName = "SurvivorF_Irena";
		exampleNPC.Position = "3703.35 402.0 5986.36";
		exampleNPC.Orientation = "0 0 0";
		exampleNPC.IsNPC = true;
		exampleNPC.Attachments = new array<TraderObjectAttachment>;
        TraderObjectAttachment att1 = new TraderObjectAttachment;
        att1.ClassName = "BandageDressing";
        exampleNPC.Attachments.Insert(att1);
        TraderObjectAttachment att2 = new TraderObjectAttachment;
        att2.ClassName = "ParamedicJacket_Green";
        exampleNPC.Attachments.Insert(att2);
        TraderObjectAttachment att3 = new TraderObjectAttachment;
        att3.ClassName = "ParamedicPants_Green";
        exampleNPC.Attachments.Insert(att3);

        //Example FireBarrel
        TraderObject exampleFireBarrel = new TraderObject;
		exampleFireBarrel.ClassName = "BarrelHoles_Green";
		exampleFireBarrel.Position = "3705.35 402.0 5986.36";
		exampleFireBarrel.Orientation = "0 0 0";
		exampleFireBarrel.IsNPC = false;

        //Example MapObject
        TraderObject exampleMapObject = new TraderObject;
		exampleMapObject.ClassName = "Land_RoadCone";
		exampleMapObject.Position = "3701.35 402.0 5987.36";
		exampleMapObject.Orientation = "0 0 0";
		exampleMapObject.IsNPC = false;

		traderObjects = new array<TraderObject>;
		traderObjects.Insert(exampleNPC);
		traderObjects.Insert(exampleFireBarrel);
		traderObjects.Insert(exampleMapObject);
		Save();
	}

	void Load() 
	{			
		if (!FileExist(m_ConfigName))
		{
		    TM_Print("[" + this.ToString() + "] " + m_ConfigName + "' does NOT exist, creating default config! This will be an example config, please customize.");
		    Default();
		}    
		JsonFileLoader<TraderObjectsConfig>.JsonLoadFile(m_ConfigName, this);
        SpawnTraderObjects();
	}

	void Save() 
	{
        JsonFileLoader<TraderObjectsConfig>.JsonSaveFile(m_ConfigName, this);
	}

    void SpawnTraderObjects()
    {
        foreach (TraderObject traderObj : traderObjects)
		{
            bool isPersistent = traderObj.IsPersistentItem;
            if(isPersistent)
            {
				array<Object> nearby_objects = new array<Object>;
		        GetGame().GetObjectsAtPosition(traderObj.Position, 2, nearby_objects, null);
				bool foundItem = false;
				for (int i = 0; i < nearby_objects.Count(); i++)
				{
                    Object objectNearby = nearby_objects.Get(i);
                    if(objectNearby && objectNearby.IsKindOf(traderObj.ClassName))
                    {                        
						foundItem = true;
                        TM_Print("TraderObject: " + traderObj.ClassName + " was found already at " + traderObj.Position + ". Persistent item won't be spawned again.");
                        
                        BarrelHoles_ColorBase barrel = BarrelHoles_ColorBase.Cast(objectNearby);
                        if (barrel)
                        {
                            barrel.IsTraderFireBarrel = true;
                            if(!barrel.IsIgnited())
                            {
                                barrel.StartFire(true);
                            }
                            break;
                        }
                    }
				}
                
				if(!foundItem)
				{
                    Object persistentObject = SpawnObject(traderObj);
					BarrelHoles_ColorBase spawnedBarrel = BarrelHoles_ColorBase.Cast(persistentObject);
					if( spawnedBarrel )
					{				
						spawnedBarrel.IsTraderFireBarrel = true;  
                        spawnedBarrel.Open();                    
                        ItemBase firewood = ItemBase.Cast(spawnedBarrel.GetInventory().CreateAttachment("FireWood"));
                        if(firewood)
                        {
                            firewood.SetQuantity(firewood.GetQuantityMax())
                        }
                        spawnedBarrel.GetInventory().CreateAttachment("Paper");
                        if(!spawnedBarrel.IsIgnited())
                        {
                            spawnedBarrel.StartFire(true);
                        }
                        spawnedBarrel.Close();                    
					}
				}
                continue;
            }
            if(traderObj.IsNPC)
            {
                PlayerBase npc = PlayerBase.Cast(SpawnObject(traderObj));
                if(npc)
                {
                    foreach(TraderObjectAttachment att : traderObj.Attachments)
                    {
                        att.SpawnAttachment(npc);
                    }
                }
                else
                {
                    TM_Print("TraderObject: " + traderObj.ClassName + " was set as NPC but it is not a PlayerBase.  Object was spawned at " + traderObj.Position + " but it won't have attachments spawned on it.");
                    continue;
                }

            }
            else
            {
                SpawnObject(traderObj);
            }
		}
    }

	EntityAI SpawnObject(TraderObject traderObj)
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

};


class TraderObjectFiles : Managed
{
    [NonSerialized()]
	private static const string m_ConfigFolder = TraderProfileFolder + "/TraderObjects";
    [NonSerialized()]
	private static const string m_ConfigName = m_ConfigFolder + "/0_TraderObjectsFiles.json";

	protected float version;
    ref array<string> TraderObjectFilesToLoad;
    
	void Default()
    {
		version = 1.0;        
        TraderObjectsConfig config = new TraderObjectsConfig;
        config.SetConfigPath("DefaultTraderObjects");
        config.Default();
        TraderObjectFilesToLoad = new array<string>;
        TraderObjectFilesToLoad.Insert("DefaultTraderObjects");
		Save();
	}

	void Load() 
	{			
        if (!FileExist(m_ConfigFolder))
        {            
            Print("[" + this.ToString() + "] " + m_ConfigFolder + "' does NOT exist, creating directory!");
            MakeDirectory(m_ConfigFolder);	
        }
		if (!FileExist(m_ConfigName))
		{
		    TM_Print("[" + this.ToString() + "] " + m_ConfigName + "' does NOT exist, creating default config! This will be an example config, please customize.");
		    Default();
		}    
		JsonFileLoader<TraderObjectFiles>.JsonLoadFile(m_ConfigName, this);
        foreach(string fileName : TraderObjectFilesToLoad)
        {
            TraderObjectsConfig config = new TraderObjectsConfig;
            config.SetConfigPath(fileName);
            config.Load();
        }
        
	}

	void Save() 
	{
        JsonFileLoader<TraderObjectFiles>.JsonSaveFile(m_ConfigName, this);
	}
};