class TraderMenu extends UIScriptedMenu
{
	PluginTraderData traderDataPlugin;
	TD_Trader trader;
    ref array<ref TR_Trader_Category> TraderCategories;
    ref array<ref TR_Trader_Item> ListOfItemsFiltered;

    MultilineTextWidget m_InfoBox;
    ButtonWidget m_BtnBuy;
	ButtonWidget m_BtnSell;
	ButtonWidget m_BtnCancel;
	TextListboxWidget m_ListboxItems;
	TextWidget m_CurrencyNameText;
	TextWidget m_CurrencyAmountText;
	TextWidget m_TraderName;
	XComboBoxWidget m_XComboboxCategories;
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
	vector m_TraderVehicleSpawn = "0 0 0";
	vector m_TraderVehicleSpawnOrientation = "0 0 0";
	
	int m_Player_CurrencyAmount;
	int m_ColorBuyable;
	int m_ColorTooExpensive;
	int m_CategoriesCurrentIndex;

	int m_LastRowIndex = -1;
	int m_LastCategoryCurrentIndex = -1;
	
	bool updateListbox = false;
	
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
		m_CurrencyNameText = TextWidget.Cast(layoutRoot.FindAnyWidget("text_saldo") );
		m_CurrencyAmountText = TextWidget.Cast(layoutRoot.FindAnyWidget("text_saldoValue") );
		m_TraderName = TextWidget.Cast(layoutRoot.FindAnyWidget("title_text") );
		m_XComboboxCategories = XComboBoxWidget.Cast( layoutRoot.FindAnyWidget( "xcombobox_Categories" ) );
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
        ListOfItemsFiltered = new array<ref TR_Trader_Item>;
		
		LoadCategories();
		m_CategoriesCurrentIndex = 0;
		
		SearchForItems();		
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
			if ((m_LastRowIndex != row_index) || (m_LastCategoryCurrentIndex != m_CategoriesCurrentIndex))
			{
				m_LastRowIndex = row_index;
				m_LastCategoryCurrentIndex = m_CategoriesCurrentIndex;
				ResetMenu();
				if(!ListOfItemsFiltered.Get(row_index))
				{
					m_UiUpdateTimer = 0;
					return;
				}
				updateItemPreview(ListOfItemsFiltered.Get(row_index));
			}

			float playerDistanceToTrader = vector.Distance(m_Player.GetPosition(), trader.Position);
			if (playerDistanceToTrader > 1.7)
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
		
		if (w == m_XComboboxCategories)
		{
			if (updateListbox)
			{
				updateListbox = false;
				
				SearchForItems();
				m_ListboxItems.SelectRow(0);

				updatePlayerCurrencyAmount();
				updateItemListboxColors();
			}
		}

		local int row_index = m_ListboxItems.GetSelectedRow();
		if(!ListOfItemsFiltered.Get(row_index))
			return false;

		string itemType = ListOfItemsFiltered.Get(row_index).ClassName;
		int itemQuantity = ListOfItemsFiltered.Get(row_index).Quantity;
		
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
			Param2<int, TR_Trader_Item> buyParams = new Param2<int, TR_Trader_Item>(m_TraderID, ListOfItemsFiltered.Get(row_index));
			GetGame().RPCSingleParam(m_Player, TRPCs.RPC_BUY, buyParams, true);
			
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
			Param2<int, TR_Trader_Item> sellParams = new Param2<int, TR_Trader_Item>(m_TraderID, ListOfItemsFiltered.Get(row_index));
			GetGame().RPCSingleParam(m_Player, TRPCs.RPC_SELL, sellParams, true);
			
			return true;
		}

		return false;
	}
	
	override bool OnChange( Widget w, int x, int y, bool finished )
	{
		super.OnChange(w, x, y, finished);
		if (!finished) return false;
		
		m_CategoriesCurrentIndex = m_XComboboxCategories.GetCurrentItem();
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
	
	void updateItemListboxColors()
	{
		for (int i = 0; i < m_ListboxItems.GetNumItems(); i++)
		{
			int itemCosts = ListOfItemsFiltered.Get(i).BuyPrice;
			
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
			
			string itemClassname = ListOfItemsFiltered.Get(i).ClassName;
			int itemQuantity = ListOfItemsFiltered.Get(i).Quantity;
			TR_Item_Type itemTRType = ListOfItemsFiltered.Get(i).Type;
			
			if (ListOfItemsFiltered.Get(i).SellPrice < 0)
			{
				m_ListboxItems.SetItemColor(i, 2, ARGBF(0, 1, 1, 1) );
			}
			else if (TraderLibrary.IsInPlayerInventory(m_Player, ListOfItemsFiltered.Get(i), itemQuantity) || (itemTRType ==  TR_Item_Type.Vehicle && TraderLibrary.GetVehicleToSell(m_Player, m_TraderID, itemClassname)))
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
				if (TraderLibrary.IsAttachedToEntity(entityInHands, itemClassname))
					m_ListboxItems.SetItemColor(i, 0, ARGBF(1, 0.4, 0.4, 1) );
				else if (TraderLibrary.IsAttachment(entityInHands, itemClassname))
					m_ListboxItems.SetItemColor(i, 0, ARGBF(1, 0.7, 0.7, 1) );
				else
					m_ListboxItems.SetItemColor(i, 0, ARGBF(1, 1, 1, 1) );
			}
		}
	}

	void updateItemPreview(TR_Trader_Item item)
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

			string lower = item.ClassName;
			int leng = -1;
			string itemName = item.ClassName;
            lower.ToLower();
			if(TraderLibrary.KitIgnoreArray.Find(itemName) == -1 && lower.Contains("kit_"))
            {
                itemName = itemName.Substring(4,itemName.Length());  
            }
            else if(TraderLibrary.KitIgnoreArray.Find(itemName) == -1 && lower.Contains("_kit"))
            {
                leng = itemName.Length() - 4;
                itemName = itemName.Substring(0,leng);
				if(lower.Contains("md_camonetshelter"))
					itemName = "Land_" + itemName;
            }
			else if(TraderLibrary.KitIgnoreArray.Find(itemName) == -1 && lower.Contains("kit"))
            {
                leng = itemName.Length() - 3;      
                itemName = itemName.Substring(0,leng);  
            }
			previewItem = EntityAI.Cast(GetGame().CreateObjectEx(itemName, "0 0 0", ECE_EQUIP | ECE_LOCAL | ECE_PLACE_ON_SURFACE));
			//TODO: check if car or it has attachments defined?
			//add the attachments for the preview
			if(item.IsPreset)
			{
				foreach(TraderObjectAttachment att : item.Attachments)
				{
					att.SpawnAttachment(previewItem);
				}
				if(item.Type == TR_Item_Type.Weapon)
				{
					Weapon_Base wpnprv = Weapon_Base.Cast(previewItem);
					if(wpnprv)
					{
						wpnprv.ForceSyncSelectionState();
					}
				}
				if(item.Type == TR_Item_Type.Vehicle)
				{
					CarScript vehprv = CarScript.Cast(previewItem);
					if(vehprv)
					{
						vehprv.Synchronize();
					}
				}
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
				int itemQuantity = ListOfItemsFiltered.Get(row_index).Quantity;
				string itemQuant = m_QuantString + "1";				
				if(itemQuantity > 0 && !TraderLibrary.HasQuantityBar(itemName))
				{
					itemQuant = m_QuantString + itemQuantity.ToString();
				}
				else
				{
					int sizeCount = TraderLibrary.GetItemSlotCount(itemName);
					if(sizeCount > 0)
					{
						itemQuant = m_SizeString + sizeCount;
					}
				}
				m_ItemWeight.SetText(GetItemWeightText());
				m_ItemQuantity.SetText(itemQuant);
				if(item.IsPreset && item.Description.Length() > 0)
				{
					m_ItemDescription.SetText(item.Description);
				}
				else if(TraderLibrary.KitIgnoreArray.Find(itemName) == -1 && lower.Contains("kit") && previewItemKit)
                {
				    m_ItemDescription.SetText(string.Format("This item is a kit. %1",TraderLibrary.TrimUntPrefix(GetEntityAITooltip(previewItemKit))));
                }
                else
                {
					m_ItemDescription.SetText(TraderLibrary.TrimUntPrefix(GetEntityAITooltip(previewItem)));
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

		return "ERROR";
	}
	
	void updatePlayerCurrencyAmount()
	{
		m_Player_CurrencyAmount = 0;
		m_Player_CurrencyAmount = TraderLibrary.GetPlayerCurrencyAmount(trader, m_Player);
		m_CurrencyAmountText.SetText(" " + m_Player_CurrencyAmount);
	}

	bool LoadCategories()
	{	
        traderDataPlugin = PluginTraderData.Cast(GetPlugin(PluginTraderData));
        if(traderDataPlugin)
        {
            trader = traderDataPlugin.GetTraderByID(m_TraderID);
			if(!trader)
			{
				TraderMessage.PlayerRed("There was an error getting the data for trader", m_Player);
				Close();		
				return false;
			}
			m_TraderName.SetText(trader.DisplayName);
			m_CurrencyNameText.SetText("Rubles" + ": "); //TODO: change this to new currency system

			m_XComboboxCategories.ClearAll();
			
			TraderCategories = new array<ref TR_Trader_Category>;
			foreach (string cat : trader.Categories)
			{
				TR_Trader_Category category = traderDataPlugin.GetCategoryByName(cat);
				if(category)
				{
					TraderCategories.Insert(category);
					m_XComboboxCategories.AddItem(category.CategoryName);
				}
			}
			return true;
        }	
		TraderMessage.PlayerRed("There was an error getting the data for trader2", m_Player);
		Close();		
		return false;
	}
	
	array<ref TR_Trader_Item> GetItemsFromCategory()
	{		
		if (TraderCategories.Count() > 0 && TraderCategories.Count() >= m_CategoriesCurrentIndex - 1)
		{
			TR_Trader_Category cat = TraderCategories.Get(m_CategoriesCurrentIndex);
			return cat.TR_Items;
		}
		
		return null;
	}
		
	void SearchForItems()
    { 
		m_ListboxItems.ClearItems();
		ListOfItemsFiltered.Clear();
        string displayName = "";
		int countFilter = 0;
        array<ref TR_Trader_Item> ListOfItems = GetItemsFromCategory();
		
		if(ListOfItems.Count() > 0)
		{
			if(m_SearchFilter && m_SearchFilter != string.Empty)
			{
				foreach(TR_Trader_Item traderItem : ListOfItems)
				{                
					displayName = TraderLibrary.GetItemDisplayName(traderItem.ClassName);
					string low_DisplayName = displayName;
					low_DisplayName.ToLower();
					string low_m_SearchFilter = m_SearchFilter;
					low_m_SearchFilter.ToLower();
					if(low_DisplayName.Contains(low_m_SearchFilter))
					{
						ListOfItemsFiltered.Insert(traderItem);
					}
				}
			}
			else
			{
				ListOfItemsFiltered = MakeACopyOfTraderItems(ListOfItems);
			}

			if(m_SellablesOnly)
			{
				ref array<ref TR_Trader_Item> ListOfItemsFilteredSellables = MakeACopyOfTraderItems(ListOfItemsFiltered);
				ListOfItemsFiltered.Clear();
				foreach(TR_Trader_Item sellableTraderItem : ListOfItemsFilteredSellables)
				{                
					if(!ShouldShowInSellablesList(sellableTraderItem))
							continue;

					ListOfItemsFiltered.Insert(sellableTraderItem);
				}
			}
			countFilter = 0;
			foreach(TR_Trader_Item catTraderItem : ListOfItemsFiltered)
			{ 
				displayName = TraderLibrary.GetItemDisplayName(catTraderItem.ClassName);
                if(catTraderItem.DisplayName.Length() > 0)
                {
					displayName = catTraderItem.DisplayName;
                }
				m_ListboxItems.AddItem( displayName, NULL, 0 );	
				m_ListboxItems.SetItem( countFilter, "" + catTraderItem.BuyPrice, NULL, 1 );
				m_ListboxItems.SetItem( countFilter, "" + catTraderItem.SellPrice, NULL, 2 );
				countFilter++;
			}
		}
        m_OldSearchFilter = m_SearchFilter;
        if(ListOfItemsFiltered.Count() > 0)
        {
            m_LastRowIndex = -1;
            m_ListboxItems.SelectRow(0);
        }
	}

	bool ShouldShowInSellablesList(TR_Trader_Item catTraderItem)
	{
		if(!m_SellablesCheckbox.IsChecked())
			return true;			
		string itemClassname = catTraderItem.ClassName;
		int itemQuantity = catTraderItem.Quantity;
		if (catTraderItem.SellPrice < 0)
			return false;
		else if (TraderLibrary.IsInPlayerInventory(m_Player, catTraderItem, itemQuantity) || (catTraderItem.Type == TR_Item_Type.Vehicle && TraderLibrary.GetVehicleToSell(m_Player, m_TraderID, itemClassname)))
			return true;

		return false;
	}

    string GetEntityAITooltip(EntityAI item)
	{
		string temp;
		if (!item.DescriptionOverride(temp))
			temp = item.ConfigGetString("descriptionShort");
		return temp;
	}

    override bool OnMouseButtonDown(Widget w, int x, int y, int button)
	{
		super.OnMouseButtonDown(w, x, y, button);
		
		if (w == m_ItemPreviewWidget)
		{
			GetGame().GetDragQueue().Call(this, "UpdateRotation");
			g_Game.GetMousePos(m_PreviewWidgetRotationX, m_PreviewWidgetRotationY);
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

	ref array<ref TR_Trader_Item> MakeACopyOfTraderItems(array<ref TR_Trader_Item> OtherArray)
	{
		array<ref TR_Trader_Item> NewArray = new array<ref TR_Trader_Item>;
		foreach( TR_Trader_Item item : OtherArray )
		{
			NewArray.Insert(item);
		}
		return NewArray;
	}
};