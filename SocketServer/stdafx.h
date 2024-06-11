#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <WS2tcpip.h>
#include <MSWSock.h>
#include <mutex>
#include <unordered_set>
#include <unordered_map>
#include <array>
#include <atomic>
#include <chrono>
#include <sqlext.h> 
#include <windows.h>  
#include <fstream>
#include <concurrent_priority_queue.h>
#include <queue>
#include <map>



#include "include/lua.hpp"
#include "define.h"

#pragma comment(lib, "WS2_32.lib")
#pragma comment(lib, "MSWSock.lib")
#pragma comment(lib, "lua54.lib")
