modded class MissionServer
{
	static const string m_Trader_ConfigFilePath = "$profile:Trader/TraderConfig.txt";
	static const string m_Trader_ObjectsFilePath = "$profile:Trader/TraderObjects.txt";
	static const string m_Trader_VehiclePartsFilePath = "$profile:Trader/TraderVehicleParts.txt";

	static const float m_Trader_SafezoneTimeout = 30;

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
		
	ref array<PlayerBase> m_Trader_SpawnedTraderCharacters;
	ref array<BarrelHoles_ColorBase> m_Trader_SpawnedFireBarrels;

	const float m_Trader_StatUpdateTimeMax = 1;
	float m_Trader_StatUpdateTime = m_Trader_StatUpdateTimeMax;
	float m_Trader_SpawnedFireBarrelsUpdateTimer = 0;
	float m_Trader_ZombieCleanupUpdateTimer = 0;
	
	override void OnInit()
	{		
		super.OnInit();

		SpawnTraderObjects();
		readTraderData();
	}
	
	override void OnUpdate(float timeslice)
	{
		super.OnUpdate(timeslice);
		

		m_Trader_StatUpdateTime += timeslice;
		if (m_Trader_StatUpdateTime >= m_Trader_StatUpdateTimeMax)
		{
			m_Trader_StatUpdateTime = 0;

			for (int j = 0; j < m_Players.Count(); j++)
			{
				PlayerBase player = PlayerBase.Cast(m_Players.Get(j));
				
				if ( !player )
					continue;
				
				if ( !player.m_Trader_WelcomeMessageHandled && player.IsAlive() )
				{				
					if (player.m_Trader_WelcomeMessageTimer > 0)
						player.m_Trader_WelcomeMessageTimer -= m_Trader_StatUpdateTimeMax;
					else
					{
						if ( !player.m_Trader_TraderModIsLoaded )
						{
							Param1<string> msgRp0 = new Param1<string>( "This Server uses Dr_J0nes Trader Mod." );
							GetGame().RPCSingleParam(player, ERPCs.RPC_USER_ACTION_MESSAGE, msgRp0, true, player.GetIdentity());
							
							msgRp0.param1 = "Please download and install it!";
							GetGame().RPCSingleParam(player, ERPCs.RPC_USER_ACTION_MESSAGE, msgRp0, true, player.GetIdentity());
						}
						
						player.m_Trader_WelcomeMessageHandled = true;
					}
				}

				if (!player.IsAlive())
					continue;
				
				if (!m_Trader_ReadAllTraderData)
					continue;

				if (!player.m_Trader_RecievedAllData)
					sendTraderDataToPlayer(player);
				
				bool isInSafezone = false;
				
				for (int k = 0; k < m_Trader_TraderPositions.Count(); k++)
				{
					if (vector.Distance(player.GetPosition(), m_Trader_TraderPositions.Get(k)) <= m_Trader_TraderSafezones.Get(k))
						isInSafezone = true;
				}

				if (player.m_Trader_IsInSafezone == true && isInSafezone == false && player.m_Trader_IsInSafezoneTimeout == m_Trader_SafezoneTimeout)
				{
					SetPlayerVehicleIsInSafezone( player, false );

					TraderMessage.Safezone(player, m_Trader_SafezoneTimeout);
				}

				if (!isInSafezone && player.m_Trader_IsInSafezoneTimeout > 0)
				{
					player.m_Trader_IsInSafezoneTimeout -= m_Trader_StatUpdateTimeMax;
				}

				if (player.m_Trader_IsInSafezone == false && isInSafezone == true)
				{
					player.m_Trader_IsInSafezone = true;
					player.m_Trader_IsInSafezoneTimeout = m_Trader_SafezoneTimeout;
					GetGame().RPCSingleParam(player, TRPCs.RPC_SEND_TRADER_IS_IN_SAFEZONE, new Param1<bool>( true ), true, player.GetIdentity());
					player.m_Trader_InfluenzaEnteredSafeZone = player.m_AgentPool.GetSingleAgentCount(eAgents.INFLUENZA);
					player.GetInputController().OverrideRaise(true, false);
					SetPlayerVehicleIsInSafezone( player, true );
					
					TraderMessage.DeleteSafezoneMessages(player);
					TraderMessage.PlayerRed("You entered the Safezone!", player);
					TraderMessage.PlayerWhite("Press 'B'-Key to open\nthe Trader Menu.", player);

					player.SetAllowDamage(false);
				}

				if (isInSafezone && player.m_Trader_IsInSafezoneTimeout < m_Trader_SafezoneTimeout)
				{
					player.m_Trader_IsInSafezoneTimeout = m_Trader_SafezoneTimeout;

					TraderMessage.DeleteSafezoneMessages(player);
					TraderMessage.PlayerWhite("Welcome back!", player);
				}
				
				if (player.m_Trader_IsInSafezone == true && isInSafezone == false && player.m_Trader_IsInSafezoneTimeout <= 0)
				{
					player.m_Trader_IsInSafezone = false;
					GetGame().RPCSingleParam(player, TRPCs.RPC_SEND_TRADER_IS_IN_SAFEZONE, new Param1<bool>( false ), true, player.GetIdentity());
					player.GetInputController().OverrideRaise(false, false);
					//SetPlayerVehicleIsInSafezone( player, false );
					
					TraderMessage.DeleteSafezoneMessages(player);
					TraderMessage.PlayerRed("You left the Safezone!", player);

					player.SetAllowDamage(true);
				}
				
				if (isInSafezone)
				{
					if ( player.m_AgentPool.GetSingleAgentCount(eAgents.INFLUENZA) < player.m_Trader_InfluenzaEnteredSafeZone) // allow influenza healing
						player.m_Trader_InfluenzaEnteredSafeZone = player.m_AgentPool.GetSingleAgentCount(eAgents.INFLUENZA);

					player.m_AgentPool.SetAgentCount(eAgents.INFLUENZA, player.m_Trader_InfluenzaEnteredSafeZone);
				}
			}
			
			for ( int i = 0; i < m_Trader_SpawnedTraderCharacters.Count(); i++ )
			{
				m_Trader_SpawnedTraderCharacters.Get(i).m_AgentPool.SetAgentCount(eAgents.INFLUENZA, 0);
			}
		}

		m_Trader_SpawnedFireBarrelsUpdateTimer += timeslice;
		if (m_Trader_SpawnedFireBarrelsUpdateTimer >= 5)
		{
			m_Trader_SpawnedFireBarrelsUpdateTimer = 0;

			for (int m = 0; m < m_Trader_SpawnedFireBarrels.Count(); m++)
			{
				BarrelHoles_ColorBase ntarget = m_Trader_SpawnedFireBarrels.Get(m);

				if (!ntarget.IsBurning() && !ntarget.IsWet())
				{
					ItemBase kindling;
					kindling = ItemBase.Cast(ntarget.GetInventory().CreateAttachment("Rag"));
					if (kindling)
						kindling.SetQuantityMax();

					//kindling = ItemBase.Cast(ntarget.GetInventory().CreateAttachment("BandageDressing"));
					//if (kindling)
					//	kindling.SetQuantityMax();

					kindling = ItemBase.Cast(ntarget.GetInventory().CreateAttachment("Bark_Oak"));
					if (kindling)
						kindling.SetQuantityMax();

					kindling = ItemBase.Cast(ntarget.GetInventory().CreateAttachment("Bark_Birch"));
					if (kindling)
						kindling.SetQuantityMax();

					ItemBase fuel;
					fuel = ItemBase.Cast(ntarget.GetInventory().CreateAttachment("Firewood"));
					if (fuel)
						fuel.SetQuantityMax();

					fuel = ItemBase.Cast(ntarget.GetInventory().CreateAttachment("WoodenStick"));
					//if (fuel)
					//	fuel.SetQuantityMax();

					EntityAI fire_source_dummy;
					ntarget.OnIgnitedThis(fire_source_dummy);
				}

				if (ntarget.IsWet())
					ntarget.SetWet(ntarget.GetWetMin());
			}
		}

		m_Trader_ZombieCleanupUpdateTimer += timeslice;
		if (m_Trader_ZombieCleanupUpdateTimer >= 30)
		{
			m_Trader_ZombieCleanupUpdateTimer = 0;

			for (int n = 0; n < m_Trader_TraderPositions.Count(); n++)
			{
				vector orientation = Vector(0, 0, 0);
				int safezoneDiameter = m_Trader_TraderSafezones.Get(n) * 2;
				vector edgeLength = Vector(safezoneDiameter, safezoneDiameter, safezoneDiameter);
				array<Object> excludedObjects = new array<Object>;
				array<Object> collidedObjects = new array<Object>;
				
				if (GetGame().IsBoxColliding(m_Trader_TraderPositions.Get(n), orientation, edgeLength, excludedObjects, collidedObjects))
				{
					for (int o = 0; o < collidedObjects.Count(); o++)
					{
						string objectClass = collidedObjects.Get(o).GetType();

						if (objectClass.Contains("ZmbF_") || objectClass.Contains("ZmbM_"))
							GetGame().ObjectDelete(collidedObjects.Get(o));	
					}
				}
			}
		}
	}
	
	void SpawnTraderObjects()
	{
		m_Trader_SpawnedTraderCharacters = new array<PlayerBase>;
		m_Trader_SpawnedFireBarrels = new array<BarrelHoles_ColorBase>;
		
		TraderMessage.ServerLog("[TRADER] DEBUG START");
		
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
			
			TraderMessage.ServerLog("[TRADER] READING OBJECT TYPE ENTRY..");
			
			string traderObjectType = line_content;
			
			// Get Object Position --------------------------------------------------------------------------------
			line_content = FileReadHelper.SearchForNextTermInFile(file_index, "<ObjectPosition>", "<FileEnd>");
			
			line_content.Replace("<ObjectPosition>", "");
			line_content = FileReadHelper.TrimComment(line_content);
			
			TraderMessage.ServerLog("[TRADER] READING OBJECT POSITION ENTRY..");
			
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

			// HANDLE TRADER FIRE BARREL BEGIN
			string persistantObjectType = "BarrelHoles_";
			bool isPersistant = traderObjectType.Contains(persistantObjectType);
			if (isPersistant)
			{
				vector persistanceCheck_size = "1 1 1";
				array<Object> persistanceCheck_excluded_objects = new array<Object>;
				array<Object> persistanceCheck_nearby_objects = new array<Object>;

				GetGame().IsBoxColliding( objectPosition, vector.Zero, persistanceCheck_size, persistanceCheck_excluded_objects, persistanceCheck_nearby_objects);

				for (int pc = 0; pc < persistanceCheck_nearby_objects.Count(); pc++)
				{
					if (persistanceCheck_nearby_objects.Get(pc).GetType() == traderObjectType)
						GetGame().ObjectDelete(persistanceCheck_nearby_objects.Get(pc));
				}
			}
			// HANDLE TRADER FIRE BARREL END

			Object obj = GetGame().CreateObject( traderObjectType, objectPosition, false, false, true );
			obj.SetPosition(objectPosition); // prevent automatic on ground placing
			TraderMessage.ServerLog("[TRADER] SPAWNED OBJECT '" + traderObjectType + "' AT '" + objectPosition + "'");
			
			// HANDLE TRADER FIRE BARREL BEGIN
			if (isPersistant)
			{
				BarrelHoles_ColorBase ntarget = BarrelHoles_ColorBase.Cast( obj );
				if( ntarget )
				{
					ntarget.Trader_IsInSafezone = true;

					ntarget.Open();
					TraderMessage.ServerLog("[TRADER] OPENED BARREL");

					m_Trader_SpawnedFireBarrels.Insert(ntarget);
				}
			}
			// HANDLE TRADER FIRE BARREL END

			bool isTrader = false;
			PlayerBase man;
			if (Class.CastTo(man, obj))
			{
				TraderMessage.ServerLog("[TRADER] Object was a Man..");
				isTrader = true;
			}
			
			// Get Object Orientation -------------------------------------------------------------------------------
			line_content = FileReadHelper.SearchForNextTermInFile(file_index, "<ObjectOrientation>", "<Object>");
			
			if (line_content == string.Empty)	
				line_content = "<FileEnd>";
			
			if (line_content.Contains("<Object>"))
			{				
				if (isTrader)
				{
					//man.m_Trader_IsTrader = true;
					m_Trader_SpawnedTraderCharacters.Insert(man);
					man.SetAllowDamage(false);
				}
				
				skipDirEntry = true;
				markerCounter++;
				continue;
			}
			
			if (!line_content.Contains("<ObjectOrientation>"))
				continue;
			
			line_content.Replace("<ObjectOrientation>", "");
			line_content = FileReadHelper.TrimComment(line_content);
			
			TraderMessage.ServerLog("[TRADER] READING OBJECT ORIENTATION ENTRY..");
			
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
			
			obj.SetOrientation(objectOrientation);

			TraderMessage.ServerLog("[TRADER] OBJECT ORIENTATION = '" + obj.GetOrientation() + "'");

			if (isTrader)
			{
				//man.m_Trader_IsInSafezone = true;
				m_Trader_SpawnedTraderCharacters.Insert(man);
				man.SetAllowDamage(false);
			}
						
			
			markerCounter++;
		}
		
		CloseFile(file_index);
		TraderMessage.ServerLog("[TRADER] DEBUG END");
	}

	void sendTraderDataToPlayer(PlayerBase player)
	{
		TraderMessage.ServerLog("[TRADER] SEND DATA TO PLAYER");

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
		for ( i = 0; i < m_Trader_TraderPositions.Count(); i++ )
		{
			player.m_Trader_TraderPositions.Insert(m_Trader_TraderPositions.Get(i));
			player.m_Trader_TraderVehicleSpawns.Insert(m_Trader_TraderVehicleSpawns.Get(i));
			player.m_Trader_TraderVehicleSpawnsOrientation.Insert(m_Trader_TraderVehicleSpawnsOrientation.Get(i));

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
		
		// confirm that all data was sended:
		player.m_Trader_RecievedAllData = true;
		
		Param1<bool> crpConf = new Param1<bool>( true );
		GetGame().RPCSingleParam(player, TRPCs.RPC_SEND_TRADER_DATA_CONFIRMATION, crpConf, true, player.GetIdentity());
		//TraderMessage.ServerLog("[TRADER] DEBUG END");
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

		TraderMessage.ServerLog("[TRADER] DEBUG START");
		
		FileHandle file_index = OpenFile(m_Trader_ConfigFilePath, FileMode.READ);
		
		if ( file_index == 0 )
		{
			TraderMessage.ServerLog("[TRADER] FOUND NO TRADERCONFIG FILE!");
			return;
		}
		
		string line_content = "";
		
		TraderMessage.ServerLog("[TRADER] READING CURRENCY NAME ENTRY..");
		line_content = FileReadHelper.SearchForNextTermInFile(file_index, "<CurrencyName>", "");
		line_content.Replace("<CurrencyName>", "");
		line_content = FileReadHelper.TrimComment(line_content);
		m_Trader_CurrencyName = line_content;

		int currencyCounter = 0;

		line_content = "";
		while (currencyCounter <= 500 && !line_content.Contains("<Trader>"))
		{
			line_content = FileReadHelper.SearchForNextTermInFile(file_index, "<Currency>", "<Trader>");
			line_content.Replace("<Currency>", "");
			line_content = FileReadHelper.TrimComment(line_content);

			if (line_content.Contains("<Trader>"))
				break;

			TraderMessage.ServerLog("[TRADER] READING CURRENCY ENTRY..");

			TStringArray crys = new TStringArray;
			line_content.Split( ",", crys );

			string currencyClassname = crys.Get(0);
			currencyClassname = FileReadHelper.TrimSpaces(currencyClassname);
			
			string currencyValue = crys.Get(1);
			currencyValue = FileReadHelper.TrimSpaces(currencyValue);

			m_Trader_CurrencyClassnames.Insert(currencyClassname);
			m_Trader_CurrencyValues.Insert(currencyValue.ToInt());
		}

		bool traderInstanceDone = true;
		int traderCounter = 0;
		
		//line_content = "";
		while (traderCounter <= 500 && line_content != "<FileEnd>")
		{
			TraderMessage.ServerLog("[TRADER] READING TRADER ENTRY..");
			
			if (traderInstanceDone == false)
				line_content = FileReadHelper.SearchForNextTermInFile(file_index, "<Trader>", "");
			else
				traderInstanceDone = false;
			
			line_content.Replace("<Trader>", "");
			line_content = FileReadHelper.TrimComment(line_content);
			
			m_Trader_TraderNames.Insert(line_content);
				
			int categoryCounter = 0;
			
			line_content = "";
			while (categoryCounter <= 500 && line_content != "<FileEnd>")
			{
				line_content = FileReadHelper.TrimComment(FileReadHelper.SearchForNextTermInFile(file_index, "<Category>", "<Trader>"));
				
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
				
				TraderMessage.ServerLog("[TRADER] READING CATEGORY ENTRY..");
				line_content.Replace("<Category>", "");
				m_Trader_Categorys.Insert(FileReadHelper.TrimComment(line_content));
				m_Trader_CategorysTraderKey.Insert(traderCounter);
				
				categoryCounter++;
			}
			
			traderCounter++;
		}
		
		CloseFile(file_index);
		
		//------------------------------------------------------------------------------------
		
		file_index = OpenFile(m_Trader_ConfigFilePath, FileMode.READ);
		
		int itemCounter = 0;
		int char_count = 0;
		int traderID = -1;
		int categoryId = -1;
		
		line_content = "";
		while ( itemCounter <= 5000 && char_count != -1 && line_content.Contains("<FileEnd>") == false)
		{
			char_count = FGets( file_index,  line_content );
			
			line_content = FileReadHelper.TrimComment(line_content);

			if (line_content.Contains("<Trader>"))
			{
				traderID++;
				
				continue;
			}
			
			if (line_content.Contains("<Category>"))
			{
				categoryId++;
				
				continue;
			}
		
			if (!line_content.Contains(","))
				continue;

			if (line_content.Contains("<Currency"))
				continue;

			TraderMessage.ServerLog("[TRADER] READING ITEM ENTRY..");
		
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

			if (qntStr.Contains("V") || qntStr.Contains("v"))
				qntStr = "-2";

			if (qntStr.Contains("M") || qntStr.Contains("m"))
				qntStr = "-3";

			if (qntStr.Contains("W") || qntStr.Contains("w"))
				qntStr = "-4";
			
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
		while ( markerCounter <= 5000 && line_content.Contains("<FileEnd>") == false)
		{
			// Get Trader Marker Trader ID:
			if (!skipLine)
				line_content = FileReadHelper.SearchForNextTermInFile(file_index, "<TraderMarker>", "<FileEnd>");
			else
				skipLine = false;					
			
			if (!line_content.Contains("<TraderMarker>"))
				continue;
			
			line_content.Replace("<TraderMarker>", "");
			line_content = FileReadHelper.TrimComment(line_content);
			line_content = FileReadHelper.TrimSpaces(line_content);
			
			TraderMessage.ServerLog("[TRADER] READING MARKER ID ENTRY..");
								
			m_Trader_TraderIDs.Insert(line_content.ToInt());
			
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
			
			TraderMessage.ServerLog("[TRADER] READING MARKER POSITION ENTRY..");
			
			m_Trader_TraderPositions.Insert(markerPosition);
			
			// Get Trader Marker Safezone Radius:					
			line_content = FileReadHelper.SearchForNextTermInFile(file_index, "<TraderMarkerSafezone>", "<FileEnd>");
			
			line_content.Replace("<TraderMarkerSafezone>", "");
			line_content = FileReadHelper.TrimComment(line_content);
			line_content = FileReadHelper.TrimSpaces(line_content);
			
			TraderMessage.ServerLog("[TRADER] READING MARKER SAFEZONE ENTRY..");
			
			m_Trader_TraderSafezones.Insert(line_content.ToInt());

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

			TraderMessage.ServerLog("[TRADER] READING MARKER VEHICLE ENTRY..");

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
				line_content = FileReadHelper.SearchForNextTermInFile(file_index, "<VehicleParts>", "<FileEnd>");
			else
				skipLine = false;
			
			if (!line_content.Contains("<VehicleParts>"))
				continue;
			
			line_content.Replace("<VehicleParts>", "");
			line_content = FileReadHelper.TrimComment(line_content);
			line_content = FileReadHelper.TrimSpaces(line_content);
			
			TraderMessage.ServerLog("[TRADER] READING VEHICLE NAME ENTRY..");

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

		TraderMessage.ServerLog("[TRADER] DONE READING!");
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

	void SetPlayerVehicleIsInSafezone( PlayerBase player, bool isInSafezone )
	{
		CarScript car = CarScript.Cast(player.GetParent());

		if (car)
			car.m_Trader_IsInSafezone = isInSafezone;

		for (int j = 0; j < m_Players.Count(); j++)
		{
			PlayerBase currentPlayer = PlayerBase.Cast(m_Players.Get(j));
			
			if ( !currentPlayer )
				continue;

			GetGame().RPCSingleParam(currentPlayer, TRPCs.RPC_SYNC_CARSCRIPT_ISINSAFEZONE, new Param2<CarScript, bool>( car, isInSafezone ), true, currentPlayer.GetIdentity());
		}
	}
}