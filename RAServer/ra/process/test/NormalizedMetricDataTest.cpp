#include <ra/process/test/NormalizedMetricDataTest.h>
#include <ra/test/test.h>
#include <cppunit/TestAssert.h>

using namespace std;
RA_BEGIN_NAMESPACE(process);

CPPUNIT_TEST_SUITE_REGISTRATION(NormalizedMetricDataTest);

NormalizedMetricDataTest::NormalizedMetricDataTest() { 
}

NormalizedMetricDataTest::~NormalizedMetricDataTest() { 
}

void NormalizedMetricDataTest::setUp() { 
}

void NormalizedMetricDataTest::tearDown() { 
}

void NormalizedMetricDataTest::testSimpleProcess() { 
    NormalizedMetricData metricData;
    CPPUNIT_ASSERT(metricData.empty());
    MetricDataItem item1;
    ValueListPtr metric1(new ValueList());
    TimeListPtr timeList1(new TimeList());
    item1.host = 1;
    item1.time = 10;
    item1.value = 100;
    item1.rawValue = metric1;
    metric1->push_back(100.0);
    item1.rawTime = timeList1;
    timeList1->push_back(10 * MICROSECONDS);
    CPPUNIT_ASSERT(metricData.addMetricData("m1", item1));
    MetricDataItem item2;
    ValueListPtr metric2(new ValueList());
    TimeListPtr timeList2(new TimeList());
    item2.host = 2;
    item2.time = 20;
    item2.value = 200;
    item2.rawValue = metric2;
    metric2->push_back(200.0);
    item2.rawTime = timeList2;
    timeList2->push_back(20 * MICROSECONDS);
    CPPUNIT_ASSERT(metricData.addMetricData("m1", item2));
    CPPUNIT_ASSERT(metricData.addMetricData("m2", item2));
    CPPUNIT_ASSERT(!metricData.addMetricData("m2", item2));
    
    set<MetricDataItem> itemSet;
    itemSet = metricData.getMetricData("m1");
    CPPUNIT_ASSERT_EQUAL((size_t)2, itemSet.size());
    CPPUNIT_ASSERT_EQUAL((size_t)1, itemSet.count(item1));
    CPPUNIT_ASSERT_EQUAL((size_t)1, itemSet.count(item2));
    itemSet = metricData.getMetricData("m2");
    CPPUNIT_ASSERT_EQUAL((size_t)1, itemSet.size());
    CPPUNIT_ASSERT_EQUAL((size_t)1, itemSet.count(item2));

    MetricDataItem item;
    CPPUNIT_ASSERT(!metricData.getHostTotalData("m1", item));
    metricData.genHostTotalData();
    CPPUNIT_ASSERT(metricData.getHostTotalData("m1", item));
    CPPUNIT_ASSERT_EQUAL((uint32_t)0, item.host);
    CPPUNIT_ASSERT_EQUAL((uint64_t)20 * MICROSECONDS, item.time);
    CPPUNIT_ASSERT_EQUAL(300.0, item.value);
    CPPUNIT_ASSERT(metricData.getHostTotalData("m2", item));
    CPPUNIT_ASSERT_EQUAL((uint32_t)0, item.host);
    CPPUNIT_ASSERT_EQUAL((uint64_t)20 * MICROSECONDS, item.time);
    CPPUNIT_ASSERT_EQUAL(200.0, item.value);
    CPPUNIT_ASSERT(!metricData.getHostTotalData("m3", item));

    CPPUNIT_ASSERT(!metricData.getMetricTotalData(item));
    metricData.genMetricTotalVal();
    CPPUNIT_ASSERT(metricData.getMetricTotalData(item));
    CPPUNIT_ASSERT_EQUAL((uint32_t)0, item.host);
    CPPUNIT_ASSERT_EQUAL((uint64_t)20 * MICROSECONDS, item.time);
    CPPUNIT_ASSERT_EQUAL(500.0, item.value);
}

void NormalizedMetricDataTest::testGetValue()
{
    //empty
    ValueList valueList;
    CPPUNIT_ASSERT_EQUAL(INVALID_METRIC_DATA, NormalizedMetricData::getValue(valueList));

    //has no validData
    valueList.push_back(INVALID_METRIC_DATA);
    valueList.push_back(INVALID_METRIC_DATA);
    CPPUNIT_ASSERT_EQUAL(INVALID_METRIC_DATA, NormalizedMetricData::getValue(valueList));

    //has validData and inValidData
    valueList.push_back(10.0);
    valueList.push_back(10.0);
    CPPUNIT_ASSERT_EQUAL(20.0/2, NormalizedMetricData::getValue(valueList));

    //all is valid data
    valueList.clear();
    valueList.push_back(10.0);
    valueList.push_back(10.0);
    CPPUNIT_ASSERT_EQUAL(20.0/2, NormalizedMetricData::getValue(valueList));
}

void NormalizedMetricDataTest::testGetTime()
{
    TimeList timeList;
    //empty
    CPPUNIT_ASSERT_EQUAL((uint64_t)0, NormalizedMetricData::getTime(timeList));

    //normal
    timeList.push_back(6);
    timeList.push_back(17);
    timeList.push_back(7);
    CPPUNIT_ASSERT_EQUAL((uint64_t)(6 + 17 + 7)/3, NormalizedMetricData::getTime(timeList));
}

void NormalizedMetricDataTest::testGenSampleTimeList()
{
    NormalizedMetricData data;
    TimeList sampleTimeList;
    uint64_t minTime = 5;
    uint64_t maxTime = 5;
    int32_t pointCount = 1;
    
    data.genSampleTimeList(minTime, maxTime, pointCount, sampleTimeList);
    CPPUNIT_ASSERT_EQUAL((size_t)1, sampleTimeList.size());
    CPPUNIT_ASSERT_EQUAL((uint64_t)5, sampleTimeList.at(0));

    maxTime = 10;
    pointCount = 2;
    data.genSampleTimeList(minTime, maxTime, pointCount, sampleTimeList);
    CPPUNIT_ASSERT_EQUAL((size_t)2, sampleTimeList.size());
    CPPUNIT_ASSERT_EQUAL((uint64_t)7, sampleTimeList.at(0));
    CPPUNIT_ASSERT_EQUAL((uint64_t)10, sampleTimeList.at(1));

    maxTime = 105;
    pointCount = 3;
    data.genSampleTimeList(minTime, maxTime, pointCount, sampleTimeList);
    CPPUNIT_ASSERT_EQUAL((size_t)3, sampleTimeList.size());
    CPPUNIT_ASSERT_EQUAL((uint64_t)38, sampleTimeList.at(0));
    CPPUNIT_ASSERT_EQUAL((uint64_t)71, sampleTimeList.at(1));
    CPPUNIT_ASSERT_EQUAL((uint64_t)105, sampleTimeList.at(2));
}

void NormalizedMetricDataTest::testSampleValues()
{
    TimeList sampleTimeList;
    sampleTimeList.push_back(10);
    ValueListPtr metric2(new ValueList());
    TimeListPtr timeList(new TimeList());
    MetricDataItem dataItem;
    dataItem.rawValue = metric2;
    dataItem.rawTime = timeList;

    ValueList avgValueList;
    NormalizedMetricData data;

    //one data, but not in sample time
    timeList->push_back(13);
    metric2->push_back(50);
    data.sampleValues(dataItem, sampleTimeList, avgValueList);
    CPPUNIT_ASSERT_EQUAL((size_t)1, avgValueList.size());
    CPPUNIT_ASSERT_EQUAL(INVALID_METRIC_DATA, avgValueList.at(0));

    //one in sampele time data
    timeList->clear();
    timeList->push_back(5);
    timeList->push_back(13);
    metric2->clear();
    metric2->push_back(20.0);
    metric2->push_back(50.0);
    data.sampleValues(dataItem, sampleTimeList, avgValueList);
    CPPUNIT_ASSERT_EQUAL((size_t)1, avgValueList.size());
    CPPUNIT_ASSERT_EQUAL(20.0, avgValueList.at(0));

    //normal, one has value, one invalid value
    sampleTimeList.push_back(20);
    sampleTimeList.push_back(30);
    timeList->clear();
    timeList->push_back(5);
    timeList->push_back(7);
    timeList->push_back(8);
    timeList->push_back(23);
    metric2->clear();
    metric2->push_back(30.0);
    metric2->push_back(INVALID_METRIC_DATA);
    metric2->push_back(80);
    metric2->push_back(70);
    data.sampleValues(dataItem, sampleTimeList, avgValueList);
    CPPUNIT_ASSERT_EQUAL((size_t)3, avgValueList.size());
    CPPUNIT_ASSERT_EQUAL(55.0, avgValueList.at(0));
    CPPUNIT_ASSERT_EQUAL(INVALID_METRIC_DATA, avgValueList.at(1));
    CPPUNIT_ASSERT_EQUAL(70.0, avgValueList.at(2));
}

void NormalizedMetricDataTest::testGenNormalizedTotalValue()
{
    vector<MetricDataItem> metricDataVec;
    TimeList sampleTimeList;

    sampleTimeList.push_back(10);
    sampleTimeList.push_back(20);
    sampleTimeList.push_back(30);
    MetricDataItem item1;
    ValueListPtr metric1(new ValueList());
    TimeListPtr timeList1(new TimeList());
    item1.rawTime = timeList1;
    item1.rawValue = metric1;
    metricDataVec.push_back(item1);

    ValueList totalValueList;
    NormalizedMetricData data;

    //one MetricDataItem
    timeList1->push_back(5);
    timeList1->push_back(7);
    metric1->push_back(INVALID_METRIC_DATA);
    metric1->push_back(50);
    data.genNormalizedTotalValue(metricDataVec, sampleTimeList, totalValueList);
    CPPUNIT_ASSERT_EQUAL((size_t)3, totalValueList.size());
    CPPUNIT_ASSERT_EQUAL(50.0, totalValueList.at(0));
    CPPUNIT_ASSERT_EQUAL(INVALID_METRIC_DATA, totalValueList.at(1));
    CPPUNIT_ASSERT_EQUAL(INVALID_METRIC_DATA, totalValueList.at(2));

    //add one MetricDataItem
    MetricDataItem item2;
    ValueListPtr metric2(new ValueList());
    TimeListPtr timeList2(new TimeList());
    item2.rawTime = timeList2;
    item2.rawValue = metric2;
    metricDataVec.push_back(item2);
    timeList2->push_back(8);
    timeList2->push_back(9);
    timeList2->push_back(25);
    metric2->push_back(INVALID_METRIC_DATA);
    metric2->push_back(30.0);
    metric2->push_back(120.0);
    data.genNormalizedTotalValue(metricDataVec, sampleTimeList, totalValueList);
    CPPUNIT_ASSERT_EQUAL((size_t)3, totalValueList.size());
    CPPUNIT_ASSERT_EQUAL(80.0, totalValueList.at(0));
    CPPUNIT_ASSERT_EQUAL(INVALID_METRIC_DATA, totalValueList.at(1));
    CPPUNIT_ASSERT_EQUAL(120.0, totalValueList.at(2));
}

void NormalizedMetricDataTest::testGetTotalValue()
{
    vector<MetricDataItem> metricDataVec;
    double totalValue; 
    uint64_t avgTime;
    
    MetricDataItem item1;
    ValueListPtr metric1(new ValueList());
    TimeListPtr timeList1(new TimeList());
    item1.rawTime = timeList1;
    item1.rawValue = metric1;
    metricDataVec.push_back(item1);

    timeList1->push_back(10 * MICROSECONDS);
    metric1->push_back(100.0);

    //one host one data
    NormalizedMetricData data;
    data.getTotalValue(metricDataVec, totalValue, avgTime);
    CPPUNIT_ASSERT_EQUAL((uint64_t)10 * MICROSECONDS, avgTime);
    CPPUNIT_ASSERT_EQUAL(100.0, totalValue);

    //one host two data
    timeList1->push_back(20 * MICROSECONDS);
    metric1->push_back(200.0);
    data.getTotalValue(metricDataVec, totalValue, avgTime);
    CPPUNIT_ASSERT_EQUAL(uint64_t(17.5 * MICROSECONDS), avgTime);
    CPPUNIT_ASSERT_EQUAL(150.0, totalValue);
    

    //two host, 3 data, on invalid data
    MetricDataItem item2;
    ValueListPtr metric2(new ValueList());
    TimeListPtr timeList2(new TimeList());
    item2.rawTime = timeList2;
    item2.rawValue = metric2;
    metricDataVec.push_back(item2);

    timeList2->push_back(12 * MICROSECONDS);
    metric2->push_back(100.0);
    timeList2->push_back(17 * MICROSECONDS);
    metric2->push_back(INVALID_METRIC_DATA);
    data.getTotalValue(metricDataVec, totalValue, avgTime);
    CPPUNIT_ASSERT_EQUAL(uint64_t(17.5 * MICROSECONDS), avgTime);
    CPPUNIT_ASSERT_EQUAL(200.0, totalValue);

    //two host, 4 data
    timeList2->clear();
    timeList2->push_back(12 * MICROSECONDS);
    timeList2->push_back(18 * MICROSECONDS);
    metric2->clear();
    metric2->push_back(100.0);
    metric2->push_back(200.0);
    data.getTotalValue(metricDataVec, totalValue, avgTime);
    CPPUNIT_ASSERT_EQUAL(uint64_t(17.5 * MICROSECONDS), avgTime);
    CPPUNIT_ASSERT_EQUAL(300.0, totalValue);
}

RA_END_NAMESPACE(process);
