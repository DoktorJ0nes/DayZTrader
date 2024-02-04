class TraderItem
{
    string ClassName;
	int BuyValue;
	int SellValue;
	int Quantity;
	int IndexId;
};


class TraderMenu extends UIScriptedMenu
{
    MultilineTextWidget m_InfoBox;
    ButtonWidget m_BtnBuy;
	ButtonWidget m_BtnSell;
	ButtonWidget m_BtnCancel;
	TextListboxWidget m_ListboxItems;
	TextWidget m_Saldo;
	TextWidget m_SaldoValue;
	TextWidget m_TraderName;
	XComboBoxWidget m_XComboboxCategorys;
	ItemPreviewWidget m_ItemPreviewWidget;
	protected EntityAI previewItem;
	MultilineTextWidget m_ItemDescription;
	TextWidget m_ItemWeight;
	float m_UiUpdateTimer = 0;
	float m_UiSellTimer = 0;
	float m_UiBuyTimer = 0;
	float m_buySellTime = 0.3;

	bool m_active = false;
	
	int m_TraderID = -1;
	int m_TraderUID = -1;
	vector m_TraderVehicleSpawn = "0 0 0";
	vector m_TraderVehicleSpawnOrientation = "0 0 0";
	
	int m_Player_CurrencyAmount;
	int m_ColorBuyable;
	int m_ColorTooExpensive;
	int m_CategorysCurrentIndex;

	int m_LastRowIndex = -1;
	int m_LastCategoryCurrentIndex = -1;
	
	bool updateListbox = false;
	
	ref array<string> m_Categorys; // unnoetig ????
	ref array<int> m_CategorysTraderKey;
	ref array<int> m_CategorysKey;
	
	ref array<string> m_ListboxItemsClassnames;
	ref array<int> m_ListboxItemsQuantity;
	ref array<int> m_ListboxItemsBuyValue;
	ref array<int> m_ListboxItemsSellValue;

	ref array<int> m_ItemIDs;
	
	ref TStringArray m_FileContent;
	private bool                m_SellablesOnly = false;
	private int                 m_PreviewWidgetRotationX;
	private int                 m_PreviewWidgetRotationY;
	private vector              m_PreviewWidgetOrientation;	
	private int 				m_characterScaleDelta;
	TextWidget 					m_ItemQuantity;
    private string              m_SearchFilter = "";
    private string              m_OldSearchFilter = "";
    private EditBoxWidget		m_SearchBox; 
    private CheckBoxWidget		m_SellablesCheckbox; 
	ref array<ref TraderItem> m_FilteredListOfTraderItems;
	ref array<ref TraderItem> m_ListOfCategoryTraderItems;
	ref array<ref TraderItem> m_ListOfTraderItems;
	const string m_QuantString = "Quantity: ";
	const string m_SizeString = "Cargo size: ";

	private PlayerBase m_Player;
    EntityAI previewItemKit;

	void TraderMenu()
	{		
		m_Player_CurrencyAmount = 0;
		m_ColorBuyable = 0;
		m_ColorTooExpensive = 1;
	}	
	
	void ~TraderMenu()
	{
		if ( previewItem ) 
		{
			GetGame().ObjectDelete( previewItem );
		}
	}

    override Widget Init()
    {
		m_Player = PlayerBase.Cast(GetGame().GetPlayer());
		layoutRoot = GetGame().GetWorkspace().CreateWidgets( "TM/Trader/scripts/layouts/TraderMenu.layout" );

        m_BtnBuy = ButtonWidget.Cast( layoutRoot.FindAnyWidget( "btn_buy" ) );
		m_BtnSell = ButtonWidget.Cast( layoutRoot.FindAnyWidget( "btn_sell" ) );
		m_BtnCancel = ButtonWidget.Cast( layoutRoot.FindAnyWidget( "btn_cancel" ) );
		m_ListboxItems = TextListboxWidget.Cast(layoutRoot.FindAnyWidget("txtlist_items") );
		m_Saldo = TextWidget.Cast(layoutRoot.FindAnyWidget("text_saldo") );
		m_SaldoValue = TextWidget.Cast(layoutRoot.FindAnyWidget("text_saldoValue") );
		m_TraderName = TextWidget.Cast(layoutRoot.FindAnyWidget("title_text") );
		m_XComboboxCategorys = XComboBoxWidget.Cast( layoutRoot.FindAnyWidget( "xcombobox_categorys" ) );
		m_ItemDescription = MultilineTextWidget.Cast( layoutRoot.FindAnyWidget( "ItemDescWidget" ) );
		m_ItemWeight = TextWidget.Cast(layoutRoot.FindAnyWidget("ItemWeight") );
		m_ItemQuantity = TextWidget.Cast(layoutRoot.FindAnyWidget("ItemQuantity"));
		m_SearchBox    = EditBoxWidget.Cast( layoutRoot.FindAnyWidget( "SearchBox" ) );
		m_SellablesCheckbox = CheckBoxWidget.Cast(layoutRoot.FindAnyWidget( "SellablesCheckBox" ) );
		m_SellablesCheckbox.SetChecked(false);
        return layoutRoot;
    }

	void InitTraderValues()
	{		
		m_Categorys = new array<string>;
		m_CategorysTraderKey = new array<int>;
		m_CategorysKey = new array<int>;
        m_ListboxItemsClassnames = new array<string>;		
        m_FilteredListOfTraderItems = new array<ref TraderItem>;
        m_ListOfCategoryTraderItems = new array<ref TraderItem>;
        m_ListOfTraderItems = new array<ref TraderItem>;
		m_ListboxItemsQuantity = new array<int>;
		m_ListboxItemsBuyValue = new array<int>;
		m_ListboxItemsSellValue = new array<int>;
		m_ItemIDs = new array<int>;
		
		LoadCategories();
		m_CategorysCurrentIndex = 0;
		
		updateItemListboxContent();		
		m_ListboxItems.SelectRow(0);

		updatePlayerCurrencyAmount();
		updateItemListboxColors();
	}
	
	override void Update(float timeslice)
	{
		super.Update(timeslice);
		if (m_UiSellTimer > 0)
			m_UiSellTimer -= timeslice;

		if (m_UiBuyTimer > 0)
			m_UiBuyTimer -= timeslice;

		
		if (m_UiUpdateTimer >= 0.05)
		{			           
			m_SearchFilter = m_SearchBox.GetText();
			if(m_SearchFilter != m_OldSearchFilter)
				SearchForItems();


			updatePlayerCurrencyAmount();				
			updateItemListboxColors();

			local int row_index = m_ListboxItems.GetSelectedRow();
			if ((m_LastRowIndex != row_index) || (m_LastCategoryCurrentIndex != m_CategorysCurrentIndex))
			{
				m_LastRowIndex = row_index;
				m_LastCategoryCurrentIndex = m_CategorysCurrentIndex;
				ResetMenu();
				if(!m_FilteredListOfTraderItems.Get(row_index))
				{
					m_UiUpdateTimer = 0;
					return;
				}

				string itemType = m_FilteredListOfTraderItems.Get(row_index).ClassName;
				updateItemPreview(itemType);
			}

			float playerDistanceToTrader = vector.Distance(m_Player.GetPosition(), m_Player.m_Trader_TraderPositions.Get(m_TraderUID));
			if (playerDistanceToTrader > TR_Helper.GetTraderAllowedTradeDistance())
				GetGame().GetUIManager().Back();

			m_UiUpdateTimer = 0;
		}
		else
		{
			m_UiUpdateTimer = m_UiUpdateTimer + timeslice;
		}
	}

	override void OnShow()
	{		
		super.OnShow();
		LockControls();
		PPEffects.SetBlurMenu(0.5);
	}
	
	override void OnHide()
	{
		super.OnHide();
		UnlockControls();
		PPEffects.SetBlurMenu(0);
		if ( previewItem ) 
		{
			GetGame().ObjectDelete( previewItem );
		}
	}
	
    override void LockControls()
    {
        GetGame().GetMission().PlayerControlDisable(INPUT_EXCLUDE_ALL);
        GetGame().GetUIManager().ShowUICursor( true );
        GetGame().GetMission().GetHud().Show( false );
    }

    override void UnlockControls()
    {
		GetGame().GetMission().PlayerControlEnable(false);
        GetGame().GetInput().ResetGameFocus();
        GetGame().GetUIManager().ShowUICursor( false );
        GetGame().GetMission().GetHud().Show( true );
    }

	override bool OnClick( Widget w, int x, int y, int button )
	{			
		if ( w == m_SellablesCheckbox )
		{
			m_SellablesCheckbox.SetChecked( !m_SellablesOnly );
			m_SellablesOnly = !m_SellablesOnly;
			SearchForItems();
		}	

		if ( w == m_BtnCancel )
		{
			Close();
			return true;
		}
		
		if (w == m_XComboboxCategorys)
		{
			if (updateListbox)
			{
				updateListbox = false;
				
				updateItemListboxContent();
				m_ListboxItems.SelectRow(0);

				updatePlayerCurrencyAmount();
				updateItemListboxColors();
			}
		}

		local int row_index = m_ListboxItems.GetSelectedRow();
		if(!m_FilteredListOfTraderItems.Get(row_index))
			return false;

		string itemType = m_FilteredListOfTraderItems.Get(row_index).ClassName;
		int itemQuantity = m_FilteredListOfTraderItems.Get(row_index).Quantity;
		
		if ( w == m_BtnBuy )
		{
            if(!previewItem)                
			{
				TraderMessage.PlayerWhite("You cannot buy this item. Item doesn't exist.", m_Player);
				return true;
			}
			if (m_UiBuyTimer > 0)
			{
				TraderMessage.PlayerWhite("#tm_not_that_fast", m_Player);
				return true;
			}
			m_UiBuyTimer = m_buySellTime;

			GetGame().RPCSingleParam(m_Player, TRPCs.RPC_BUY, new Param3<int, int, string>( m_TraderUID, m_FilteredListOfTraderItems.Get(row_index).IndexId, getItemDisplayName(itemType)), true);
			
			return true;
		}
		
		if ( w == m_BtnSell )
		{
			if (m_UiSellTimer > 0)
			{
				TraderMessage.PlayerWhite("#tm_not_that_fast", m_Player);
				return true;
			}
			m_UiSellTimer = m_buySellTime;

			GetGame().RPCSingleParam(m_Player, TRPCs.RPC_SELL, new Param3<int, int, string>( m_TraderUID, m_FilteredListOfTraderItems.Get(row_index).IndexId, getItemDisplayName(itemType)), true);
			
			return true;
		}

		return false;
	}
	
	override bool OnChange( Widget w, int x, int y, bool finished )
	{
		super.OnChange(w, x, y, finished);
		if (!finished) return false;
		
		m_CategorysCurrentIndex = m_XComboboxCategorys.GetCurrentItem();
		updateListbox = true;
		
		return false;
	}
	
	void ResetMenu()
	{
		if(previewItemKit)
			previewItemKit.Delete();
		if(previewItem)
			previewItem.Delete();
		m_ItemWeight.SetText("");
		m_ItemQuantity.SetText("");
		m_ItemDescription.SetText("");
	}

	void updateItemListboxContent()
	{		
		LoadItemsFromFile();	
		SearchForItems();
	}
	
	void updateItemListboxColors()
	{
		for (int i = 0; i < m_ListboxItems.GetNumItems(); i++)
		{
			int itemCosts = m_FilteredListOfTraderItems.Get(i).BuyValue;
			
			if (itemCosts < 0)
			{
				m_ListboxItems.SetItemColor(i, 1, ARGBF(0, 1, 1, 1) );
			}
			else if (m_Player_CurrencyAmount >= itemCosts)
			{
				m_ListboxItems.SetItemColor(i, 1, ARGBF(1, 1, 1, 1) );
			}
			else
			{
				m_ListboxItems.SetItemColor(i, 1, ARGBF(1, 1, 0, 0) );
			}
			
			string itemClassname = m_FilteredListOfTraderItems.Get(i).ClassName;
			int itemQuantity = m_FilteredListOfTraderItems.Get(i).Quantity;
			
			if (m_FilteredListOfTraderItems.Get(i).SellValue < 0)
			{
				m_ListboxItems.SetItemColor(i, 2, ARGBF(0, 1, 1, 1) );
			}
			else if (IsSellableOrInInventory(itemClassname, itemQuantity))
			{
				m_ListboxItems.SetItemColor(i, 2, ARGBF(1, 0, 1, 0) );
			}
			else
			{
				m_ListboxItems.SetItemColor(i, 2, ARGBF(1, 1, 1, 1) );
			}

			EntityAI entityInHands = m_Player.GetHumanInventory().GetEntityInHands();
			if (entityInHands)
			{
				if (IsAttached(entityInHands, itemClassname))
					m_ListboxItems.SetItemColor(i, 0, ARGBF(1, 0.4, 0.4, 1) );
				else if (IsAttachment(entityInHands, itemClassname))
					m_ListboxItems.SetItemColor(i, 0, ARGBF(1, 0.7, 0.7, 1) );
				else
					m_ListboxItems.SetItemColor(i, 0, ARGBF(1, 1, 1, 1) );
			}
		}
	}

	bool IsSellableOrInInventory(string itemClassname, int itemQuantity)
	{
		return isInPlayerInventory(itemClassname, itemQuantity) || ((itemQuantity == -6 || itemQuantity == -2) && GetVehicleToSell(itemClassname));
	}

	void updateItemPreview(string itemType)
	{
		if ( !m_ItemPreviewWidget )
			{
				Widget preview_frame = layoutRoot.FindAnyWidget("ItemFrameWidget");

				if ( preview_frame ) 
				{
					float width;
					float height;
					preview_frame.GetSize(width, height);

					m_ItemPreviewWidget = ItemPreviewWidget.Cast( GetGame().GetWorkspace().CreateWidget(ItemPreviewWidgetTypeID, 0, 0, 1, 1, WidgetFlags.VISIBLE, ARGB(255, 255, 255, 255), 10, preview_frame) );
				}
			}

			if ( previewItem )
				GetGame().ObjectDelete( previewItem );

			string lower = itemType;
			int leng = -1;
			string itemName = itemType;
            lower.ToLower();
			if(TR_Helper.KitIgnoreArray.Find(itemType) == -1 && lower.Contains("kit_"))
            {
                itemName = itemType.Substring(4,itemType.Length());  
            }
            else if(TR_Helper.KitIgnoreArray.Find(itemType) == -1 && lower.Contains("_kit"))
            {
                leng = itemType.Length() - 4;
                itemName = itemType.Substring(0,leng);
				if(lower.Contains("md_camonetshelter"))
					itemName = "Land_" + itemName;
            }
			else if(TR_Helper.KitIgnoreArray.Find(itemType) == -1 && lower.Contains("kit"))
            {
                leng = itemType.Length() - 3;      
                itemName = itemType.Substring(0,leng);  
            }
			previewItem = EntityAI.Cast(GetGame().CreateObject( itemName, "0 0 0", true, false, true ));
			if(!previewItem)
			{
				previewItem = EntityAI.Cast(GetGame().CreateObject( itemType, "0 0 0", true, false, true ));
			}
			m_ItemPreviewWidget.SetItem( previewItem );
            
			m_ItemPreviewWidget.SetModelPosition( Vector(1.0,1.0,0.5) );
			m_ItemPreviewWidget.SetModelOrientation( Vector(0,0,0) );

			float itemx, itemy;		
			m_ItemPreviewWidget.GetPos(itemx, itemy);
			m_ItemPreviewWidget.SetSize( 1.0, 1.0 );
			m_ItemPreviewWidget.SetPos( 0, 0 );

			if (previewItem)
			{
				local int row_index = m_ListboxItems.GetSelectedRow();
				int itemQuantity = m_FilteredListOfTraderItems.Get(row_index).Quantity;
				string itemQuant = m_QuantString + "1";				
				if(itemQuantity > 0 && !TR_Helper.HasQuantityBar(itemName))
				{
					itemQuant = m_QuantString + itemQuantity.ToString();
				}
				else
				{
					int sizeCount = TR_Helper.GetItemSlotCount(itemName);
					if(sizeCount > 0)
					{
						itemQuant = m_SizeString + sizeCount;
					}
				}
				m_ItemWeight.SetText(GetItemWeightText());
				m_ItemQuantity.SetText(itemQuant);
                if(TR_Helper.KitIgnoreArray.Find(itemType) == -1 && lower.Contains("kit") && previewItemKit)
                {
				    m_ItemDescription.SetText(string.Format("This item is a kit. %1",GetEntityAITooltip(previewItemKit)));
                }
                else
                {
				    m_ItemDescription.SetText(GetEntityAITooltip(previewItem));
                }
			}
			else
			{
				m_ItemWeight.SetText("");
				m_ItemDescription.SetText("ERROR FINDING ITEM. DO NOT BUY THIS ITEM.");
				m_ItemQuantity.SetText("");
			}
	}

	string GetItemWeightText()
	{
		ItemBase item_IB = ItemBase.Cast( previewItem );
		if(item_IB)
		{
			int weight = item_IB.GetSingleInventoryItemWeight();
			//string weightStr = "";
			
			if (weight >= 1000)
			{
				int kilos = Math.Round(weight / 1000.0);
				return  "#inv_inspect_about" + " " + kilos.ToString() + " " + "#inv_inspect_kg";
			}
			else if (weight >= 500)
			{
				return "#inv_inspect_under_1";
			} 
			else if (weight >= 250)
			{
				return "#inv_inspect_under_05";
			}
			else 
			{
				return "#inv_inspect_under_025";
			}			
		}

		return "";
	}
	
	void updatePlayerCurrencyAmount()
	{
		m_Player_CurrencyAmount = 0;
		m_Player_CurrencyAmount = getPlayerCurrencyAmount();
		m_SaldoValue.SetText(" " + m_Player_CurrencyAmount);
	}
	
	int getPlayerCurrencyAmount() // duplicate
	{		
		int currencyAmount = 0;
		
		array<EntityAI> itemsArray = new array<EntityAI>;
		m_Player.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, itemsArray);

		ItemBase item;
		
		for (int i = 0; i < itemsArray.Count(); i++)
		{
			Class.CastTo(item, itemsArray.Get(i));

			if (!item)
				continue;

			for (int j = 0; j < m_Player.m_Trader_CurrencyClassnames.Count(); j++)
			{
				if(item.GetType() == m_Player.m_Trader_CurrencyClassnames.Get(j))
				{
					currencyAmount += getItemAmount(item) * m_Player.m_Trader_CurrencyValues.Get(j);
				}
			}
		}
		
		return currencyAmount;
	}
	
	bool isInPlayerInventory(string itemClassname, int amount) // duplicate
	{
		itemClassname.ToLower();
		
		bool isMagazine = false;
		if (amount == -3)
			isMagazine = true;

		bool isWeapon = false;
		if (amount == -4)
			isWeapon = true;

		bool isSteak = false;
		if (amount == -5)
			isSteak = true;

		array<EntityAI> itemsArray = new array<EntityAI>;		
		m_Player.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, itemsArray);
		
		//TraderMessage.PlayerWhite("--------------");

		ItemBase item;		
		for (int i = 0; i < itemsArray.Count(); i++)
		{
			Class.CastTo(item, itemsArray.Get(i));
			string itemPlayerClassname = "";

			if (!item)
				continue;

			if (item.IsRuined())
				continue;

			if (isAttached(item))
				continue;

			itemPlayerClassname = item.GetType();
			itemPlayerClassname.ToLower();

			//TraderMessage.PlayerWhite("I: " + itemPlayerClassname + " == " + itemClassname);

			if(itemPlayerClassname == itemClassname && ((getItemAmount(item) >= amount && !isMagazine && !isWeapon && !isSteak) || isMagazine || isWeapon || (isSteak && (getItemAmount(item) >= GetItemMaxQuantity(itemPlayerClassname) * 0.5))))
			{
				return true;
			}
		}
		
		return false;
	}

	int GetItemMaxQuantity(string itemClassname) // duplicate
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
	
	int getItemAmount(ItemBase item) // duplicate
	{
		Magazine mgzn = Magazine.Cast(item);
				
		int itemAmount = 0;
		if( item.IsMagazine() )
		{
			itemAmount = mgzn.GetAmmoCount();
		}
		else
		{
			itemAmount = QuantityConversions.GetItemQuantity(item);
		}
		
		return itemAmount;
	}
	
	string getItemDisplayName(string itemClassname) // duplicate
	{
		TStringArray itemInfos = new TStringArray;
		
		string cfg = "CfgVehicles " + itemClassname + " displayName";
		string displayName;
		GetGame().ConfigGetText(cfg, displayName);
	
		if (displayName == "")
		{
			cfg = "CfgAmmo " + itemClassname + " displayName";
			GetGame().ConfigGetText(cfg, displayName);
		}
		
		if (displayName == "")
		{
			cfg = "CfgMagazines " + itemClassname + " displayName";
			GetGame().ConfigGetText(cfg, displayName);
		}
		
		if (displayName == "")
		{
			cfg = "cfgWeapons " + itemClassname + " displayName";
			GetGame().ConfigGetText(cfg, displayName);
		}
	
		if (displayName == "")
		{
			cfg = "CfgNonAIVehicles " + itemClassname + " displayName";
			GetGame().ConfigGetText(cfg, displayName);
		}
		
		
		if (displayName != "")
			return TrimUntPrefix(displayName);
		else
			return itemClassname;
	}

	Object GetVehicleToSell(string vehicleClassname) // duplicate
	{
		vector size = "3 5 9";
		array<Object> excluded_objects = new array<Object>;
		array<Object> nearby_objects = new array<Object>;

		if (GetGame().IsBoxColliding( m_TraderVehicleSpawn, m_TraderVehicleSpawnOrientation, size, excluded_objects, nearby_objects))
		{
			for (int i = 0; i < nearby_objects.Count(); i++)
			{
				if (nearby_objects.Get(i).GetType() == vehicleClassname)
				{
					// Check if there is any Player in the Vehicle:
					bool vehicleIsEmpty = true;

					Transport transport;
					Class.CastTo(transport, nearby_objects.Get(i));
					if (transport)
					{
						int crewSize = transport.CrewSize();
						for (int c = 0; c < crewSize; c++)
						{
							if (transport.CrewMember(c))
								vehicleIsEmpty = false;
						}
					}
					else
					{
						continue;
					}

					if (!vehicleIsEmpty)
						continue;

					// Check if Engine is running:
					/*Car car;
					Class.CastTo(car, nearby_objects.Get(i));
					if (car && vehicleIsEmpty)
					{
						if (car.EngineIsOn())
							return nearby_objects.Get(i);
					}*/

					return nearby_objects.Get(i);		
				}					
			}
		}

		return NULL;
	}

	bool IsAttached(EntityAI parentEntity, string attachmentClassname)
	{
		for ( int i = 0; i < parentEntity.GetInventory().AttachmentCount(); i++ )
		{
			EntityAI attachment = parentEntity.GetInventory().GetAttachmentFromIndex ( i );
			if ( attachment.IsKindOf ( attachmentClassname ) )
				return true;
		}

		return false;
	}

	bool isAttached(ItemBase item) // duplicate
	{
		EntityAI parent = item.GetHierarchyParent();

		if (!parent)
			return false;

		if (item.GetInventory().IsAttachment() || item.GetNumberOfItems() > 0)
			return true;

		if (parent.IsWeapon() || parent.IsMagazine())
			return true;

		return false;
	}

	bool IsAttachment(EntityAI parentEntity, string attachmentClassname)
	{
		// Check chamberable Ammunitions
		string type_name = parentEntity.GetType();
		TStringArray cfg_chamberables = new TStringArray;

		GetGame().ConfigGetTextArray(CFG_WEAPONSPATH + " " + type_name + " chamberableFrom", cfg_chamberables);

		for (int i = 0; i < cfg_chamberables.Count(); i++)
		{
			if (attachmentClassname == cfg_chamberables[i])
				return true;
		}


		// Check Magazines
		TStringArray cfg_magazines = new TStringArray;

		GetGame().ConfigGetTextArray(CFG_WEAPONSPATH + " " + type_name + " magazines", cfg_magazines);

		for (i = 0; i < cfg_magazines.Count(); i++)
		{
			if (attachmentClassname == cfg_magazines[i])
				return true;
		}


		return false;
	}
	
	bool LoadCategories()
	{		
		m_TraderName.SetText(m_Player.m_Trader_TraderNames.Get(m_TraderID));
		m_Saldo.SetText(m_Player.m_Trader_CurrencyName + ": ");

		m_XComboboxCategorys.ClearAll();
		m_Categorys = new array<string>;
		m_CategorysTraderKey = new array<int>;
		m_CategorysKey = new array<int>;
		
		for ( int i = 0; i < m_Player.m_Trader_Categorys.Count(); i++ )
		{
			if (m_TraderID != m_Player.m_Trader_CategorysTraderKey.Get(i))
				continue;
			
			m_XComboboxCategorys.AddItem(m_Player.m_Trader_Categorys.Get(i));
			m_Categorys.Insert(m_Player.m_Trader_Categorys.Get(i));
			m_CategorysTraderKey.Insert(m_Player.m_Trader_CategorysTraderKey.Get(i));
			m_CategorysKey.Insert(i);
		}
		
		return true;
	}
	
	bool LoadItemsFromFile()
	{
		m_ListOfTraderItems.Clear();
		m_ListOfCategoryTraderItems.Clear();
		m_FilteredListOfTraderItems.Clear();

		for ( int i = 0; i < m_Player.m_Trader_ItemsClassnames.Count(); i++ )
		{	
			if(m_Player.m_Trader_ItemsTraderId.Get(i) != m_TraderID)
				continue;

			TraderItem item = new TraderItem;
			item.ClassName = m_Player.m_Trader_ItemsClassnames.Get(i);
			item.Quantity = m_Player.m_Trader_ItemsQuantity.Get(i);
			item.BuyValue = m_Player.m_Trader_ItemsBuyValue.Get(i);
			item.SellValue = m_Player.m_Trader_ItemsSellValue.Get(i);
			item.IndexId = i;
			m_ListOfTraderItems.Insert(item);
			if ( m_Player.m_Trader_ItemsCategoryId.Get(i) == m_CategorysKey.Get(m_CategorysCurrentIndex) )
			{		
				m_ListOfCategoryTraderItems.Insert(item);
				m_FilteredListOfTraderItems.Insert(item);
			}
		}
		
		return true;
	}
		
	void SearchForItems()
    { 
		m_ListboxItems.ClearItems();
		m_FilteredListOfTraderItems.Clear();
        string displayName = "";
		int countFilter = 0;
       
		if(m_SearchFilter && m_SearchFilter != string.Empty)
		{
			countFilter = 0;
			foreach(TraderItem traderItem : m_ListOfTraderItems)
			{                
				displayName = getItemDisplayName(traderItem.ClassName);
				string low_DisplayName = displayName;
				low_DisplayName.ToLower();
				string low_m_SearchFilter = m_SearchFilter;
				low_m_SearchFilter.ToLower();
				if(low_DisplayName.Contains(low_m_SearchFilter))
				{
					m_FilteredListOfTraderItems.Insert(traderItem);
					m_ListboxItems.AddItem( displayName, NULL, 0 );	
					m_ListboxItems.SetItem( countFilter, "" + traderItem.BuyValue, NULL, 1 );
					m_ListboxItems.SetItem( countFilter, "" + traderItem.SellValue, NULL, 2 );
					countFilter++;
				}
			}
		}
		else if(m_SellablesOnly)
		{
			countFilter = 0;
			foreach(TraderItem sellableTraderItem : m_ListOfTraderItems)
			{                
				if(!ShouldShowInSellablesList(sellableTraderItem))
						continue;
				displayName = getItemDisplayName(sellableTraderItem.ClassName);
				m_FilteredListOfTraderItems.Insert(sellableTraderItem);
				m_ListboxItems.AddItem( displayName, NULL, 0 );	
				m_ListboxItems.SetItem( countFilter, "" + sellableTraderItem.BuyValue, NULL, 1 );
				m_ListboxItems.SetItem( countFilter, "" + sellableTraderItem.SellValue, NULL, 2 );
				countFilter++;
			}
		}
		else
		{
			countFilter = 0;
			foreach(TraderItem catTraderItem : m_ListOfCategoryTraderItems)
			{ 
				displayName = getItemDisplayName(catTraderItem.ClassName);    
				m_FilteredListOfTraderItems.Insert(catTraderItem);
				m_ListboxItems.AddItem( displayName, NULL, 0 );	
				m_ListboxItems.SetItem( countFilter, "" + catTraderItem.BuyValue, NULL, 1 );
				m_ListboxItems.SetItem( countFilter, "" + catTraderItem.SellValue, NULL, 2 );
				countFilter++;
			}
		}

        m_OldSearchFilter = m_SearchFilter;
        if(m_FilteredListOfTraderItems.Count() > 0)
        {
            m_LastRowIndex = -1;
            m_ListboxItems.SelectRow(0);
        }
        
	}

	bool ShouldShowInSellablesList(TraderItem catTraderItem)
	{
		if(!m_SellablesCheckbox.IsChecked())
			return true;			
		string itemClassname = catTraderItem.ClassName;
		int itemQuantity = catTraderItem.Quantity;
		if (catTraderItem.SellValue < 0)
			return false;
		
		return IsSellableOrInInventory(itemClassname, itemQuantity);
	}

    string GetEntityAITooltip(EntityAI item)
	{
		string temp;
		if (!item.DescriptionOverride(temp))
		{
			temp = item.ConfigGetString("descriptionShort");
		}
		return TrimUntPrefix(temp);
	}

    override bool OnMouseButtonDown(Widget w, int x, int y, int button)
	{
		super.OnMouseButtonDown(w, x, y, button);
		
		if (w == m_ItemPreviewWidget)
		{
			GetGame().GetDragQueue().Call(this, "UpdateRotation");
			GetMousePos(m_PreviewWidgetRotationX, m_PreviewWidgetRotationY);
			return true;
		}
		return false;
	}

	void UpdateRotation(int mouse_x, int mouse_y, bool is_dragging)
	{
		vector o = m_PreviewWidgetOrientation;
		o[0] = o[0] + (m_PreviewWidgetRotationY - mouse_y);
		o[1] = o[1] - (m_PreviewWidgetRotationX - mouse_x);
		
		m_ItemPreviewWidget.SetModelOrientation( o );
		
		if (!is_dragging)
		{
			m_PreviewWidgetOrientation = o;
		}
	}

	override bool OnMouseWheel(Widget  w, int  x, int  y, int wheel)
	{
		super.OnMouseWheel(w, x, y, wheel);
		
		if ( w == m_ItemPreviewWidget )
		{
			m_characterScaleDelta = wheel;
			UpdateScale();
		}
		return false;
	}
	
	void UpdateScale()
	{
		float w, h, x, y;		
		m_ItemPreviewWidget.GetPos(x, y);
		m_ItemPreviewWidget.GetSize(w,h);
		w = w + ( m_characterScaleDelta / 4);
		h = h + ( m_characterScaleDelta / 4 );
		if ( w > 0.5 && w < 3 )
		{
			m_ItemPreviewWidget.SetSize( w, h );
	
			// align to center 
			int screen_w, screen_h;
			GetScreenSize(screen_w, screen_h);
			float new_x = x - ( m_characterScaleDelta / 8 );
			float new_y = y - ( m_characterScaleDelta / 8 );
			m_ItemPreviewWidget.SetPos( new_x, new_y );
		}
	}

	string TrimUntPrefix(string str) // duplicate
	{
		str.Replace("$UNT$", "");
		return str;
	}	
	
	override bool OnKeyDown(Widget w, int x, int y, int key)
	{
		if ( key == KeyCode.KC_ESCAPE )
		{
			Close();
		}
		
		return super.OnKeyDown(w, x, y, key);
	}
};