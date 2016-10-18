/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   InterpreteurTest.cpp
 * Author: perrinan
 *
 * Created on Oct 18, 2016, 7:52:00 AM
 */

#include "InterpreteurTest.h"
#include "Interpreteur.h"


CPPUNIT_TEST_SUITE_REGISTRATION(InterpreteurTest);

InterpreteurTest::InterpreteurTest() {
}

InterpreteurTest::~InterpreteurTest() {
}

void InterpreteurTest::setUp() {
}

void InterpreteurTest::tearDown() {
}


void InterpreteurTest::testAnalyse() {
 
    //test de repeter
    //string nomFich = "testRepeterComplet.txt"; 
    string nomFich = "programme.txt";
    try{
    ifstream fichier(nomFich.c_str());
    Interpreteur interpreteur(fichier);
    interpreteur.analyse();
    }
    catch(InterpreteurException e){
        cout << e.what() << endl;
    }
    
    
    //faire les assertions ici
    //sur les noeuds?
   
    if (true /*check result*/) {
        CPPUNIT_ASSERT(false);
    }
    
    CPPUNIT_ASSERT_MESSAGE();
}

