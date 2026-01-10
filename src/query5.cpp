#include "query5.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <thread>
#include <mutex>
#include <algorithm>
#include <vector>
#include <map>

bool readTable(const std::string &filename, const std::vector<std::string> &columns, std::vector<std::map<std::string, std::string>> &data)
{

    std::ifstream file(filename);

    if (!file.is_open())
        return false;

    std::string line;

    while (std::getline(file, line))
    {
        std::map<std::string, std::string> row;
        std::stringstream ss(line);
        std::string value;
        size_t colIdx = 0;

        while (std::getline(ss, value, '|'))
        {
            if (colIdx < columns.size())
            {
                row[columns[colIdx]] = value;
            }
            ++colIdx;
        }

        if (!row.empty())
        {
            data.push_back(row);
        }
    }

    return true;
};

std::map<std::string, std::string> readArgs(int argc, char *argv[])
{
    std::map<std::string, std::string> result;
    for (int i = 1; i < argc; i += 2)
    {
        std::string key = argv[i];
        // todo: handle only required keys?
        if (i + 1 < argc && !key.empty() && key.substr(0, 2) == "--")
        {

            std::string value = argv[i + 1];
            result[key.substr(2)] = value;
        }
    }
    return result;
}

// todo: improve error handling, wrong input args
bool parseArgs(int argc, char *argv[], std::string &r_name, std::string &start_date, std::string &end_date, int &num_threads, std::string &table_path, std::string &result_path)
{
    // Example: --r_name ASIA --start_date 1994-01-01 --end_date 1995-01-01 --threads 4 --table_path /path/to/tables --result_path /path/to/results
    auto args = readArgs(argc, argv);

    const char *required[] = {"r_name", "start_date", "end_date", "threads", "table_path", "result_path"};
    for (const char *key : required)
    {
        if (args.find(key) == args.end())
        {
            return false;
        }
    }

    r_name = args["r_name"];
    start_date = args["start_date"];
    end_date = args["end_date"];
    num_threads = std::stoi(args["threads"]);
    table_path = args["table_path"];
    result_path = args["result_path"];

    return true;
}

// Function to read TPCH data from the specified paths
bool readTPCHData(const std::string &table_path, std::vector<std::map<std::string, std::string>> &customer_data, std::vector<std::map<std::string, std::string>> &orders_data, std::vector<std::map<std::string, std::string>> &lineitem_data, std::vector<std::map<std::string, std::string>> &supplier_data, std::vector<std::map<std::string, std::string>> &nation_data, std::vector<std::map<std::string, std::string>> &region_data)
{

    std::vector<std::string> customer_cols = {"C_CUSTKEY", "C_NAME", "C_ADDRESS", "C_NATIONKEY", "C_PHONE", "C_ACCTBAL", "C_MKTSEGMENT", "C_COMMENT"};
    std::vector<std::string> orders_cols = {"O_ORDERKEY", "O_CUSTKEY", "O_ORDERSTATUS", "O_TOTALPRICE", "O_ORDERDATE", "O_ORDERPRIORITY", "O_CLERK", "O_SHIPPRIORITY", "O_COMMENT"};
    std::vector<std::string> lineitem_cols = {"L_ORDERKEY", "L_PARTKEY", "L_SUPPKEY", "L_LINENUMBER", "L_QUANTITY", "L_EXTENDEDPRICE", "L_DISCOUNT", "L_TAX", "L_RETURNFLAG", "L_LINESTATUS", "L_SHIPDATE", "L_COMMITDATE", "L_RECEIPTDATE", "L_SHIPINSTRUCT", "L_SHIPMODE", "L_COMMENT"};
    std::vector<std::string> supplier_cols = {"S_SUPPKEY", "S_NAME", "S_ADDRESS", "S_NATIONKEY", "S_PHONE", "S_ACCTBAL", "S_COMMENT"};
    std::vector<std::string> nation_cols = {"N_NATIONKEY", "N_NAME", "N_REGIONKEY", "N_COMMENT"};
    std::vector<std::string> region_cols = {"R_REGIONKEY", "R_NAME", "R_COMMENT"};

    bool sucess = true;
    sucess &= readTable(table_path + "/customer.tbl", customer_cols, customer_data);
    sucess &= readTable(table_path + "/orders.tbl", orders_cols, orders_data);
    sucess &= readTable(table_path + "/lineitem.tbl", lineitem_cols, lineitem_data);
    sucess &= readTable(table_path + "/supplier.tbl", supplier_cols, supplier_data);
    sucess &= readTable(table_path + "/nation.tbl", nation_cols, nation_data);
    sucess &= readTable(table_path + "/region.tbl", region_cols, region_data);

    return sucess;
}

// Function to execute TPCH Query 5 using multithreading
bool executeQuery5(const std::string &r_name, const std::string &start_date, const std::string &end_date, int num_threads, const std::vector<std::map<std::string, std::string>> &customer_data, const std::vector<std::map<std::string, std::string>> &orders_data, const std::vector<std::map<std::string, std::string>> &lineitem_data, const std::vector<std::map<std::string, std::string>> &supplier_data, const std::vector<std::map<std::string, std::string>> &nation_data, const std::vector<std::map<std::string, std::string>> &region_data, std::map<std::string, double> &results)
{
    // TODO: Implement TPCH Query 5 using multithreading
    return false;
}

// Function to output results to the specified path
bool outputResults(const std::string &result_path, const std::map<std::string, double> &results)
{
    // TODO: Implement outputting results to a file
    return false;
}