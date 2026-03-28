void runGraphEngineTests();
void runMergePdcTests();
void runFlexGraphSerializationTests();
void runPhaseAlignTests();
void runDspDeterminismTests();

int main()
{
    runGraphEngineTests();
    runMergePdcTests();
    runFlexGraphSerializationTests();
    runPhaseAlignTests();
    runDspDeterminismTests();
    return 0;
}
