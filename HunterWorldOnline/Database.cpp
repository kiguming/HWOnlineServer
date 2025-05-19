#include "Database.h"
#include <iostream>

Database::Database()
{
	conn = mysql_init(NULL);
	result = nullptr;
}

Database::~Database()
{
	Disconnect();
}

bool Database::Connect(const char* host, const char* user, const char* password, const char* db, unsigned int port)
{
	if (!mysql_real_connect(conn, host, user, password, db, port, NULL, 0)) {
		std::cerr << "MySQL 연결 실패: " << mysql_error(conn) << std::endl;
		return false;
	}
	return true;
}

void Database::Disconnect()
{
	if (result) {
		mysql_free_result(result);
		result = nullptr;
	}
	if (conn) {
		mysql_close(conn);
		conn = nullptr;
	}
}

bool Database::IsConnected() const
{
	return conn != nullptr;
}

bool Database::Query(const std::string& sql)
{
	if (mysql_query(conn, sql.c_str()) != 0) {
		std::cerr << "쿼리 실패: " << mysql_error(conn) << std::endl;
		return false;
	}

	result = mysql_store_result(conn); // SELECT문일 경우만 결과 받음
	return true;
}

MYSQL_RES* Database::GetResult()
{
	return result;
}

void Database::FreeResult()
{
	if (result) {
		mysql_free_result(result);
		result = nullptr;
	}
}
