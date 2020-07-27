#ifndef _GLOBALCONSTS_H
#define _GLOBALCONSTS_H

//--------------- Geometry setup -----------------
#define nStations 7
#define nChamberPlanes 30
#define nHodoPlanes 16
#define nPropPlanes 8
#define nDarkPhotonPlanes 8

//--------------- Physics quantities -------------
#define M_MU 0.1056583745

//-------------- Track finding exit code ---------------
#define TFEXIT_SUCCESS 0;
#define VFEXIT_SUCCESS 0;
#define TFEXIT_FAIL_MULTIPLICITY -1;
#define TFEXIT_FAIL_ROUGH_MUONID -2;
#define TFEXIT_FAIL_ST2_TRACKLET -3;
#define TFEXIT_FAIL_ST3_TRACKLET -4;
#define TFEXIT_FAIL_BACKPARTIAL -5;
#define TFEXIT_FAIL_GLOABL -6;
#define TFEXIT_FAIL_NO_DIMUON -7;
#define VFEXIT_FAIL_DIMUONPAIR -10;
#define VFEXIT_FAIL_ITERATION -20;

//-------------- Useful marcros -----------------
#define LogInfo(message) std::cout << "DEBUG: " << __FILE__ << "  " << __LINE__ << "  " << __FUNCTION__ << " :::  " << message << std::endl
#define varName(x) #x

#endif
