class TR_Preset
{
    string PresetName;
    string ClassName;
    string DisplayName;
    string Description;
    ref array<ref TraderObjectAttachment> Attachments;
};

class TraderPreset
{
	[NonSerialized()]
	private string m_ConfigName = TraderProfileFolder;       

	protected float version;
    ref array<ref TR_Preset> Presets;

    void SetConfigPath(string fileName)
    {
        m_ConfigName = TraderProfileFolder + "/TraderPresets/" + fileName + ".json";
    }

    void Default()
    {
		version = 1.0;
        TR_Preset Cr = new TR_Preset;
        Cr.PresetName = "Truck_01_Covered";
        Cr.ClassName = "Truck_01_Covered";
        Cr.Attachments = new array<ref TraderObjectAttachment>;
        ref TraderObjectAttachment att1 = new TraderObjectAttachment;
        att1.ClassName = "TruckBattery";
        Cr.Attachments.Insert(att1);
        Presets = new array<ref TR_Preset>;
        Presets.Insert(Cr);
		Save();
	}

	void Load() 
	{			
		if (!FileExist(m_ConfigName))
		{
		    TM_Print("[" + this.ToString() + "] " + m_ConfigName + "' does NOT exist, creating default config! This will be an example config, please customize.");
		    Default();
		}
		JsonFileLoader<TraderPreset>.JsonLoadFile(m_ConfigName, this);
	}

	void Save() 
	{
        JsonFileLoader<TraderPreset>.JsonSaveFile(m_ConfigName, this);
	}
};


class TraderPresetsFiles : Managed
{
    [NonSerialized()]
	private static const string m_ConfigFolder = TraderProfileFolder + "/TraderPresets";
    [NonSerialized()]
	private static const string m_ConfigName = m_ConfigFolder + "/0_TraderPresetsFiles.json";
        
	protected float version;
    ref array<string> TraderPresetsFilesToLoad;
    ref array<ref TR_Preset> TraderPresets;
    
	void Default()
    {
		version = 1.0;        
        TraderPreset config = new TraderPreset;
        config.SetConfigPath("DefaultTraderPresets");
        config.Default();
        TraderPresetsFilesToLoad = new array<string>;
        TraderPresetsFilesToLoad.Insert("DefaultTraderPresets");
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
		JsonFileLoader<TraderPresetsFiles>.JsonLoadFile(m_ConfigName, this);
        TraderPresets = new array<ref TR_Preset>;
        foreach(string fileName : TraderPresetsFilesToLoad)
        {
            TraderPreset category = new TraderPreset;
            category.SetConfigPath(fileName);
            category.Load();
            foreach(TR_Preset pr : category.Presets)
            {
                Object itemObj = GetGame().CreateObjectEx(pr.ClassName, "0 0 0", ECE_PLACE_ON_SURFACE);
                if(!itemObj)
                {
                    TM_Print("Could not identify preset item: " + pr.PresetName + " with class name: " + pr.ClassName);
                    continue;
                }
                itemObj.Delete();
                TraderPresets.Insert(pr);
            }
        }
	}

	void Save() 
	{
        JsonFileLoader<TraderPresetsFiles>.JsonSaveFile(m_ConfigName, this);
	}

    TR_Preset GetPresetByName(string name)
    {
        foreach (TR_Preset preset : TraderPresets)
		{
            if(preset.PresetName == name)
            {
                return preset;
            }
        }
        return null;
    }
};