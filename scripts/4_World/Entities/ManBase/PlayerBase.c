modded class PlayerBase
{
    protected bool m_IsInSafeZone = false;
    bool m_Trader_IsTrader = false;
    int m_Trader_TraderID = -1;
	protected int m_SafeZoneCount = 0;
	
	bool m_Trader_TraderModIsLoaded = false;
	bool m_Trader_TraderModIsLoadedHandled = false;
	
	bool m_Trader_IsInSafezone = false;
	
	bool m_Trader_RecievedAllData = false;
	
	ref array<string> m_Trader_AdminPlayerUIDs;

//Client
	ref TraderNotifications m_Trader_TraderNotifications;
    ref TraderMenu m_TraderMenu;
	
	void PlayerBase()
	{
		RegisterNetSyncVariableBool("m_IsInSafeZone");
        RegisterNetSyncVariableBool("m_Trader_IsTrader");
        RegisterNetSyncVariableInt("m_Trader_TraderID");
	}

	bool IsInSafeZone()
	{
		return m_IsInSafeZone;
	}

	void AddSafeZoneTrigger()
	{
		if (!GetGame().IsServer())
			return;

		m_SafeZoneCount++;
		//Print("Adding to count: " + m_SafeZoneCount + ", in safezone: " + m_IsInSafeZone);
		if(m_SafeZoneCount > 0)
		{
			SendToGameLabsTrader(this, "", "", "has entered a safezone");

			TraderMessage.DeleteSafezoneMessages(this);
			TraderMessage.PlayerRed("#tm_entered_safezone", this);
			if (!m_IsInSafeZone)
			{
				SetAllowDamage(false);
				SetInSafeZone(true);
			}			
		}
	}

	void RemoveSafeZoneTrigger(int exitTimer)
	{
		if (!GetGame().IsServer())
			return;

		if (m_SafeZoneCount > 0)
		{
			m_SafeZoneCount--;
			//Print("Removing from count: " + m_SafeZoneCount + ", in safezone: " + m_IsInSafeZone);
		}
		
		if (m_IsInSafeZone && m_SafeZoneCount == 0)
		{
			TraderMessage.SafezoneExit(this, exitTimer);
			if(exitTimer > 0)
			{
				GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(OnExitSafeZoneCountdownComplete, exitTimer * 1000, false);
			}
			else
			{
				OnExitSafeZoneCountdownComplete();
			}
		}
	}

	protected void OnExitSafeZoneCountdownComplete()
	{
		TraderMessage.DeleteSafezoneMessages(this);
		TraderMessage.PlayerRed("#tm_left_safezone", this);

		SendToGameLabsTrader(this, "", "", "has left a safezone");
		
		bool godModeVPP, godModeCOT;
		EnScript.GetClassVar(this, "hasGodmode", 0, godModeVPP);
		EnScript.GetClassVar(this, "m_COT_GodMode", 0, godModeCOT);

		if (!godModeVPP && !godModeCOT)
			SetAllowDamage(true);

		SetInSafeZone(false);
	}

	void SetInSafeZone(bool state)
	{
		m_IsInSafeZone = state;
		SetSynchDirty();
	}
	
	bool Trader_IsAdmin()
    {
        // for (int i = 0; i < m_Trader_AdminPlayerUIDs.Count(); i++)
        // {
        //     if (m_Trader_AdminPlayerUIDs.Get(i) == m_Trader_PlayerUID)
        //         return true;
        // }

        return false;
    }

	void SetPlayerAsTrader(int traderID)
	{
		m_Trader_IsTrader = true;
		m_Trader_TraderID = traderID;
		SetAllowDamage(false);
		SetSynchDirty();
	}

	override bool CanBeTargetedByAI( EntityAI ai )
	{
		if(m_Trader_IsTrader)
		{
			return false;
		}
		return super.CanBeTargetedByAI(ai);
	}
	
	override bool CanBeRestrained()
	{
		if(m_Trader_IsTrader)
		{
			return false;
		}
		return super.CanBeRestrained();
	}

	override bool IsInventoryVisible()
	{
		if(m_Trader_IsTrader)
		{
			return false;
		}
		return super.IsInventoryVisible();
	}

	override bool CanDisplayCargo()
	{
		if(m_Trader_IsTrader)
		{
			return false;
		}
		return super.CanDisplayCargo();
	}
	override void OnRPC(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
	{
		super.OnRPC(sender, rpc_type, ctx);
		PluginTraderData traderData = PluginTraderData.Cast(GetPlugin(PluginTraderData));
        if(traderData)
        {
			traderData.OnRPC(sender, rpc_type, ctx);
		}
		
		if (GetGame().IsServer())
			handleServerRPCs(sender, rpc_type, ctx);
		else
			handleClientRPCs(sender, rpc_type, ctx);
	};	

	override void SetSuicide(bool state)
	{
		super.SetSuicide(state);

		if (state && m_Trader_IsInSafezone && GetGame().IsServer())
			SetAllowDamage(true);
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// SERVER RPC HANDLING
	void handleServerRPCs(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
	{
		if (rpc_type == TRPCs.RPC_TRADER_MOD_IS_LOADED && !m_Trader_TraderModIsLoaded)
				handleTraderModIsLoadedRPC(sender, rpc_type, ctx);

		if (rpc_type == TRPCs.RPC_BUY)
			handleBuyRPC(sender, rpc_type, ctx);

		if (rpc_type == TRPCs.RPC_SELL)
			handleSellRPC(sender, rpc_type, ctx);
	}

	void handleTraderModIsLoadedRPC(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
	{
		Param1<PlayerBase> rp0 = new Param1<PlayerBase>( NULL );
		ctx.Read(rp0);
		
		PlayerBase player = rp0.param1;
		
		m_Trader_TraderModIsLoaded = true;
		
		GetGame().RPCSingleParam(player, TRPCs.RPC_TRADER_MOD_IS_LOADED_CONFIRM, new Param1<PlayerBase>( player ), true, player.GetIdentity());
	}

	void handleBuyRPC(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
	{
		PluginTraderData traderDataPlugin = PluginTraderData.Cast(GetPlugin(PluginTraderData));
        if(traderDataPlugin == null)
        {
			return;
		}
		Param2<int, TR_Trader_Item> rpb = new Param2<int, TR_Trader_Item>(-1, null);
		ctx.Read(rpb);

		int traderUID = rpb.param1;
		TR_Trader_Item item = rpb.param2;
		TD_Trader trader = traderDataPlugin.GetTraderByID(traderUID);
		if(trader == NULL || item == NULL)
		{
			return;
		}

		vector playerPosition = GetPosition();

		if (vector.Distance(playerPosition, trader.Position) > 1.7)
		{
			traderServerLog("tried to access the Trader out of Range! This could be an Hacker!");
			return;
		}

		int currencyAmount = TraderLibrary.GetPlayerCurrencyAmount(trader, PlayerBase.Cast(this));

		if (item.BuyPrice < 0)
		{
			TraderMessage.PlayerWhite("#tm_cant_be_bought", PlayerBase.Cast(this));
			return;
		}

		if (currencyAmount < item.BuyPrice)
		{
			TraderMessage.PlayerWhite("#tm_cant_afford", PlayerBase.Cast(this));
			return;
		}
		PlayerBase player = PlayerBase.Cast(this);
		switch (item.Type)
		{
			case TR_Item_Type.Vehicle:
				string blockingObject = IsVehicleSpawnFree(trader);

				if (blockingObject != "FREE")
				{
					TraderMessage.PlayerWhite(TraderLibrary.GetItemDisplayName(blockingObject) + " " + "#tm_way_blocked", player);
					return;
				}

				TraderLibrary.DeductPlayerCurrency(player, trader, item.BuyPrice);

				TraderMessage.PlayerWhite("" + TraderLibrary.GetItemDisplayName(item.ClassName) + "\n" + "#tm_parked_next_to_you", player);

				SpawnVehicle(trader, item);

				GetGame().RPCSingleParam(player, TRPCs.RPC_SEND_MENU_BACK, new Param1<bool>(false), true, GetIdentity());
				break;
			
			default:
				TraderLibrary.DeductPlayerCurrency(player, trader, item.BuyPrice);
				TraderLibrary.CreateItemInPlayerInventory(player, item, item.Quantity);
				break;
		}
	}

	void handleSellRPC(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
	{
		PluginTraderData traderDataPlugin = PluginTraderData.Cast(GetPlugin(PluginTraderData));
        if(traderDataPlugin == null)
        {
			return;
		}
		Param2<int, TR_Trader_Item> rpb = new Param2<int, TR_Trader_Item>(-1, null);
		ctx.Read(rpb);

		int traderUID = rpb.param1;
		TR_Trader_Item item = rpb.param2;
		TD_Trader trader = traderDataPlugin.GetTraderByID(traderUID);
		if(trader == null || item == null)
		{
			return;
		}

		vector playerPosition = GetPosition();	

		if (vector.Distance(playerPosition, trader.Position) > 1.7)
		{
			traderServerLog("tried to access the Trader out of Range! This could be an Hacker!");
			return;
		}

		PlayerBase playerbase = PlayerBase.Cast(this);
		Object vehicleToSell = TraderLibrary.GetVehicleToSell(this, traderUID, item.ClassName);

		if (item.SellPrice < 0)
		{
			TraderMessage.PlayerWhite("#tm_cant_be_sold", playerbase);
			return;
		}

		if (!TraderLibrary.IsInPlayerInventory(playerbase, item, item.Quantity) && !vehicleToSell)
		{
			TraderMessage.PlayerWhite("#tm_you_cant_sell", playerbase);

			if (item.Type == TR_Item_Type.Vehicle)
			{
				TraderMessage.PlayerWhite("#tm_cant_sell_vehicle", playerbase);
				//TraderMessage.PlayerWhite("Turn the Engine on and place it inside the Traffic Cones!", PlayerBase.Cast(this));
			}

			return;
		}

		traderServerLog("#tm_sold" + " " + TraderLibrary.GetItemDisplayName(item.ClassName) + " (" + item.ClassName + ")");

		TraderMessage.PlayerWhite("" + TraderLibrary.GetItemDisplayName(item.ClassName) + "\n" + "#tm_was_sold", PlayerBase.Cast(this));

		if (vehicleToSell)
		{
			GetGame().ObjectDelete(vehicleToSell);
		}
		else
		{
			TraderLibrary.RemoveFromPlayerInventory(playerbase, item, item.Quantity);
		}
		
		TraderLibrary.IncreasePlayerCurrency(playerbase, trader, item.SellPrice);
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// CLIENT RPC HANDLING
	void handleClientRPCs(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
	{
		switch(rpc_type)
		{			
			case TRPCs.RPC_SEND_MENU_BACK:
				handleSendMenuBackRPC(sender, rpc_type, ctx);
			break;

			case TRPCs.RPC_SEND_NOTIFICATION:
				HandleSendNotificationRPC(sender, rpc_type, ctx);
			break;

			case TRPCs.RPC_DELETE_SAFEZONE_MESSAGES:					
				DeleteAllMessages();
			break;
		}
	}

	
	void handleTraderModIsLoadedConfirmRPC(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
	{
		m_Trader_TraderModIsLoaded = true;
		m_Trader_TraderModIsLoadedHandled = true;
	}

	void handleSyncObjectOrientationRPC(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
	{
		Param2<Object, vector> syncObject_rp = new Param2<Object, vector>( NULL, "0 0 0" );
		ctx.Read( syncObject_rp );
		
		Object objectToSync = syncObject_rp.param1;
		vector objectToSyncOrientation  = syncObject_rp.param2;

		objectToSync.SetOrientation(objectToSyncOrientation);
	}

	void handleSendMenuBackRPC(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
	{
		GetGame().GetUIManager().Back();
	}

	void HandleSendNotificationRPC(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
	{
		Param4<string, float, int, bool> msg = new Param4<string, float, int, bool>( "", 0, 0, false);
		ctx.Read( msg );
		if(msg.param4)
		{
			ShowExitSafezoneMessage(msg.param2);
		}
		else
		{
			ShowTraderMessage(msg.param1, msg.param2, msg.param3);
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// LOGS & MESSAGES
	void traderServerLog(string message)
	{
		TM_Print("[TRADER] Player: (" + GetIdentity().GetName() + ") " + GetIdentity().GetId() + " " + message);
	}

	void ShowTraderMessage(string message, float time, int color = 0)
	{
		if (!m_Trader_TraderNotifications)
		{
			m_Trader_TraderNotifications = new TraderNotifications();
			m_Trader_TraderNotifications.Init();
		}

		m_Trader_TraderNotifications.ShowMessage(message, time, color);
	}

	void ShowExitSafezoneMessage(float time)
	{
		if (!m_Trader_TraderNotifications)
		{
			m_Trader_TraderNotifications = new TraderNotifications();
			m_Trader_TraderNotifications.Init();
		}

		m_Trader_TraderNotifications.ShowExitSafezoneMessage(time);
	}

	void DeleteAllMessages()
	{
		if (!m_Trader_TraderNotifications)
			return;

		m_Trader_TraderNotifications.DeleteAllMessages();
	}

	//TODO: We need something like this but better
	//removed for now
	// void DeleteItemAndOfferRewardsForAllAttachments(ItemBase item)
	// {
	// 	int totalSellValue = 0;
	// 	for ( int i = 0; i < item.GetInventory().AttachmentCount(); i++ )
	// 	{
	// 		EntityAI attachment = item.GetInventory().GetAttachmentFromIndex ( i );
	// 		int itemID = m_Trader_ItemsClassnames.Find(attachment.GetType());
	// 		if ( itemID != -1 )
	// 		{
	// 			int itemQuantity = m_Trader_ItemsQuantity.Get(itemID);
	// 			int itemSellValue = m_Trader_ItemsSellValue.Get(itemID);
	// 			if (itemSellValue < 0)
	// 				continue;
	// 			totalSellValue += itemSellValue;
	// 		}
	// 	}
	// 	IncreasePlayerCurrency(totalSellValue);
	// 	item.Delete();
	// }

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// CURRENCY
	

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// VEHICLE
	string IsVehicleSpawnFree(TD_Trader trader)
	{
		//TODO: check size and make it more leanient
		vector size = "3 5 9";
		array<Object> excluded_objects = new array<Object>;
		array<Object> nearby_objects = new array<Object>;

		GetGame().IsBoxColliding(trader.VehiclePosition, trader.VehicleOrientation, size, excluded_objects, nearby_objects);
		if (nearby_objects.Count() > 0)
			return nearby_objects.Get(0).GetType();

		return "FREE";
	}

	void SpawnVehicle(TD_Trader trader, TR_Trader_Item item)
	{
		EntityAI vehicle = EntityAI.Cast(GetGame().CreateObjectEx(item.ClassName, trader.VehiclePosition, ECE_LOCAL | ECE_CREATEPHYSICS | ECE_TRACE));
		if(!vehicle)
		{
			//throw error
			return;
		}			
		GetGame().RemoteObjectCreate(vehicle);
		vehicle.SetOrientation(trader.VehicleOrientation);
		foreach(TraderObjectAttachment att : item.Attachments)
		{
			att.SpawnAttachment(vehicle);
		}

		// Try to fill Fuel, Oil, Brakeliquid, Coolantliquid and lock Vehicle:
		CarScript car = CarScript.Cast(vehicle);
		if (car)
		{
			car.Fill( CarFluid.FUEL, car.GetFluidCapacity( CarFluid.FUEL ));
			car.Fill( CarFluid.OIL, car.GetFluidCapacity( CarFluid.OIL ));
			car.Fill( CarFluid.BRAKE, car.GetFluidCapacity( CarFluid.BRAKE ));
			car.Fill( CarFluid.COOLANT, car.GetFluidCapacity( CarFluid.COOLANT ));

			car.Fill( CarFluid.USER1, car.GetFluidCapacity( CarFluid.USER1 ));
			car.Fill( CarFluid.USER2, car.GetFluidCapacity( CarFluid.USER2 ));
			car.Fill( CarFluid.USER3, car.GetFluidCapacity( CarFluid.USER3 ));
			car.Fill( CarFluid.USER4, car.GetFluidCapacity( CarFluid.USER4 ));
			car.m_Trader_IsInSafezone = true;

			//TODO: MCK integration to lock vehicle?
			// if (vehicleKeyHash != 0)
			// {
			// 	car.m_Trader_Locked = true;
			// 	car.m_Trader_HasKey = true;
			// 	car.m_Trader_VehicleKeyHash = vehicleKeyHash;
			// }

			car.SynchronizeValues();
			car.SetAllowDamage(false);
		}
	}

    override void SetActions(out TInputActionMap InputActionMap)
	{
		super.SetActions(InputActionMap);

		AddAction(ActionUnlockVehicle, InputActionMap);
        AddAction(ActionLockVehicle, InputActionMap);
        AddAction(ActionUnlockVehicleInside, InputActionMap);
        AddAction(ActionLockVehicleInside, InputActionMap);
        AddAction(ActionTrade, InputActionMap);
	}
}