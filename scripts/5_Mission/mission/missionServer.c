modded class MissionServer
{
	static const string m_Trader_ObjectsFilePath = "$profile:Trader/TraderObjects.txt";

	// WORKAROUND PREVENT SIMULTAIN FILE READING BEGIN
	bool IsReadingTraderConfigFile = false;
	// WORKAROUND PREVENT SIMULTAIN FILE READING END
		
	ref array<PlayerBase> m_Trader_SpawnedTraderCharacters;
	float m_Trader_PlayerHiveUpdateTime = 0;
	
	override void OnInit()
	{		
		super.OnInit();

		SpawnTraderObjects();
	}
	
	override void OnUpdate(float timeslice)
	{
		super.OnUpdate(timeslice);
		

		m_Trader_PlayerHiveUpdateTime += timeslice;
		if (m_Trader_PlayerHiveUpdateTime >= 1)
			m_Trader_PlayerHiveUpdateTime = 0;
		
		// WORKAROUND PREVENT SIMULTAIN FILE READING BEGIN
		bool somePlayerIsReadingTraderConfigFile = false;
		for (int z = 0; z < m_Players.Count(); z++)
		{
			PlayerBase playerz = PlayerBase.Cast(m_Players.Get(z));

			if (playerz.m_Trader_IsReadingTraderFileEntrys)
			{
				somePlayerIsReadingTraderConfigFile = true;
				break;
			}
		}
		// WORKAROUND PREVENT SIMULTAIN FILE READING END

		for (int j = 0; j < m_Players.Count(); j++)
		{
			PlayerBase player = PlayerBase.Cast(m_Players.Get(j));
			
			if ( !player )
				continue;

			// WORKAROUND PREVENT SIMULTAIN FILE READING BEGIN
			if (somePlayerIsReadingTraderConfigFile)
				player.serverIsReadingTheTraderConfigFile = true;
			else
				player.serverIsReadingTheTraderConfigFile = false;
			// WORKAROUND PREVENT SIMULTAIN FILE READING END
			
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

						Print("PLAYER WITHOUT TRADER MOD DETECTED!")
					}
					
					player.m_Trader_WelcomeMessageHandled = true;
				}
			}

			// Kill player after some Time when not have the Mod loaded:
			if (player.m_Trader_WelcomeMessageHandled && !player.m_Trader_TraderModIsLoaded && player.IsAlive())
			{
				player.m_Trader_WelcomeMessageTimer -= timeslice;
				if (player.m_Trader_WelcomeMessageTimer < -5)
				{
					player.m_Trader_WelcomeMessageHandled = false;
					player.m_Trader_WelcomeMessageTimer = 25;

					player.SetHealth( "", "", 0 );
					player.SetHealth( "", "Blood", 0 );
				}
			}
			
			if (!player.m_Trader_RecievedAllData) // TODO: Wenn Server die TraderPositions und Safezoneradien kennt, dann immer zulassen! 
				continue;	
			
			bool isInSafezone = false;
			
			for (int k = 0; k < player.m_Trader_TraderPositions.Count(); k++)
			{
				if (vector.Distance(player.GetPosition(), player.m_Trader_TraderPositions.Get(k)) <= player.m_Trader_TraderSafezones.Get(k))
					isInSafezone = true;
			}
			
			if (player.m_Trader_IsInSafezone == false && isInSafezone == true)
			{
				player.m_Trader_IsInSafezone = true;
				GetGame().RPCSingleParam(player, TRPCs.RPC_SEND_TRADER_IS_IN_SAFEZONE, new Param1<bool>( true ), true, player.GetIdentity());
				player.m_Trader_HealthEnteredSafeZone = player.GetHealth( "", "");
				player.m_Trader_HealthBloodEnteredSafeZone = player.GetHealth( "", "Blood" );
				
				GetGame().RPCSingleParam(player, ERPCs.RPC_USER_ACTION_MESSAGE, new Param1<string>( "[Trader] You entered the Safezone!" ), true, player.GetIdentity());
				GetGame().RPCSingleParam(player, ERPCs.RPC_USER_ACTION_MESSAGE, new Param1<string>( "Press 'B'-Key to open the Trader Menu." ), true, player.GetIdentity());
			}
			
			if (player.m_Trader_IsInSafezone == true && isInSafezone == false)
			{
				player.m_Trader_IsInSafezone = false;
				GetGame().RPCSingleParam(player, TRPCs.RPC_SEND_TRADER_IS_IN_SAFEZONE, new Param1<bool>( false ), true, player.GetIdentity());
				
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
	}
	
	private void SpawnTraderObjects()
	{
		m_Trader_SpawnedTraderCharacters = new array<PlayerBase>;
		
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
			
			Object obj = GetGame().CreateObject( traderObjectType, objectPosition, false, false, true );
			obj.SetPosition(objectPosition); // prevent automatic on ground placing
			TraderServerLogs.PrintS("[TRADER] SPAWNED OBJECT '" + traderObjectType + "' AT '" + objectPosition + "'");
			
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

			if (isTrader)
			{
				man.m_Trader_IsInSafezone = true;
				m_Trader_SpawnedTraderCharacters.Insert(man);
			}
			
			TraderServerLogs.PrintS("[TRADER] OBJECT ORIENTATION = '" + obj.GetOrientation() + "'");
			
			markerCounter++;
		}
		
		CloseFile(file_index);
		TraderServerLogs.PrintS("[TRADER] DEBUG END");
	}
}