#include "Core/Inc/CAN_messages.h"
#include "build/test/mocks/mock_Pack_packdata.h"
#include "Documents/GIT/firmware_v3/tools/ceedling/vendor/unity/src/unity.h"






uint16_t TEST_PACK_CURRENT = 7;



void setUp(void)

{

}



void tearDown(void)

{

}







void test_message_currentStatus_bitsAreSetProperly()

{

    short electricCurrent;



    uint8_t * expectedDataFramePtr;

    uint8_t * actualDataFramePtr;







    electricCurrent = 123;





    Pack_getPackCurrent_CMockExpectAndReturn(44, electricCurrent);

    expectedDataFramePtr = CAN_createExpectedMessage624(electricCurrent);

    actualDataFramePtr = CAN_createMessage624();













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

   ), (UNITY_UINT)(63), UNITY_DISPLAY_STYLE_INT);

    UnityAssertEqualNumber((UNITY_INT)((expectedDataFramePtr[1])), (UNITY_INT)((actualDataFramePtr[1])), (

   ((void *)0)

   ), (UNITY_UINT)(64), UNITY_DISPLAY_STYLE_INT);

    UnityAssertEqualNumber((UNITY_INT)((expectedDataFramePtr[2])), (UNITY_INT)((actualDataFramePtr[2])), (

   ((void *)0)

   ), (UNITY_UINT)(65), UNITY_DISPLAY_STYLE_INT);

    UnityAssertEqualNumber((UNITY_INT)((expectedDataFramePtr[3])), (UNITY_INT)((actualDataFramePtr[3])), (

   ((void *)0)

   ), (UNITY_UINT)(66), UNITY_DISPLAY_STYLE_INT);

    UnityAssertEqualNumber((UNITY_INT)((expectedDataFramePtr[4])), (UNITY_INT)((actualDataFramePtr[4])), (

   ((void *)0)

   ), (UNITY_UINT)(67), UNITY_DISPLAY_STYLE_INT);

    UnityAssertEqualNumber((UNITY_INT)((expectedDataFramePtr[5])), (UNITY_INT)((actualDataFramePtr[5])), (

   ((void *)0)

   ), (UNITY_UINT)(68), UNITY_DISPLAY_STYLE_INT);







    electricCurrent = 111;

    Pack_getPackCurrent_CMockIgnoreAndReturn(73, electricCurrent);

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

   ), (UNITY_UINT)(91), UNITY_DISPLAY_STYLE_INT);

    UnityAssertEqualNumber((UNITY_INT)((expectedDataFramePtr[1])), (UNITY_INT)((actualDataFramePtr[1])), (

   ((void *)0)

   ), (UNITY_UINT)(92), UNITY_DISPLAY_STYLE_INT);

    UnityAssertEqualNumber((UNITY_INT)((expectedDataFramePtr[2])), (UNITY_INT)((actualDataFramePtr[2])), (

   ((void *)0)

   ), (UNITY_UINT)(93), UNITY_DISPLAY_STYLE_INT);

    UnityAssertEqualNumber((UNITY_INT)((expectedDataFramePtr[3])), (UNITY_INT)((actualDataFramePtr[3])), (

   ((void *)0)

   ), (UNITY_UINT)(94), UNITY_DISPLAY_STYLE_INT);

    UnityAssertEqualNumber((UNITY_INT)((expectedDataFramePtr[4])), (UNITY_INT)((actualDataFramePtr[4])), (

   ((void *)0)

   ), (UNITY_UINT)(95), UNITY_DISPLAY_STYLE_INT);

    UnityAssertEqualNumber((UNITY_INT)((expectedDataFramePtr[5])), (UNITY_INT)((actualDataFramePtr[5])), (

   ((void *)0)

   ), (UNITY_UINT)(96), UNITY_DISPLAY_STYLE_INT);



}
