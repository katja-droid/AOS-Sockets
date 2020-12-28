#pragma comment(lib, "ws2_32.lib")
#pragma warning(disable:4996)
#include <winsock2.h>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <ctime>
#include <string>
#include <fstream>

SOCKET Connection;
const int POINTS_NUMBER = 25;
const int HEIGHT = 12;
const int WIDTH = 12;
bool SERVER_CONNECTED = false;

void InfoToStream(std::string info)
{
    std::ofstream stream_out;
    stream_out.open("D:\\File.txt", std::ios::app);
    time_t sec = time(NULL);
    tm* time_info = localtime(&sec);
    stream_out << "Client: " << asctime(time_info) << "\t" << info << "\n";
    stream_out.close();
}

void ConnectToServer()
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
    InfoToStream("inet_addr\tЗадання ip адреси на клієнті");
    addr.sin_port = htons(1052);
    InfoToStream("htons\tЗадання порту на клієнті");
    addr.sin_family = AF_INET;

    Connection = socket(AF_INET, SOCK_STREAM, NULL);
    InfoToStream("socket\tСтворення сокету для підключення до сервера");
    if (connect(Connection, (SOCKADDR*)&addr, sizeof(addr)) != 0)
    {
        std::cout << "Failed to connect\n";
        return;
    }
    InfoToStream("connect\tПідключення до сервера");
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



void DrawPoints()
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
    while (true)
    {
        std::cout << "Command:\n";
        std::string cmd;
        std::cin >> cmd;
        if (!SERVER_CONNECTED)
        {
            ConnectToServer();
            SERVER_CONNECTED = true;
        }
        if (cmd == "--Who")
        {
            char cmd_c[5] = { '-', '-', 'W', 'h', 'o' };
            send(Connection, cmd_c, sizeof(cmd_c), NULL);
            InfoToStream("send\tНадсилання команди серверу");
            char info[26];
            {
                int bytesRecv = SOCKET_ERROR;
                while (bytesRecv == SOCKET_ERROR) {
                    bytesRecv = recv(Connection, info, sizeof(info), NULL);
                    if (bytesRecv == 0 || bytesRecv == WSAECONNRESET) {
                        printf("Connection Closed.\n");
                        break;
                    }
                }
            }
            InfoToStream("recv\tОтримання інформації про виконавця");
            std::cout << info << "\n";
        }
        else if (cmd == "-Quit")
        {
            exit(1);
        }
        else if (cmd == "Start")
        {
            char cmd_c[5] = { 'S', 't', 'a', 'r', 't' };
            send(Connection, cmd_c, sizeof(cmd_c), NULL);
            InfoToStream("send\tНадсилання команди серверу");
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

            //For debugging
            /*for (int i = 0; i < POINTS_NUMBER; ++i)
            {
                std::cout << "x = " << figure[i].x << "\ty = " << figure[i].y << "\n";
            }*/
            char temp[WIDTH * HEIGHT];
            for (int i = 0; i < WIDTH * HEIGHT; ++i)
            {
                temp[i] = draw[i];
            }
            send(Connection, temp, sizeof(temp), NULL);
            InfoToStream("send\tНадсилання згенерованих точок серверу");
            {
                int bytesRecv = SOCKET_ERROR;
                while (bytesRecv == SOCKET_ERROR) {
                    bytesRecv = recv(Connection, temp, sizeof(temp), NULL);
                    if (bytesRecv == 0 || bytesRecv == WSAECONNRESET) {
                        printf("Connection Closed.\n");
                        break;
                    }
                }
            }
            InfoToStream("recv\tОтримання від сервера вгаданих точок");
            std::cout << "Guessed points:\n";
            for (int i = 0; i < WIDTH * HEIGHT; ++i)
            {
                std::cout << temp[i];
                if (i != 0 && (i + 1) % WIDTH == 0)
                {
                    std::cout << "\n";
                }
            }
            int guessed = 0;
            for (int i = 0; i < WIDTH * HEIGHT; ++i)
            {
                if (temp[i] == '*')
                {
                    ++guessed;
                }
            }
            std::cout << guessed << " points guessed\n";
        }
        else
        {
            std::cout << "Undefined command!\n";
        }
    }
}

int main()
{
    DrawPoints();


    WSACleanup();
    return 0;
}