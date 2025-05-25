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

bool Database::LoginOK(std::string& id, std::string& password)
{
	if (!conn) return false;

	std::string query = "SELECT * FROM account WHERE ID = '" + id + "' AND Password = '" + password + "'";

	if (mysql_query(conn, query.c_str()) != 0) {
		std::cerr << "쿼리 실패: " << mysql_error(conn) << std::endl;
		return false;
	}

	MYSQL_RES* result = mysql_store_result(conn);
	bool found = mysql_num_rows(result) > 0;
	mysql_free_result(result);

	return found;
}
