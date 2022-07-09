#include "Core/Inc/CANbus_functions.h"
#include "build/test/mocks/mock_stm32f1xx_hal_can.h"
#include "build/test/mocks/mock_analysis.h"
#include "build/test/mocks/mock_ltc6813_btm.h"
#include "build/test/mocks/mock_stm32f1xx_hal.h"
#include "Core/Inc/CAN_OnlyTestingFunctions.h"
#include "build/test/mocks/mock_Pack_packdata.h"
#include "Documents/GIT/firmware_v3/tools/ceedling/vendor/unity/src/unity.h"
































uint16_t TEST_PACK_CURRENT = 7;











void hello_world(int i);

void compareArraysOfSize6(uint8_t* expectedArray, uint8_t* actualArray);

void compareArraysOfSize7(uint8_t* expectedArray, uint8_t* actualArray);













void setUp(void)

{

}



void tearDown(void)

{

}















void test_message624_currentStatus()

{

    short electricCurrent;



    uint8_t * expectedDataFramePtr;

    uint8_t * actualDataFramePtr;







    electricCurrent = 123;





    Pack_getPackCurrent_CMockExpectAndReturn(68, electricCurrent);

    expectedDataFramePtr = CAN_createExpectedMessage624(electricCurrent);

    actualDataFramePtr = CAN_createMessage624();

    UnityAssertEqualNumber((UNITY_INT)((expectedDataFramePtr[0])), (UNITY_INT)((actualDataFramePtr[0])), (

   ((void *)0)

   ), (UNITY_UINT)(89), UNITY_DISPLAY_STYLE_INT);

    UnityAssertEqualNumber((UNITY_INT)((expectedDataFramePtr[1])), (UNITY_INT)((actualDataFramePtr[1])), (

   ((void *)0)

   ), (UNITY_UINT)(90), UNITY_DISPLAY_STYLE_INT);

    UnityAssertEqualNumber((UNITY_INT)((expectedDataFramePtr[2])), (UNITY_INT)((actualDataFramePtr[2])), (

   ((void *)0)

   ), (UNITY_UINT)(91), UNITY_DISPLAY_STYLE_INT);

    UnityAssertEqualNumber((UNITY_INT)((expectedDataFramePtr[3])), (UNITY_INT)((actualDataFramePtr[3])), (

   ((void *)0)

   ), (UNITY_UINT)(92), UNITY_DISPLAY_STYLE_INT);

    UnityAssertEqualNumber((UNITY_INT)((expectedDataFramePtr[4])), (UNITY_INT)((actualDataFramePtr[4])), (

   ((void *)0)

   ), (UNITY_UINT)(93), UNITY_DISPLAY_STYLE_INT);

    UnityAssertEqualNumber((UNITY_INT)((expectedDataFramePtr[5])), (UNITY_INT)((actualDataFramePtr[5])), (

   ((void *)0)

   ), (UNITY_UINT)(94), UNITY_DISPLAY_STYLE_INT);







    electricCurrent = 111;

    Pack_getPackCurrent_CMockIgnoreAndReturn(99, electricCurrent);

    expectedDataFramePtr = CAN_createExpectedMessage624(electricCurrent);

    actualDataFramePtr = CAN_createMessage624(electricCurrent);



    for(int i = 0; i < 6; ++i)

    {

            printf("%i\r\n",expectedDataFramePtr[i]);

    }



    for(int i = 0; i < 6; ++i)

    {

            printf("%i\r\n",actualDataFramePtr[i]);

    }











    UnityAssertEqualNumber((UNITY_INT)((expectedDataFramePtr[0])), (UNITY_INT)((actualDataFramePtr[0])), (

   ((void *)0)

   ), (UNITY_UINT)(117), UNITY_DISPLAY_STYLE_INT);

    UnityAssertEqualNumber((UNITY_INT)((expectedDataFramePtr[1])), (UNITY_INT)((actualDataFramePtr[1])), (

   ((void *)0)

   ), (UNITY_UINT)(118), UNITY_DISPLAY_STYLE_INT);

    UnityAssertEqualNumber((UNITY_INT)((expectedDataFramePtr[2])), (UNITY_INT)((actualDataFramePtr[2])), (

   ((void *)0)

   ), (UNITY_UINT)(119), UNITY_DISPLAY_STYLE_INT);

    UnityAssertEqualNumber((UNITY_INT)((expectedDataFramePtr[3])), (UNITY_INT)((actualDataFramePtr[3])), (

   ((void *)0)

   ), (UNITY_UINT)(120), UNITY_DISPLAY_STYLE_INT);

    UnityAssertEqualNumber((UNITY_INT)((expectedDataFramePtr[4])), (UNITY_INT)((actualDataFramePtr[4])), (

   ((void *)0)

   ), (UNITY_UINT)(121), UNITY_DISPLAY_STYLE_INT);

    UnityAssertEqualNumber((UNITY_INT)((expectedDataFramePtr[5])), (UNITY_INT)((actualDataFramePtr[5])), (

   ((void *)0)

   ), (UNITY_UINT)(122), UNITY_DISPLAY_STYLE_INT);



}

void test_message622_faultsMessage_formatVerification()

{

    uint8_t* expectedMessage = 

                              ((void *)0)

                                  ;

    uint8_t* actualMessage = 

                            ((void *)0)

                                ;

    int expectedSize = 0;

    int actualSize = 0;



    const char * debugStrings_faults[64];

    debugStrings_faults[0] = "Bit  0: BMS fault bit";

    debugStrings_faults[29] = "Bit 29: HLIM is set: cannot charge";

    debugStrings_faults[30] = "Bit 30: LLIM is set: cannot discharge";

    debugStrings_faults[41] = "Bit 41: Interlock is tripped";

    debugStrings_faults[42] = "Bit 42: Communication fault with cell";

    debugStrings_faults[43] = "Bit 43: Charge overcurrent";

    debugStrings_faults[44] = "Bit 44: Discharge overcurrent";

    debugStrings_faults[45] = "Bit 45: Over temperature";

    debugStrings_faults[46] = "Bit 46: Under voltage";

    debugStrings_faults[47] = "Bit 47: Over voltage";

    debugStrings_faults[48] = "Bit 48: Low voltage warning";

    debugStrings_faults[49] = "Bit 49: High voltage warning";

    debugStrings_faults[50] = "Bit 50: Charge overcurrent warning";

    debugStrings_faults[51] = "Bit 51: Discharge overcurrent warning";

    debugStrings_faults[52] = "Bit 52: Cold temperature warning";

    debugStrings_faults[53] = "Bit 53: Hot temperature warning";

    debugStrings_faults[54] = "Bit 54: Low SOH warning";

    debugStrings_faults[55] = "Bit 55: Isolation fault warning";









    for(int i; i<= 19; ++i)

    {

        expectedMessage = CAN_createExpectedMessage622withFaultFlag(i);



        actualMessage = CAN_createIdealMessage622withFaultFlag(i);





        expectedSize = sizeof(expectedMessage)/sizeof(expectedMessage[0]);

        actualSize = sizeof(actualMessage) /sizeof(actualMessage[0]);





        UnityAssertEqualNumber((UNITY_INT)((expectedMessage[0])), (UNITY_INT)((actualMessage[0])), (

       ((void *)0)

       ), (UNITY_UINT)(186), UNITY_DISPLAY_STYLE_INT);

        UnityAssertEqualNumber((UNITY_INT)((expectedMessage[1])), (UNITY_INT)((actualMessage[1])), (

       ((void *)0)

       ), (UNITY_UINT)(187), UNITY_DISPLAY_STYLE_INT);

        UnityAssertEqualNumber((UNITY_INT)((expectedMessage[2])), (UNITY_INT)((actualMessage[2])), (

       ((void *)0)

       ), (UNITY_UINT)(188), UNITY_DISPLAY_STYLE_INT);

        UnityAssertEqualNumber((UNITY_INT)((expectedMessage[3])), (UNITY_INT)((actualMessage[3])), (

       ((void *)0)

       ), (UNITY_UINT)(189), UNITY_DISPLAY_STYLE_INT);

        UnityAssertEqualNumber((UNITY_INT)((expectedMessage[4])), (UNITY_INT)((actualMessage[4])), (

       ((void *)0)

       ), (UNITY_UINT)(190), UNITY_DISPLAY_STYLE_INT);

        UnityAssertEqualNumber((UNITY_INT)((expectedMessage[5])), (UNITY_INT)((actualMessage[5])), (

       ((void *)0)

       ), (UNITY_UINT)(191), UNITY_DISPLAY_STYLE_INT);

        UnityAssertEqualNumber((UNITY_INT)((expectedMessage[6])), (UNITY_INT)((actualMessage[6])), (

       ((void *)0)

       ), (UNITY_UINT)(192), UNITY_DISPLAY_STYLE_INT);

    }









    for(int i; i < 64; ++i)

    {

        int bitNumber = i;

        if(

                (bitNumber == 0)

            || (bitNumber == 29)

            || (bitNumber == 30)

            || (bitNumber >= 41 && bitNumber <= 55)

        )

        {

            printf("iter %2.0i,  %s \r\n", i, debugStrings_faults[i]);

            expectedMessage = CAN_createExpectedMessage622withBitFlag(i);



            actualMessage = CAN_createIdealMessage622withBitFlag(i);





            expectedSize = sizeof(expectedMessage)/sizeof(expectedMessage[0]);

            actualSize = sizeof(actualMessage) /sizeof(actualMessage[0]);



            UnityAssertEqualNumber((UNITY_INT)((expectedSize)), (UNITY_INT)((actualSize)), (

           ((void *)0)

           ), (UNITY_UINT)(217), UNITY_DISPLAY_STYLE_INT);



            {

                UnityAssertEqualNumber((UNITY_INT)((expectedMessage[0])), (UNITY_INT)((actualMessage[0])), (

               ((void *)0)

               ), (UNITY_UINT)(220), UNITY_DISPLAY_STYLE_INT);

                UnityAssertEqualNumber((UNITY_INT)((expectedMessage[1])), (UNITY_INT)((actualMessage[1])), (

               ((void *)0)

               ), (UNITY_UINT)(221), UNITY_DISPLAY_STYLE_INT);

                UnityAssertEqualNumber((UNITY_INT)((expectedMessage[2])), (UNITY_INT)((actualMessage[2])), (

               ((void *)0)

               ), (UNITY_UINT)(222), UNITY_DISPLAY_STYLE_INT);

                UnityAssertEqualNumber((UNITY_INT)((expectedMessage[3])), (UNITY_INT)((actualMessage[3])), (

               ((void *)0)

               ), (UNITY_UINT)(223), UNITY_DISPLAY_STYLE_INT);

                UnityAssertEqualNumber((UNITY_INT)((expectedMessage[4])), (UNITY_INT)((actualMessage[4])), (

               ((void *)0)

               ), (UNITY_UINT)(224), UNITY_DISPLAY_STYLE_INT);

                UnityAssertEqualNumber((UNITY_INT)((expectedMessage[5])), (UNITY_INT)((actualMessage[5])), (

               ((void *)0)

               ), (UNITY_UINT)(225), UNITY_DISPLAY_STYLE_INT);

                UnityAssertEqualNumber((UNITY_INT)((expectedMessage[6])), (UNITY_INT)((actualMessage[6])), (

               ((void *)0)

               ), (UNITY_UINT)(226), UNITY_DISPLAY_STYLE_INT);

            }

        }

    }





    expectedMessage = CAN_createExpectedMessage622withTime(0);



    actualMessage = CAN_createIdealMessage622withTime(0);





    expectedSize = sizeof(expectedMessage)/sizeof(expectedMessage[0]);

    actualSize = sizeof(actualMessage) /sizeof(actualMessage[0]);

    UnityAssertEqualNumber((UNITY_INT)((expectedSize)), (UNITY_INT)((actualSize)), (

   ((void *)0)

   ), (UNITY_UINT)(239), UNITY_DISPLAY_STYLE_INT);



    {

        UnityAssertEqualNumber((UNITY_INT)((expectedMessage[0])), (UNITY_INT)((actualMessage[0])), (

       ((void *)0)

       ), (UNITY_UINT)(242), UNITY_DISPLAY_STYLE_INT);

        UnityAssertEqualNumber((UNITY_INT)((expectedMessage[1])), (UNITY_INT)((actualMessage[1])), (

       ((void *)0)

       ), (UNITY_UINT)(243), UNITY_DISPLAY_STYLE_INT);

        UnityAssertEqualNumber((UNITY_INT)((expectedMessage[2])), (UNITY_INT)((actualMessage[2])), (

       ((void *)0)

       ), (UNITY_UINT)(244), UNITY_DISPLAY_STYLE_INT);

        UnityAssertEqualNumber((UNITY_INT)((expectedMessage[3])), (UNITY_INT)((actualMessage[3])), (

       ((void *)0)

       ), (UNITY_UINT)(245), UNITY_DISPLAY_STYLE_INT);

        UnityAssertEqualNumber((UNITY_INT)((expectedMessage[4])), (UNITY_INT)((actualMessage[4])), (

       ((void *)0)

       ), (UNITY_UINT)(246), UNITY_DISPLAY_STYLE_INT);

        UnityAssertEqualNumber((UNITY_INT)((expectedMessage[5])), (UNITY_INT)((actualMessage[5])), (

       ((void *)0)

       ), (UNITY_UINT)(247), UNITY_DISPLAY_STYLE_INT);

        UnityAssertEqualNumber((UNITY_INT)((expectedMessage[6])), (UNITY_INT)((actualMessage[6])), (

       ((void *)0)

       ), (UNITY_UINT)(248), UNITY_DISPLAY_STYLE_INT);

    }





    expectedMessage = CAN_createExpectedMessage622withTime(100);



    actualMessage = CAN_createIdealMessage622withTime(100);





    expectedSize = sizeof(expectedMessage)/sizeof(expectedMessage[0]);

    actualSize = sizeof(actualMessage) /sizeof(actualMessage[0]);

    UnityAssertEqualNumber((UNITY_INT)((expectedSize)), (UNITY_INT)((actualSize)), (

   ((void *)0)

   ), (UNITY_UINT)(259), UNITY_DISPLAY_STYLE_INT);



    {

        UnityAssertEqualNumber((UNITY_INT)((expectedMessage[0])), (UNITY_INT)((actualMessage[0])), (

       ((void *)0)

       ), (UNITY_UINT)(262), UNITY_DISPLAY_STYLE_INT);

        UnityAssertEqualNumber((UNITY_INT)((expectedMessage[1])), (UNITY_INT)((actualMessage[1])), (

       ((void *)0)

       ), (UNITY_UINT)(263), UNITY_DISPLAY_STYLE_INT);

        UnityAssertEqualNumber((UNITY_INT)((expectedMessage[2])), (UNITY_INT)((actualMessage[2])), (

       ((void *)0)

       ), (UNITY_UINT)(264), UNITY_DISPLAY_STYLE_INT);

        UnityAssertEqualNumber((UNITY_INT)((expectedMessage[3])), (UNITY_INT)((actualMessage[3])), (

       ((void *)0)

       ), (UNITY_UINT)(265), UNITY_DISPLAY_STYLE_INT);

        UnityAssertEqualNumber((UNITY_INT)((expectedMessage[4])), (UNITY_INT)((actualMessage[4])), (

       ((void *)0)

       ), (UNITY_UINT)(266), UNITY_DISPLAY_STYLE_INT);

        UnityAssertEqualNumber((UNITY_INT)((expectedMessage[5])), (UNITY_INT)((actualMessage[5])), (

       ((void *)0)

       ), (UNITY_UINT)(267), UNITY_DISPLAY_STYLE_INT);

        UnityAssertEqualNumber((UNITY_INT)((expectedMessage[6])), (UNITY_INT)((actualMessage[6])), (

       ((void *)0)

       ), (UNITY_UINT)(268), UNITY_DISPLAY_STYLE_INT);

    }





    expectedMessage = CAN_createExpectedMessage622withTime(65535);



    actualMessage = CAN_createIdealMessage622withTime(65535);





    expectedSize = sizeof(expectedMessage)/sizeof(expectedMessage[0]);

    actualSize = sizeof(actualMessage) /sizeof(actualMessage[0]);

    UnityAssertEqualNumber((UNITY_INT)((expectedSize)), (UNITY_INT)((actualSize)), (

   ((void *)0)

   ), (UNITY_UINT)(279), UNITY_DISPLAY_STYLE_INT);



    {

        UnityAssertEqualNumber((UNITY_INT)((expectedMessage[0])), (UNITY_INT)((actualMessage[0])), (

       ((void *)0)

       ), (UNITY_UINT)(282), UNITY_DISPLAY_STYLE_INT);

        UnityAssertEqualNumber((UNITY_INT)((expectedMessage[1])), (UNITY_INT)((actualMessage[1])), (

       ((void *)0)

       ), (UNITY_UINT)(283), UNITY_DISPLAY_STYLE_INT);

        UnityAssertEqualNumber((UNITY_INT)((expectedMessage[2])), (UNITY_INT)((actualMessage[2])), (

       ((void *)0)

       ), (UNITY_UINT)(284), UNITY_DISPLAY_STYLE_INT);

        UnityAssertEqualNumber((UNITY_INT)((expectedMessage[3])), (UNITY_INT)((actualMessage[3])), (

       ((void *)0)

       ), (UNITY_UINT)(285), UNITY_DISPLAY_STYLE_INT);

        UnityAssertEqualNumber((UNITY_INT)((expectedMessage[4])), (UNITY_INT)((actualMessage[4])), (

       ((void *)0)

       ), (UNITY_UINT)(286), UNITY_DISPLAY_STYLE_INT);

        UnityAssertEqualNumber((UNITY_INT)((expectedMessage[5])), (UNITY_INT)((actualMessage[5])), (

       ((void *)0)

       ), (UNITY_UINT)(287), UNITY_DISPLAY_STYLE_INT);

        UnityAssertEqualNumber((UNITY_INT)((expectedMessage[6])), (UNITY_INT)((actualMessage[6])), (

       ((void *)0)

       ), (UNITY_UINT)(288), UNITY_DISPLAY_STYLE_INT);

    }





}







void test_message623_voltageStatus()

{

uint8_t* expectedMessage = 

                          ((void *)0)

                              ;

uint8_t* actualMessage = 

                        ((void *)0)

                            ;





expectedMessage = CAN_createExpectedMessage623(0,0,0,0,0);

actualMessage = CAN_createExpectedMessage623(0,0,0,0,0);



compareArraysOfSize6(expectedMessage, actualMessage);





expectedMessage = CAN_createExpectedMessage623(65535,0,0,0,0);

actualMessage = CAN_createExpectedMessage623(65535,0,0,0,0);



compareArraysOfSize6(expectedMessage, actualMessage);





expectedMessage = CAN_createExpectedMessage623(0,255,0,0,0);

actualMessage = CAN_createExpectedMessage623(0,255,0,0,0);



compareArraysOfSize6(expectedMessage, actualMessage);





expectedMessage = CAN_createExpectedMessage623(0,0,255,0,0);

actualMessage = CAN_createExpectedMessage623(0,0,255,0,0);



compareArraysOfSize6(expectedMessage, actualMessage);





expectedMessage = CAN_createExpectedMessage623(0,0,0,255,0);

actualMessage = CAN_createExpectedMessage623(0,0,0,255,0);



compareArraysOfSize6(expectedMessage, actualMessage);





expectedMessage = CAN_createExpectedMessage623(0,0,0,0,255);

actualMessage = CAN_createExpectedMessage623(0,0,0,0,255);



compareArraysOfSize6(expectedMessage, actualMessage);

}





void test_message623_packHealth()

{



}



void test_message623_Temperature()

{



}



void compareArraysOfSize6(uint8_t* expectedArray, uint8_t* actualArray)

{

    int expectedSize = 0;

    int actualSize = 0;





    expectedSize = sizeof(&expectedArray)/sizeof(expectedArray[0]);

    actualSize = sizeof(&actualArray) /sizeof(actualArray[0]);

    UnityAssertEqualNumber((UNITY_INT)((expectedSize)), (UNITY_INT)((actualSize)), (

   ((void *)0)

   ), (UNITY_UINT)(357), UNITY_DISPLAY_STYLE_INT);

    printf("actualSize %i\r\n",actualSize);

    printf("actualArray %i\r\n",sizeof(actualArray));

    printf("actualArray[0] %i\r\n",sizeof(actualArray[0]));

    UnityAssertEqualNumber((UNITY_INT)((6)), (UNITY_INT)((actualSize)), (

   ((void *)0)

   ), (UNITY_UINT)(361), UNITY_DISPLAY_STYLE_INT);

    UnityAssertEqualNumber((UNITY_INT)((6)), (UNITY_INT)((expectedSize)), (

   ((void *)0)

   ), (UNITY_UINT)(362), UNITY_DISPLAY_STYLE_INT);



    {

        UnityAssertEqualNumber((UNITY_INT)((expectedArray[0])), (UNITY_INT)((actualArray[0])), (

       ((void *)0)

       ), (UNITY_UINT)(365), UNITY_DISPLAY_STYLE_INT);

        UnityAssertEqualNumber((UNITY_INT)((expectedArray[1])), (UNITY_INT)((actualArray[1])), (

       ((void *)0)

       ), (UNITY_UINT)(366), UNITY_DISPLAY_STYLE_INT);

        UnityAssertEqualNumber((UNITY_INT)((expectedArray[2])), (UNITY_INT)((actualArray[2])), (

       ((void *)0)

       ), (UNITY_UINT)(367), UNITY_DISPLAY_STYLE_INT);

        UnityAssertEqualNumber((UNITY_INT)((expectedArray[3])), (UNITY_INT)((actualArray[3])), (

       ((void *)0)

       ), (UNITY_UINT)(368), UNITY_DISPLAY_STYLE_INT);

        UnityAssertEqualNumber((UNITY_INT)((expectedArray[4])), (UNITY_INT)((actualArray[4])), (

       ((void *)0)

       ), (UNITY_UINT)(369), UNITY_DISPLAY_STYLE_INT);

        UnityAssertEqualNumber((UNITY_INT)((expectedArray[5])), (UNITY_INT)((actualArray[5])), (

       ((void *)0)

       ), (UNITY_UINT)(370), UNITY_DISPLAY_STYLE_INT);

    }

}



void compareArraysOfSize7(uint8_t* expectedArray, uint8_t* actualArray)

{

    int expectedSize = 0;

    int actualSize = 0;





    expectedSize = sizeof(expectedArray)/sizeof(expectedArray[0]);

    actualSize = sizeof(actualArray) /sizeof(actualArray[0]);

    UnityAssertEqualNumber((UNITY_INT)((expectedSize)), (UNITY_INT)((actualSize)), (

   ((void *)0)

   ), (UNITY_UINT)(382), UNITY_DISPLAY_STYLE_INT);

    UnityAssertEqualNumber((UNITY_INT)((7)), (UNITY_INT)((actualSize)), (

   ((void *)0)

   ), (UNITY_UINT)(383), UNITY_DISPLAY_STYLE_INT);

    UnityAssertEqualNumber((UNITY_INT)((7)), (UNITY_INT)((expectedSize)), (

   ((void *)0)

   ), (UNITY_UINT)(384), UNITY_DISPLAY_STYLE_INT);



    {

        UnityAssertEqualNumber((UNITY_INT)((expectedArray[0])), (UNITY_INT)((actualArray[0])), (

       ((void *)0)

       ), (UNITY_UINT)(387), UNITY_DISPLAY_STYLE_INT);

        UnityAssertEqualNumber((UNITY_INT)((expectedArray[1])), (UNITY_INT)((actualArray[1])), (

       ((void *)0)

       ), (UNITY_UINT)(388), UNITY_DISPLAY_STYLE_INT);

        UnityAssertEqualNumber((UNITY_INT)((expectedArray[2])), (UNITY_INT)((actualArray[2])), (

       ((void *)0)

       ), (UNITY_UINT)(389), UNITY_DISPLAY_STYLE_INT);

        UnityAssertEqualNumber((UNITY_INT)((expectedArray[3])), (UNITY_INT)((actualArray[3])), (

       ((void *)0)

       ), (UNITY_UINT)(390), UNITY_DISPLAY_STYLE_INT);

        UnityAssertEqualNumber((UNITY_INT)((expectedArray[4])), (UNITY_INT)((actualArray[4])), (

       ((void *)0)

       ), (UNITY_UINT)(391), UNITY_DISPLAY_STYLE_INT);

        UnityAssertEqualNumber((UNITY_INT)((expectedArray[5])), (UNITY_INT)((actualArray[5])), (

       ((void *)0)

       ), (UNITY_UINT)(392), UNITY_DISPLAY_STYLE_INT);

        UnityAssertEqualNumber((UNITY_INT)((expectedArray[6])), (UNITY_INT)((actualArray[6])), (

       ((void *)0)

       ), (UNITY_UINT)(393), UNITY_DISPLAY_STYLE_INT);

    }

}





void hello_world(int i)

{

    if(i == 1)

        printf("hello world, my name is Edward\r\n");

}

void test_CANstate_staleCheck()

{

        uint8_t expectValue = 1;

        HAL_CAN_GetTxMailboxesFreeLevel_CMockIgnoreAndReturn(414, expectValue);

        uint8_t actualValue = CANstate_staleCheck();



        UnityAssertEqualNumber((UNITY_INT)(UNITY_UINT8 )((expectValue)), (UNITY_INT)(UNITY_UINT8 )((actualValue)), (

       ((void *)0)

       ), (UNITY_UINT)(417), UNITY_DISPLAY_STYLE_UINT8);

}
