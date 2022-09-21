//
//  dummy.c
//  Duke3D
//
//  Created by fabien sanglard on 12-12-12.
//  Copyright (c) 2012 fabien sanglard. All rights reserved.
//

#include "build.h"


short numplayers = 1, myconnectindex = 0;
short connecthead, connectpoint2[MAXPLAYERS];


uint8_t  syncstate=0;