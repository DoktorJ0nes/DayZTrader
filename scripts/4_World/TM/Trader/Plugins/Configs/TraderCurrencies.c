class TR_Currency
{
    string ClassName;
    int Value;
};

class TR_Trader_Currency
{	
	string CurrencyName;
	//string CurrencyPrefix;
    ref array<ref TR_Currency> CurrencyNotes;
};

class TraderCurrencies
{
	[NonSerialized()]
	private string m_ConfigName = TraderProfileFolder + "/Currencies.json";       

	protected float version;
    ref array<ref TR_Trader_Currency> Currencies;

    void Default()
    {
		version = 1.0;
        TR_Trader_Currency Cr = new TR_Trader_Currency;
        Cr.CurrencyName = "Rubles";
        Cr.CurrencyNotes = new array<ref TR_Currency>;
        TR_Currency Rubles1 = new TR_Currency;
        Rubles1.ClassName = "MoneyRuble1";
        Rubles1.Value = 1;
        Cr.CurrencyNotes.Insert(Rubles1);
        TR_Currency Rubles5 = new TR_Currency;
        Rubles5.ClassName = "MoneyRuble5";
        Rubles5.Value = 5;
        Cr.CurrencyNotes.Insert(Rubles5);
        TR_Currency Rubles10 = new TR_Currency;
        Rubles10.ClassName = "MoneyRuble10";
        Rubles10.Value = 10;
        Cr.CurrencyNotes.Insert(Rubles10);
        TR_Currency Rubles50 = new TR_Currency;
        Rubles50.ClassName = "MoneyRuble50";
        Rubles50.Value = 50;
        Cr.CurrencyNotes.Insert(Rubles50);
        TR_Currency Rubles100 = new TR_Currency;
        Rubles100.ClassName = "MoneyRuble100";
        Rubles100.Value = 100;
        Cr.CurrencyNotes.Insert(Rubles100);
        Currencies = new array<ref TR_Trader_Currency>;
        Currencies.Insert(Cr);
		Save();
	}

	void Load() 
	{			
		if (!FileExist(m_ConfigName))
		{
		    TM_Print("[" + this.ToString() + "] " + m_ConfigName + "' does NOT exist, creating default config! This will be an example config, please customize.");
		    Default();
		}
		JsonFileLoader<TraderCurrencies>.JsonLoadFile(m_ConfigName, this);
	}

	void Save() 
	{
        JsonFileLoader<TraderCurrencies>.JsonSaveFile(m_ConfigName, this);
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