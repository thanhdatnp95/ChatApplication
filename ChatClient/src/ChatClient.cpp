#include "ConnectToServer.h"

int main()
{
    ConnectToServer& client = ConnectToServer::getInstance();

    string serverIP;
    cout << "Enter Server IP: ";
    getline(cin, serverIP);
    if (client.connect(serverIP) == 0)
    {
        while(!client.getStatus());

        string cmd;
        cout << "Your alias: " << client.getAlias() << endl;

        while (client.getStatus() == CONNECTED)
        {
            getline(cin, cmd);

            switch (client.headerCompare(cmd))
            {
            case 0:
                if (client.singleChat(cmd))
                {
                    cout << "Cannot send message" << endl;
                }
                break;
            case 1:
                if (client.groupChat(cmd))
                {
                    cout << "Cannot send message" << endl;
                }
                break;
            case 2:
                if (client.createGroup(cmd))
                {
                    cout << "Cannot create this group" << endl;
                }
                break;
            case 3:
                if (client.joinGroup(cmd))
                {
                    cout << "Cannot join in this group" << endl;
                }
                break;
            case 4:
                if (client.leaveGroup(cmd))
                {
                    cout << "Cannot leave this group" << endl;
                }
                break;
            case 5:
                client.disconnect();
                return 0;
            default:
                cout << "Invalid command" << endl;
                break;
            }
        }
        cout << "Disconnected" << endl;
    }
    else
    {
        cout << "Failed to connect to server" << endl;
    }

    return 0;
}