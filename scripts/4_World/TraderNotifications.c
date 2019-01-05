class TraderNotification
{
    Widget m_Widget;
    MultilineTextWidget m_Message;

    float m_Timer = 0;
    float m_vsize = 0;
    bool m_isActive = true;
    bool m_isSafezoneMessage = false;

    void TraderNotifictaion() {}

    void ~TraderNotification() 
    {
        m_Widget.Show(false);
    }

    void Init(string message, float time, bool isSafezoneMessage = false)
    {
		m_Widget = GetGame().GetWorkspace().CreateWidgets( "TM/Trader/scripts/layouts/TraderNotification.layout" );
        m_Message = MultilineTextWidget.Cast(m_Widget.FindAnyWidget("text_message") );

        if (isSafezoneMessage)
        {
            message = getSafezoneMessage(time);
            m_isSafezoneMessage = true;
        }

        m_Message.SetText(message);

        TStringArray messageLines = new TStringArray;
		message.Split( "\n", messageLines );
        m_vsize = (messageLines.Count() * 20) + 5;
        m_Widget.SetSize(0.14, m_vsize);

        m_Timer = time;

        m_Widget.Show(true);
    }

    void Update(float timeslice)
	{
        m_Timer -= timeslice;

        if (m_isSafezoneMessage)
            m_Message.SetText(getSafezoneMessage(m_Timer));

        if (m_Timer <= 0)
        {
            m_isActive = false;
        }
    }

    void SetTextColor(int color)
    {
        m_Message.SetColor(color);
    }

    string getSafezoneMessage(float time)
    {
        return "You are leaving the\nSafezone in " + (Math.Floor(m_Timer) + 1) + " Seconds!";
    }
}

class TraderNotifications
{
    ref array<ref TraderNotification> m_Messages;
    
    void TraderNotifications()
    {
        m_Messages = new array<ref TraderNotification>;
    }

    void ~TraderNotifications() {}

    void Update(float timeslice)
	{
		for (int i = 0; i < m_Messages.Count(); i++)
        {
            TraderNotification notification = m_Messages.Get(i);
            notification.Update(timeslice);

            if (!notification.m_isActive)
            {
                m_Messages.RemoveOrdered(i);
                i--;

                UpdateMessagePositions();
            }
        }
    }

    void ShowMessage(string message, float time, int color)
    {
        TraderNotification notification = new TraderNotification();
        notification.Init(message, time);

        if (color != 0)
            notification.SetTextColor(color);

        if (m_Messages.Count() >= 15)
            m_Messages.RemoveOrdered(0);

        m_Messages.Insert(notification);

        UpdateMessagePositions();   
    }

    void ShowSafezoneMessage(float time)
    {
        DeleteAllSafezoneMessages();

        TraderNotification notification = new TraderNotification();
        notification.Init("", time, true);
        notification.SetTextColor(0xFFFA6B6B);

        m_Messages.Insert(notification);

        UpdateMessagePositions();   
    }

    void UpdateMessagePositions()
    {
        float voffset = 200;

        for (int i = 0; i < m_Messages.Count(); i++)
        {
            TraderNotification notification = m_Messages.Get(i);
            notification.m_Widget.SetPos(0.01, voffset);
            voffset += notification.m_vsize + 2;
        }
    }

    void DeleteAllSafezoneMessages()
    {
        for (int i = 0; i < m_Messages.Count(); i++)
        {
            TraderNotification notification = m_Messages.Get(i);

            if (notification.m_isSafezoneMessage)
            {
                m_Messages.RemoveOrdered(i);
                i--;

                UpdateMessagePositions();
            }
        }
    }
}