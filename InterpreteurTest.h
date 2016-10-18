/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   InterpreteurTest.h
 * Author: perrinan
 *
 * Created on Oct 18, 2016, 7:52:00 AM
 */

#ifndef INTERPRETEURTEST_H
#define INTERPRETEURTEST_H

#include <cppunit/extensions/HelperMacros.h>

class InterpreteurTest : public CPPUNIT_NS::TestFixture {
    CPPUNIT_TEST_SUITE(InterpreteurTest);

    CPPUNIT_TEST(testAnalyse);

    CPPUNIT_TEST_SUITE_END();

public:
    InterpreteurTest();
    virtual ~InterpreteurTest();
    void setUp();
    void tearDown();

private:
    void testAnalyse();

};

#endif /* INTERPRETEURTEST_H */

