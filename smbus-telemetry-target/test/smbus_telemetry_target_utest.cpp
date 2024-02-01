#include <gtest/gtest.h>
#include "gmock/gmock.h"
#include "smbus_telemetry_update.hpp"
#include "smbus_telemetry_target_api.hpp"

using namespace std;

TEST(SmbusTelemetryUpdateApi_1, loadFromCSV) 
{
    // Test case for init failure
    int rc = smbus_telemetry_update::loadFromCSV("./wrong-fileName.csv");
    EXPECT_EQ(rc, -1);

    // Invalid data in row1 of csv configuration
    rc = smbus_telemetry_update::loadFromCSV("../smbus-telemetry-target/test/config/row1-failure.csv");
    EXPECT_EQ(rc, -1);

    // Invalid data in row2 of csv configuration
    rc = smbus_telemetry_update::loadFromCSV("../smbus-telemetry-target/test/config/row1-failure.csv");
    EXPECT_EQ(rc, -1);

    // Invalid data in row3 of csv configuration
    rc = smbus_telemetry_update::loadFromCSV("../smbus-telemetry-target/test/config/row1-failure.csv");
    EXPECT_EQ(rc, -1);

    // Invalid data in row4 of csv configuration
    rc = smbus_telemetry_update::loadFromCSV("../smbus-telemetry-target/test/config/row1-failure.csv");
    EXPECT_EQ(rc, -1);

    // Test case for init success
    rc = smbus_telemetry_update::loadFromCSV("../smbus-telemetry-target/test/config/smbus-telemetry-config.csv");
    EXPECT_EQ(rc, 0);
}

TEST(SmbusTelemetryUpdateApi_2, smbusSlaveUpdate) 
{
    // Test case for smbusSlaveUpdate 
    std::string dbusObjPath = "tmp_objpath";
    std::string iface = "tmp_iface";
    std::string propName = "tmp_propertyname";
    std::vector<uint8_t> val = {};
    uint64_t ts = 0;
    int retVal = 0;    
    // call smbusSlaveUpdate with wrong data
    int rc = smbus_telemetry_update::smbusSlaveUpdate(dbusObjPath, iface,propName,
                                                            val, ts, retVal);
    EXPECT_EQ(rc, -1);
}

TEST(SmbusTelemetryTargetApi_1, smbusSlaveInit)
{
    // Default smbus device is /dev/null
    // So it should fail
    bool status = smbusSlaveInit();
    EXPECT_EQ(status, false);
}
