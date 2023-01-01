#pragma once
#include <cstdint>
#include <vector>
#include <Windows.h>
#include <iostream>
#include <string>


uintptr_t sig_scan(const char* module, const char* pattern);