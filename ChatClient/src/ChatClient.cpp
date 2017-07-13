#include "ConnectToServer.h"

int main()
{
    ConnectToServer client;

    string serverIP;
    cout << "Enter Server IP: ";
    getline(cin, serverIP);
    if (client.connect(serverIP) == 0)
    {
        string cmd;
        cout << "Your alias: " << client.getAlias() << endl;
        // cout << "1. Single chat" << endl;
        // cout << "2. Group chat" << endl;
        // cout << "3. Create group" << endl;
        // cout << "4. Join in group" << endl;
        // cout << "5. Leave group" << endl;

        while (true)
        {
            cout << "Command: ";
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
    }
    else
    {
        cout << "Failed to connect to server" << endl;
    }

    return 0;
}