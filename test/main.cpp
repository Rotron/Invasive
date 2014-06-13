#include "test.h"

int main(int argc, char** argv)
{
   int status = 0;
   {
      Test t;
      status |= QTest::qExec(&t, argc, argv);
   }
   return status;
}
