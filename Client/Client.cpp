#include <iostream>
#include <EssNet.h>
#include <Essentials.h>
#include <json.hpp>

using json = nlohmann::json;

string Version = "2.0.0";

#include "Funzioni.h"
#include "Classi.h"

using namespace std;

int main()
{
    TcpIP Client;

    Client.Host = "127.0.0.1";
    Client.Port = 5555;

    if (Client.StartClient() != 0)
    {
        cout << "Errore nell'inizializzazione del Client" << endl;
        Client.Stop();
        _getch();
        return 0;
    }

    for (bool i = true; i;)
    {
        while (!Client.Connect()) {}
        cout << "Connesso" << endl;
        i = !COMUNICAZIONI::Inizializzazione(Client.Sock);
    }

    system("pause");

    return 0;
}