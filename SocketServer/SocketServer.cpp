#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>

using namespace std;
int main() {
    // Структура для хранения информации о сокете Windows
    WSADATA wsaData;

    // Структуры для хранения информации об адресе
    ADDRINFO hints;
    ADDRINFO* addrResult;

    // Сокеты для прослушивания и подключения
    SOCKET ListenSocket = INVALID_SOCKET;
    SOCKET ConnectSocket = INVALID_SOCKET;

    // Буфер для приема данных
    char recvBuffer[512];

    // Сообщение для отправки клиенту
    const char* sendBuffer = "Hello from server";

    // Инициализация Winsock API
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        cout << "WSAStartup failed with result: " << result << endl;
        return 1;
    }

    // Очистка структуры hints перед заполнением
    ZeroMemory(&hints, sizeof(hints));
    // Установка параметров для получения информации об адресе
    hints.ai_family = AF_INET; // Использование IPv4
    hints.ai_socktype = SOCK_STREAM; // TCP-соединение
    hints.ai_protocol = IPPROTO_TCP; // Протокол TCP
    hints.ai_flags = AI_PASSIVE; // Пассивный режим для сервера

    // Получение информации об адресе для привязки сокета
    result = getaddrinfo(NULL, "666", &hints, &addrResult);
    if (result != 0) {
        cout << "getaddrinfo failed with error: " << result << endl;
        freeaddrinfo(addrResult); // Освобождение памяти
        WSACleanup(); // Завершение использования Winsock
        return 1;
    }

    // Создание сокета
    ListenSocket = socket(addrResult->ai_family, addrResult->ai_socktype, addrResult->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        cout << "Socket creation failed" << endl;
        freeaddrinfo(addrResult);
        WSACleanup();
        return 1;
    }

    // Привязка сокета к адресу и порту
    result = bind(ListenSocket, addrResult->ai_addr, (int)addrResult->ai_addrlen);
    if (result == SOCKET_ERROR) {
        cout << "Bind failed, error: " << result << endl;
        closesocket(ListenSocket); // Закрытие сокета
        freeaddrinfo(addrResult);
        WSACleanup();
        return 1;
    }

    // Перевод сокета в режим прослушивания
    result = listen(ListenSocket, SOMAXCONN); // Максимальное количество ожидающих соединений
    if (result == SOCKET_ERROR) {
        cout << "Listen failed, error: " << result << endl;
        closesocket(ListenSocket);
        freeaddrinfo(addrResult);
        WSACleanup();
        return 1;
    }

    // Прием входящего соединения
    ConnectSocket = accept(ListenSocket, NULL, NULL);
    if (ConnectSocket == INVALID_SOCKET) {
        cout << "Accept failed, error: " << WSAGetLastError() << endl; // Вывод ошибки
        closesocket(ListenSocket);
        freeaddrinfo(addrResult);
        WSACleanup();
        return 1;
    }
    // Закрытие сокета для прослушивания после установления соединения
    closesocket(ListenSocket);

    do {
        // Очистка буфера перед приемом новых данных
        ZeroMemory(recvBuffer, 512);
        // Прием данных от клиента
        result = recv(ConnectSocket, recvBuffer, 512, 0);
        if (result > 0) {
            // Вывод количества полученных байт
            cout << "Received " << result << " bytes" << endl;
            // Вывод полученных данных
            cout << "Received data: " << recvBuffer << endl;

            // Отправка ответа клиенту
            result = send(ConnectSocket, sendBuffer, (int)strlen(sendBuffer), 0);
            if (result == SOCKET_ERROR) {
                // Обработка ошибки при отправке
                cout << "Send failed, error: " << result << endl;
                closesocket(ConnectSocket);
                freeaddrinfo(addrResult);
                WSACleanup();
                return 1;
            }
        }
        else if (result == 0) {
            // Клиент закрыл соединение
            cout << "Connection closing" << endl;
        }
        else {
            // Обработка ошибки при приеме данных
            cout << "Recv failed, error: " << WSAGetLastError() << endl;
            closesocket(ConnectSocket);
            freeaddrinfo(addrResult);
            WSACleanup();
            return 1;
        }
    } while (result > 0); // Повторять цикл пока есть данные для чтения

    // Завершение записи в сокет, но разрешение чтения из него
    result = shutdown(ConnectSocket, SD_SEND);
    if (result == SOCKET_ERROR) {
        // Обработка ошибки при завершении работы сокета
        cout << "Shutdown failed, error: " << result << endl;
        closesocket(ConnectSocket);
        freeaddrinfo(addrResult);
        WSACleanup();
        return 1;
    }

    // Закрытие сокета после завершения общения
    closesocket(ConnectSocket);
    freeaddrinfo(addrResult); // Освобождение памяти, выделенной под информацию об адресе
    WSACleanup(); // Завершение использования Winsock API
    return 0; // Успешное завершение программы