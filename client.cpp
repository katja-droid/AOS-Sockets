#pragma comment(lib, "ws2_32.lib")
#pragma warning(disable:4996)
#include <winsock2.h>
#include <stdio.h>
#include <vector>
#include <string>
#include <conio.h>
#include <ctime>
#include <iostream>
#include <fstream>
using namespace std;

SOCKET newConnection;
const int POINTS_NUMBER = 25;
const int HEIGHT = 12;
const int WIDTH = 12;
bool SERVER_STARTED = false;

void InfoToStream(std::string info)
{
    std::ofstream stream_out;
    stream_out.open("D:\\File.txt", std::ios::app);
    time_t sec = time(NULL);
    tm* time_info = localtime(&sec);
    stream_out << "Server: " << asctime(time_info) << "\t" << info << "\n";
    stream_out.close();
}

void StartServer()
{
    WSAData wsaData;
    WORD DllVersion = MAKEWORD(2, 1);
    if (WSAStartup(DllVersion, &wsaData) != 0)
    {
        std::cout << "Error at WSAStartup\n";
        return;
    }
    SOCKADDR_IN addr;
    int sizeofaddr = sizeof(addr);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    InfoToStream("inet_addr\tЗадання ip адреси на сервері");
    addr.sin_port = htons(1052);
    InfoToStream("htons\tЗаданя порту на сервері");
    addr.sin_family = AF_INET;

    SOCKET sListen = socket(AF_INET, SOCK_STREAM, NULL);
    InfoToStream("socket\tСтворення сокета для прослуховування");
    bind(sListen, (SOCKADDR*)&addr, sizeofaddr);
    InfoToStream("bind\tЗв'язування сокета з ip адресою та портом");
    std::cout << "Waiting for client to connect...\n";
    listen(sListen, SOMAXCONN);
    InfoToStream("listen\tПрослуховування сокета");

    newConnection = accept(sListen, (SOCKADDR*)&addr, &sizeofaddr);
    InfoToStream("accept\tПідтвердження підключення клієнта");

    if (newConnection == 0)
    {
        std::cout << "Error at accept()\n";
        return;
    }
    else
    {
        std::cout << "Client connected\n";
    }
    return;
}

struct Point
{
    int x;
    int y;
    Point(int _x, int _y)
    {
        x = _x;
        y = _y;
    }
    Point()
    {

    }
    bool operator ==(Point obj)
    {
        return this->x == obj.x && this->y == obj.y;
    }
};

class Figure
{
public:
    std::vector<Point> points;
    Figure()
    {
        srand(time(NULL));
        Point p_start(rand() % WIDTH, rand() % HEIGHT);
        points.push_back(p_start);
        for (int i = 1; i <= POINTS_NUMBER - 1; ++i)
        {
            Point p;
            do
            {
                p.x = rand() % WIDTH;
                p.y = rand() % HEIGHT;
            } while (!CheckPoint(p, points));
            points.push_back(p);
        }
    }

    Point operator[](size_t index)
    {
        return points[index];
    }
private:
    bool CheckPoint(Point p, std::vector<Point>& v)
    {
        for (auto i : v)
        {
            if (i == p)
            {
                return false;
            }
        }
        return true;
    }
};

void ProcessPoints()
{
    char draw_temp[HEIGHT][WIDTH];
    Figure figure;

    //preparing array
    for (int i = 0; i < HEIGHT; ++i)
    {
        for (int j = 0; j < HEIGHT; ++j)
        {
            draw_temp[i][j] = ' ';
        }
    }

    //fill draw_temp
    for (int i = 0; i < POINTS_NUMBER; ++i)
    {
        int x = figure[i].x;
        int y = figure[i].y;
        draw_temp[y][x] = '*';
    }
    char draw[HEIGHT * WIDTH];

    //convert char** to char*
    int k = 0;
    for (int i = 0; i < HEIGHT; ++i)
    {
        for (int j = 0; j < WIDTH; ++j)
        {
            draw[k] = draw_temp[i][j];
            ++k;
        }
    }
    
    std::cout << "Generated set of points:\n";
//draw array
    for (int i = 0; i < WIDTH * HEIGHT; ++i)
    {
        std::cout << draw[i];
        if (i != 0 && (i + 1) % WIDTH == 0)
        {
            std::cout << "\n";
        }
    }
    std::cout << "==============================================================\n";
    //For debugging
    /*for (int i = 0; i < POINTS_NUMBER; ++i)
    {
        std::cout << "x = " << figure[i].x << "\ty = " << figure[i].y << "\n";
    }*/
    if (!SERVER_STARTED)
    {
        StartServer();
        SERVER_STARTED = true;
    }
    while (true)
    {
        char cmd[5];
        {
            int bytesRecv = SOCKET_ERROR;
            while (bytesRecv == SOCKET_ERROR) {
                bytesRecv = recv(newConnection, cmd, sizeof(cmd), NULL);
                if (bytesRecv == 0 || bytesRecv == WSAECONNRESET) {
                    printf("Connection Closed.\n");
                    break;
                }
            }
        }
        
        InfoToStream("recv\tОтримання команди");
        std::string cmd_s = "--Who";
        for (int i = 0; i < 5; ++i)
        {
            cmd_s[i] = cmd[i];
        }
        if (cmd_s == "--Who")
        {
            char inf[26] = "Соснюк Катя K-24. В-27.";
            send(newConnection, inf, sizeof(inf), NULL);
            InfoToStream("send\tВідправлення даних про виконавця");
        }
        else
        {
            char temp[WIDTH * HEIGHT];
            
            {
                int bytesRecv = SOCKET_ERROR;
                while (bytesRecv == SOCKET_ERROR) {
                    bytesRecv = recv(newConnection, temp, sizeof(temp), NULL);
                    if (bytesRecv == 0 || bytesRecv == WSAECONNRESET) {
                        printf("Connection Closed.\n");
                        break;
                    }
                }
            }
            InfoToStream("recv\tОтримання точок згенерованих клієнтом");
            for (int i = 0; i < WIDTH * HEIGHT; ++i)
            {
                if (draw[i] == ' ' && temp[i] == '*')
                {
                    temp[i] = ' ';
                }
            }
            send(newConnection, temp, sizeof(temp), NULL);
            InfoToStream("send\nВідправлення точок які співпали");
        }
    }
}




int main()
{
    ProcessPoints();



    WSACleanup();
    return 0;
}