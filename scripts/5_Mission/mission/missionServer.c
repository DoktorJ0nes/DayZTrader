modded class MissionServer
{
	static const string m_Trader_ConfigFilePath = "$profile:Trader/TraderConfig.txt";
	static const string m_Trader_ObjectsFilePath = "$profile:Trader/TraderObjects.txt";
	static const string m_Trader_VehiclePartsFilePath = "$profile:Trader/TraderVehicleParts.txt";

	bool m_Trader_ReadAllTraderData = false;

	string m_Trader_CurrencyItemType;
	
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

	float m_Trader_PlayerHiveUpdateTime = 0;
	float m_Trader_SpawnedFireBarrelsUpdateTimer = 0;
	
	override void OnInit()
	{		
		super.OnInit();

		SpawnTraderObjects();
		readTraderData();
	}
	
	override void OnUpdate(float timeslice)
	{
		super.OnUpdate(timeslice);
		

		m_Trader_PlayerHiveUpdateTime += timeslice;
		if (m_Trader_PlayerHiveUpdateTime >= 1)
			m_Trader_PlayerHiveUpdateTime = 0;

		for (int j = 0; j < m_Players.Count(); j++)
		{
			PlayerBase player = PlayerBase.Cast(m_Players.Get(j));
			
			if ( !player )
				continue;
			
			if ( !player.m_Trader_WelcomeMessageHandled && player.IsAlive() )
			{				
				if (player.m_Trader_WelcomeMessageTimer > 0)
					player.m_Trader_WelcomeMessageTimer -= timeslice;
				else
				{
					if ( !player.m_Trader_TraderModIsLoaded )
					{
						Param1<string> msgRp0 = new Param1<string>( " " );
						GetGame().RPCSingleParam(player, ERPCs.RPC_USER_ACTION_MESSAGE, msgRp0, true, player.GetIdentity());
						
						msgRp0.param1 = "This Server uses Dr_J0nes Trader Mod.";
						GetGame().RPCSingleParam(player, ERPCs.RPC_USER_ACTION_MESSAGE, msgRp0, true, player.GetIdentity());
						
						msgRp0.param1 = "Please download and install it!";
						GetGame().RPCSingleParam(player, ERPCs.RPC_USER_ACTION_MESSAGE, msgRp0, true, player.GetIdentity());
						
						msgRp0.param1 = " ";
						GetGame().RPCSingleParam(player, ERPCs.RPC_USER_ACTION_MESSAGE, msgRp0, true, player.GetIdentity());

						Print("PLAYER WITHOUT TRADER MOD DETECTED!");
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
			
			if (player.m_Trader_IsInSafezone == false && isInSafezone == true)
			{
				player.m_Trader_IsInSafezone = true;
				GetGame().RPCSingleParam(player, TRPCs.RPC_SEND_TRADER_IS_IN_SAFEZONE, new Param1<bool>( true ), true, player.GetIdentity());
				player.m_Trader_HealthEnteredSafeZone = player.GetHealth( "", "");
				player.m_Trader_HealthBloodEnteredSafeZone = player.GetHealth( "", "Blood" );
				player.GetInputController().OverrideRaise(true, false);
				SetPlayerVehicleInSafezone( player, true );
				
				GetGame().RPCSingleParam(player, ERPCs.RPC_USER_ACTION_MESSAGE, new Param1<string>( "[Trader] You entered the Safezone!" ), true, player.GetIdentity());
				GetGame().RPCSingleParam(player, ERPCs.RPC_USER_ACTION_MESSAGE, new Param1<string>( "Press 'B'-Key to open the Trader Menu." ), true, player.GetIdentity());
			}
			
			if (player.m_Trader_IsInSafezone == true && isInSafezone == false)
			{
				player.m_Trader_IsInSafezone = false;
				GetGame().RPCSingleParam(player, TRPCs.RPC_SEND_TRADER_IS_IN_SAFEZONE, new Param1<bool>( false ), true, player.GetIdentity());
				player.GetInputController().OverrideRaise(false, false);
				SetPlayerVehicleInSafezone( player, false );
				
				GetGame().RPCSingleParam(player, ERPCs.RPC_USER_ACTION_MESSAGE, new Param1<string>( "[Trader] You left the Safezone!" ), true, player.GetIdentity());
			}
			
			if (isInSafezone)
			{
				if (player.GetHealth( "", "") > player.m_Trader_HealthEnteredSafeZone) // allow regaining Health
					player.m_Trader_HealthEnteredSafeZone = player.GetHealth( "", "");
					
				if (player.GetHealth( "", "Blood" ) > player.m_Trader_HealthBloodEnteredSafeZone) // allow regaining Blood
					player.m_Trader_HealthBloodEnteredSafeZone = player.GetHealth( "", "Blood" );
				
				player.SetHealth( "", "", player.m_Trader_HealthEnteredSafeZone );
				player.SetHealth( "","Blood", player.m_Trader_HealthBloodEnteredSafeZone );

				player.SetHealth( "","Shock", player.GetMaxHealth( "", "Shock" ) );

				//player.GetStatStamina().Set(1000);

				if (m_Trader_PlayerHiveUpdateTime == 0 && player.IsAlive())
					GetHive().CharacterSave(player);
			}
		}
		
		for ( int i = 0; i < m_Trader_SpawnedTraderCharacters.Count(); i++ )
		{
			m_Trader_SpawnedTraderCharacters.Get(i).SetHealth( m_Trader_SpawnedTraderCharacters.Get(i).GetMaxHealth( "", "" ) );
			m_Trader_SpawnedTraderCharacters.Get(i).SetHealth( "","Blood", m_Trader_SpawnedTraderCharacters.Get(i).GetMaxHealth( "", "Blood" ) );
			m_Trader_SpawnedTraderCharacters.Get(i).GetStatEnergy().Set(1000);
			m_Trader_SpawnedTraderCharacters.Get(i).GetStatWater().Set(1000);
			m_Trader_SpawnedTraderCharacters.Get(i).GetStatStomachVolume().Set(300);		
			m_Trader_SpawnedTraderCharacters.Get(i).GetStatStomachWater().Set(300);
			m_Trader_SpawnedTraderCharacters.Get(i).GetStatStomachEnergy().Set(300);
			m_Trader_SpawnedTraderCharacters.Get(i).GetStatHeatComfort().Set(0);
			m_Trader_SpawnedTraderCharacters.Get(i).SetHealth( "","Shock", m_Trader_SpawnedTraderCharacters.Get(i).GetMaxHealth( "", "Shock" ) );
		}

		m_Trader_SpawnedFireBarrelsUpdateTimer += timeslice;
		if (m_Trader_SpawnedFireBarrelsUpdateTimer >= 1)
		{
			m_Trader_SpawnedFireBarrelsUpdateTimer = 0;

			for (int m = 0; m < m_Trader_SpawnedFireBarrels.Count(); m++)
			{
				BarrelHoles_ColorBase ntarget = m_Trader_SpawnedFireBarrels.Get(m);

				if (!ntarget.IsBurning() && !ntarget.IsWet())
				{
					ItemBase kindling;
					kindling = ItemBase.Cast(ntarget.GetInventory().CreateAttachment("Rag"));
					kindling.SetQuantityMax();
					//kindling = ItemBase.Cast(ntarget.GetInventory().CreateAttachment("BandageDressing"));
					//kindling.SetQuantityMax();
					kindling = ItemBase.Cast(ntarget.GetInventory().CreateAttachment("Bark_Oak"));
					kindling.SetQuantityMax();
					kindling = ItemBase.Cast(ntarget.GetInventory().CreateAttachment("Bark_Birch"));
					kindling.SetQuantityMax();

					ItemBase fuel;
					fuel = ItemBase.Cast(ntarget.GetInventory().CreateAttachment("Firewood"));
					fuel.SetQuantityMax();
					fuel = ItemBase.Cast(ntarget.GetInventory().CreateAttachment("WoodenStick"));
					fuel.SetQuantityMax();

					EntityAI fire_source_dummy;
					ntarget.OnIgnitedThis(fire_source_dummy);
				}

				if (ntarget.IsWet())
					ntarget.SetWet(ntarget.GetWetMin());
			}
		}
	}
	
	private void SpawnTraderObjects()
	{
		m_Trader_SpawnedTraderCharacters = new array<PlayerBase>;
		m_Trader_SpawnedFireBarrels = new array<BarrelHoles_ColorBase>;
		
		TraderServerLogs.PrintS("[TRADER] DEBUG START");
		
		FileHandle file_index = OpenFile(m_Trader_ObjectsFilePath, FileMode.READ);
				
		if ( file_index == 0 )
		{
			TraderServerLogs.PrintS( "[TRADER] FOUND NO TRADEROBJECTS FILE!" );
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
			
			TraderServerLogs.PrintS("[TRADER] READING OBJECT TYPE ENTRY..");
			
			string traderObjectType = line_content;
			
			// Get Object Position --------------------------------------------------------------------------------
			line_content = FileReadHelper.SearchForNextTermInFile(file_index, "<ObjectPosition>", "<FileEnd>");
			
			line_content.Replace("<ObjectPosition>", "");
			line_content = FileReadHelper.TrimComment(line_content);
			
			TraderServerLogs.PrintS("[TRADER] READING OBJECT POSITION ENTRY..");
			
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
			TraderServerLogs.PrintS("[TRADER] SPAWNED OBJECT '" + traderObjectType + "' AT '" + objectPosition + "'");
			
			// HANDLE TRADER FIRE BARREL BEGIN
			if (isPersistant)
			{
				BarrelHoles_ColorBase ntarget = BarrelHoles_ColorBase.Cast( obj );
				if( ntarget )
				{
					ntarget.Trader_IsInSafezone = true;

					ntarget.Open();
					TraderServerLogs.PrintS("[TRADER] OPENED BARREL");

					m_Trader_SpawnedFireBarrels.Insert(ntarget);
				}
			}
			// HANDLE TRADER FIRE BARREL END

			bool isTrader = false;
			PlayerBase man;
			if (Class.CastTo(man, obj))
			{
				TraderServerLogs.PrintS("[TRADER] Object was a Man..");
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
					man.m_Trader_IsTrader = true;
					m_Trader_SpawnedTraderCharacters.Insert(man);
				}
				
				skipDirEntry = true;
				markerCounter++;
				continue;
			}
			
			if (!line_content.Contains("<ObjectOrientation>"))
				continue;
			
			line_content.Replace("<ObjectOrientation>", "");
			line_content = FileReadHelper.TrimComment(line_content);
			
			TraderServerLogs.PrintS("[TRADER] READING OBJECT ORIENTATION ENTRY..");
			
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

			TraderServerLogs.PrintS("[TRADER] OBJECT ORIENTATION = '" + obj.GetOrientation() + "'");

			if (isTrader)
			{
				man.m_Trader_IsInSafezone = true;
				m_Trader_SpawnedTraderCharacters.Insert(man);
			}
						
			
			markerCounter++;
		}
		
		CloseFile(file_index);
		TraderServerLogs.PrintS("[TRADER] DEBUG END");
	}

	private void sendTraderDataToPlayer(PlayerBase player)
	{
		TraderServerLogs.PrintS("[TRADER] SEND DATA TO PLAYER");

		// request client to clear all data:
		Param1<bool> crpClr = new Param1<bool>( true );
		GetGame().RPCSingleParam(player, TRPCs.RPC_SEND_TRADER_CLEAR, crpClr, true, player.GetIdentity());

		Param1<string> crp1 = new Param1<string>( m_Trader_CurrencyItemType );
		GetGame().RPCSingleParam(player, TRPCs.RPC_SEND_TRADER_CURRENCYTYPE_ENTRY, crp1, true, player.GetIdentity());
		//TraderServerLogs.PrintS("[TRADER] CURRENCYTYPE: " + m_Trader_CurrencyItemType);
		
		int i = 0;
		for ( i = 0; i < m_Trader_TraderNames.Count(); i++ )
		{
			Param1<string> crp2 = new Param1<string>( m_Trader_TraderNames.Get(i) );
			GetGame().RPCSingleParam(player, TRPCs.RPC_SEND_TRADER_NAME_ENTRY, crp2, true, player.GetIdentity());
			//TraderServerLogs.PrintS("[TRADER] TRADERNAME: " + m_Trader_TraderNames.Get(i));
		}
		
		for ( i = 0; i < m_Trader_Categorys.Count(); i++ )
		{
			Param2<string, int> crp3 = new Param2<string, int>( m_Trader_Categorys.Get(i), m_Trader_CategorysTraderKey.Get(i) );
			GetGame().RPCSingleParam(player, TRPCs.RPC_SEND_TRADER_CATEGORY_ENTRY, crp3, true, player.GetIdentity());
			//TraderServerLogs.PrintS("[TRADER] TRADERCATEGORY: " + m_Trader_Categorys.Get(i) + ", " + m_Trader_CategorysTraderKey.Get(i));
		}
		
		for ( i = 0; i < m_Trader_ItemsClassnames.Count(); i++ )
		{
			Param6<int, int, string, int, int, int> crp4 = new Param6<int, int, string, int, int, int>( m_Trader_ItemsTraderId.Get(i), m_Trader_ItemsCategoryId.Get(i), m_Trader_ItemsClassnames.Get(i), m_Trader_ItemsQuantity.Get(i), m_Trader_ItemsBuyValue.Get(i), m_Trader_ItemsSellValue.Get(i) );
			GetGame().RPCSingleParam(player, TRPCs.RPC_SEND_TRADER_ITEM_ENTRY, crp4, true, player.GetIdentity());
			//TraderServerLogs.PrintS("[TRADER] ITEMENTRY: " + m_Trader_ItemsTraderId.Get(i) + ", " + m_Trader_ItemsCategoryId.Get(i) + ", " + m_Trader_ItemsClassnames.Get(i) + ", " + m_Trader_ItemsQuantity.Get(i) + ", " + m_Trader_ItemsBuyValue.Get(i) + ", " + m_Trader_ItemsSellValue.Get(i));
		}
		
		for ( i = 0; i < m_Trader_TraderPositions.Count(); i++ )
		{
			Param5<int, vector, int, vector, vector> crp5 = new Param5<int, vector, int, vector, vector>( m_Trader_TraderIDs.Get(i), m_Trader_TraderPositions.Get(i), m_Trader_TraderSafezones.Get(i), m_Trader_TraderVehicleSpawns.Get(i), m_Trader_TraderVehicleSpawnsOrientation.Get(i) );
			GetGame().RPCSingleParam(player, TRPCs.RPC_SEND_TRADER_MARKER_ENTRY, crp5, true, player.GetIdentity());
			//TraderServerLogs.PrintS("[TRADER] MARKERENTRY: " + m_Trader_TraderIDs.Get(i) + ", " + m_Trader_TraderPositions.Get(i) + ", " + m_Trader_TraderSafezones.Get(i) + ", " + m_Trader_TraderVehicleSpawns.Get(i) + ", " + m_Trader_TraderVehicleSpawnsOrientation.Get(i));
		}

		// Only stored Serverside:
		player.m_Trader_Vehicles = new array<string>;
		for ( i = 0; i < m_Trader_Vehicles.Count(); i++ )
		{
			player.m_Trader_Vehicles.Insert(m_Trader_Vehicles.Get(i));
			//TraderServerLogs.PrintS("[TRADER] VEHICLEENTRY: " + m_Trader_Vehicles.Get(i));
		}

		// Only stored Serverside:
		player.m_Trader_VehiclesParts = new array<string>;
		player.m_Trader_VehiclesPartsVehicleId = new array<int>;
		for ( i = 0; i < m_Trader_VehiclesParts.Count(); i++ )
		{
			player.m_Trader_VehiclesParts.Insert(m_Trader_VehiclesParts.Get(i));
			player.m_Trader_VehiclesPartsVehicleId.Insert(m_Trader_VehiclesPartsVehicleId.Get(i));
			//TraderServerLogs.PrintS("[TRADER] VEHICLEPARTENTRY: " + m_Trader_VehiclesPartsVehicleId.Get(i) + ", " + m_Trader_VehiclesParts.Get(i));
		}
		
		// confirm that all data was sended:
		player.m_Trader_RecievedAllData = true;
		
		Param1<bool> crpConf = new Param1<bool>( true );
		GetGame().RPCSingleParam(player, TRPCs.RPC_SEND_TRADER_DATA_CONFIRMATION, crpConf, true, player.GetIdentity());
		//TraderServerLogs.PrintS("[TRADER] DEBUG END");
	}

	private void readTraderData()
	{		
		// clear all data here:
		m_Trader_ReadAllTraderData = false;	
		m_Trader_CurrencyItemType = "";
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

		TraderServerLogs.PrintS("[TRADER] DEBUG START");
		
		FileHandle file_index = OpenFile(m_Trader_ConfigFilePath, FileMode.READ);
		
		if ( file_index == 0 )
		{
			TraderServerLogs.PrintS("[TRADER] FOUND NO TRADERCONFIG FILE!");
			return;
		}
		
		string line_content = "";
		
		TraderServerLogs.PrintS("[TRADER] READING CURRENCY ENTRY..");
		line_content = FileReadHelper.SearchForNextTermInFile(file_index, "<Currency>", "");
		line_content.Replace("<Currency>", "");
		line_content = FileReadHelper.TrimComment(line_content);
		m_Trader_CurrencyItemType = line_content;
		
		bool traderInstanceDone = false;
		int traderCounter = 0;
		
		line_content = "";
		while (traderCounter <= 500 && line_content != "<FileEnd>")
		{
			TraderServerLogs.PrintS("[TRADER] READING TRADER ENTRY..");
			
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
				
				TraderServerLogs.PrintS("[TRADER] READING CATEGORY ENTRY..");
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
			TraderServerLogs.PrintS("[TRADER] READING ITEM ENTRY..");
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
			TraderServerLogs.PrintS("[TRADER] FOUND NO TRADEROBJECTS FILE!");
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
			
			TraderServerLogs.PrintS("[TRADER] READING MARKER ID ENTRY..");
								
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
			
			TraderServerLogs.PrintS("[TRADER] READING MARKER POSITION ENTRY..");
			
			m_Trader_TraderPositions.Insert(markerPosition);
			
			// Get Trader Marker Safezone Radius:					
			line_content = FileReadHelper.SearchForNextTermInFile(file_index, "<TraderMarkerSafezone>", "<FileEnd>");
			
			line_content.Replace("<TraderMarkerSafezone>", "");
			line_content = FileReadHelper.TrimComment(line_content);
			line_content = FileReadHelper.TrimSpaces(line_content);
			
			TraderServerLogs.PrintS("[TRADER] READING MARKER SAFEZONE ENTRY..");
			
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

			TraderServerLogs.PrintS("[TRADER] READING MARKER VEHICLE ENTRY..");

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
			Print("[TRADER] FOUND NO VEHICLEPARTS FILE!");
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
			
			TraderServerLogs.PrintS("[TRADER] READING VEHICLE NAME ENTRY..");

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

		TraderServerLogs.PrintS("[TRADER] DONE READING!");
		m_Trader_ReadAllTraderData = true;
	}

	private int GetItemMaxQuantity(string itemClassname)
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

	private void SetPlayerVehicleInSafezone( PlayerBase player, bool isInSafezone )
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