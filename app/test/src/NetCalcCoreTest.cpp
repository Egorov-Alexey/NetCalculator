/**
 * This file contains unit-tests for NetCalcCore class.
 * Tests set unit_test_mode flag in NetCalcCore instance and NetCalcCore mocks net operations.
 * main function returns 0 if all test passed.
 * main function returns 1 if one of test failed.
 */

#include <NetCalcCore.h>

#include <string>

class NetCalcCoreTest
{
public:
    NetCalcCoreTest();
    bool test();

private:
    bool check_accept_mode();
    bool accept();

    bool check_receive_mode();
    bool receive(const std::string& data);
    bool receive_failed();

    bool check_send_mode();
    bool send(const std::string& expected_outgoind_data, bool processing_error = false);
    bool send_failed();

    bool net_calc_core_testcase_1();
    bool net_calc_core_testcase_2();
    bool net_calc_core_testcase_3();
    bool net_calc_core_testcase_4();
    bool net_calc_core_testcase_5();
    bool net_calc_core_testcase_6();

private:
    Config cfg;
    NetCalcCore core;
};

static const unsigned int ci = 0; //client index
static const boost::system::error_code success{boost::system::errc::success, boost::system::system_category()};
static const boost::system::error_code error{boost::system::errc::connection_aborted, boost::system::system_category()};
static const std::string div_by_zero = "Division by zero\n";
static const std::string invalid_expr = "Invalid expression\n";

NetCalcCoreTest::NetCalcCoreTest()
    : cfg{"127.0.0.1", 0, 1, 0}, core(cfg)
{
    core.unit_test_mode = true;
    core.start();
}

bool NetCalcCoreTest::test()
{
    return
        net_calc_core_testcase_1() &&
        net_calc_core_testcase_2() &&
        net_calc_core_testcase_3() &&
        net_calc_core_testcase_4() &&
        net_calc_core_testcase_5() &&
        net_calc_core_testcase_6();
}

bool NetCalcCoreTest::check_accept_mode()
{
    return core.clients[ci].unit_test_mode == NetCalcCore::client_unit_test_mode::async_accept;
}

bool NetCalcCoreTest::accept()
{
    if (!check_accept_mode())
    {
        return false;
    }

    core.handle_accept(ci, success);
    return true;
}

bool NetCalcCoreTest::check_receive_mode()
{
    return core.clients[ci].unit_test_mode == NetCalcCore::client_unit_test_mode::async_receive;
}

bool NetCalcCoreTest::receive(const std::string& data)
{
    if (!check_receive_mode())
    {
        return false;
    }

    memcpy(core.clients[ci].buffer, data.data(), data.size());
    core.handle_receive(ci, success, data.size());

    return true;
}

bool NetCalcCoreTest::receive_failed()
{
    if (!check_receive_mode())
    {
        return false;
    }

    core.handle_receive(ci, error, 0);

    return true;
}

bool NetCalcCoreTest::check_send_mode()
{
    return core.clients[ci].unit_test_mode == NetCalcCore::client_unit_test_mode::async_send;
}

bool NetCalcCoreTest::send(const std::string& expected_outgoind_data, bool processing_error/* = false*/)
{
    if (!check_send_mode())
    {
        return false;
    }

    std::string data(core.clients[ci].buffer, expected_outgoind_data.size());
    if (expected_outgoind_data != data)
    {
        return false;
    }

    core.handle_send(ci, processing_error, success, expected_outgoind_data.size());
    return true;
}

bool NetCalcCoreTest::send_failed()
{
    if (!check_send_mode())
    {
        return false;
    }

    core.handle_send(ci, false, error, 0);
    return true;

}

bool NetCalcCoreTest::net_calc_core_testcase_1()
{
    //Test usual expression.
    if (!accept())                  { return false; }
    if (!receive("53/(17-(19+23)")) { return false; }
    if (!receive(")*11-(31+(37+83")){ return false; }
    if (!receive("))+11"))          { return false; }
    if (!receive("3\n"))            { return false; }
    if (!send("-60\n"))             { return false; }
    if (!receive_failed())          { return false; } //set accept mode for the next test
    return true;
}

bool NetCalcCoreTest::net_calc_core_testcase_2()
{
    //Test correct multi-expression.
    std::string expr = "1741 + 7079 * 367  / 13 - 83\n1741 * ((7079 / 367)  * 13) / 83\n2861 + (1931 * 3271 - (3511 + 3631) / 419)\n";
    if (!accept())                  { return false; }
    if (!receive(expr))             { return false; }
    if (!send("201503\n"))          { return false; }
    if (!send("5181\n"))            { return false; }
    if (!send("6319145\n"))         { return false; }
    if (!receive_failed())          { return false; } //set accept mode for the next test
    return true;
}

bool NetCalcCoreTest::net_calc_core_testcase_3()
{
    //Test incorrect multi-expression.
    std::string expr = "1 + 2\n5/(2/7)\n";
    if (!accept())                  { return false; }
    if (!receive(expr))             { return false; }
    if (!send("3\n"))               { return false; }
    if (!send(div_by_zero, true))   { return false; }
    if (!check_accept_mode())       { return false; }
    return true;
}

bool NetCalcCoreTest::net_calc_core_testcase_4()
{
    //Test incorrect usual expression.
    std::string expr = "(53/(17-(19+23))*11-(31+(37+83))+113\n";
    if (!accept())                  { return false; }
    if (!receive(expr))             { return false; }
    if (!send(invalid_expr, true))  { return false; }
    if (!check_accept_mode())       { return false; }
    return true;
}

bool NetCalcCoreTest::net_calc_core_testcase_5()
{
    //Test unsuccessful receive.
    if (!accept())                  { return false; }
    if (!receive_failed())          { return false; }
    if (!check_accept_mode())       { return false; }
    return true;
}

bool NetCalcCoreTest::net_calc_core_testcase_6()
{
    //Test unsuccessful send.
    std::string expr = "1 + 2\n";
    if (!accept())                  { return false; }
    if (!receive(expr))             { return false; }
    if (!send_failed())             { return false; }
    if (!check_accept_mode())       { return false; }
    return true;
}

int main()
{
    NetCalcCoreTest obj;
    return obj.test() ? 0 : 1;
}
