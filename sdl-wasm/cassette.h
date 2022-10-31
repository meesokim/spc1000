#include "common.h"

enum casmode {CAS_STOP, CAS_PLAY, CAS_REC};

void ResetCassette(CassetteTape *cas);
void InitCassette(CassetteTape *cas);
void CasWrite1(CassetteTape *cas, int val);
void CasWrite(CassetteTape *cas, int val);
int CasRead(CassetteTape *cas);
int ReadVal(void);