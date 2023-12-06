class TR_Currency
{
    string ClassName;
    int Value;
};

class TR_Trader_Currency
{	
	string CurrencyName;
    ref array<ref TR_Currency> CurrencyNotes;
};

class TraderCurrencies
{
	[NonSerialized()]
	private string m_ConfigName = TraderProfileFolder;    

	protected float version;
    ref array<ref TR_Trader_Currency> Currencies;

    void Default()
    {
		version = 1.0;
        Currencies = new array<ref TR_Trader_Currency>;
        array<ref TR_Currency> CurrencyNotes = new array<ref TR_Currency>;
        TR_Currency Rubles1 = new TR_Currency;
        Rubles1.ClassName = "MoneyRuble1";
        Rubles1.Value = 1;
        TR_Currency Rubles5 = new TR_Currency;
        Rubles5.ClassName = "MoneyRuble5";
        Rubles5.Value = 5;
        TR_Currency Rubles10 = new TR_Currency;
        Rubles10.ClassName = "MoneyRuble10";
        Rubles10.Value = 10;
        TR_Currency Rubles50 = new TR_Currency;
        Rubles50.ClassName = "MoneyRuble50";
        Rubles50.Value = 50;
        TR_Currency Rubles100 = new TR_Currency;
        Rubles100.ClassName = "MoneyRuble100";
        Rubles100.Value = 100;
        TR_Trader_Currency Cr = new TR_Trader_Currency;
        Cr.CategoryName = "Rubles";
        CurrencyNotes.Insert(Rubles1);
        CurrencyNotes.Insert(Rubles5);
        CurrencyNotes.Insert(Rubles10);
        CurrencyNotes.Insert(Rubles50);
        CurrencyNotes.Insert(Rubles100);
        Cr.CurrencyNotes = CurrencyNotes;
		Save();
	}

	void Load() 
	{			
		if (!FileExist(m_ConfigName))
		{
		    TM_Print("[" + this.ToString() + "] " + m_ConfigName + "' does NOT exist, creating default config! This will be an example config, please customize.");
		    Default();
		}
		JsonFileLoader<TR_Trader_Currency>.JsonLoadFile(m_ConfigName, this);
	}

	void Save() 
	{
        JsonFileLoader<TR_Trader_Currency>.JsonSaveFile(m_ConfigName, this);
	}

    TR_Trader_Currency GetCurrencyByName(string Name)
    {
        foreach (TR_Trader_Currency currency : Currencies)
		{
            if(currency.CurrencyName == Name)
            {
                return currency;
            }
        }
        return null;
    }
};