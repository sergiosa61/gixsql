#pragma once

/*
This file is part of Gix-IDE, an IDE and platform for GnuCOBOL
Copyright (C) 2021 Marco Ridoni

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or (at
your option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
USA.
*/


#include <string>
#include <vector>
#include <map>
#include <memory>

#include "ICursor.h"
#include "IDbInterface.h"
#include "IDbManagerInterface.h"
#include "IDataSourceInfo.h"
#include "ISchemaManager.h"

extern "C" {
#include "dpi.h"
#include "dpiImpl.h"
}

#define DECODE_BINARY_ON		1
#define DECODE_BINARY_OFF		0
#define DECODE_BINARY_DEFAULT	DECODE_BINARY_ON

struct OdpiStatementData : public IPrivateStatementData {

	OdpiStatementData();
	~OdpiStatementData();

	void resizeParams(int n);
	void resizeColumnData(int n);

	dpiStmt* statement = nullptr;
	
	dpiVar** params = nullptr;
	dpiData** params_bfrs = nullptr;

	dpiVar** coldata = nullptr;
	dpiData** coldata_bfrs = nullptr;

	int params_count = 0;
	int coldata_count = 0;

private:
	void cleanup();
};

class DbInterfaceOracle : public IDbInterface, public IDbManagerInterface
{
public:
	DbInterfaceOracle();
	~DbInterfaceOracle();

	virtual int init(const std::shared_ptr<spdlog::logger>& _logger) override;
	virtual int connect(const std::shared_ptr<IDataSourceInfo>& conn_string, const std::shared_ptr<IConnectionOptions>& g_opts) override;
	virtual int reset() override;
	virtual int terminate_connection() override;
	virtual int exec(std::string) override;
	virtual int exec_params(std::string query, int nParams, const std::vector<int>& paramTypes, const std::vector<std::string>& paramValues, const std::vector<int>& paramLengths, const std::vector<int>& paramFormats) override;
	virtual int close_cursor(const std::shared_ptr<ICursor>& crsr) override;
	virtual int cursor_declare(const std::shared_ptr<ICursor>& crsr, bool, int) override;
	virtual int cursor_declare_with_params(const std::shared_ptr<ICursor>& crsr, char**, bool, int) override;
	virtual int cursor_open(const std::shared_ptr<ICursor>& cursor);
	virtual int fetch_one(const std::shared_ptr<ICursor>& crsr, int) override;
	virtual bool get_resultset_value(ResultSetContextType resultset_context_type, const IResultSetContextData& context, int row, int col, char* bfr, int bfrlen, int* value_len);
	virtual bool move_to_first_record(std::string stmt_name = "") override;
	virtual uint64_t get_native_features() override;
	virtual int get_num_rows(const std::shared_ptr<ICursor>& crsr) override;
	virtual int get_num_fields(const std::shared_ptr<ICursor>& crsr) override;
	virtual char* get_error_message() override;
	virtual int get_error_code() override;
	virtual std::string get_state() override;
	virtual void set_owner(std::shared_ptr<IConnection>) override;
	virtual std::shared_ptr<IConnection> get_owner() override;
	virtual int prepare(std::string stmt_name, std::string sql) override;
	virtual int exec_prepared(const std::string& stmt_name, std::vector<std::string>& paramValues, std::vector<int> paramLengths, std::vector<int> paramFormats) override;
	virtual DbPropertySetResult set_property(DbProperty p, std::variant<bool, int, std::string> v) override;

	virtual bool getSchemas(std::vector<SchemaInfo*>& res) override;
	virtual bool getTables(std::string table, std::vector<TableInfo*>& res) override;
	virtual bool getColumns(std::string schema, std::string table, std::vector<ColumnInfo*>& columns) override;
	virtual bool getIndexes(std::string schema, std::string tabl, std::vector<IndexInfo*>& idxs) override;

private:
	dpiConn *connaddr = nullptr;
	std::shared_ptr<OdpiStatementData > current_statement_data;

	static dpiContext* odpi_global_context;
	static int odpi_global_context_usage_count;

	int last_rc = 0;
	std::string last_error;
	std::string last_state;

	std::map<std::string, std::shared_ptr<ICursor>> _declared_cursors;
	std::map<std::string, std::shared_ptr<OdpiStatementData>> _prepared_stmts;

	int decode_binary = DECODE_BINARY_DEFAULT;

	bool is_autocommit_suspended = false;

	int dpiRetrieveError(int rc);
	void dpiClearError(); 
	void dpiSetError(int err_code, std::string sqlstate, std::string err_msg); 

	int _odpi_exec(std::shared_ptr<ICursor> crsr, std::string, std::shared_ptr<OdpiStatementData> prep_stmt = nullptr);
	int _odpi_exec_params(std::shared_ptr<ICursor> crsr, std::string query, int nParams, const std::vector<int>& paramTypes, const std::vector<std::string>& paramValues, const std::vector<int>& paramLengths, const std::vector<int>& paramFormats, std::shared_ptr<OdpiStatementData> prep_stmt = nullptr);

	int _odpi_get_num_rows(dpiStmt *r);

	std::shared_ptr<OdpiStatementData> retrieve_prepared_statement(const std::string& prep_stmt_name);
	bool is_cursor_from_prepared_statement(const std::shared_ptr<ICursor>& cursor);
};

