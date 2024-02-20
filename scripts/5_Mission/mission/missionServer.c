modded class MissionServer
{
	static const string m_Trader_ConfigFilePath = "$profile:Trader/TraderConfig.txt";
	static const string m_Trader_ObjectsFilePath = "$profile:Trader/TraderObjects.txt";
	static const string m_Trader_VehiclePartsFilePath = "$profile:Trader/TraderVehicleParts.txt";
	static const string m_Trader_VariableFilePath = "$profile:Trader/TraderVariables.txt";
	static const string m_Trader_AdminsFilePath = "$profile:Trader/TraderAdmins.txt";

	float m_Trader_SafezoneTimeout = 30;
	bool m_Trader_SafezoneRemoveAnimals = false;
	bool m_Trader_SafezoneRemoveInfected = false;
	bool m_Trader_SafezoneShowDebugShapes = false;
	float m_Trader_TradingDistance = 3.0;

	bool m_Trader_ReadAllTraderData = false;

	string m_Trader_CurrencyName;
	ref array<string> m_Trader_CurrencyClassnames;
	ref array<int> m_Trader_CurrencyValues;
	
	ref array<string> m_Trader_TraderNames;
	ref array<vector> m_Trader_TraderPositions;
	ref array<int> m_Trader_TraderIDs;
	ref array<int> m_Trader_TraderSafezones;
	ref array<vector> m_Trader_TraderVehicleSpawns;
	ref array<vector> m_Trader_TraderVehicleSpawnsOrientation;
	
	ref array<string> m_Trader_Categorys;
	ref array<int> m_Trader_CategorysTraderKey;
	
	ref array<int> m_Trader_ItemsTraderId;
	ref array<int> m_Trader_ItemsCategoryId;
	ref array<string> m_Trader_ItemsClassnames;
	ref array<int> m_Trader_ItemsQuantity;
	ref array<int> m_Trader_ItemsBuyValue;
	ref array<int> m_Trader_ItemsSellValue;	

	ref array<string> m_Trader_Vehicles;
	ref array<string> m_Trader_VehiclesParts;
	ref array<int> m_Trader_VehiclesPartsVehicleId;

	ref array<string> m_Trader_AdminPlayerUIDs;
	ref array<string> m_Trader_NPCDummyClasses;

	float m_Trader_BuySellTimer = 0.3;
		
	ref array<PlayerBase> m_Trader_SpawnedTraderCharacters;

	float m_Trader_VehicleCleanupUpdateTimerMax = 15 * 60;
	float m_Trader_VehicleCleanupUpdateTimer = m_Trader_VehicleCleanupUpdateTimerMax;
	ref array<Object> m_Trader_ObjectsList;
	
	override void OnInit()
	{		
		super.OnInit(); 		
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.LoadServerConfigs, 1000, false);
	}

    void LoadServerConfigs()
    {
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(this.LoadServerConfigs);
		TraderMessage.ServerLog("[TRADER] LOADING TRADER CONFIG");
		SpawnTraderObjects();
		readTraderVariables();
		readTraderData();
		readTraderAdmins();
		g_Game.SetTraderPositionsAndSafezones(m_Trader_TraderPositions, m_Trader_TraderSafezones);
		TraderMessage.ServerLog("[TRADER] FINISHED LOADING TRADER CONFIG");
    }

	override void HandleBody(PlayerBase player)
    {
		if (player.IsUnconscious() || player.IsRestrained())
		{
			if(player.IsInSafeZone())
			{
            	player.SetAllowDamage(true);
			}
		}

        super.HandleBody(player);
    }

	override void OnUpdate(float timeslice)
	{
		super.OnUpdate(timeslice);
		if(m_Trader_VehicleCleanupUpdateTimerMax > 0)
		{
			m_Trader_VehicleCleanupUpdateTimer += timeslice;
			if (m_Trader_VehicleCleanupUpdateTimer >= m_Trader_VehicleCleanupUpdateTimerMax && m_Trader_TraderVehicleSpawns)
			{
				m_Trader_VehicleCleanupUpdateTimer = 0;

				for (int p = 0; p < m_Trader_TraderVehicleSpawns.Count(); p++)
				{
					vector size = "3 5 9";
					array<Object> excludedObjects = new array<Object>;
					array<Object> collidedObjects = new array<Object>;
					//is this working?
					if (GetGame().IsBoxColliding(m_Trader_TraderVehicleSpawns.Get(p), m_Trader_TraderVehicleSpawnsOrientation.Get(p), size, excludedObjects, collidedObjects))
					{
						for (int q = 0; q < collidedObjects.Count(); q++)
						{
							CarScript carScript = CarScript.Cast(collidedObjects.Get(q));
							if (!carScript)
								continue;

							if (carScript.m_Trader_Locked)
							{
								carScript.m_Trader_CleanupCount++;

								if (carScript.m_Trader_CleanupCount >= 3)
								{
									carScript.m_Trader_CleanupCount = 0;
									carScript.m_Trader_Locked = false;
								}

								carScript.SynchronizeValues();
							}
						}
					}
				}
			}
		}
	}
	
	void SpawnTraderObjects()
	{
		m_Trader_NPCDummyClasses = new array<string>;
		m_Trader_ObjectsList = new array<Object>;
		TraderMessage.ServerLog("[TRADER] READING TRADER OBJECTS FILE");
		FileHandle file_index = OpenFile(m_Trader_ObjectsFilePath, FileMode.READ);
				
		if ( file_index == 0 )
		{
			TraderMessage.ServerLog( "[TRADER] FOUND NO TRADEROBJECTS FILE!" );
			return;
		}
		
		int markerCounter = 0;
		bool skipDirEntry = false;
		
		string line_content = "";
		while ( markerCounter <= 5000 && line_content.Contains("<FileEnd>") == false)
		{
			// Get Object Type ------------------------------------------------------------------------------------
			if (skipDirEntry)
				skipDirEntry = false;
			else
				line_content = FileReadHelper.SearchForNextTermInFile(file_index, "<Object>", "<FileEnd>");
			
			if (!line_content.Contains("<Object>"))
				continue;
			
			line_content.Replace("<Object>", "");
			line_content = FileReadHelper.TrimComment(line_content);
			line_content = FileReadHelper.TrimSpaces(line_content);
			
			
			string traderObjectType = line_content;
			TraderMessage.ServerLog("[TRADER] OBJECT TYPE ENTRY " + line_content);
			
			// Get Object Position --------------------------------------------------------------------------------
			line_content = FileReadHelper.SearchForNextTermInFile(file_index, "<ObjectPosition>", "<FileEnd>");
			
			line_content.Replace("<ObjectPosition>", "");
			line_content = FileReadHelper.TrimComment(line_content);
			
			//TraderMessage.ServerLog("[TRADER] READING OBJECT POSITION ENTRY..");
			
			TStringArray strso = new TStringArray;
			line_content.Split( ",", strso );
			
			string traderObjectPosX = strso.Get(0);
			traderObjectPosX = FileReadHelper.TrimSpaces(traderObjectPosX);
			
			string traderObjectPosY = strso.Get(1);
			traderObjectPosY = FileReadHelper.TrimSpaces(traderObjectPosY);
			
			string traderObjectPosZ = strso.Get(2);
			traderObjectPosZ = FileReadHelper.TrimSpaces(traderObjectPosZ);
			
			vector objectPosition = "0 0 0";
			objectPosition[0] = traderObjectPosX.ToFloat();
			objectPosition[1] = traderObjectPosY.ToFloat();
			objectPosition[2] = traderObjectPosZ.ToFloat();

			TraderMessage.ServerLog("[TRADER] OBJECT POSITION = '" + objectPosition + "'");
			// Get Object Orientation -------------------------------------------------------------------------------
			line_content = FileReadHelper.SearchForNextTermInFile(file_index, "<ObjectOrientation>", "<FileEnd>");
			
			line_content.Replace("<ObjectOrientation>", "");
			line_content = FileReadHelper.TrimComment(line_content);
			
			//TraderMessage.ServerLog("[TRADER] READING OBJECT ORIENTATION ENTRY..");

			TStringArray strsod = new TStringArray;
			line_content.Split( ",", strsod );
			
			string traderObjectOriX = strsod.Get(0);
			traderObjectOriX = FileReadHelper.TrimSpaces(traderObjectOriX);
			
			string traderObjectOriY = strsod.Get(1);
			traderObjectOriY = FileReadHelper.TrimSpaces(traderObjectOriY);
			
			string traderObjectOriZ = strsod.Get(2);
			traderObjectOriZ = FileReadHelper.TrimSpaces(traderObjectOriZ);
			
			vector objectOrientation = vector.Zero;
			objectOrientation[0] = traderObjectOriX.ToFloat();
			objectOrientation[1] = traderObjectOriY.ToFloat();
			objectOrientation[2] = traderObjectOriZ.ToFloat();

			TraderMessage.ServerLog("[TRADER] OBJECT ORIENTATION = '" + objectOrientation + "'");

			// HANDLE TRADER PERSISTENT ITEMS BEGIN
			array<Object> persistanceCheck_nearby_objects = new array<Object>;
			GetGame().GetObjectsAtPosition(objectPosition, 2, persistanceCheck_nearby_objects, null);
			bool foundItem = false;
			Object objectNearby ;
			//TraderMessage.ServerLog("TraderObject: persistanceCheck_nearby_objects " + persistanceCheck_nearby_objects.Count().ToString() + " was found at " + objectPosition);
			for (int i = 0; i < persistanceCheck_nearby_objects.Count(); i++)
			{
				objectNearby = persistanceCheck_nearby_objects.Get(i);
				//TraderMessage.ServerLog("TraderObject: " + objectNearby.GetType() + " was found at " + objectPosition);
				if(objectNearby && objectNearby.IsKindOf(traderObjectType))
				{                        
					foundItem = true;
					TraderMessage.ServerLog("TraderObject: " + traderObjectType + " was found already at " + objectPosition + ". Persistent item won't be spawned again.");
					m_Trader_ObjectsList.Insert(objectNearby);
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
			Object newtraderObj;
			bool isTrader = false;	
			PlayerBase man;
			if(!foundItem)
			{
				newtraderObj = GetGame().CreateObjectEx(traderObjectType, objectPosition, ECE_SETUP | ECE_UPDATEPATHGRAPH | ECE_CREATEPHYSICS | ECE_NOPERSISTENCY_WORLD);
				if (newtraderObj)
				{			
					m_Trader_ObjectsList.Insert(newtraderObj);
					newtraderObj.SetPosition(objectPosition);
					newtraderObj.SetOrientation(objectOrientation);
					EntityAI entity = EntityAI.Cast(newtraderObj);
					if(entity)
					{
						entity.SetLifetime(316224000);
                    	entity.SetLifetimeMax(316224000);
					}
					TraderMessage.ServerLog("TraderObject: " + traderObjectType + " spawned at " + objectPosition);

					BarrelHoles_ColorBase spawnedBarrel = BarrelHoles_ColorBase.Cast(newtraderObj);
					if( spawnedBarrel )
					{				
						spawnedBarrel.IsTraderFireBarrel = true;  
						spawnedBarrel.Open();                    
						ItemBase firewood = ItemBase.Cast(spawnedBarrel.GetInventory().CreateAttachment("FireWood"));
						if(firewood)
						{
							firewood.SetQuantity(firewood.GetQuantityMax());
						}
						spawnedBarrel.GetInventory().CreateAttachment("Paper");
						if(!spawnedBarrel.IsIgnited())
						{
							spawnedBarrel.StartFire(true);
						}
						spawnedBarrel.Close();   
						spawnedBarrel.SoundSynchRemoteReset();                 
					}				
					if (Class.CastTo(man, newtraderObj))
					{
						TraderMessage.ServerLog("[TRADER] Object was a Man..");
						isTrader = true;
						man.SetAllowDamage(false);
					}
				}
				else
				{
					TraderMessage.ServerLog("TraderObject: " + traderObjectType + " could NOT be spawned at " + objectPosition + ". Please check class name is correct.");
				}       			
			}			

			int attachmentCounter = 0;
			while ( attachmentCounter <= 1000 && line_content.Contains("<Object>") == false)
			{
				line_content = FileReadHelper.SearchForNextTermsInFile(file_index, {"<ObjectAttachment>", "<OpenFile>"}, "<Object>");

				if (line_content == string.Empty)	
				{
					line_content = "<FileEnd>";
					break;
				}

				if (line_content.Contains("<OpenFile>"))
				{
					if (OpenNewFileForReading(line_content, file_index))
						continue;
					else
						return;
				}

				if (line_content.Contains("<Object>"))
				{
					skipDirEntry = true;
					markerCounter++;
					break;
				}

				//if (!line_content.Contains("<ObjectAttachment>"))
				//	continue;

				line_content.Replace("<ObjectAttachment>", "");
				line_content = FileReadHelper.TrimComment(line_content);
				
				//TraderMessage.ServerLog("[TRADER] READING OBJECT ATTACHMENT ENTRY..");

				if (isTrader)
				{
					man.GetInventory().CreateInInventory(line_content);
					TraderMessage.ServerLog("[TRADER] '" + line_content + "' WAS ATTACHED");
				}
				else
				{
					if (line_content == "NPC_DUMMY")
					{
						if(foundItem)
						{
							RegisterDummy(objectNearby);
						}
						else
						{							
							RegisterDummy(newtraderObj);
						}
					}
					else
					{	
						TraderMessage.ServerLog("[TRADER] OBJECT TO ATTACH WAS INVALID!");
					}
				}

				attachmentCounter++;
			}
		}
		
		CloseFile(file_index);
	}

	void RegisterDummy(Object traderObj)
	{
		if (traderObj)
		{
			m_Trader_NPCDummyClasses.Insert(traderObj.GetType());
			TraderMessage.ServerLog("[TRADER] NPC DUMMY WAS REGISTERED!");
		}
	}

	override void InvokeOnConnect(PlayerBase player, PlayerIdentity identity)
    {
        super.InvokeOnConnect(player, identity);
		if(!m_Trader_ReadAllTraderData)
		{
			TraderMessage.ServerLog( "[TRADER] Trader data was not ready!" );
		}
		if (!player.HasReceivedAllTraderData())
		{	
			sendTraderDataToPlayer(player);
		}
	}

	void sendTraderDataToPlayer(PlayerBase player)
	{
		//TraderMessage.ServerLog("[TRADER] SENDING DATA TO PLAYER " + player.GetIdentity());

		// request client to clear all data:
		Param1<bool> crpClr = new Param1<bool>( true );
		GetGame().RPCSingleParam(player, TRPCs.RPC_SEND_TRADER_CLEAR, crpClr, true, player.GetIdentity());

		player.m_Trader_CurrencyName = m_Trader_CurrencyName;
		Param1<string> crp0 = new Param1<string>( m_Trader_CurrencyName );
		GetGame().RPCSingleParam(player, TRPCs.RPC_SEND_TRADER_CURRENCYNAME_ENTRY, crp0, true, player.GetIdentity());
		//TraderMessage.ServerLog("[TRADER] CURRENCYTYPE: " + m_Trader_CurrencyName);

		int i = 0;
		player.m_Trader_CurrencyClassnames = new array<string>;
		player.m_Trader_CurrencyValues = new array<int>;
		for ( i = 0; i < m_Trader_CurrencyClassnames.Count(); i++ )
		{
			player.m_Trader_CurrencyClassnames.Insert(m_Trader_CurrencyClassnames.Get(i));
			player.m_Trader_CurrencyValues.Insert(m_Trader_CurrencyValues.Get(i));

			Param2<string, int> crp1 = new Param2<string, int>( m_Trader_CurrencyClassnames.Get(i), m_Trader_CurrencyValues.Get(i) );
			GetGame().RPCSingleParam(player, TRPCs.RPC_SEND_TRADER_CURRENCY_ENTRY, crp1, true, player.GetIdentity());
		}
		
		for ( i = 0; i < m_Trader_TraderNames.Count(); i++ )
		{
			Param1<string> crp2 = new Param1<string>( m_Trader_TraderNames.Get(i) );
			GetGame().RPCSingleParam(player, TRPCs.RPC_SEND_TRADER_NAME_ENTRY, crp2, true, player.GetIdentity());
			//TraderMessage.ServerLog("[TRADER] TRADERNAME: " + m_Trader_TraderNames.Get(i));
		}
		
		for ( i = 0; i < m_Trader_Categorys.Count(); i++ )
		{
			Param2<string, int> crp3 = new Param2<string, int>( m_Trader_Categorys.Get(i), m_Trader_CategorysTraderKey.Get(i) );
			GetGame().RPCSingleParam(player, TRPCs.RPC_SEND_TRADER_CATEGORY_ENTRY, crp3, true, player.GetIdentity());
			//TraderMessage.ServerLog("[TRADER] TRADERCATEGORY: " + m_Trader_Categorys.Get(i) + ", " + m_Trader_CategorysTraderKey.Get(i));
		}

		for ( i = 0; i < m_Trader_NPCDummyClasses.Count(); i++ )
		{
			Param<string> crp6 = new Param1<string>( m_Trader_NPCDummyClasses.Get(i) );
			GetGame().RPCSingleParam(player, TRPCs.RPC_SEND_TRADER_NPCDUMMY_ENTRY, crp6, true, player.GetIdentity());
			//TraderMessage.ServerLog("[TRADER] TRADERNPCDUMMY: " + m_Trader_NPCDummyClasses.Get(i));
		}
		
		player.m_Trader_ItemsClassnames = new array<string>;
		player.m_Trader_ItemsQuantity = new array<int>;
		player.m_Trader_ItemsBuyValue = new array<int>;
		player.m_Trader_ItemsSellValue = new array<int>;
		for ( i = 0; i < m_Trader_ItemsClassnames.Count(); i++ )
		{
			player.m_Trader_ItemsClassnames.Insert(m_Trader_ItemsClassnames.Get(i));
			player.m_Trader_ItemsQuantity.Insert(m_Trader_ItemsQuantity.Get(i));
			player.m_Trader_ItemsBuyValue.Insert(m_Trader_ItemsBuyValue.Get(i));
			player.m_Trader_ItemsSellValue.Insert(m_Trader_ItemsSellValue.Get(i));

			Param6<int, int, string, int, int, int> crp4 = new Param6<int, int, string, int, int, int>( m_Trader_ItemsTraderId.Get(i), m_Trader_ItemsCategoryId.Get(i), m_Trader_ItemsClassnames.Get(i), m_Trader_ItemsQuantity.Get(i), m_Trader_ItemsBuyValue.Get(i), m_Trader_ItemsSellValue.Get(i) );
			GetGame().RPCSingleParam(player, TRPCs.RPC_SEND_TRADER_ITEM_ENTRY, crp4, true, player.GetIdentity());
			//TraderMessage.ServerLog("[TRADER] ITEMENTRY: " + m_Trader_ItemsTraderId.Get(i) + ", " + m_Trader_ItemsCategoryId.Get(i) + ", " + m_Trader_ItemsClassnames.Get(i) + ", " + m_Trader_ItemsQuantity.Get(i) + ", " + m_Trader_ItemsBuyValue.Get(i) + ", " + m_Trader_ItemsSellValue.Get(i));
		}
		
		player.m_Trader_TraderPositions = new array<vector>;
		player.m_Trader_TraderVehicleSpawns = new array<vector>;
		player.m_Trader_TraderVehicleSpawnsOrientation = new array<vector>;
		player.m_Trader_TraderSafezones = new array<int>;
		for ( i = 0; i < m_Trader_TraderPositions.Count(); i++ )
		{
			player.m_Trader_TraderPositions.Insert(m_Trader_TraderPositions.Get(i));
			player.m_Trader_TraderVehicleSpawns.Insert(m_Trader_TraderVehicleSpawns.Get(i));
			player.m_Trader_TraderVehicleSpawnsOrientation.Insert(m_Trader_TraderVehicleSpawnsOrientation.Get(i));
			player.m_Trader_TraderSafezones.Insert(m_Trader_TraderSafezones.Get(i));

			Param5<int, vector, int, vector, vector> crp5 = new Param5<int, vector, int, vector, vector>( m_Trader_TraderIDs.Get(i), m_Trader_TraderPositions.Get(i), m_Trader_TraderSafezones.Get(i), m_Trader_TraderVehicleSpawns.Get(i), m_Trader_TraderVehicleSpawnsOrientation.Get(i) );
			GetGame().RPCSingleParam(player, TRPCs.RPC_SEND_TRADER_MARKER_ENTRY, crp5, true, player.GetIdentity());
			//TraderMessage.ServerLog("[TRADER] MARKERENTRY: " + m_Trader_TraderIDs.Get(i) + ", " + m_Trader_TraderPositions.Get(i) + ", " + m_Trader_TraderSafezones.Get(i) + ", " + m_Trader_TraderVehicleSpawns.Get(i) + ", " + m_Trader_TraderVehicleSpawnsOrientation.Get(i));
		}

		// Only stored Serverside:
		player.m_Trader_Vehicles = new array<string>;
		for ( i = 0; i < m_Trader_Vehicles.Count(); i++ )
		{
			player.m_Trader_Vehicles.Insert(m_Trader_Vehicles.Get(i));
			//TraderMessage.ServerLog("[TRADER] VEHICLEENTRY: " + m_Trader_Vehicles.Get(i));
		}

		// Only stored Serverside:
		player.m_Trader_VehiclesParts = new array<string>;
		player.m_Trader_VehiclesPartsVehicleId = new array<int>;
		for ( i = 0; i < m_Trader_VehiclesParts.Count(); i++ )
		{
			player.m_Trader_VehiclesParts.Insert(m_Trader_VehiclesParts.Get(i));
			player.m_Trader_VehiclesPartsVehicleId.Insert(m_Trader_VehiclesPartsVehicleId.Get(i));
			//TraderMessage.ServerLog("[TRADER] VEHICLEPARTENTRY: " + m_Trader_VehiclesPartsVehicleId.Get(i) + ", " + m_Trader_VehiclesParts.Get(i));
		}

		player.m_Trader_BuySellTimer = m_Trader_BuySellTimer;
		GetGame().RPCSingleParam(player, TRPCs.RPC_SEND_TRADER_VARIABLES_ENTRY, new Param1<float>(m_Trader_BuySellTimer), true, player.GetIdentity());

		player.m_Trader_PlayerUID = player.GetIdentity().GetPlainId();
		GetGame().RPCSingleParam(player, TRPCs.RPC_SEND_TRADER_PLAYERUID, new Param1<string>(player.GetIdentity().GetPlainId()), true, player.GetIdentity());

		player.m_Trader_AdminPlayerUIDs = new array<string>;
		for ( i = 0; i < m_Trader_AdminPlayerUIDs.Count(); i++ )
		{
			player.m_Trader_AdminPlayerUIDs.Insert(m_Trader_AdminPlayerUIDs.Get(i));
			GetGame().RPCSingleParam(player, TRPCs.RPC_SEND_TRADER_ADMINS_ENTRY, new Param1<string>(m_Trader_AdminPlayerUIDs.Get(i)), true, player.GetIdentity());
		}
				
		// confirm that all data was sended:
		player.SetReceivedAllTraderData(true);
		player.m_Trader_SafezoneShowDebugShapes = m_Trader_SafezoneShowDebugShapes;
		player.m_Trader_TradingDistance = m_Trader_TradingDistance;
		player.SetSynchDirty();
		//TraderMessage.ServerLog("[TRADER] SENT DATA TO PLAYER");		
		//TraderMessage.ServerLog("[TRADER] DEBUG END");
	}

	void readTraderVariables()
	{
		TraderMessage.ServerLog("[TRADER] READING TRADER VARIABLES FILE");

		FileHandle file_index = OpenFile(m_Trader_VariableFilePath, FileMode.READ);
		
		if ( file_index == 0 )
		{
			TraderMessage.ServerLog("[TRADER] FOUND NO TRADERVARIABLE FILE!");
			return;
		}

		int variableCounter = 0;
		
		string line_content = "";
		while (variableCounter <= 500 && !line_content.Contains("<FileEnd>"))
		{
			bool validEntry = false;

			line_content = "";
			int char_count = FGets( file_index,  line_content );
			line_content = FileReadHelper.TrimComment(line_content);

			if (line_content.Contains("<BuySellTimer>"))
			{
				line_content.Replace("<BuySellTimer>", "");
				line_content = FileReadHelper.TrimComment(line_content);

				m_Trader_BuySellTimer = line_content.ToFloat();
				validEntry = true;

				TraderMessage.ServerLog("[TRADER] BuySellTimer = " + line_content);
			}

			if (line_content.Contains("<VehicleCleanupTimer>"))
			{
				line_content.Replace("<VehicleCleanupTimer>", "");
				line_content = FileReadHelper.TrimComment(line_content);

				m_Trader_VehicleCleanupUpdateTimerMax = line_content.ToFloat() * 60;
				m_Trader_VehicleCleanupUpdateTimer = m_Trader_VehicleCleanupUpdateTimerMax;
				validEntry = true;

				TraderMessage.ServerLog("[TRADER] VehicleCleanupTimer = " + line_content);
			}

			if (line_content.Contains("<SafezoneTimeout>"))
			{
				line_content.Replace("<SafezoneTimeout>", "");
				line_content = FileReadHelper.TrimComment(line_content);

				m_Trader_SafezoneTimeout = line_content.ToFloat();
				validEntry = true;

				TraderMessage.ServerLog("[TRADER] SafezoneTimeout = " + line_content);
			}
			string lowerLine;
			if (line_content.Contains("<SafezoneRemoveAnimals>"))
			{
				line_content.Replace("<SafezoneRemoveAnimals>", "");
				line_content = FileReadHelper.TrimComment(line_content);
				lowerLine = line_content;
				lowerLine.ToLower();
				if(lowerLine.Contains("yes"))
				{
					m_Trader_SafezoneRemoveAnimals = true;
				}
				validEntry = true;

				TraderMessage.ServerLog("[TRADER] SafezoneRemoveAnimals = " + line_content);
			}
			
			if (line_content.Contains("<SafezoneRemoveInfected>"))
			{
				line_content.Replace("<SafezoneRemoveInfected>", "");
				line_content = FileReadHelper.TrimComment(line_content);
				lowerLine = line_content;
				lowerLine.ToLower();
				if(lowerLine.Contains("yes"))
				{
					m_Trader_SafezoneRemoveInfected = true;
				}
				validEntry = true;

				TraderMessage.ServerLog("[TRADER] SafezoneRemoveInfected = " + line_content);
			}
			if (line_content.Contains("<SafezoneShowDebugShape>"))
			{
				line_content.Replace("<SafezoneShowDebugShape>", "");
				line_content = FileReadHelper.TrimComment(line_content);
				lowerLine = line_content;
				lowerLine.ToLower();
				if(lowerLine.Contains("yes"))
				{
					m_Trader_SafezoneShowDebugShapes = true;
				}
				validEntry = true;

				TraderMessage.ServerLog("[TRADER] SafezoneShowDebugShape = " + line_content);
			}
			if (line_content.Contains("<TradingDistance>"))
			{
				line_content.Replace("<TradingDistance>", "");
				line_content = FileReadHelper.TrimComment(line_content);
				m_Trader_TradingDistance = line_content.ToFloat();
				validEntry = true;

				TraderMessage.ServerLog("[TRADER] TradingDistance = " + line_content);
			}

			if (validEntry)
				variableCounter++;
		}

		CloseFile(file_index);

		TraderMessage.ServerLog("[TRADER] DONE END!");
	}

	void readTraderAdmins()
	{
		TraderMessage.ServerLog("[TRADER] READING TRADER ADMINS FILE");
		// clear all data here:
		m_Trader_AdminPlayerUIDs = new array<string>;	

		FileHandle file_index = OpenFile(m_Trader_AdminsFilePath, FileMode.READ);
		
		if ( file_index == 0 )
		{
			TraderMessage.ServerLog("[TRADER] FOUND NO TRADERADMINS FILE!");
			return;
		}

		int adminsCounter = 0;
		
		string line_content = "";
		while (adminsCounter <= 500 && !line_content.Contains("<FileEnd>"))
		{
			line_content = "";
			int char_count = FGets( file_index,  line_content );
			line_content = FileReadHelper.TrimComment(line_content);

			if (line_content.Contains("<FileEnd>") || line_content.Length() < 16)
				continue;

			TraderMessage.ServerLog("[TRADER] ADMIN PLAYER UID ENTRY " + line_content);
			m_Trader_AdminPlayerUIDs.Insert(line_content);

			adminsCounter++;
		}

		CloseFile(file_index);
	}

	bool OpenNewFileForReading(string line_content, out FileHandle file_index)
	{
		line_content.Replace("<OpenFile>", "");
		line_content = FileReadHelper.TrimComment(line_content);

		CloseFile(file_index);
		file_index = OpenFile("$profile:Trader/" + line_content, FileMode.READ);

		if ( file_index == 0 )
		{
			TraderMessage.ServerLog("[TRADER] CANT FIND LINKED FILE " + "$profile:Trader/" + line_content + "!");
			return false;
		}
		
		return true;
	}

	Object FindTraderObjectAtPosition(vector position)
	{
		foreach(Object traderObj : m_Trader_ObjectsList)
		{
			if(traderObj)
			{
				float distance = vector.Distance(position, traderObj.GetPosition());
				if(distance < 0.99)
				{
					return traderObj;
				}
			}
		}
		return null;
	}

	void readTraderData()
	{		
		// clear all data here:
		m_Trader_ReadAllTraderData = false;	
		m_Trader_CurrencyName = "";
		m_Trader_CurrencyClassnames = new array<string>;
		m_Trader_CurrencyValues = new array<int>;
		m_Trader_TraderNames = new array<string>;
		m_Trader_TraderPositions = new array<vector>;
		m_Trader_TraderIDs = new array<int>;
		m_Trader_TraderSafezones = new array<int>;
		m_Trader_TraderVehicleSpawns = new array<vector>;
		m_Trader_TraderVehicleSpawnsOrientation = new array<vector>;
		m_Trader_Categorys = new array<string>;
		m_Trader_CategorysTraderKey = new array<int>;
		m_Trader_ItemsTraderId = new array<int>;
		m_Trader_ItemsCategoryId = new array<int>;
		m_Trader_ItemsClassnames = new array<string>;
		m_Trader_ItemsQuantity = new array<int>;
		m_Trader_ItemsBuyValue = new array<int>;
		m_Trader_ItemsSellValue = new array<int>;
		m_Trader_Vehicles = new array<string>;
		m_Trader_VehiclesParts = new array<string>;
		m_Trader_VehiclesPartsVehicleId = new array<int>;		
		
		FileHandle file_index = OpenFile(m_Trader_ConfigFilePath, FileMode.READ);
		
		TraderMessage.ServerLog("[TRADER] READING TRADER CURRENCY, PRICES AND CATEGORIES FILES");
		if ( file_index == 0 )
		{
			TraderMessage.ServerLog("[TRADER] FOUND NO TRADERCONFIG FILE!");
			return;
		}
		
		string line_content = "";
		
		line_content = FileReadHelper.SearchForNextTermInFile(file_index, "<CurrencyName>", "");
		line_content.Replace("<CurrencyName>", "");
		line_content = FileReadHelper.TrimComment(line_content);
		m_Trader_CurrencyName = line_content;
		TraderMessage.ServerLog("[TRADER] CURRENCY NAME ENTRY " + m_Trader_CurrencyName);

		int currencyCounter = 0;

		line_content = "";
		while (currencyCounter <= 500 && !line_content.Contains("<Trader>"))
		{
			line_content = FileReadHelper.SearchForNextTermInFile(file_index, "<Currency>", "<Trader>");
			line_content.Replace("<Currency>", "");
			line_content = FileReadHelper.TrimComment(line_content);

			if (line_content.Contains("<Trader>"))
				break;

			TStringArray crys = new TStringArray;
			line_content.Split( ",", crys );

			string currencyClassname = crys.Get(0);
			currencyClassname = FileReadHelper.TrimSpaces(currencyClassname);
			
			string currencyValue = crys.Get(1);
			currencyValue = FileReadHelper.TrimSpaces(currencyValue);

			m_Trader_CurrencyClassnames.Insert(currencyClassname);
			m_Trader_CurrencyValues.Insert(currencyValue.ToInt());

			TraderMessage.ServerLog("[TRADER] CURRENCY ENTRY " + currencyClassname + " value: " + currencyValue);

			currencyCounter++;
		}

		bool traderInstanceDone = true;
		int traderCounter = 0;
		
		//line_content = "";
		while (traderCounter <= 5000 && line_content != "<FileEnd>")
		{			
			if (traderInstanceDone == false)
				line_content = FileReadHelper.SearchForNextTermsInFile(file_index, {"<Trader>", "<OpenFile>"}, "");
			else
				traderInstanceDone = false;
			
			if (line_content.Contains("<OpenFile>"))
			{
				if (OpenNewFileForReading(line_content, file_index))
					continue;
				else
					return;
			}

			line_content.Replace("<Trader>", "");
			line_content = FileReadHelper.TrimComment(line_content);
			
			TraderMessage.ServerLog("[TRADER] READING TRADER ENTRY " + line_content);
			m_Trader_TraderNames.Insert(line_content);
				
			int categoryCounter = 0;
			
			line_content = "";
			while (categoryCounter <= 5000 && line_content != "<FileEnd>")
			{
				line_content = FileReadHelper.TrimComment(FileReadHelper.SearchForNextTermsInFile(file_index, {"<Category>", "<OpenFile>"}, "<Trader>"));
				
				if (line_content.Contains("<OpenFile>"))
				{
					if (OpenNewFileForReading(line_content, file_index))
						continue;
					else
						return;
				}

				if (line_content.Contains("<Trader>"))
				{
					traderInstanceDone = true;
					break;
				}
				
				if (line_content == string.Empty)
				{
					line_content = "<FileEnd>";
					break;
				}
				
				line_content.Replace("<Category>", "");
				string category = FileReadHelper.TrimComment(line_content);
				m_Trader_Categorys.Insert(category);
				m_Trader_CategorysTraderKey.Insert(traderCounter);
				TraderMessage.ServerLog("[TRADER] READING CATEGORY ENTRY " + category);
				
				categoryCounter++;
			}
			
			traderCounter++;
		}
		
		CloseFile(file_index);
		
		//------------------------------------------------------------------------------------
		
		file_index = OpenFile(m_Trader_ConfigFilePath, FileMode.READ);
		
		int itemCounter = 0;
		int itemCounterTotal = 0;
		int char_count = 0;
		int traderID = -1;
		int categoryId = -1;
		
		line_content = "";
		while ( itemCounter <= 10000 && char_count != -1 && line_content.Contains("<FileEnd>") == false)
		{
			char_count = FGets( file_index,  line_content );
			
			line_content = FileReadHelper.TrimComment(line_content);

			if (line_content.Contains("<OpenFile>"))
			{
				if (OpenNewFileForReading(line_content, file_index))
					continue;
				else
					return;
			}

			if (line_content.Contains("<Trader>"))
			{
				traderID++;
				itemCounter = 0;
				
				continue;
			}
			
			if (line_content.Contains("<Category>"))
			{
				categoryId++;
				itemCounter = 0;
				
				continue;
			}
		
			if (!line_content.Contains(","))
				continue;

			if (line_content.Contains("<Currency"))
				continue;

			//TraderMessage.ServerLog("[TRADER] READING ITEM ENTRY..");
		
			TStringArray strs = new TStringArray;
			line_content.Split( ",", strs );
			
			string itemStr = strs.Get(0);
			itemStr = FileReadHelper.TrimSpaces(itemStr);
			
			string qntStr = strs.Get(1);
			qntStr = FileReadHelper.TrimSpaces(qntStr);
			
			if (qntStr.Contains("*") || qntStr.Contains("-1"))
			{
				qntStr = GetItemMaxQuantity(itemStr).ToString();
			}

			if (qntStr == "VNK" || qntStr == "vnk")
				qntStr = "-6";

			if (qntStr == "V" || qntStr == "v")
				qntStr = "-2";

			if (qntStr == "M" || qntStr == "m")
				qntStr = "-3";

			if (qntStr == "W" || qntStr == "w")
				qntStr = "-4";

			if (qntStr == "S" || qntStr == "s")
				qntStr = "-5";

			if (qntStr == "K" || qntStr == "k")
				qntStr = "-7";

			if (qntStr == "KL" || qntStr == "kl")
				qntStr = "-8";
			
			string buyStr = strs.Get(2);
			buyStr = FileReadHelper.TrimSpaces(buyStr);
			
			string sellStr = strs.Get(3);
			sellStr = FileReadHelper.TrimSpaces(sellStr);
			
			m_Trader_ItemsTraderId.Insert(traderID);
			m_Trader_ItemsCategoryId.Insert(categoryId);
			m_Trader_ItemsClassnames.Insert(itemStr);
			m_Trader_ItemsQuantity.Insert(qntStr.ToInt());
			m_Trader_ItemsBuyValue.Insert(buyStr.ToInt());
			m_Trader_ItemsSellValue.Insert(sellStr.ToInt());
			
			itemCounter++;
		}
		
		CloseFile(file_index);
		
		//------------------------------------------------------------------------------------
		
		file_index = OpenFile(m_Trader_ObjectsFilePath, FileMode.READ);
		
		if ( file_index == 0 )
		{
			TraderMessage.ServerLog("[TRADER] FOUND NO TRADEROBJECTS FILE!");
			return;
		}
		
		bool skipLine = false;
		int markerCounter = 0;				

		line_content = "";
		int currentTraderID = 0;		
		while ( markerCounter <= 5000 && line_content.Contains("<FileEnd>") == false)
		{
			// Get Trader Marker Trader ID:
			if (!skipLine)
				line_content = FileReadHelper.SearchForNextTermsInFile(file_index, {"<TraderMarker>", "<OpenFile>"}, "<FileEnd>");
			else
				skipLine = false;	

			if (line_content.Contains("<OpenFile>"))
			{
				if (!OpenNewFileForReading(line_content, file_index))
					return;
			}				
			
			if (!line_content.Contains("<TraderMarker>"))
				continue;
			
			line_content.Replace("<TraderMarker>", "");
			line_content = FileReadHelper.TrimComment(line_content);
			line_content = FileReadHelper.TrimSpaces(line_content);
			
			TraderMessage.ServerLog("[TRADER] MARKER ID ENTRY " + line_content);
			currentTraderID = line_content.ToInt();
			m_Trader_TraderIDs.Insert(currentTraderID);
			
			// Get Trader Marker Position:		
			line_content = FileReadHelper.SearchForNextTermInFile(file_index, "<TraderMarkerPosition>", "<FileEnd>");
			
			line_content.Replace("<TraderMarkerPosition>", "");
			line_content = FileReadHelper.TrimComment(line_content);
			
			TStringArray strsm = new TStringArray;
			line_content.Split( ",", strsm );
			
			string traderMarkerPosX = strsm.Get(0);
			traderMarkerPosX = FileReadHelper.TrimSpaces(traderMarkerPosX);
			
			string traderMarkerPosY = strsm.Get(1);
			traderMarkerPosY = FileReadHelper.TrimSpaces(traderMarkerPosY);
			
			string traderMarkerPosZ = strsm.Get(2);
			traderMarkerPosZ = FileReadHelper.TrimSpaces(traderMarkerPosZ);
			
			vector markerPosition = "0 0 0";
			markerPosition[0] = traderMarkerPosX.ToFloat();
			markerPosition[1] = traderMarkerPosY.ToFloat();
			markerPosition[2] = traderMarkerPosZ.ToFloat();
			
			m_Trader_TraderPositions.Insert(markerPosition);
			Object traderAtPos = FindTraderObjectAtPosition(markerPosition);			
			if(traderAtPos)
			{
				BuildingBase buildingItem = BuildingBase.Cast(traderAtPos);
				if(buildingItem)
				{
					buildingItem.m_Trader_TraderIndex = m_Trader_TraderIDs.Count() - 1;
					buildingItem.SetSynchDirty();
					//TraderMessage.ServerLog("[TRADER] TRADER MARKER buildingItem " + buildingItem);
				}
				PlayerBase playerTrader = PlayerBase.Cast(traderAtPos);
				if(playerTrader)
				{
					playerTrader.m_Trader_IsTrader = true;
					playerTrader.m_Trader_TraderIndex = m_Trader_TraderIDs.Count() - 1;
					playerTrader.SetSynchDirty();
					//TraderMessage.ServerLog("[TRADER] TRADER MARKER playerTrader " + playerTrader);
				}			
				TraderMessage.ServerLog("[TRADER] TRADER MARKER POSITION ENTRY " + markerPosition);
			}
			else
			{
				TraderMessage.ServerLog("[TRADER][ERROR] Marker couldn't find an object at position " + markerPosition);
			}
			
			// Get Trader Marker Safezone Radius:					
			line_content = FileReadHelper.SearchForNextTermInFile(file_index, "<TraderMarkerSafezone>", "<FileEnd>");
			
			line_content.Replace("<TraderMarkerSafezone>", "");
			line_content = FileReadHelper.TrimComment(line_content);
			line_content = FileReadHelper.TrimSpaces(line_content);	
			
			m_Trader_TraderSafezones.Insert(line_content.ToInt());

			/*	
				Why the fuck is this like this? - MDC 
				
				We create a trigger for the safezone of the trader.
			*/
			int triggerRadius = line_content.ToInt();
			if(triggerRadius > 0)
			{
				SafeZoneTrigger newTrigger; // Should be stored in an array later
				if (Class.CastTo(newTrigger, GetGame().CreateObjectEx("SafeZoneTrigger", markerPosition, ECE_NONE)))
				{
					vector triggerPosition = markerPosition;
					int halfheight = 100;
					triggerPosition[1] = triggerPosition[1] - halfheight; // Sane default values
					newTrigger.SetPosition(triggerPosition);
					newTrigger.SetCollisionCylinder( triggerRadius, halfheight * 2 );
					newTrigger.InitSafeZone(m_Trader_SafezoneTimeout, m_Trader_SafezoneRemoveAnimals, m_Trader_SafezoneRemoveInfected);
					
					TraderMessage.ServerLog("[TRADER] SPAWNED SAFEZONE AT " + triggerPosition + " with radius " + triggerRadius);
				}
			}
			
			// Get Trader Marker Vehicle Spawnpoint:					
			line_content = FileReadHelper.SearchForNextTermInFile(file_index, "<VehicleSpawn>", "<TraderMarker>");

			if(line_content == string.Empty)
				break;

			if (line_content.Contains("<TraderMarker>"))
			{
				skipLine = true;
				m_Trader_TraderVehicleSpawns.Insert("0 0 0");
				m_Trader_TraderVehicleSpawnsOrientation.Insert("0 0 0");
				continue;
			}

			line_content.Replace("<VehicleSpawn>", "");
			line_content = FileReadHelper.TrimComment(line_content);

			TStringArray strtmv = new TStringArray;
			line_content.Split( ",", strtmv );
			
			string traderMarkerVehiclePosX = strtmv.Get(0);
			traderMarkerVehiclePosX = FileReadHelper.TrimSpaces(traderMarkerVehiclePosX);
			
			string traderMarkerVehiclePosY = strtmv.Get(1);
			traderMarkerVehiclePosY = FileReadHelper.TrimSpaces(traderMarkerVehiclePosY);
			
			string traderMarkerVehiclePosZ = strtmv.Get(2);
			traderMarkerVehiclePosZ = FileReadHelper.TrimSpaces(traderMarkerVehiclePosZ);
			
			vector markerVehiclePosition = "0 0 0";
			markerVehiclePosition[0] = traderMarkerVehiclePosX.ToFloat();
			markerVehiclePosition[1] = traderMarkerVehiclePosY.ToFloat();
			markerVehiclePosition[2] = traderMarkerVehiclePosZ.ToFloat();

			TraderMessage.ServerLog("[TRADER] TRADER MARKER VEHICLE ENTRY " + markerVehiclePosition);

			m_Trader_TraderVehicleSpawns.Insert(markerVehiclePosition);

			// Get Trader Marker Vehicle Orientation:
			line_content = FileReadHelper.SearchForNextTermInFile(file_index, "<VehicleSpawnOri>", "<TraderMarker>");

			if(line_content == string.Empty)
				break;

			if (line_content.Contains("<TraderMarker>"))
			{
				skipLine = true;
				m_Trader_TraderVehicleSpawnsOrientation.Insert("0 0 0");
				continue;
			}

			line_content.Replace("<VehicleSpawnOri>", "");
			line_content = FileReadHelper.TrimComment(line_content);

			TStringArray strtmvd = new TStringArray;
			line_content.Split( ",", strtmvd );
			
			string traderMarkerVehicleOriX = strtmvd.Get(0);
			traderMarkerVehicleOriX = FileReadHelper.TrimSpaces(traderMarkerVehicleOriX);
			
			string traderMarkerVehicleOriY = strtmvd.Get(1);
			traderMarkerVehicleOriY = FileReadHelper.TrimSpaces(traderMarkerVehicleOriY);
			
			string traderMarkerVehicleOriZ = strtmvd.Get(2);
			traderMarkerVehicleOriZ = FileReadHelper.TrimSpaces(traderMarkerVehicleOriZ);
			
			vector markerVehicleOrientation = "0 0 0";
			markerVehicleOrientation[0] = traderMarkerVehicleOriX.ToFloat();
			markerVehicleOrientation[1] = traderMarkerVehicleOriY.ToFloat();
			markerVehicleOrientation[2] = traderMarkerVehicleOriZ.ToFloat();

			m_Trader_TraderVehicleSpawnsOrientation.Insert(markerVehicleOrientation);

			markerCounter++;
		}
		
		CloseFile(file_index);
		
		//------------------------------------------------------------------------------------
		
		file_index = OpenFile(m_Trader_VehiclePartsFilePath, FileMode.READ);
		
		if ( file_index == 0 )
		{
			TraderMessage.ServerLog("[TRADER] FOUND NO VEHICLEPARTS FILE!");
			return;
		}
		
		skipLine = false;
		int vehicleCounter = 0;
		
		line_content = "";
		while ( vehicleCounter <= 5000 && line_content.Contains("<FileEnd>") == false)
		{
			// Get Vehicle Name Entrys:
			if (!skipLine)
				line_content = FileReadHelper.SearchForNextTermsInFile(file_index, {"<VehicleParts>", "<OpenFile>"}, "<FileEnd>");
			else
				skipLine = false;

			if (line_content.Contains("<OpenFile>"))
			{
				if (!OpenNewFileForReading(line_content, file_index))
					return;
			}	
			
			if (!line_content.Contains("<VehicleParts>"))
				continue;
			
			line_content.Replace("<VehicleParts>", "");
			line_content = FileReadHelper.TrimComment(line_content);
			line_content = FileReadHelper.TrimSpaces(line_content);
			
			TraderMessage.ServerLog("[TRADER] VEHICLE NAME ENTRY " + line_content);

			m_Trader_Vehicles.Insert(line_content);

			char_count = 0;
			int vehiclePartsCounter = 0;
			//while ( vehiclePartsCounter <= 5000  && char_count != -1 && line_content.Contains("<FileEnd>") == false)
			while (true)
			{
				// Get Vehicle Parts Entrys:
				char_count = FGets( file_index,  line_content );

				line_content = FileReadHelper.TrimComment(line_content);
				line_content = FileReadHelper.TrimSpaces(line_content);

				if (line_content == "")
					continue;

				if (line_content.Contains("<OpenFile>"))
				{
					if (OpenNewFileForReading(line_content, file_index))
						continue;
					else
						return;
				}	

				if (line_content.Contains("<VehicleParts>"))
				{
					skipLine = true;						
					break;
				}

				if (line_content.Contains("<FileEnd>") || char_count == -1 || vehiclePartsCounter > 5000)
				{
					line_content = "<FileEnd>";
					break;
				}

				m_Trader_VehiclesParts.Insert(line_content);
				m_Trader_VehiclesPartsVehicleId.Insert(vehicleCounter);

				vehiclePartsCounter++;
			}
			
			vehicleCounter++;
		}
		
		CloseFile(file_index);
		
		//------------------------------------------------------------------------------------
		m_Trader_ObjectsList.Clear();
		m_Trader_ReadAllTraderData = true;
	}

	int GetItemMaxQuantity(string itemClassname)
	{
		TStringArray searching_in = new TStringArray;
		searching_in.Insert( CFG_MAGAZINESPATH  + " " + itemClassname + " count");
		//searching_in.Insert( CFG_WEAPONSPATH );
		searching_in.Insert( CFG_VEHICLESPATH + " " + itemClassname + " varQuantityMax");

		for ( int s = 0; s < searching_in.Count(); ++s )
		{
			string path = searching_in.Get( s );

			if ( GetGame().ConfigIsExisting( path ) )
			{
				return g_Game.ConfigGetInt( path );
			}
		}

		return 0;
	}

	//DEPRECATED - NOT USED ANYMORE
	void SetPlayerVehicleIsInSafezone( PlayerBase player, bool isInSafezone )
	{
		Print("A mod is using SetPlayerVehicleIsInSafezone from Trader mod. Function has been deprecated.");
	}
}