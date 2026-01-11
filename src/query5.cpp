#include "query5.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <thread>
#include <mutex>
#include <algorithm>
#include <vector>
#include <map>

using namespace std;

bool readTable(const string &filename, const vector<string> &columns, vector<map<string, string>> &data)
{

    ifstream file(filename);

    if (!file.is_open())
        return false;

    string line;

    while (getline(file, line))
    {
        map<string, string> row;
        stringstream ss(line);
        string value;
        size_t colIdx = 0;

        while (getline(ss, value, '|'))
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

map<string, string> readArgs(int argc, char *argv[])
{
    map<string, string> result;
    for (int i = 1; i < argc; i += 2)
    {
        string key = argv[i];
        // todo: handle only required keys?
        if (i + 1 < argc && !key.empty() && key.substr(0, 2) == "--")
        {

            string value = argv[i + 1];
            result[key.substr(2)] = value;
        }
    }
    return result;
}

// todo: improve error handling, wrong input args
bool parseArgs(int argc, char *argv[], string &r_name, string &start_date, string &end_date, int &num_threads, string &table_path, string &result_path)
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
    num_threads = stoi(args["threads"]);
    table_path = args["table_path"];
    result_path = args["result_path"];

    return true;
}

// Function to read TPCH data from the specified paths
bool readTPCHData(const string &table_path, vector<map<string, string>> &customer_data, vector<map<string, string>> &orders_data, vector<map<string, string>> &lineitem_data, vector<map<string, string>> &supplier_data, vector<map<string, string>> &nation_data, vector<map<string, string>> &region_data)
{

    vector<string> customer_cols = {"C_CUSTKEY", "C_NAME", "C_ADDRESS", "C_NATIONKEY", "C_PHONE", "C_ACCTBAL", "C_MKTSEGMENT", "C_COMMENT"};
    vector<string> orders_cols = {"O_ORDERKEY", "O_CUSTKEY", "O_ORDERSTATUS", "O_TOTALPRICE", "O_ORDERDATE", "O_ORDERPRIORITY", "O_CLERK", "O_SHIPPRIORITY", "O_COMMENT"};
    vector<string> lineitem_cols = {"L_ORDERKEY", "L_PARTKEY", "L_SUPPKEY", "L_LINENUMBER", "L_QUANTITY", "L_EXTENDEDPRICE", "L_DISCOUNT", "L_TAX", "L_RETURNFLAG", "L_LINESTATUS", "L_SHIPDATE", "L_COMMITDATE", "L_RECEIPTDATE", "L_SHIPINSTRUCT", "L_SHIPMODE", "L_COMMENT"};
    vector<string> supplier_cols = {"S_SUPPKEY", "S_NAME", "S_ADDRESS", "S_NATIONKEY", "S_PHONE", "S_ACCTBAL", "S_COMMENT"};
    vector<string> nation_cols = {"N_NATIONKEY", "N_NAME", "N_REGIONKEY", "N_COMMENT"};
    vector<string> region_cols = {"R_REGIONKEY", "R_NAME", "R_COMMENT"};

    bool sucess = true;
    sucess &= readTable(table_path + "/customer.tbl", customer_cols, customer_data);
    sucess &= readTable(table_path + "/orders.tbl", orders_cols, orders_data);
    sucess &= readTable(table_path + "/lineitem.tbl", lineitem_cols, lineitem_data);
    sucess &= readTable(table_path + "/supplier.tbl", supplier_cols, supplier_data);
    sucess &= readTable(table_path + "/nation.tbl", nation_cols, nation_data);
    sucess &= readTable(table_path + "/region.tbl", region_cols, region_data);

    return sucess;
}

tm parseDate(const string &s)
{
    tm time = {};
    istringstream ss(s);
    ss >> get_time(&time, "%Y-%m-%d");
    return time;
}

string getRegionKey(const vector<map<string, string>> &region_data, const string &r_name)
{
    for (const auto &row : region_data)
    {
        if (row.at("R_NAME") == r_name)
        {
            return row.at("R_REGIONKEY");
        }
    }
    return "";
}

// Function to execute TPCH Query 5 using multithreading
bool executeQuery5(const string &r_name, const string &start_date, const string &end_date, int num_threads, const vector<map<string, string>> &customer_data, const vector<map<string, string>> &orders_data, const vector<map<string, string>> &lineitem_data, const vector<map<string, string>> &supplier_data, const vector<map<string, string>> &nation_data, const vector<map<string, string>> &region_data, map<string, double> &results)
{
    const string region_key = getRegionKey(region_data, r_name);
    if (region_key.empty())
        return false;

    unordered_map<string, string> nation_name_map;
    nation_name_map.reserve(nation_data.size() / region_data.size());
    for (const auto &row : nation_data)
    {
        if (row.at("N_REGIONKEY") == region_key)
        {
            nation_name_map[row.at("N_NATIONKEY")] = row.at("N_NAME");
        }
    }

    unordered_map<string, string> supplier_nation_map;
    supplier_nation_map.reserve(supplier_data.size() / nation_data.size());
    for (const auto &row : supplier_data)
    {
        const string &nation_key = row.at("S_NATIONKEY");
        if (nation_name_map.find(nation_key) != nation_name_map.end())
        {
            supplier_nation_map[row.at("S_SUPPKEY")] = nation_key;
        }
    }

    unordered_map<string, string> customer_nation_map;
    customer_nation_map.reserve(customer_data.size() / nation_data.size());

    for (const auto &row : customer_data)
    {
        const string &nation_key = row.at("C_NATIONKEY");
        if (nation_name_map.find(nation_key) != nation_name_map.end())
        {
            customer_nation_map[row.at("C_CUSTKEY")] = nation_key;
        }
    }

    tm tm_start = parseDate(start_date);
    tm tm_end = parseDate(end_date);
    time_t t_start = mktime(&tm_start);
    time_t t_end = mktime(&tm_end);

    unordered_map<string, string> filtered_orders_customer_map;
    filtered_orders_customer_map.reserve(orders_data.size() / 10);

    for (const auto &row : orders_data)
    {

        const string &date_str = row.at("O_ORDERDATE");
        tm tm_date = parseDate(date_str);
        time_t t_date = mktime(&tm_date);
        if (t_date >= t_start && t_date < t_end)
        {
            const string &cust_key = row.at("O_CUSTKEY");
            if (customer_nation_map.find(cust_key) != customer_nation_map.end())
            {
                filtered_orders_customer_map[row.at("O_ORDERKEY")] = cust_key;
            }
        }
    }

    vector<thread> threads;
    vector<unordered_map<string, double>> local_results(num_threads);

    for (auto &local_map : local_results)
    {
        local_map.reserve(nation_name_map.size());
    }

    auto worker = [&](int t_id, size_t start, size_t end)
    {
        for (size_t i = start; i < end; ++i)
        {
            const auto &row = lineitem_data[i];
            const string &order_key = row.at("L_ORDERKEY");
            auto order_customer_object = filtered_orders_customer_map.find(order_key);

            if (order_customer_object != filtered_orders_customer_map.end())
            {
                const string &supp_key = row.at("L_SUPPKEY");

                auto suplier_nation_object = supplier_nation_map.find(supp_key);
                if (suplier_nation_object != supplier_nation_map.end())
                {

                    const string &cust_key = order_customer_object->second;
                    const string &cust_nation_key = customer_nation_map[cust_key];
                    const string &supp_nation_key = suplier_nation_object->second;

                    if (cust_nation_key != supp_nation_key)
                        continue;

                    double extended_price = stod(row.at("L_EXTENDEDPRICE"));
                    double discount = stod(row.at("L_DISCOUNT"));
                    double revenue = extended_price * (1 - discount);

                    const string &nation_name = nation_name_map[supplier_nation_map[supp_key]];

                    local_results[t_id][nation_name] += revenue;
                }
            }
        }
    };

    size_t partition_size = lineitem_data.size() / num_threads;
    for (int t = 0; t < num_threads; t++)
    {
        size_t start = t * partition_size;
        size_t end = (t == num_threads - 1) ? lineitem_data.size() : start + partition_size;
        threads.emplace_back(worker, t, start, end);
    }

    for (auto &th : threads)
    {
        th.join();
    }

    for (const auto &local_map : local_results)
    {
        for (const auto &entry : local_map)
        {
            results[entry.first] += entry.second;
        }
    }

    return true;
}

// Function to output results to the specified path
bool outputResults(const string &result_path, const map<string, double> &results)
{
    ofstream result_file(result_path);

    if (result_file.is_open())
    {
        vector<pair<string, double>> sorted_results(results.begin(), results.end());
        sort(sorted_results.begin(), sorted_results.end(), [](const pair<string, double> &a, const pair<string, double> &b)
             { return a.second > b.second; });

        for (const auto &entry : sorted_results)
        {
            result_file << entry.first << "|" << entry.second << endl;
        }

        result_file.close();

        return true;
    }

    return false;
}