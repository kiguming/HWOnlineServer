#pragma once
#include <mysql.h>
#include <string>

class Database
{
public:
    Database();
    ~Database();


    bool Connect(const char* host, const char* user, const char* password, const char* db, unsigned int port);
    void Disconnect();
    bool IsConnected() const;

    bool Query(const std::string& sql);
    MYSQL_RES* GetResult();
    void FreeResult();

    bool LoginOK(std::string& id, std::string& password);

private:
    MYSQL* conn;
    MYSQL_RES* result;

};