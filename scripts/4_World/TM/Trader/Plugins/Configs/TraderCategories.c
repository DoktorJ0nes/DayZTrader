class TR_Trader_Category
{	
	[NonSerialized()]
	private string m_ConfigName = TraderProfileFolder;    

	protected float version;
	string CategoryName;
    ref array<string> Items;
    
    int ID;
    ref array<ref TR_Trader_Item> TR_Items;

    void SetConfigPath(string fileName)
    {
        m_ConfigName = TraderProfileFolder + "/TraderCategories/" + fileName + ".json";
    }

	void Default()
    {
		version = 1.0;
        Items = new array<string>;
        CategoryName = "Test cat";
        Items.Insert("BoxCerealCrunchin,    *,   -1,   50");
        Items.Insert("WolfSteakMeat,   S,   -1,   80");
		Save();
	}

	void Load() 
	{			
		if (!FileExist(m_ConfigName))
		{
		    TM_Print("[" + this.ToString() + "] " + m_ConfigName + "' does NOT exist, creating default config! This will be an example config, please customize.");
		    Default();
		}    
		JsonFileLoader<TR_Trader_Category>.JsonLoadFile(m_ConfigName, this);
        TransformConfig();
	}

	void Save() 
	{
        JsonFileLoader<TR_Trader_Category>.JsonSaveFile(m_ConfigName, this);
	}

    void TransformConfig()
    {
        TR_Items = new array<ref TR_Trader_Item>;
        int count = 0 ;
        foreach(string item : Items)
        {
            ref TR_Trader_Item tItem = new TR_Trader_Item;
            bool loaded = tItem.Load(count, item);
            if(loaded)
            {
                count++;
                TR_Items.Insert(tItem);
            }
        }
    }
};


class TraderCategoryFiles : Managed
{
    [NonSerialized()]
	private static const string m_ConfigFolder = TraderProfileFolder + "/TraderCategories";
    [NonSerialized()]
	private static const string m_ConfigName = m_ConfigFolder + "/0_TraderCategoriesFiles.json";
        
	protected float version;
    ref array<string> TraderCategoryFilesToLoad;
    ref array<ref TR_Trader_Category> TraderCategories;
    
	void Default()
    {
		version = 1.0;        
        TR_Trader_Category config = new TR_Trader_Category;
        config.SetConfigPath("DefaultTraderCategories");
        config.Default();
        TraderCategoryFilesToLoad = new array<string>;
        TraderCategoryFilesToLoad.Insert("DefaultTraderCategories");
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
		JsonFileLoader<TraderCategoryFiles>.JsonLoadFile(m_ConfigName, this);
        TraderCategories = new array<ref TR_Trader_Category>;
        foreach(string fileName : TraderCategoryFilesToLoad)
        {
            ref TR_Trader_Category category = new TR_Trader_Category;
            category.SetConfigPath(fileName);
            category.Load();
            TraderCategories.Insert(category);
        }
	}

	void Save() 
	{
        JsonFileLoader<TraderCategoryFiles>.JsonSaveFile(m_ConfigName, this);
	}

    TR_Trader_Category GetCategoryByName(string Name)
    {
        foreach (TR_Trader_Category cat : TraderCategories)
		{
            if(cat.CategoryName == Name)
            {
                return cat;
            }
        }
        return null;
    }

    
    TR_Trader_Item GetTraderItemByID(int itemID)
    {
        foreach (TR_Trader_Category cat : TraderCategories)
		{        
            foreach (TR_Trader_Item item : cat.TR_Items)
            {
                if(item.ID == itemID)
                {
                    return item;
                }
            }
        }
        return null;
    }
};