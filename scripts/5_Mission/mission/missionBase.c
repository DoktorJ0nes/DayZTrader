modded class MissionBase 
{
    override UIScriptedMenu CreateScriptedMenu(int id) 
    {
        UIScriptedMenu menu = NULL;
        menu = super.CreateScriptedMenu(id);
        if (!menu) 
        {
            if(id == TRADERMENU_UI)
            {
                menu = new TraderMenu;
                if (menu) 
                {
                    menu.SetID(id);
                }
            }
        }
        return menu;
    }
};