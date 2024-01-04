class PluginTraderData : PluginBase
{
	ref TraderDefinitionsConfig TraderDefinitionsData;
    ref TraderPresetsFiles TraderPresets;
    ref TraderCategoryFiles TraderCategories;
    ref TraderCurrencies m_TraderCurrencies;

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
            GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.LoadServerConfigs, 1000, false);
        }
    }

    void LoadServerConfigs()
    {
        LoadPresets();
        LoadCategories();
        LoadTraderDefinitionsData();
        LoadTraderCurrencies();
        DoConfigSanityCheck();
    }

    void LoadPresets()
    {
        TraderPresets = new TraderPresetsFiles;
        TraderPresets.Load();
    }

    void LoadTraderDefinitionsData()
    {        
        TraderDefinitionsData = new TraderDefinitionsConfig;    
        TraderDefinitionsData.Load();  
    }

    void LoadCategories()
    {
        TraderCategories = new TraderCategoryFiles;
        TraderCategories.Load();
    }

    void LoadTraderCurrencies()
    {
        m_TraderCurrencies = new TraderCurrencies;
        m_TraderCurrencies.Load();
    }

    void DoConfigSanityCheck()
    {
        //check if currencies from each trader matches with given currencies
        //check if the categories from each trader matches with given categories
    }

    void Client_RequestData(PlayerIdentity identity)
    {
        Param3<TraderDefinitionsConfig, TraderCategoryFiles, TraderCurrencies> serverData = new Param3<TraderDefinitionsConfig, TraderCategoryFiles, TraderCurrencies>(TraderDefinitionsData, TraderCategories, m_TraderCurrencies);
        GetGame().RPCSingleParam(identity.GetPlayer(), TRPCs.RPC_SERVER_SEND_DATA, serverData, true, identity);
    }

    void OnRPC(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
	{
        #ifdef SERVER
        ServerOnRPC(sender, rpc_type, ctx);
        #else
        ClientOnRPC(sender, rpc_type, ctx);
        #endif
    }

    void ServerOnRPC(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
    {
        switch (rpc_type)
		{
			case TRPCs.RPC_CLIENT_REQUEST_DATA:
				Client_RequestData(sender);
			break;
        }
    }

    void ClientOnRPC(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
    {
        switch (rpc_type)
		{
			case TRPCs.RPC_SERVER_SEND_DATA:
                Print("[TRADER] TRPCs.RPC_SERVER_SEND_DATA");
                Param3<TraderDefinitionsConfig, TraderCategoryFiles, TraderCurrencies> serverData = new Param3<TraderDefinitionsConfig, TraderCategoryFiles, TraderCurrencies>(TraderDefinitionsData, TraderCategories, m_TraderCurrencies);
                if (ctx.Read(serverData))
                {
                    TraderDefinitionsData = serverData.param1;
                    TraderCategories = serverData.param2;
                    m_TraderCurrencies = serverData.param3;
                    PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
                    if(player)
                    {
                        player.m_Trader_RecievedAllData = true;
                        Print("[TRADER] Received data from plugin");
                    }
                }
			break;
        }
    }

    //
    TD_Trader GetTraderByID(int ID)
    {
        if(TraderDefinitionsData)
        {
            return TraderDefinitionsData.GetTraderByID(ID);
        }
        return null;
    }
    
    TR_Trader_Category GetCategoryByName(string Name)
    {
        if(TraderCategories)
        {
            return TraderCategories.GetCategoryByName(Name);
        }
        return null;
    }
    
    TR_Trader_Currency GetCurrencyByName(string name)
    {
        if(m_TraderCurrencies)
        {
            return m_TraderCurrencies.GetCurrencyByName(name);
        }
        return null;
    }
    
    TR_Preset GetPresetByName(string name)
    {
        if(TraderPresets)
        {
            return TraderPresets.GetPresetByName(name);
        }
        return null;
    }
}