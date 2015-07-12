#include <iostream>
#include "Serial.h"

using namespace std;

int main()
{
    Serial s;
    s.ConnectSerial("\\\\.\\COM6");
    cout << "Hello world!" << endl;
    return 0;
}
